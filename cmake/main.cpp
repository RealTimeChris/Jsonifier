// The following code was based heavily on this code: https://github.com/simdjson/simdjson/blob/master/src/internal/isadetection.h
#include <cstring>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <string>
#include <thread>
#include <vector>

#if defined(__aarch64__) || defined(_M_ARM64)
	#if defined(__linux__)
		#include <sys/auxv.h>
		#include <asm/hwcap.h>
	#endif
#else
	#if defined(_MSC_VER)
		#include <intrin.h>
	#elif defined(__GNUC__) || defined(__clang__)
		#include <cpuid.h>
	#endif
#endif

#if defined(_WIN32) || defined(_WIN64)
	#include <Windows.h>
#endif

#if defined(__APPLE__) && defined(__MACH__)
	#include <sys/sysctl.h>
	#include <sys/types.h>
#endif

enum class instruction_sets {
	fallback = 0x0,
	lzcnt	 = 0x1,
	popcnt	 = 0x2,
	bmi1	 = 0x4,
	neon	 = 0x8,
	avx		 = 0x10,
	avx_2	 = 0x20,
	avx_512	 = 0x40,
};

enum class cache_level {
	one	  = 1,
	two	  = 2,
	three = 3,
};

#if defined(__aarch64__) || defined(_M_ARM64)
inline static uint32_t detect_supported_architectures() {
	uint32_t host_isa = static_cast<uint32_t>(instruction_sets::neon);

	#if defined(__linux__) && defined(HWCAP_SVE)
	unsigned long hwcap = getauxval(AT_HWCAP);
	if (hwcap & HWCAP_SVE) {
		host_isa |= static_cast<uint32_t>(instruction_sets::sve_2);
	}
	#endif

	return host_isa;
}

#elif defined(__x86_64__) || defined(_M_X64)

enum instruction_set {
	DEFAULT		= 0x0,
	NEON		= 0x1,
	AVX2		= 0x4,
	BMI1		= 0x20,
	AVX512F		= 0x100,
	AVX512BW	= 0x4000,
	AVX512VBMI2 = 0x10000,
};

static constexpr uint32_t cpuid_avx_bit			= 1ul << 28;
static constexpr uint32_t cpuid_avx2_bit		= 1ul << 5;
static constexpr uint32_t cpuid_bmi1_bit		= 1ul << 3;
static constexpr uint32_t cpuid_avx512_bit		= 1ul << 16;
static constexpr uint64_t cpuid_avx256_saved	= 1ULL << 2;
static constexpr uint64_t cpuid_avx512_saved	= 7ULL << 5;
static constexpr uint32_t cpuid_osx_save		= (1ul << 26) | (1ul << 27);
static constexpr uint32_t cpuid_lzcnt_bit		= 1ul << 5;
static constexpr uint32_t cpuid_popcnt_bit		= 1ul << 23;
static constexpr uint32_t cpuid_avx512f_bit		= 1 << 16;
static constexpr uint32_t cpuid_avx512bw_bit	= 1 << 30;
static constexpr uint32_t cpuid_avx512vbmi2_bit = 1 << 6;

static inline void cpuid(int32_t* eax, int32_t* ebx, int32_t* ecx, int32_t* edx) {
	#if defined(_MSC_VER)
	int cpu_info[4];
	__cpuidex(cpu_info, *eax, *ecx);
	*eax = cpu_info[0];
	*ebx = cpu_info[1];
	*ecx = cpu_info[2];
	*edx = cpu_info[3];
	#elif (defined(HAVE_GCC_GET_CPUID) && defined(USE_GCC_GET_CPUID)) || defined(__FILC__)
	uint32_t level = *eax;
	__get_cpuid(level, eax, ebx, ecx, edx);
	#else
	uint32_t a = *eax, b, c = *ecx, d;
	asm volatile("cpuid\n\t" : "+a"(a), "=b"(b), "+c"(c), "=d"(d));
	*eax = a;
	*ebx = b;
	*ecx = c;
	*edx = d;
	#endif
}

inline static uint64_t xgetbv() {
	#if defined(_MSC_VER)
	return _xgetbv(0);
	#else
	uint32_t eax, edx;
	asm volatile("xgetbv" : "=a"(eax), "=d"(edx) : "c"(0));
	return (( uint64_t )edx << 32) | eax;
	#endif
}

inline static uint32_t detect_supported_architectures() {
	std::int32_t eax	   = 0;
	std::int32_t ebx	   = 0;
	std::int32_t ecx	   = 0;
	std::int32_t edx	   = 0;
	std::uint32_t host_isa = static_cast<uint32_t>(instruction_sets::fallback);

	eax = 0x1;
	ecx = 0x0;
	cpuid(&eax, &ebx, &ecx, &edx);

	if (ecx & cpuid_popcnt_bit) {
		host_isa |= static_cast<uint32_t>(instruction_sets::popcnt);
	}

	bool os_uses_osxsave = (ecx & cpuid_osx_save) == cpuid_osx_save;
	bool cpu_has_avx	 = (ecx & cpuid_avx_bit) != 0;

	std::int32_t ext_eax = 0x80000001;
	std::int32_t ext_ebx = 0;
	std::int32_t ext_ecx = 0;
	std::int32_t ext_edx = 0;
	cpuid(&ext_eax, &ext_ebx, &ext_ecx, &ext_edx);

	if (ext_ecx & cpuid_lzcnt_bit) {
		host_isa |= static_cast<uint32_t>(instruction_sets::lzcnt);
	}

	eax = 0x7;
	ecx = 0x0;
	cpuid(&eax, &ebx, &ecx, &edx);

	if (ebx & cpuid_bmi1_bit) {
		host_isa |= static_cast<uint32_t>(instruction_sets::bmi1);
	}

	if (!os_uses_osxsave) {
		return host_isa;
	}

	uint64_t xcr0 = xgetbv();
	if ((xcr0 & cpuid_avx256_saved) == 0) {
		return host_isa;
	}

	if (cpu_has_avx) {
		host_isa |= static_cast<uint32_t>(instruction_sets::avx);
	}

	if (ebx & cpuid_avx2_bit) {
		host_isa |= static_cast<uint32_t>(instruction_sets::avx_2);
	}

	if (!((xcr0 & cpuid_avx512_saved) == cpuid_avx512_saved)) {
		return host_isa;
	}

	if (!(ebx & cpuid_avx512f_bit)) {
		return host_isa;
	}

	if (!(ebx & cpuid_avx512bw_bit)) {
		return host_isa;
	}

	if (!(ecx & cpuid_avx512vbmi2_bit)) {
		return host_isa;
	}

	if ((xcr0 & cpuid_avx512_saved) == cpuid_avx512_saved) {
		if (ebx & cpuid_avx512_bit) {
			host_isa |= static_cast<uint32_t>(instruction_sets::avx_512);
		}
	}

	return host_isa;
}

#else
inline static uint32_t detect_supported_architectures() {
	return static_cast<uint32_t>(instruction_sets::fallback);
}
#endif

int main() {
	const uint32_t supported_isa = detect_supported_architectures();

	uint32_t cpu_arch_index = 0;
	if (supported_isa & static_cast<uint32_t>(instruction_sets::avx_512)) {
		cpu_arch_index = 3;
	} else if (supported_isa & static_cast<uint32_t>(instruction_sets::avx_2)) {
		cpu_arch_index = 2;
	} else if (supported_isa & static_cast<uint32_t>(instruction_sets::avx)) {
		cpu_arch_index = 1;
	} else if (supported_isa & static_cast<uint32_t>(instruction_sets::neon)) {
		cpu_arch_index = 1;
	} else {
		cpu_arch_index = 0;
	}

	std::cout << "CPU_ARCH_INDEX=" << cpu_arch_index << std::endl;
	std::cout << "HAS_AVX=" << ((supported_isa & static_cast<uint32_t>(instruction_sets::avx)) ? 1ULL : 0) << std::endl;
	std::cout << "HAS_AVX2=" << ((supported_isa & static_cast<uint32_t>(instruction_sets::avx_2)) ? 1ULL : 0) << std::endl;
	std::cout << "HAS_AVX512=" << ((supported_isa & static_cast<uint32_t>(instruction_sets::avx_512)) ? 1ULL : 0) << std::endl;
	std::cout << "HAS_NEON=" << ((supported_isa & static_cast<uint32_t>(instruction_sets::neon)) ? 1ULL : 0) << std::endl;
	std::cout << "HAS_POPCNT=" << ((supported_isa & static_cast<uint32_t>(instruction_sets::popcnt)) ? 1ULL : 0) << std::endl;
	std::cout << "HAS_LZCNT=" << ((supported_isa & static_cast<uint32_t>(instruction_sets::lzcnt)) ? 1ULL : 0) << std::endl;
	std::cout << "HAS_BMI1=" << ((supported_isa & static_cast<uint32_t>(instruction_sets::bmi1)) ? 1ULL : 0) << std::endl;
	std::cout << "CPU_SUCCESS=1" << std::endl;
	return static_cast<int32_t>(supported_isa);
}