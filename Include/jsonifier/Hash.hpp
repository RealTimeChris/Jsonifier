/*
	MIT License

	Copyright (c) 2024 RealTimeChris

	Permission is hereby granted, free of charge, to any person obtaining a copy of this
	software and associated documentation files (the "Software"), to deal in the Software
	without restriction, including without limitation the rights to use, copy, modify, merge,
	publish, distribute, sublicense, and/or sell copies of the Software, and to permit
	persons to whom the Software is furnished to do so, subject to the following conditions:

	The above copyright notice and this permission notice shall be included in all copies or
	substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
	INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
	PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE
	FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
	OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
	DEALINGS IN THE SOFTWARE.
*/
/// Much of the code in this header was sampled from xxHash library: https://github.com/Cyan4973/xxHash/
/// https://github.com/RealTimeChris/jsonifier
#pragma once

#include <jsonifier/Config.hpp>
#include <source_location>
#include <unordered_map>
#include <exception>
#include <string>
#include <array>
#include <bit>

namespace jsonifier_internal {

	JSONIFIER_ALIGN constexpr size_t jsonifierPrime641{ 0x9E3779B185EBCA87ull };
	JSONIFIER_ALIGN constexpr size_t jsonifierPrime642{ 0xC2B2AE3D27D4EB4Full };
	JSONIFIER_ALIGN constexpr size_t jsonifierPrime643{ 0x165667B19E3779F9ull };
	JSONIFIER_ALIGN constexpr size_t jsonifierPrime644{ 0x85EBCA77C2B2AE63ull };
	JSONIFIER_ALIGN constexpr size_t jsonifierPrime645{ 0x27D4EB2F165667C5ull };

	JSONIFIER_ALIGN constexpr size_t jsonifierPrime321{ 0x9E3779B1U };
	JSONIFIER_ALIGN constexpr size_t jsonifierPrime322{ 0x85EBCA77U };
	JSONIFIER_ALIGN constexpr size_t jsonifierPrime323{ 0xC2B2AE3DU };
	JSONIFIER_ALIGN constexpr size_t jsonifierPrime324{ 0x27D4EB2FU };
	JSONIFIER_ALIGN constexpr size_t jsonifierPrime325{ 0x165667B1U };
	JSONIFIER_ALIGN constexpr size_t jsonifierStripeLength{ 64 };
	JSONIFIER_ALIGN constexpr size_t jsonifierAccNb{ jsonifierStripeLength / sizeof(size_t) };
	JSONIFIER_ALIGN constexpr size_t jsonifierSecretConsumeRate{ 8 };

	JSONIFIER_ALIGN constexpr std::array<size_t, 8> jsonifierInitAcc{ jsonifierPrime323, jsonifierPrime641, jsonifierPrime642, jsonifierPrime643, jsonifierPrime644,
		jsonifierPrime322, jsonifierPrime645, jsonifierPrime321 };

	JSONIFIER_ALIGN constexpr size_t primeMx1{ 0x165667919E3779F9ull };
	JSONIFIER_ALIGN constexpr size_t primeMx2{ 0x9FB21C651E98DF25ULL };
	JSONIFIER_ALIGN constexpr size_t secretDefaultSize{ 192 };

	JSONIFIER_ALIGN constexpr uint8_t jsonifier3KSecret[secretDefaultSize]{ 0xb8u, 0xfeu, 0x6cu, 0x39u, 0x23u, 0xa4u, 0x4bu, 0xbeu, 0x7cu, 0x01u, 0x81u, 0x2cu, 0xf7u, 0x21u, 0xadu,
		0x1cu, 0xdeu, 0xd4u, 0x6du, 0xe9u, 0x83u, 0x90u, 0x97u, 0xdbu, 0x72u, 0x40u, 0xa4u, 0xa4u, 0xb7u, 0xb3u, 0x67u, 0x1fu, 0xcbu, 0x79u, 0xe6u, 0x4eu, 0xccu, 0xc0u, 0xe5u,
		0x78u, 0x82u, 0x5au, 0xd0u, 0x7du, 0xccu, 0xffu, 0x72u, 0x21u, 0xb8u, 0x08u, 0x46u, 0x74u, 0xf7u, 0x43u, 0x24u, 0x8eu, 0xe0u, 0x35u, 0x90u, 0xe6u, 0x81u, 0x3au, 0x26u,
		0x4cu, 0x3cu, 0x28u, 0x52u, 0xbbu, 0x91u, 0xc3u, 0x00u, 0xcbu, 0x88u, 0xd0u, 0x65u, 0x8bu, 0x1bu, 0x53u, 0x2eu, 0xa3u, 0x71u, 0x64u, 0x48u, 0x97u, 0xa2u, 0x0du, 0xf9u,
		0x4eu, 0x38u, 0x19u, 0xefu, 0x46u, 0xa9u, 0xdeu, 0xacu, 0xd8u, 0xa8u, 0xfau, 0x76u, 0x3fu, 0xe3u, 0x9cu, 0x34u, 0x3fu, 0xf9u, 0xdcu, 0xbbu, 0xc7u, 0xc7u, 0x0bu, 0x4fu,
		0x1du, 0x8au, 0x51u, 0xe0u, 0x4bu, 0xcdu, 0xb4u, 0x59u, 0x31u, 0xc8u, 0x9fu, 0x7eu, 0xc9u, 0xd9u, 0x78u, 0x73u, 0x64u, 0xeau, 0xc5u, 0xacu, 0x83u, 0x34u, 0xd3u, 0xebu,
		0xc3u, 0xc5u, 0x81u, 0xa0u, 0xffu, 0xfau, 0x13u, 0x63u, 0xebu, 0x17u, 0x0du, 0xddu, 0x51u, 0xb7u, 0xf0u, 0xdau, 0x49u, 0xd3u, 0x16u, 0x55u, 0x26u, 0x29u, 0xd4u, 0x68u,
		0x9eu, 0x2bu, 0x16u, 0xbeu, 0x58u, 0x7du, 0x47u, 0xa1u, 0xfcu, 0x8fu, 0xf8u, 0xb8u, 0xd1u, 0x7au, 0xd0u, 0x31u, 0xceu, 0x45u, 0xcbu, 0x3au, 0x8fu, 0x95u, 0x16u, 0x04u,
		0x28u, 0xafu, 0xd7u, 0xfbu, 0xcau, 0xbbu, 0x4bu, 0x40u, 0x7eu };

#if JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_AVX512)

	JSONIFIER_INLINE void accumulateSimdRt(size_t* acc, const char* input, const uint8_t* secret) {
		{
			const simd_int_512 dataVec	 = _mm512_loadu_si512(input);
			const simd_int_512 keyVec	 = _mm512_loadu_si512(secret);
			const simd_int_512 dataKey	 = _mm512_xor_si512(dataVec, keyVec);
			const simd_int_512 dataKeyLo = _mm512_srli_epi64(dataKey, 32);
			const simd_int_512 product	 = _mm512_mul_epu32(dataKey, dataKeyLo);
			const simd_int_512 dataSwap	 = _mm512_shuffle_epi32(dataVec, ( _MM_PERM_ENUM )_MM_SHUFFLE(1, 0, 3, 2));
			const simd_int_512 sum		 = _mm512_add_epi64(_mm512_loadu_si512(acc), dataSwap);
			_mm512_store_si512(acc, _mm512_add_epi64(product, sum));
		}
	}

	constexpr void accumulateSimdCt(size_t* acc, const char* input, const uint8_t* secret) {
		{
			const simd_fb_type dataVec	 = mm512LoadUSi512(input);
			const simd_fb_type keyVec	 = mm512LoadUSi512(secret);
			const simd_fb_type dataKey	 = mm512XorSi512(dataVec, keyVec);
			const simd_fb_type dataKeyLo = mm512SrliEpi64(dataKey, 32);
			const simd_fb_type product	 = mm512MulEpi32(dataKey, dataKeyLo);
			const simd_fb_type dataSwap	 = mm512ShuffleEpi32(dataVec, ( _MM_PERM_ENUM )_MM_SHUFFLE(1, 0, 3, 2));
			const simd_fb_type sum		 = mm512AddEpi64(mm512LoadUSi512(acc), dataSwap);
			mm512StoreUSi512(acc, mm512AddEpi64(product, sum));
		}
	}

	JSONIFIER_INLINE void scrambleSimdRt(size_t* acc, const uint8_t* secret) {
		{
			const simd_int_512 accVec	 = _mm512_loadu_si512(acc);
			const simd_int_512 prime32	 = _mm512_set1_epi32(( int32_t )jsonifierPrime321);
			const simd_int_512 shifted	 = _mm512_srli_epi64(accVec, 47);
			const simd_int_512 keyVec	 = _mm512_loadu_si512(secret);
			const simd_int_512 dataKey	 = _mm512_ternarylogic_epi32(keyVec, accVec, shifted, 0x96);
			const simd_int_512 dataKeyHi = _mm512_srli_epi64(dataKey, 32);
			const simd_int_512 prodLo	 = _mm512_mul_epu32(dataKey, prime32);
			const simd_int_512 prodHi	 = _mm512_mul_epu32(dataKeyHi, prime32);
			_mm512_store_si512(acc, _mm512_add_epi64(prodLo, _mm512_slli_epi64(prodHi, 32)));
		}
	}

	constexpr void scrambleSimdCt(size_t* acc, const uint8_t* secret) {
		{
			const simd_fb_type prime32	 = mm512Set1Epi32(( int32_t )jsonifierPrime321);
			const simd_fb_type accVec	 = mm512LoadUSi512(acc);
			const simd_fb_type shifted	 = mm512SrliEpi64(accVec, 47);
			const simd_fb_type keyVec	 = mm512LoadUSi512(secret);
			const simd_fb_type dataKey	 = mm512TernarylogicEpi32(keyVec, accVec, shifted, 0x96);
			const simd_fb_type dataKeyHi = mm512SrliEpi64(dataKey, 32);
			const simd_fb_type prodLo	 = mm512MulEpi32(dataKey, prime32);
			const simd_fb_type prodHi	 = mm512MulEpi32(dataKeyHi, prime32);
			mm512StoreUSi512(acc, mm512AddEpi64(prodLo, mm512SlliEpi64(prodHi, 32)));
		}
	}

	JSONIFIER_INLINE void initCustomSecretRt(size_t seedNew, uint8_t* secret) {
		prefetchInternal(secret);
		{
			int32_t const nbRounds	   = secretDefaultSize / sizeof(simd_int_512);
			const simd_int_512 seedPos = _mm512_set1_epi64(seedNew);
			const simd_int_512 seed	   = _mm512_mask_sub_epi64(seedPos, 0xAA, _mm512_set1_epi8(0), seedPos);
			for (int32_t i = 0; i < nbRounds; ++i) {
				_mm512_store_si512(secret + i * sizeof(simd_int_512), _mm512_add_epi64(_mm512_loadu_si512(jsonifier3KSecret + i * sizeof(simd_int_512)), seed));
			}
		}
	}

	constexpr void initCustomSecretCt(size_t seedNew, uint8_t* secret) {
		{
			int32_t const nbRounds	   = secretDefaultSize / sizeof(simd_fb_type);
			const simd_fb_type seedPos = mm512Set1Epi64(seedNew);
			const simd_fb_type seed	   = mm512MaskSubEpi64(seedPos, 0xAA, mm512Set1Epi8(0), seedPos);
			for (int32_t i = 0; i < nbRounds; ++i) {
				mm512StoreUSi512(secret + i * sizeof(simd_fb_type), mm512AddEpi64(mm512LoadUSi512(jsonifier3KSecret + i * sizeof(simd_fb_type)), seed));
			}
		}
	}

#elif JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_AVX2)

	JSONIFIER_INLINE void accumulateSimdRt(size_t* acc, const char* input, const uint8_t* secret) {
		{
			for (size_t i = 0; i < jsonifierStripeLength / sizeof(simd_int_256); ++i) {
				const simd_int_256 dataVec	 = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(input));
				const simd_int_256 keyVec	 = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(secret));
				const simd_int_256 dataKey	 = _mm256_xor_si256(dataVec, keyVec);
				const simd_int_256 dataKeyLo = _mm256_srli_epi64(dataKey, 32);
				const simd_int_256 product	 = _mm256_mul_epi32(dataKey, dataKeyLo);
				const simd_int_256 dataSwap	 = _mm256_shuffle_epi32(dataVec, _MM_SHUFFLE(1, 0, 3, 2));
				const simd_int_256 sum		 = _mm256_add_epi64(_mm256_loadu_si256(reinterpret_cast<const __m256i*>(acc)), dataSwap);
				_mm256_store_si256(reinterpret_cast<__m256i*>(acc), _mm256_add_epi64(dataKeyLo, sum));
				acc += sizeof(simd_int_256) / sizeof(size_t);
				secret += sizeof(simd_int_256);
				input += sizeof(simd_int_256);
			}
		}
	}

	constexpr void accumulateSimdCt(size_t* acc, const char* input, const uint8_t* secret) {
		{
			for (size_t i = 0; i < jsonifierStripeLength / sizeof(simd_int_256); ++i) {
				const simd_fb_type dataVec	 = mm256LoadUSi256(input);
				const simd_fb_type keyVec	 = mm256LoadUSi256(secret);
				const simd_fb_type dataKey	 = mm256XorSi256(dataVec, keyVec);
				const simd_fb_type dataKeyLo = mm256SrliEpi64(dataKey, 32);
				const simd_fb_type product	 = mm256MulEpi32(dataKey, dataKeyLo);
				const simd_fb_type dataSwap	 = mm256ShuffleEpi32(dataVec, _MM_SHUFFLE(1, 0, 3, 2));
				const simd_fb_type sum		 = mm256AddEpi64(mm256LoadUSi256(acc), dataSwap);
				mm256StoreUSi256(acc, mm256AddEpi64(dataKeyLo, sum));
				acc += sizeof(simd_int_256) / sizeof(size_t);
				secret += sizeof(simd_int_256);
				input += sizeof(simd_int_256);
			}
		}
	}

	JSONIFIER_INLINE void scrambleSimdRt(size_t* acc, const uint8_t* secret) {
		{
			const simd_int_256 prime32 = _mm256_set1_epi32(( int32_t )jsonifierPrime321);
			for (size_t i = 0; i < jsonifierStripeLength / sizeof(simd_int_256); i++) {
				const simd_int_256 accVec	 = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(acc));
				const simd_int_256 shifted	 = _mm256_srli_epi64(accVec, 47);
				const simd_int_256 dataVec	 = _mm256_xor_si256(accVec, shifted);
				const simd_int_256 keyVec	 = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(secret));
				const simd_int_256 dataKey	 = _mm256_xor_si256(dataVec, keyVec);
				const simd_int_256 dataKeyHi = _mm256_srli_epi64(dataKey, 32);
				const simd_int_256 prodLo	 = _mm256_mul_epu32(dataKey, prime32);
				const simd_int_256 prodHi	 = _mm256_mul_epu32(dataKeyHi, prime32);
				_mm256_store_si256(reinterpret_cast<__m256i*>(acc), _mm256_add_epi64(prodLo, _mm256_slli_epi64(prodHi, 32)));
				acc += sizeof(simd_int_256) / sizeof(size_t);
				secret += sizeof(simd_int_256);
			}
		}
	}

	constexpr void scrambleSimdCt(size_t* acc, const uint8_t* secret) {
		{
			const simd_fb_type prime32 = mm256Set1Epi32(jsonifierPrime321);
			for (size_t i = 0; i < jsonifierStripeLength / sizeof(simd_int_256); i++) {
				const simd_fb_type accVec	 = mm256LoadUSi256(acc);
				const simd_fb_type shifted	 = mm256SrliEpi64(accVec, 47);
				const simd_fb_type dataVec	 = mm256XorSi256(accVec, shifted);
				const simd_fb_type keyVec	 = mm256LoadUSi256(secret);
				const simd_fb_type dataKey	 = mm256XorSi256(dataVec, keyVec);
				const simd_fb_type dataKeyHi = mm256SrliEpi64(dataKey, 32);
				const simd_fb_type prodLo	 = mm256MulEpi32(dataKey, prime32);
				const simd_fb_type prodHi	 = mm256MulEpi32(dataKeyHi, prime32);
				mm256StoreUSi256(acc, mm256AddEpi64(prodLo, mm256SlliEpi64(prodHi, 32)));
				acc += sizeof(simd_int_256) / sizeof(size_t);
				secret += sizeof(simd_int_256);
			}
		}
	}

	JSONIFIER_INLINE void initCustomSecretRt(size_t seedNew, uint8_t* secret) {
		prefetchInternal(secret);
		{
			__m256i const seed =
				_mm256_set_epi64x(static_cast<int64_t>(0U - seedNew), static_cast<int64_t>(seedNew), static_cast<int64_t>(0U - seedNew), static_cast<int64_t>(seedNew));
			_mm256_store_si256(reinterpret_cast<__m256i*>(secret), _mm256_add_epi64(_mm256_load_si256(reinterpret_cast<const __m256i*>(jsonifier3KSecret)), seed));
			_mm256_store_si256(reinterpret_cast<__m256i*>(secret + 32), _mm256_add_epi64(_mm256_load_si256(reinterpret_cast<const __m256i*>(jsonifier3KSecret + 32)), seed));
			_mm256_store_si256(reinterpret_cast<__m256i*>(secret + 64), _mm256_add_epi64(_mm256_load_si256(reinterpret_cast<const __m256i*>(jsonifier3KSecret + 64)), seed));
			_mm256_store_si256(reinterpret_cast<__m256i*>(secret + 96), _mm256_add_epi64(_mm256_load_si256(reinterpret_cast<const __m256i*>(jsonifier3KSecret + 96)), seed));
			_mm256_store_si256(reinterpret_cast<__m256i*>(secret + 128), _mm256_add_epi64(_mm256_load_si256(reinterpret_cast<const __m256i*>(jsonifier3KSecret + 128)), seed));
			_mm256_store_si256(reinterpret_cast<__m256i*>(secret + 160), _mm256_add_epi64(_mm256_load_si256(reinterpret_cast<const __m256i*>(jsonifier3KSecret + 160)), seed));
		}
	}

	constexpr void initCustomSecretCt(size_t seedNew, uint8_t* secret) {
		{
			const simd_fb_type seed = mm256SetrEpi64x((0U - seedNew), seedNew, (0U - seedNew), seedNew);
			mm256StoreUSi256(secret, mm256AddEpi64(mm256LoadUSi256(jsonifier3KSecret), seed));
			mm256StoreUSi256(secret + 32, mm256AddEpi64(mm256LoadUSi256(jsonifier3KSecret + 32), seed));
			mm256StoreUSi256(secret + 64, mm256AddEpi64(mm256LoadUSi256(jsonifier3KSecret + 64), seed));
			mm256StoreUSi256(secret + 96, mm256AddEpi64(mm256LoadUSi256(jsonifier3KSecret + 96), seed));
			mm256StoreUSi256(secret + 128, mm256AddEpi64(mm256LoadUSi256(jsonifier3KSecret + 128), seed));
			mm256StoreUSi256(secret + 160, mm256AddEpi64(mm256LoadUSi256(jsonifier3KSecret + 160), seed));
		}
	}

#elif JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_AVX)

	JSONIFIER_INLINE void accumulateSimdRt(size_t* acc, const char* input, const uint8_t* secret) {
		{
			for (size_t i = 0; i < jsonifierStripeLength / sizeof(simd_int_128); ++i) {
				const simd_int_128 dataVec	 = _mm_loadu_si128(reinterpret_cast<const __m128i*>(input));
				const simd_int_128 keyVec	 = _mm_loadu_si128(reinterpret_cast<const __m128i*>(secret));
				const simd_int_128 dataKey	 = _mm_xor_si128(dataVec, keyVec);
				const simd_int_128 dataKeyLo = _mm_srli_epi64(dataKey, 32);
				const simd_int_128 product	 = _mm_mul_epi32(dataKey, dataKeyLo);
				const simd_int_128 dataSwap	 = _mm_shuffle_epi32(dataVec, _MM_SHUFFLE(1, 0, 3, 2));
				const simd_int_128 sum		 = _mm_add_epi64(_mm_loadu_si128(reinterpret_cast<const __m128i*>(acc)), dataSwap);
				_mm_store_si128(reinterpret_cast<__m128i*>(acc), _mm_add_epi64(dataKeyLo, sum));
				acc += sizeof(simd_int_128) / sizeof(size_t);
				secret += sizeof(simd_int_128);
				input += sizeof(simd_int_128);
			}
		}
	}

	constexpr void accumulateSimdCt(size_t* acc, const char* input, const uint8_t* secret) {
		{
			for (size_t i = 0; i < jsonifierStripeLength / sizeof(simd_int_128); ++i) {
				const simd_fb_type dataVec	 = mm128LoadUSi128(input);
				const simd_fb_type keyVec	 = mm128LoadUSi128(secret);
				const simd_fb_type dataKey	 = mm128XorSi128(dataVec, keyVec);
				const simd_fb_type dataKeyLo = mm128SrliEpi64(dataKey, 32);
				const simd_fb_type product	 = mm128MulEpi32(dataKey, dataKeyLo);
				const simd_fb_type dataSwap	 = mm128ShuffleEpi32(dataVec, _MM_SHUFFLE(1, 0, 3, 2));
				simd_fb_type sum			 = mm128AddEpi64(mm128LoadUSi128(acc), dataSwap);
				mm128StoreUSi128(acc + (i / 8), mm128AddEpi64(dataKeyLo, sum));
				acc += sizeof(simd_int_128) / sizeof(size_t);
				secret += sizeof(simd_int_128);
				input += sizeof(simd_int_128);
			}
		}
	}

	JSONIFIER_INLINE void scrambleSimdRt(size_t* acc, const uint8_t* secret) {
		{
			const simd_int_128 prime32 = _mm_set1_epi32(( int32_t )jsonifierPrime321);
			for (size_t i = 0; i < jsonifierStripeLength / sizeof(simd_int_128); ++i) {
				const simd_int_128 accVec	 = _mm_loadu_si128(reinterpret_cast<const __m128i*>(acc));
				const simd_int_128 shifted	 = _mm_srli_epi64(accVec, 47);
				const simd_int_128 dataVec	 = _mm_xor_si128(accVec, shifted);
				const simd_int_128 keyVec	 = _mm_loadu_si128(reinterpret_cast<const __m128i*>(secret));
				const simd_int_128 dataKey	 = _mm_xor_si128(dataVec, keyVec);
				const simd_int_128 dataKeyHi = _mm_srli_epi64(dataKey, 32);
				const simd_int_128 prodLo	 = _mm_mul_epi32(dataKey, prime32);
				const simd_int_128 prodHi	 = _mm_mul_epi32(dataKeyHi, prime32);
				_mm_store_si128(reinterpret_cast<__m128i*>(acc), _mm_add_epi64(prodLo, _mm_slli_epi64(prodHi, 32)));
				acc += sizeof(simd_int_128) / sizeof(size_t);
				secret += sizeof(simd_int_128);
			}
		}
	}

	constexpr void scrambleSimdCt(size_t* acc, const uint8_t* secret) {
		{
			const simd_fb_type prime32 = mm128Set1Epi32(jsonifierPrime321);

			for (size_t i = 0; i < jsonifierStripeLength / sizeof(simd_int_128); ++i) {
				const simd_fb_type accVec	 = mm128LoadUSi128(acc);
				const simd_fb_type shifted	 = mm128SrliEpi64(accVec, 47);
				const simd_fb_type dataVec	 = mm128XorSi128(accVec, shifted);
				const simd_fb_type keyVec	 = mm128LoadUSi128(secret);
				const simd_fb_type dataKey	 = mm128XorSi128(dataVec, keyVec);
				const simd_fb_type dataKeyHi = mm128SrliEpi64(dataKey, 32);
				const simd_fb_type prodLo	 = mm128MulEpi32(dataKey, prime32);
				simd_fb_type prodHi			 = mm128MulEpi32(dataKeyHi, prime32);
				mm128StoreUSi128(acc, mm128AddEpi64(prodLo, mm128SlliEpi64(prodHi, 32)));
				acc += sizeof(simd_int_128) / sizeof(size_t);
				secret += sizeof(simd_int_128);
			}
		}
	}

	JSONIFIER_INLINE void initCustomSecretRt(size_t seedNew, uint8_t* secret) {
		static constexpr int32_t nbRounds = secretDefaultSize / sizeof(__m128i);
		__m128i const seed				  = _mm_set_epi64x((0U - seedNew), seedNew);
		const uint8_t* src16			  = jsonifier3KSecret;
		for (int32_t i = 0; i < nbRounds; ++i) {
			_mm_store_si128(reinterpret_cast<__m128i*>(secret), _mm_add_epi64(_mm_loadu_si128(reinterpret_cast<const __m128i*>(src16)), seed));
			secret += sizeof(simd_int_128);
			src16 += sizeof(simd_int_128);
		}
	}

	constexpr void initCustomSecretCt(size_t seedNew, uint8_t* secret) {
		constexpr int32_t nbRounds = secretDefaultSize / sizeof(__m128x);
		simd_fb_type const seed	   = mm128SetrEpi64x((0U - seedNew), seedNew);
		const uint8_t* src16	   = jsonifier3KSecret;
		for (uint32_t i = 0; i < nbRounds; ++i) {
			mm128StoreUSi128(secret, mm128AddEpi64(mm128LoadUSi128(src16), seed));
			secret += sizeof(simd_int_128);
			src16 += sizeof(simd_int_128);
		}
	}

#elif JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_NEON)

	JSONIFIER_INLINE void accumulateSimdRt(size_t* acc, const char* input, const uint8_t* secret) {
		for (uint64_t i = 0; i < jsonifierStripeLength / sizeof(uint64x2_t); ++i) {
			const uint64x2_t dataVec   = vld1q_u64(reinterpret_cast<const uint64_t*>(input));
			const uint64x2_t keyVec	   = vld1q_u64(reinterpret_cast<const uint64_t*>(secret));
			const uint64x2_t dataKey   = veorq_u64(dataVec, keyVec);
			const uint32x4_t dataKeyLo = vreinterpretq_u32_u64(vshrq_n_u64(dataKey, 32));
			const uint32x4_t product   = vmulq_u32(vreinterpretq_u32_u64(dataKey), dataKeyLo);
			const uint32x4_t dataSwap  = vextq_u32(vreinterpretq_u32_u64(dataVec), vreinterpretq_u32_u64(dataVec), 2);
			const uint64x2_t accVec	   = vld1q_u64(reinterpret_cast<const uint64_t*>(acc));
			const uint64x2_t sum	   = vaddq_u64(accVec, vreinterpretq_u64_u32(dataSwap));
			const uint64x2_t newSum	   = vaddq_u64(vreinterpretq_u64_u32(dataKeyLo), sum);
			vst1q_u64(reinterpret_cast<uint64_t*>(acc), newSum);
			acc += sizeof(uint64x2_t) / sizeof(uint64_t);
			secret += sizeof(uint64x2_t);
			input += sizeof(uint64x2_t);
		}
	}

	constexpr void accumulateSimdCt(size_t* acc, const char* input, const uint8_t* secret) {
		{
			for (uint64_t i = 0; i < jsonifierStripeLength / sizeof(uint64x2_t); ++i) {
				const simd_fb_type dataVec	 = mm128LoadUSi128(input);
				const simd_fb_type keyVec	 = mm128LoadUSi128(secret);
				const simd_fb_type dataKey	 = mm128XorSi128(dataVec, keyVec);
				const simd_fb_type dataKeyLo = mm128SrliEpi64(dataKey, 32);
				const simd_fb_type product	 = mm128MulEpi32(dataKey, dataKeyLo);
				const simd_fb_type dataSwap	 = mm128ShuffleEpi32(dataVec, mmShuffle(1, 0, 3, 2));
				const simd_fb_type xacc		 = mm128LoadUSi128(acc);
				simd_fb_type sum			 = mm128AddEpi64(xacc, dataSwap);
				sum							 = mm128AddEpi64(dataKeyLo, sum);
				mm128StoreUSi128(acc, sum);
				acc += sizeof(uint64x2_t) / sizeof(uint64_t);
				secret += sizeof(uint64x2_t);
				input += sizeof(uint64x2_t);
			}
		}
	}

	JSONIFIER_INLINE void scrambleSimdRt(size_t* acc, const uint8_t* secret) {
		const uint32x4_t prime32 = vdupq_n_u32(static_cast<uint32_t>(jsonifierPrime321));

		for (uint64_t i = 0; i < jsonifierStripeLength / sizeof(uint64x2_t); ++i) {
			const uint64x2_t accVec			= vld1q_u64(reinterpret_cast<const uint64_t*>(acc));
			const uint64x2_t shifted		= vshrq_n_u64(accVec, 47);
			const uint64x2_t dataVec		= veorq_u64(accVec, shifted);
			const uint64x2_t keyVec			= vld1q_u64(reinterpret_cast<const uint64_t*>(secret));
			const uint64x2_t dataKey		= veorq_u64(dataVec, keyVec);
			const uint32x4_t dataKeyLo		= vreinterpretq_u32_u64(dataKey);
			const uint32x4_t dataKeyHi		= vreinterpretq_u32_u64(vshrq_n_u64(dataKey, 32));
			const uint32x4_t prodLo			= vmulq_u32(dataKeyLo, prime32);
			const uint32x4_t prodHi			= vmulq_u32(dataKeyHi, prime32);
			const uint64x2_t prodHi_shifted = vreinterpretq_u64_u32(vextq_u32(vdupq_n_u32(0), prodHi, 2));
			vst1q_u64(reinterpret_cast<uint64_t*>(acc), vaddq_u64(vreinterpretq_u64_u32(prodLo), prodHi_shifted));
			acc += sizeof(uint64x2_t) / sizeof(uint64_t);
			secret += sizeof(uint64x2_t);
		}
	}

	constexpr void scrambleSimdCt(size_t* acc, const uint8_t* secret) {
		{
			const simd_fb_type prime32 = mm128Set1Epi32(jsonifierPrime321);

			for (uint64_t i = 0; i < jsonifierStripeLength / sizeof(uint64x2_t); ++i) {
				const simd_fb_type accVec	 = mm128LoadUSi128(acc);
				const simd_fb_type shifted	 = mm128SrliEpi64(accVec, 47);
				const simd_fb_type dataVec	 = mm128XorSi128(accVec, shifted);
				const simd_fb_type keyVec	 = mm128LoadUSi128(secret);
				const simd_fb_type dataKey	 = mm128XorSi128(dataVec, keyVec);
				const simd_fb_type dataKeyHi = mm128SrliEpi64(dataKey, 32);
				const simd_fb_type prodLo	 = mm128MulEpi32(dataKey, prime32);
				simd_fb_type prodHi			 = mm128MulEpi32(dataKeyHi, prime32);
				mm128StoreUSi128(acc, mm128AddEpi64(prodLo, mm128SlliEpi64(prodHi, 32)));
				acc += sizeof(uint64x2_t) / sizeof(uint64_t);
				secret += sizeof(uint64x2_t);
			}
		}
	}

	JSONIFIER_INLINE void initCustomSecretRt(uint64_t seedNew, uint8_t* secret) {
		constexpr auto secretDefaultSize = 64;
		constexpr auto nbRounds			 = secretDefaultSize / sizeof(uint8x16x2_t);

		uint64x2_t seedPos	  = vdupq_n_u64(seedNew);
		uint64x2_t mask		  = vsetq_lane_u64(UINT64_MAX, vdupq_n_u64(0), 1);
		uint64x2_t zeros	  = vdupq_n_u64(0);
		uint64x2_t negSeedPos = vsubq_u64(zeros, seedPos);
		uint64x2_t seedNewer  = vbslq_u64(mask, seedPos, negSeedPos);

		uint8x16x2_t jsonifier3KSecret[nbRounds];

		for (uint64_t i = 0; i < nbRounds; ++i) {
			uint8x16x2_t newSource = jsonifier3KSecret[i];
			uint64x2_t newSource0  = vreinterpretq_u64_u8(newSource.val[0]);
			uint64x2_t newSource1  = vreinterpretq_u64_u8(newSource.val[1]);

			uint64x2_t newValue0 = vaddq_u64(newSource0, seedNewer);
			uint64x2_t newValue1 = vaddq_u64(newSource1, seedNewer);

			uint8x16x2_t newValue;
			newValue.val[0] = vreinterpretq_u8_u64(newValue0);
			newValue.val[1] = vreinterpretq_u8_u64(newValue1);

			vst1q_u8(secret + (sizeof(uint8x16x2_t) * i), newValue.val[0]);
			vst1q_u8(secret + (sizeof(uint8x16x2_t) * i) + 16, newValue.val[1]);
		}
	}


	constexpr void initCustomSecretCt(uint64_t seedNew, uint8_t* secret) {
		constexpr auto nbRounds = secretDefaultSize / sizeof(simd_int_128);
		const auto seedPos		= mm128Set1Epi64x(seedNew);

		const auto mask = mm128SetrEpi64x(0ull, std::numeric_limits<uint64_t>::max());

		const __m128x zeros{};

		const auto negSeedPos = mm128SubEpi64(zeros, seedPos);
		const auto seedNewer  = mm128BlendVEpi8(seedPos, negSeedPos, mask);
		for (uint64_t i = 0; i < nbRounds; ++i) {
			auto newSource = mm128LoadUSi128(jsonifier3KSecret + (sizeof(simd_fb_type) * i));
			auto newValue  = mm128AddEpi64(newSource, seedNewer);
			mm128StoreUSi128(secret + (sizeof(simd_fb_type) * i), newValue);
		}
	}

#else

	JSONIFIER_INLINE void accumulateSimdRt(size_t* acc, const char* input, const uint8_t* secret) {
		{
			for (size_t i = 0; i < jsonifierStripeLength / sizeof(simd_int_128); ++i) {
				const simd_fb_type dataVec	 = mm128LoadUSi128(input);
				const simd_fb_type keyVec	 = mm128LoadUSi128(secret);
				const simd_fb_type dataKey	 = mm128XorSi128(dataVec, keyVec);
				const simd_fb_type dataKeyLo = mm128SrliEpi64(dataKey, 32);
				const simd_fb_type product	 = mm128MulEpi32(dataKey, dataKeyLo);
				const simd_fb_type dataSwap	 = mm128ShuffleEpi32(dataVec, _MM_SHUFFLE(1, 0, 3, 2));
				const simd_fb_type sum		 = mm128AddEpi64(mm128LoadUSi128(acc), dataSwap);
				mm128StoreUSi128(acc + (i / 8), mm128AddEpi64(dataKeyLo, sum));
				acc += sizeof(simd_int_128) / sizeof(size_t);
				secret += sizeof(simd_int_128);
				input += sizeof(simd_int_128);
			}
		}
	}

	constexpr void accumulateSimdCt(size_t* acc, const char* input, const uint8_t* secret) {
		{
			for (size_t i = 0; i < jsonifierStripeLength / sizeof(simd_int_128); ++i) {
				const simd_fb_type dataVec	 = mm128LoadUSi128(input);
				const simd_fb_type keyVec	 = mm128LoadUSi128(secret);
				const simd_fb_type dataKey	 = mm128XorSi128(dataVec, keyVec);
				const simd_fb_type dataKeyLo = mm128SrliEpi64(dataKey, 32);
				const simd_fb_type product	 = mm128MulEpi32(dataKey, dataKeyLo);
				const simd_fb_type dataSwap	 = mm128ShuffleEpi32(dataVec, _MM_SHUFFLE(1, 0, 3, 2));
				simd_fb_type sum			 = mm128AddEpi64(mm128LoadUSi128(acc), dataSwap);
				mm128StoreUSi128(acc + (i / 8), mm128AddEpi64(dataKeyLo, sum));
				acc += sizeof(simd_int_128) / sizeof(size_t);
				secret += sizeof(simd_int_128);
				input += sizeof(simd_int_128);
			}
		}
	}

	JSONIFIER_INLINE void scrambleSimdRt(size_t* acc, const uint8_t* secret) {
		{
			const simd_fb_type prime32 = mm128Set1Epi32(jsonifierPrime321);

			for (size_t i = 0; i < jsonifierStripeLength; i += 16) {
				const simd_fb_type xacc		 = mm128LoadUSi128(acc + (i / 8));
				const simd_fb_type accVec	 = xacc;
				const simd_fb_type shifted	 = mm128SrliEpi64(accVec, 47);
				const simd_fb_type dataVec	 = mm128XorSi128(accVec, shifted);
				const simd_fb_type keyVec	 = mm128LoadUSi128(secret + i);
				const simd_fb_type dataKey	 = mm128XorSi128(dataVec, keyVec);
				const simd_fb_type dataKeyHi = mm128SrliEpi64(dataKey, 32);
				const simd_fb_type prodLo	 = mm128MulEpi32(dataKey, prime32);
				simd_fb_type prodHi			 = mm128MulEpi32(dataKeyHi, prime32);
				prodHi						 = mm128AddEpi64(prodLo, mm128SlliEpi64(prodHi, 32));
				mm128StoreUSi128(acc + (i / 8), prodHi);
			}
		}
	}

	constexpr void scrambleSimdCt(size_t* acc, const uint8_t* secret) {
		{
			const simd_fb_type prime32 = mm128Set1Epi32(jsonifierPrime321);

			for (size_t i = 0; i < jsonifierStripeLength; i += 16) {
				const simd_fb_type xacc		 = mm128LoadUSi128(acc + (i / 8));
				const simd_fb_type accVec	 = xacc;
				const simd_fb_type shifted	 = mm128SrliEpi64(accVec, 47);
				const simd_fb_type dataVec	 = mm128XorSi128(accVec, shifted);
				const simd_fb_type keyVec	 = mm128LoadUSi128(secret + i);
				const simd_fb_type dataKey	 = mm128XorSi128(dataVec, keyVec);
				const simd_fb_type dataKeyHi = mm128SrliEpi64(dataKey, 32);
				const simd_fb_type prodLo	 = mm128MulEpi32(dataKey, prime32);
				simd_fb_type prodHi			 = mm128MulEpi32(dataKeyHi, prime32);
				prodHi						 = mm128AddEpi64(prodLo, mm128SlliEpi64(prodHi, 32));
				mm128StoreUSi128(acc + (i / 8), prodHi);
			}
		}
	}

	JSONIFIER_INLINE void initCustomSecretRt(size_t seedNew, uint8_t* secret) {
		constexpr int32_t nbRounds = secretDefaultSize / sizeof(__m128x);
		simd_fb_type const seed	   = mm128SetrEpi64x((0U - seedNew), seedNew);
		const uint8_t* src16	   = jsonifier3KSecret;
		for (int32_t i = 0; i < nbRounds; ++i) {
			mm128StoreUSi128(secret, mm128AddEpi64(mm128LoadUSi128(src16), seed));
			secret += sizeof(simd_int_128);
			src16 += sizeof(simd_int_128);
		}
	}

	constexpr void initCustomSecretCt(size_t seedNew, uint8_t* secret) {
		constexpr int32_t nbRounds = secretDefaultSize / sizeof(__m128x);
		simd_fb_type const seed	   = mm128SetrEpi64x((0U - seedNew), seedNew);
		const uint8_t* src16	   = jsonifier3KSecret;
		for (int32_t i = 0; i < nbRounds; ++i) {
			mm128StoreUSi128(secret, mm128AddEpi64(mm128LoadUSi128(src16), seed));
			secret += sizeof(simd_int_128);
			src16 += sizeof(simd_int_128);
		}
	}

#endif

	struct xoshiro256 {
		size_t state[4]{};

		constexpr xoshiro256() {
			constexpr auto x   = 7185499250578500046ull >> 12ull;
			constexpr auto x01 = x ^ x << 25ull;
			constexpr auto x02 = x01 ^ x01 >> 27ull;
			size_t s		   = x02 * 0x2545F4914F6CDD1Dull;
			for (size_t y = 0; y < 4; ++y) {
				state[y] = splitmix64(s);
			}
		}

		constexpr size_t operator()() {
			const size_t result = rotl(state[1ull] * 5ull, 7ull) * 9ull;

			const size_t t = state[1ull] << 17ull;

			state[2ull] ^= state[0ull];
			state[3ull] ^= state[1ull];
			state[1ull] ^= state[2ull];
			state[0ull] ^= state[3ull];

			state[2ull] ^= t;

			state[3ull] = rotl(state[3ull], 45ull);

			return result;
		}

	  protected:
		constexpr size_t rotl(const size_t x, size_t k) const {
			return (x << k) | (x >> (64ull - k));
		}

		constexpr size_t splitmix64(size_t& seed) const {
			size_t result = seed += 0x9E3779B97F4A7C15ull;
			result		  = (result ^ (result >> 30ull)) * 0xBF58476D1CE4E5B9ull;
			result		  = (result ^ (result >> 27ull)) * 0x94D049BB133111EBull;
			return result ^ (result >> 31ull);
		}
	};

	JSONIFIER_INLINE void accumulateExternalRt(size_t* acc, const char* input, const uint8_t* secret, size_t nbStripes) {
		size_t n;
		for (n = 0; n < nbStripes; n++) {
			const char* const in = input + n * 64;
			prefetchInternal(in);
			accumulateSimdRt(acc, in, secret + n * 8);
		}
	}

	constexpr void accumulateExternalCt(size_t* acc, const char* input, const uint8_t* secret, size_t nbStripes) {
		size_t n;
		for (n = 0; n < nbStripes; n++) {
			const char* const in = input + n * 64;
			accumulateSimdCt(acc, in, secret + n * 8);
		}
	}

	JSONIFIER_INLINE constexpr size_t swap64Ct(size_t x) {
		return ((x << 56) & 0xff00000000000000ULL) | ((x << 40) & 0x00ff000000000000ULL) | ((x << 24) & 0x0000ff0000000000ULL) | ((x << 8) & 0x000000ff00000000ULL) |
			((x >> 8) & 0x00000000ff000000ULL) | ((x >> 24) & 0x0000000000ff0000ULL) | ((x >> 40) & 0x000000000000ff00ULL) | ((x >> 56) & 0x00000000000000ffULL);
	}

#if defined(JSONIFIER_MSVC)
	#define swap64Internal _byteswap_uint64
#elif JSONIFIER_GCC_VERSION >= 403
	#define swap64Internal __builtin_bswap64
#else
	#define swap64Internal swap64Ct
#endif

	JSONIFIER_INLINE size_t swap64Rt(size_t x) {
		return swap64Internal(x);
	}

	template<typename value_type> JSONIFIER_INLINE value_type readBitsRt(const void* ptr) {
		JSONIFIER_ALIGN value_type returnValue{};
		std::memcpy(&returnValue, ptr, sizeof(value_type));
		return returnValue;
	}

	template<typename value_type, typename char_type> static constexpr value_type readBitsCt(const char_type* ptr) {
		JSONIFIER_ALIGN value_type returnValue{};
		for (size_t x = 0; x < sizeof(value_type); ++x) {
			returnValue |= static_cast<value_type>(static_cast<uint8_t>(ptr[x])) << (x * 8);
		}
		return returnValue;
	}

	template<size_t shiftNew> JSONIFIER_INLINE constexpr size_t xorShift64(size_t v64) {
		return v64 ^ (v64 >> shiftNew);
	}

	JSONIFIER_INLINE constexpr size_t avalanche(size_t hash) {
		hash ^= hash >> 33;
		hash *= jsonifierPrime641;
		hash ^= hash >> 29;
		hash *= jsonifierPrime643;
		hash ^= hash >> 32;
		return hash;
	}

	JSONIFIER_INLINE __m128x mult64To128Rt(size_t lhs, size_t rhs) {
		{
#if (defined(__GNUC__) || defined(__clang__)) && !defined(__wasm__) && defined(__SIZEOF_INT128__) || (defined(_INTEGRAL_MAX_BITS) && _INTEGRAL_MAX_BITS >= 128)
			__uint128_t const product = ( __uint128_t )lhs * ( __uint128_t )rhs;
			__m128x r128;
			r128.m128x_uint64[0] = (product);
			r128.m128x_uint64[1] = (product >> 64);
			return r128;
#elif (defined(_M_X64) || defined(_M_IA64)) && !defined(_M_ARM64EC)

	#if !defined(JSONIFIER_MSVC)
		#pragma intrinsic(_umul128)
	#endif

			__m128x r128;
			r128.m128x_uint64[0] = _umul128(lhs, rhs, &r128.m128x_uint64[1]);
			return r128;
#elif defined(_M_ARM64) || defined(_M_ARM64EC)

	#if !defined(JSONIFIER_MSVC)
		#pragma intrinsic(__umulh)
	#endif
			__m128x r128;
			r128.m128x_uint64[0] = lhs * rhs;
			r128.m128x_uint64[1] = __umulh(lhs, rhs);
			return r128;

#else
			size_t const loLo	 = mult32To64(lhs & 0xFFFFFFFF, rhs & 0xFFFFFFFF);
			size_t const hiLo	 = mult32To64(lhs >> 32, rhs & 0xFFFFFFFF);
			size_t const loHi	 = mult32To64(lhs & 0xFFFFFFFF, rhs >> 32);
			size_t const hiHi	 = mult32To64(lhs >> 32, rhs >> 32);
			size_t const cross = (loLo >> 32) + (hiLo & 0xFFFFFFFF) + loHi;
			size_t const upper = (hiLo >> 32) + (cross >> 32) + hiHi;
			size_t const lower = (cross << 32) | (loLo & 0xFFFFFFFF);

			__m128x r128;
			r128.m128x_uint64[0] = lower;
			r128.m128x_uint64[1] = upper;
			return r128;
#endif
		}
	}

	JSONIFIER_INLINE constexpr size_t mult32To64(size_t x, size_t y) {
		return (x & 0xFFFFFFFF) * (y & 0xFFFFFFFF);
	}

	constexpr __m128x mult64To128Ct(size_t lhs, size_t rhs) {
		size_t const loLo	 = mult32To64(lhs & 0xFFFFFFFF, rhs & 0xFFFFFFFF);
		size_t const hiLo	 = mult32To64(lhs >> 32, rhs & 0xFFFFFFFF);
		size_t const loHi	 = mult32To64(lhs & 0xFFFFFFFF, rhs >> 32);
		size_t const hiHi	 = mult32To64(lhs >> 32, rhs >> 32);
		size_t const cross = (loLo >> 32) + (hiLo & 0xFFFFFFFF) + loHi;
		size_t const upper = (hiLo >> 32) + (cross >> 32) + hiHi;
		size_t const lower = (cross << 32) | (loLo & 0xFFFFFFFF);

		__m128x r128;
		r128.m128x_uint64[0] = lower;
		r128.m128x_uint64[1] = upper;
		return r128;
	}

	JSONIFIER_INLINE size_t mul128Fold64Rt(size_t lhs, size_t rhs) {
		{
			__m128x product = mult64To128Rt(lhs, rhs);
			return product.m128x_uint64[0] ^ product.m128x_uint64[1];
		}
	}

	constexpr size_t mul128Fold64Ct(size_t lhs, size_t rhs) {
		__m128x product = mult64To128Ct(lhs, rhs);
		return product.m128x_uint64[0] ^ product.m128x_uint64[1];
	}

	JSONIFIER_INLINE size_t mix2AccsRt(const size_t* acc, const uint8_t* secret) {
		return mul128Fold64Rt(acc[0] ^ readBitsRt<size_t>(secret), acc[1] ^ readBitsRt<size_t>(secret + 8));
	}

	constexpr size_t mix2AccsCt(const size_t* acc, const uint8_t* secret) {
		return mul128Fold64Ct(acc[0] ^ readBitsCt<size_t>(secret), acc[1] ^ readBitsCt<size_t>(secret + 8));
	}

	JSONIFIER_INLINE size_t mergeAccsRt(const size_t* acc, const uint8_t* secret, size_t start) {
		size_t result64 = start;
		size_t i		  = 0;

		for (i = 0; i < 4; i++) {
			result64 += mix2AccsRt(acc + 2 * i, secret + 16 * i);
		}

		return avalanche(result64);
	}

	constexpr size_t mergeAccsCt(const size_t* acc, const uint8_t* secret, size_t start) {
		size_t result64 = start;
		size_t i		  = 0;

		for (i = 0; i < 4; i++) {
			result64 += mix2AccsCt(acc + 2 * i, secret + 16 * i);
		}

		return avalanche(result64);
	}

	JSONIFIER_INLINE void hashLongInternalLoopRt(size_t* acc, const char* input, size_t len, const uint8_t* secret) {
		{
			static constexpr size_t nbStripesPerBlock = (secretDefaultSize - jsonifierStripeLength) / jsonifierSecretConsumeRate;
			static constexpr size_t block_len		  = jsonifierStripeLength * nbStripesPerBlock;
			size_t const nb_blocks					  = (len - 1) / block_len;

			for (size_t n = 0; n < nb_blocks; n++) {
				prefetchInternal(input + n * block_len);
				accumulateExternalRt(acc, input + n * block_len, secret, nbStripesPerBlock);
				scrambleSimdRt(acc, secret + secretDefaultSize - jsonifierStripeLength);
			}

			size_t const nbStripes = ((len - 1) - (block_len * nb_blocks)) / jsonifierStripeLength;
			accumulateExternalRt(acc, input + nb_blocks * block_len, secret, nbStripes);
			{
				const char* const p = input + len - jsonifierStripeLength;
				static constexpr size_t jsonifierSecretLastAccStart{ 7 };
				accumulateSimdRt(acc, p, secret + secretDefaultSize - jsonifierStripeLength - jsonifierSecretLastAccStart);
			}
		}
	}

	constexpr void hashLongInternalLoopCt(size_t* acc, const char* input, size_t len, const uint8_t* secret) {
		constexpr size_t nbStripesPerBlock = (secretDefaultSize - jsonifierStripeLength) / jsonifierSecretConsumeRate;
		constexpr size_t block_len		   = jsonifierStripeLength * nbStripesPerBlock;
		size_t const nb_blocks			   = (len - 1) / block_len;

		for (size_t n = 0; n < nb_blocks; n++) {
			accumulateExternalCt(acc, input + n * block_len, secret, nbStripesPerBlock);
			scrambleSimdCt(acc, secret + secretDefaultSize - jsonifierStripeLength);
		}

		size_t const nbStripes = ((len - 1) - (block_len * nb_blocks)) / jsonifierStripeLength;
		accumulateExternalCt(acc, input + nb_blocks * block_len, secret, nbStripes);
		{
			const char* const p = input + len - jsonifierStripeLength;
			constexpr size_t jsonifierSecretLastAccStart{ 7 };
			accumulateSimdCt(acc, p, secret + secretDefaultSize - jsonifierStripeLength - jsonifierSecretLastAccStart);
		}
	}

	template<size_t length> JSONIFIER_INLINE size_t hashxBytesRt(const char* value, size_t hashValue = 0) {
		if constexpr (length >= 8) {
			static constexpr size_t evenVal{ length / 8 };
			static constexpr bool even{ evenVal % 2 == 0 };
			hashValue ^= static_cast<size_t>(readBitsRt<size_t>(value)) * (evenVal + (even ? 1 : 0));
			value += 8;
			static constexpr auto newLength = static_cast<int64_t>(length) - 8;
			if constexpr (newLength > 0) {
				return hashxBytesRt<newLength>(value, hashValue);
			} else {
				return avalanche(hashValue);
			}
		} else if constexpr (length == 7) {
			static constexpr size_t evenVal{ length / 4 };
			static constexpr bool even{ evenVal % 2 == 0 };
			hashValue ^= static_cast<size_t>(readBitsRt<uint32_t>(value)) * (evenVal + (even ? 1 : 0));
			value += 4;
			static constexpr size_t evenVal02{ length / 2 };
			static constexpr bool even02{ evenVal02 % 2 == 0 };
			hashValue ^= static_cast<size_t>(readBitsRt<uint16_t>(value)) * (evenVal02 + (even02 ? 1 : 0));
			value += 2;
			hashValue ^= *value ^ 1;
			return avalanche(hashValue);
		} else if constexpr (length == 6) {
			static constexpr size_t evenVal{ length / 4 };
			static constexpr bool even{ evenVal % 2 == 0 };
			hashValue ^= static_cast<size_t>(readBitsRt<uint32_t>(value)) * (evenVal + (even ? 1 : 0));
			value += 4;
			static constexpr size_t evenVal02{ length / 2 };
			static constexpr bool even02{ evenVal02 % 2 == 0 };
			hashValue ^= static_cast<size_t>(readBitsRt<uint16_t>(value)) * (evenVal02 + (even02 ? 1 : 0));
			return avalanche(hashValue);
		} else if constexpr (length == 5) {
			static constexpr size_t evenVal{ length / 4 };
			static constexpr bool even{ evenVal % 2 == 0 };
			hashValue ^= static_cast<size_t>(readBitsRt<uint32_t>(value)) * (evenVal + (even ? 1 : 0));
			value += 4;
			hashValue ^= *value ^ 1;
			return avalanche(hashValue);
		} else if constexpr (length == 4) {
			static constexpr size_t evenVal{ length / 4 };
			static constexpr bool even{ evenVal % 2 == 0 };
			hashValue ^= static_cast<size_t>(readBitsRt<uint32_t>(value)) * (evenVal + (even ? 1 : 0));
			return avalanche(hashValue);
		} else if constexpr (length == 3) {
			static constexpr size_t evenVal{ length / 2 };
			static constexpr bool even{ evenVal % 2 == 0 };
			hashValue ^= static_cast<size_t>(readBitsRt<uint16_t>(value)) * (evenVal + (even ? 1 : 0));
			value += 2;
			hashValue ^= *value ^ 1;
			return avalanche(hashValue);
		} else if constexpr (length == 2) {
			static constexpr size_t evenVal{ length / 2 };
			static constexpr bool even{ evenVal % 2 == 0 };
			hashValue ^= static_cast<size_t>(readBitsRt<uint16_t>(value)) * (evenVal + (even ? 1 : 0));
			return avalanche(hashValue);
		} else if constexpr (length == 1) {
			hashValue ^= *value ^ 1;
			return avalanche(hashValue);
		} else {
			return avalanche(hashValue);
		}
	}

	template<size_t length> constexpr size_t hashxBytesCt(const char* value, size_t hashValue = 0) {
		if constexpr (length >= 8) {
			constexpr size_t evenVal{ length / 8 };
			constexpr bool even{ evenVal % 2 == 0 };
			hashValue ^= static_cast<size_t>(readBitsCt<size_t>(value)) * (evenVal + (even ? 1 : 0));
			value += 8;
			constexpr auto newLength = static_cast<int64_t>(length) - 8;
			if constexpr (newLength > 0) {
				return hashxBytesCt<newLength>(value, hashValue);
			} else {
				return avalanche(hashValue);
			}
		} else if constexpr (length == 7) {
			constexpr size_t evenVal{ length / 4 };
			constexpr bool even{ evenVal % 2 == 0 };
			hashValue ^= static_cast<size_t>(readBitsCt<uint32_t>(value)) * (evenVal + (even ? 1 : 0));
			value += 4;
			constexpr size_t evenVal02{ length / 2 };
			constexpr bool even02{ evenVal02 % 2 == 0 };
			hashValue ^= static_cast<size_t>(readBitsCt<uint16_t>(value)) * (evenVal02 + (even02 ? 1 : 0));
			value += 2;
			hashValue ^= *value ^ 1;
			return avalanche(hashValue);
		} else if constexpr (length == 6) {
			constexpr size_t evenVal{ length / 4 };
			constexpr bool even{ evenVal % 2 == 0 };
			hashValue ^= static_cast<size_t>(readBitsCt<uint32_t>(value)) * (evenVal + (even ? 1 : 0));
			value += 4;
			constexpr size_t evenVal02{ length / 2 };
			constexpr bool even02{ evenVal02 % 2 == 0 };
			hashValue ^= static_cast<size_t>(readBitsCt<uint16_t>(value)) * (evenVal02 + (even02 ? 1 : 0));
			return avalanche(hashValue);
		} else if constexpr (length == 5) {
			constexpr size_t evenVal{ length / 4 };
			constexpr bool even{ evenVal % 2 == 0 };
			hashValue ^= static_cast<size_t>(readBitsCt<uint32_t>(value)) * (evenVal + (even ? 1 : 0));
			value += 4;
			hashValue ^= *value ^ 1;
			return avalanche(hashValue);
		} else if constexpr (length == 4) {
			constexpr size_t evenVal{ length / 4 };
			constexpr bool even{ evenVal % 2 == 0 };
			hashValue ^= static_cast<size_t>(readBitsCt<uint32_t>(value)) * (evenVal + (even ? 1 : 0));
			return avalanche(hashValue);
		} else if constexpr (length == 3) {
			constexpr size_t evenVal{ length / 2 };
			constexpr bool even{ evenVal % 2 == 0 };
			hashValue ^= static_cast<size_t>(readBitsCt<uint16_t>(value)) * (evenVal + (even ? 1 : 0));
			value += 2;
			hashValue ^= *value ^ 1;
			return avalanche(hashValue);
		} else if constexpr (length == 2) {
			constexpr size_t evenVal{ length / 2 };
			constexpr bool even{ evenVal % 2 == 0 };
			hashValue ^= static_cast<size_t>(readBitsCt<uint16_t>(value)) * (evenVal + (even ? 1 : 0));
			return avalanche(hashValue);
		} else if constexpr (length == 1) {
			hashValue ^= *value ^ 1;
			return avalanche(hashValue);
		} else {
			return avalanche(hashValue);
		}
	}

	template<size_t N> struct hash_rt_wrapper {
		static JSONIFIER_INLINE size_t func(const char* data, size_t hashValue) {
			return hashxBytesRt<N>(data, hashValue);
		}
	};

	template<size_t N> struct hash_ct_wrapper {
		static constexpr size_t func(const char* data, size_t hashValue) {
			return hashxBytesCt<N>(data, hashValue);
		}
	};

	template<template<size_t> typename hash_wrapper, std::size_t... indices> constexpr auto generateArrayOfFunctionPtrs(std::index_sequence<indices...>) {
		return std::array<size_t (*)(const char*, size_t), sizeof...(indices)>{ &hash_wrapper<indices>::func... };
	}

	constexpr auto arrayOfRtFunctionPtrs = generateArrayOfFunctionPtrs<hash_rt_wrapper>(std::make_index_sequence<2049>{});

	constexpr auto arrayOfCtFunctionPtrs = generateArrayOfFunctionPtrs<hash_ct_wrapper>(std::make_index_sequence<2049>{});

	/**
	 * @brief Class for hashing keys with runtime and compile-time support.
	 */
	struct key_hasher {
		/**
		 * @brief Default constructor that initializes the seed using a random value.
		 */
		constexpr key_hasher() {
			setSeedCt(jsonifier_internal::xoshiro256{}.operator()());
		}

		/**
		 * @brief Sets the seed value at runtime.
		 * 
		 * @param seedNew The new seed value.
		 */
		JSONIFIER_INLINE void setSeedRt(size_t seedNew) {
			seed = seedNew;
			jsonifier_internal::initCustomSecretRt(seedNew, secret);
		}

		/**
		 * @brief Sets the seed value at compile-time.
		 * 
		 * @param seedNew The new seed value.
		 */
		constexpr void setSeedCt(size_t seedNew) {
			seed = seedNew;
			jsonifier_internal::initCustomSecretCt(seedNew, secret);
		}

		/**
		 * @brief Conversion operator to get the current seed value.
		 * 
		 * @return The current seed value.
		 */
		constexpr operator size_t() const {
			return seed;
		}

		/**
		 * @brief Hashes a key at runtime.
		 * 
		 * @param value The value to be hashed.
		 * @param length The length of the value.
		 * @return The hashed value.
		 */
		JSONIFIER_INLINE size_t hashKeyRt(const void* value, size_t length) const {
			if (length <= 2048) {
				return (jsonifier_internal::arrayOfRtFunctionPtrs[length])(static_cast<const char*>(value), seed);
			} else {
				return len241ToAnyRt(static_cast<const char*>(value), length);
			}
		}

		/**
		 * @brief Hashes a key at compile-time.
		 * 
		 * @param value The value to be hashed.
		 * @param length The length of the value.
		 * @return The hashed value.
		 */
		constexpr size_t hashKeyCt(const char* value, size_t length) const {
			if (length <= 2048) {
				return (jsonifier_internal::arrayOfCtFunctionPtrs[length])(value, seed);
			} else {
				return len241ToAnyCt(value, length);
			}
		}

	  protected:
		JSONIFIER_ALIGN uint8_t secret[jsonifier_internal::secretDefaultSize]{};///< Secret key used for hashing.
		size_t seed{};///< Seed value for the hashing algorithm.

		/**
		 * @brief Structure to hold an array that can be modified at compile-time.
		 */
		struct mutable_constexpr_array {
			/// Array of accumulated values.
			JSONIFIER_ALIGN mutable std::array<size_t, jsonifier_internal::jsonifierAccNb> acc{ jsonifier_internal::jsonifierInitAcc };
		};

		/**
		 * @brief Hashes input longer than 2048 bytes at runtime.
		 * 
		 * @param input The input data to be hashed.
		 * @param len The length of the input data.
		 * @return The hashed value.
		 */
		JSONIFIER_INLINE size_t len241ToAnyRt(const char* input, size_t len) const {
			constexpr mutable_constexpr_array acc{};
			jsonifier_internal::hashLongInternalLoopRt(acc.acc.data(), input, len, secret);
			static constexpr size_t jsonifierSecretMergeAccsStart{ 11 };
			return jsonifier_internal::mergeAccsRt(acc.acc.data(), secret + jsonifierSecretMergeAccsStart, len * jsonifier_internal::jsonifierPrime641);
		}

		/**
		 * @brief Hashes input longer than 2048 bytes at compile-time.
		 * 
		 * @param input The input data to be hashed.
		 * @param len The length of the input data.
		 * @return The hashed value.
		 */
		constexpr size_t len241ToAnyCt(const char* input, size_t len) const {
			constexpr mutable_constexpr_array acc{};
			jsonifier_internal::hashLongInternalLoopCt(acc.acc.data(), input, len, secret);
			constexpr size_t jsonifierSecretMergeAccsStart{ 11 };
			return jsonifier_internal::mergeAccsCt(acc.acc.data(), secret + jsonifierSecretMergeAccsStart, len * jsonifier_internal::jsonifierPrime641);
		}
	};

}