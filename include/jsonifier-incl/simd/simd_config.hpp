/*
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2026 Nihilai Collective Corp
 * https://github.com/nihilai-collective/jsonifier
 * include/jsonifier-incl/simd/simd_config.hpp
 */
#pragma once

#include <jsonifier-incl/core/config.hpp>
#include <jsonifier-incl/simd/simd_x.hpp>

namespace jsonifier {

	using restricted_read_buffer_ptr  = const uint8_t* __restrict;
	using restricted_write_buffer_ptr = uint8_t* __restrict;
	using read_structural_index_ptr	  = const uint32_t*;
	using read_buffer_ptr			  = const uint8_t*;
	using write_structural_index_ptr  = uint32_t*;
	using write_buffer_ptr			  = uint8_t*;

	inline constexpr uint64_t maxSimdBytesPerRegister{ internal::cpu_properties::getValue(internal::cpu_property_types::alignment) };

	enum class jsonifier_backend : uint8_t {
		fallback,
		avx,
		avx2,
		avx512,
		neon,
		sve2,
	};

	template<jsonifier_backend backend> struct backend_traits;

	template<uint64_t tapeStepNew, uint64_t blocksPerStepNew> struct backend_step_tuning {
		static constexpr uint64_t tapeStep{ tapeStepNew };
		static constexpr uint64_t blocksPerStep{ blocksPerStepNew };
	};

#if JSONIFIER_COMPILER_CLANG
	template<uint64_t clangTapeStep, uint64_t clangBlocksPerStep, uint64_t gccTapeStep, uint64_t gccBlocksPerStep, uint64_t msvcTapeStep, uint64_t msvcBlocksPerStep>
	using compiler_step_tuning = backend_step_tuning<clangTapeStep, clangBlocksPerStep>;
#elif JSONIFIER_COMPILER_GCC
	template<uint64_t clangTapeStep, uint64_t clangBlocksPerStep, uint64_t gccTapeStep, uint64_t gccBlocksPerStep, uint64_t msvcTapeStep, uint64_t msvcBlocksPerStep>
	using compiler_step_tuning = backend_step_tuning<gccTapeStep, gccBlocksPerStep>;
#else
	template<uint64_t clangTapeStep, uint64_t clangBlocksPerStep, uint64_t gccTapeStep, uint64_t gccBlocksPerStep, uint64_t msvcTapeStep, uint64_t msvcBlocksPerStep>
	using compiler_step_tuning = backend_step_tuning<msvcTapeStep, msvcBlocksPerStep>;
#endif

#if JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_ANY_AVX)

	using jsonifier_simd_int_128 = __m128i;
	using jsonifier_simd_int_256 = __m256i;
	using jsonifier_simd_int_512 = __m512i;

	template<> struct backend_traits<jsonifier_backend::avx512> : compiler_step_tuning<4, 4, 4, 4, 4, 4> {
		static constexpr const char* name{ "AVX512" };
		using simd_int_t = jsonifier_simd_int_512;
	};

	template<> struct backend_traits<jsonifier_backend::avx2> : compiler_step_tuning<4, 4, 4, 8, 4, 8> {
		static constexpr const char* name{ "AVX2" };
		using simd_int_t = jsonifier_simd_int_256;
	};

	template<> struct backend_traits<jsonifier_backend::avx> : compiler_step_tuning<2, 4, 2, 8, 1, 4> {
		static constexpr const char* name{ "AVX" };
		using simd_int_t = jsonifier_simd_int_128;
	};

#elif JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_SVE2)

	static_assert(JSONIFIER_SVE2_VECTOR_BITS == 128, "Jsonifier's SVE2 path is only implemented for a 128-bit vector length.");

	using jsonifier_simd_int_128 = svuint8_t __attribute__((arm_sve_vector_bits(JSONIFIER_SVE2_VECTOR_BITS)));
	using jsonifier_simd_int_256 = uint32_t;
	using jsonifier_simd_int_512 = uint64_t;

	template<> struct backend_traits<jsonifier_backend::sve2> : compiler_step_tuning<4, 4, 8, 4, 1, 4> {
		static constexpr const char* name{ "SVE2" };
		using simd_int_t = jsonifier_simd_int_128;
	};

#elif JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_NEON)

	using jsonifier_simd_int_128 = uint8x16_t;
	using jsonifier_simd_int_256 = uint32_t;
	using jsonifier_simd_int_512 = uint64_t;

	template<> struct backend_traits<jsonifier_backend::neon> : compiler_step_tuning<8, 4, 8, 4, 1, 4> {
		static constexpr const char* name{ "NEON" };
		using simd_int_t = jsonifier_simd_int_128;
	};

#else

	using jsonifier_simd_int_128 = jsonifier::internal::simd::simd_x;
	using jsonifier_simd_int_256 = uint32_t;
	using jsonifier_simd_int_512 = uint64_t;

	template<> struct backend_traits<jsonifier_backend::fallback> : backend_step_tuning<4, 8> {
		static constexpr const char* name{ "FALLBACK" };
		using simd_int_t = jsonifier_simd_int_128;
	};

#endif

	template<jsonifier_backend backend> inline constexpr uint64_t backendBytesPerRegister{ sizeof(typename backend_traits<backend>::simd_int_t) };
	template<jsonifier_backend backend> inline constexpr uint64_t backendRegistersPerBlock{ 64 / backendBytesPerRegister<backend> };
	template<jsonifier_backend backend> inline constexpr uint64_t backendBytesPerStep{ backend_traits<backend>::blocksPerStep * 64 };

}

namespace jsonifier {

#if JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_AVX512)
	inline constexpr jsonifier_backend defaultBackend{ jsonifier_backend::avx512 };
#elif JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_AVX2)
	inline constexpr jsonifier_backend defaultBackend{ jsonifier_backend::avx2 };
#elif JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_AVX)
	inline constexpr jsonifier_backend defaultBackend{ jsonifier_backend::avx };
#elif JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_SVE2)
	inline constexpr jsonifier_backend defaultBackend{ jsonifier_backend::sve2 };
#elif JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_NEON)
	inline constexpr jsonifier_backend defaultBackend{ jsonifier_backend::neon };
#else
	inline constexpr jsonifier_backend defaultBackend{ jsonifier_backend::fallback };
#endif

	using default_backend_traits = backend_traits<defaultBackend>;

	inline constexpr const char* cpuArchName{ default_backend_traits::name };

	using jsonifier_simd_int_t = typename default_backend_traits::simd_int_t;

	inline constexpr uint64_t simdTapeStep{ default_backend_traits::tapeStep };
	inline constexpr uint64_t simdBlocksPerStep{ default_backend_traits::blocksPerStep };
	inline constexpr uint64_t simdBytesPerRegister{ backendBytesPerRegister<defaultBackend> };

	inline constexpr uint64_t simdRegistersPerBlock{ 64 / simdBytesPerRegister };
	inline constexpr uint64_t simdBytesPerBlock{ 64 };
	inline constexpr uint64_t simdBytesPerStep = simdBlocksPerStep * simdBytesPerBlock;

	static_assert(simdBytesPerRegister == sizeof(jsonifier_simd_int_t),
		"simdBytesPerRegister disagrees with the actual register width; simdRegistersPerBlock and every bitmask collapse depend on these matching.");
	static_assert(simdBytesPerBlock % simdBytesPerRegister == 0, "Register width must evenly divide the 64-byte block.");

	template<uint64_t registerBytes> struct simd_register {
		static_assert(registerBytes == simdBytesPerRegister);
		using type = jsonifier_simd_int_t;
	};

#if JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_ANY_AVX)
	template<> struct simd_register<16> {
		using type = jsonifier_simd_int_128;
	};

	template<> struct simd_register<32> {
		using type = jsonifier_simd_int_256;
	};

	template<> struct simd_register<64> {
		using type = jsonifier_simd_int_512;
	};
#endif

	template<uint64_t registerCount, uint64_t registerBytes = simdBytesPerRegister> struct simd_register_array {
		using simd_type = typename simd_register<registerBytes>::type;
		alignas(registerBytes) simd_type values[registerCount];

		template<uint64_t indexNew> JSONIFIER_INLINE void set(const simd_type value) noexcept {
			static_assert(indexNew < registerCount, "simd_register_array::set index out of range.");
			values[indexNew] = value;
		}

		template<uint64_t indexNew> JSONIFIER_INLINE simd_type get() const noexcept {
			static_assert(indexNew < registerCount, "simd_register_array::get index out of range.");
			return values[indexNew];
		}
	};

	template<uint64_t registerCount> using simd_array = simd_register_array<registerCount>;

	using simd_array_t = simd_array<simdRegistersPerBlock>;

	template<uint64_t registerCount, uint64_t registerBytes> using pod_simd_array_t = simd_register_array<registerCount, registerBytes>;
}
