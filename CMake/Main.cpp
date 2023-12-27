/*
* From: https://github.com/simdjson/simdjson/blob/master/src/internal/isadetection.h
*/
#include <iostream>
#include <fstream>

#include <cstdint>
#include <cstdlib>
#if defined(_MSC_VER)
	#include <intrin.h>
#elif defined(HAVE_GCC_GET_CPUID) && defined(USE_GCC_GET_CPUID)
	#include <cpuid.h>
#endif

enum instruction_set {
	DEFAULT = 0x0,
	LZCNT	= 0x1,
	POPCNT	= 0x2,
	BMI1	= 0x4,
	BMI2	= 0x8,
	NEON	= 0x10,
	AVX		= 0x20,
	AVX2	= 0x40,
	AVX512F = 0x80,
};

#if defined(__PPC64__)

		static inline uint32_t detect_supported_architectures() {
			return instruction_set::ALTIVEC;
		}

#elif defined(__aarch64__) || defined(_M_ARM64) || defined(_M_ARM64EC)

		static inline uint32_t detect_supported_architectures() {
			return instruction_set::NEON;
		}

#elif defined(__x86_64__) || defined(_M_AMD64)

namespace {
const uint32_t cpuid_avx2_bit = 1 << 5;
const uint32_t cpuid_bmi1_bit = 1 << 3;
const uint32_t cpuid_bmi2_bit = 1 << 8;
const uint32_t cpuid_avx512f_bit = 1 << 16;
const uint32_t cpuid_avx512dq_bit = 1 << 17;
const uint32_t cpuid_avx512ifma_bit = 1 << 21;
const uint32_t cpuid_avx512pf_bit = 1 << 26;
const uint32_t cpuid_avx512er_bit = 1 << 27;
const uint32_t cpuid_avx512cd_bit = 1 << 28;
const uint32_t cpuid_avx512bw_bit = 1 << 30;
const uint32_t cpuid_avx512vl_bit = 1U << 31;
const uint32_t cpuid_avx512vbmi2_bit = 1 << 6;
const uint64_t cpuid_avx256_saved = uint64_t(1) << 2;
const uint64_t cpuid_avx512_saved = uint64_t(7) << 5;
const uint32_t cpuid_sse42_bit = 1 << 20;
const uint32_t cpuid_osxsave = (uint32_t(1) << 26) | (uint32_t(1) << 27);
const uint32_t cpuid_pclmulqdq_bit = 1 << 1;
const uint32_t cpuid_lzcnt_bit		 = 1 << 5;
const uint32_t cpuid_tzcnt_bit		 = 1 << 23;
const uint32_t cpuid_popcnt_bit		 = 1 << 23;
}

static inline void cpuid(uint32_t *eax, uint32_t *ebx, uint32_t *ecx,
                         uint32_t *edx) {
	#if defined(_MSC_VER)
  int cpu_info[4];
  __cpuidex(cpu_info, *eax, *ecx);
  *eax = cpu_info[0];
  *ebx = cpu_info[1];
  *ecx = cpu_info[2];
  *edx = cpu_info[3];
	#elif defined(HAVE_GCC_GET_CPUID) && defined(USE_GCC_GET_CPUID)
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

static inline uint64_t xgetbv() {
	#if defined(_MSC_VER)
  return _xgetbv(0);
	#else
	std::uint32_t xcr0_lo, xcr0_hi;
  asm volatile("xgetbv\n\t" : "=a" (xcr0_lo), "=d" (xcr0_hi) : "c" (0));
  return xcr0_lo | (uint64_t(xcr0_hi) << 32);
	#endif
}

static inline uint32_t detect_supported_architectures() {
	std::uint32_t eax	   = 0;
	std::uint32_t ebx	   = 0;
	std::uint32_t ecx	   = 0;
	std::uint32_t edx	   = 0;
	std::uint32_t host_isa = 0x0;

  eax = 0x1;
  ecx = 0x0;
  cpuid(&eax, &ebx, &ecx, &edx);

  if (ecx & cpuid_lzcnt_bit) {
	  host_isa |= instruction_set::LZCNT;
  }

  if (ecx & cpuid_popcnt_bit) {
	  host_isa |= instruction_set::POPCNT;
  }

  if ((ecx & cpuid_osxsave) != cpuid_osxsave) {
    return host_isa;
  }

  uint64_t xcr0 = xgetbv();

  if ((xcr0 & cpuid_avx256_saved) == 0) {
    return host_isa;
  }

  if (ecx & cpuid_avx256_saved) {
	  host_isa |= instruction_set::AVX;
  }

  eax = 0x7;
  ecx = 0x0;
  cpuid(&eax, &ebx, &ecx, &edx);

  if (ebx & cpuid_avx2_bit) {
	  host_isa |= instruction_set::AVX2;
  }

  if (ebx & cpuid_bmi1_bit) {
    host_isa |= instruction_set::BMI1;
  }

  if (ebx & cpuid_bmi2_bit) {
    host_isa |= instruction_set::BMI2;
  }

  if (!((xcr0 & cpuid_avx512_saved) == cpuid_avx512_saved)) {
     return host_isa;
  }

  if (ebx & cpuid_avx512f_bit) {
    host_isa |= instruction_set::AVX512F;
  }

  return host_isa;
}

#elif defined(__loongarch_sx) && !defined(__loongarch_asx)

static inline uint32_t detect_supported_architectures() {
  return instruction_set::LSX;
}

#elif defined(__loongarch_asx)

static inline uint32_t detect_supported_architectures() {
  return instruction_set::LASX;
}

#else

static inline uint32_t detect_supported_architectures() {
  return instruction_set::DEFAULT;
}
#endif

int main() {
	return detect_supported_architectures();
}