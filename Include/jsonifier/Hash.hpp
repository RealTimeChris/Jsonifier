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
/// https://github.com/RealTimeChris/
#pragma once

#include <jsonifier/Config.hpp>
#include <jsonifier/TypeEntities.hpp>
#include <source_location>
#include <unordered_map>
#include <exception>
#include <cstring>
#include <string>
#include <iostream>
#include <array>
#include <bit>

namespace jsonifier_internal {

	constexpr uint64_t stripeLength{ 64ull };
	constexpr uint64_t accNb{ stripeLength / sizeof(uint64_t) };
	constexpr uint64_t prime641{ 0x9E3779B185EBCA87ull };
	constexpr uint64_t prime642{ 0xC2B2AE3D27D4EB4Full };
	constexpr uint64_t prime643{ 0x165667B19E3779F9ull };
	constexpr uint64_t prime644{ 0x85EBCA77C2B2AE63ull };
	constexpr uint64_t prime645{ 0x27D4EB2F165667C5ull };
	constexpr uint64_t primeMx1{ 0x165667919E3779F9ull };
	constexpr uint64_t primeMx2{ 0x9FB21C651E98DF25ull };
	constexpr uint64_t secretDefaultSize{ 192ull };
	constexpr uint64_t midSizeStartOffset{ 3ull };
	constexpr uint64_t midSizeLastOffset{ 17ull };
	constexpr uint64_t secretConsumeRate{ 8ull };
	constexpr uint32_t prime321{ 0x9E3779B1U };
	constexpr uint32_t prime322{ 0x85EBCA77U };
	constexpr uint32_t prime323{ 0xC2B2AE3DU };
	constexpr uint32_t prime324{ 0x27D4EB2FU };
	constexpr uint32_t prime325{ 0x165667B1U };
	constexpr uint64_t secretSizeMin{ 136ull };

	JSONIFIER_ALIGN constexpr uint64_t initAcc[]{ prime323, prime641, prime642, prime643, prime644, prime322, prime645, prime321 };

	JSONIFIER_ALIGN constexpr uint8_t kSecret[secretDefaultSize]{ 0xb8u, 0xfeu, 0x6cu, 0x39u, 0x23u, 0xa4u, 0x4bu, 0xbeu, 0x7cu, 0x01u, 0x81u, 0x2cu, 0xf7u, 0x21u, 0xadu, 0x1cu,
		0xdeu, 0xd4u, 0x6du, 0xe9u, 0x83u, 0x90u, 0x97u, 0xdbu, 0x72u, 0x40u, 0xa4u, 0xa4u, 0xb7u, 0xb3u, 0x67u, 0x1fu, 0xcbu, 0x79u, 0xe6u, 0x4eu, 0xccu, 0xc0u, 0xe5u, 0x78u,
		0x82u, 0x5au, 0xd0u, 0x7du, 0xccu, 0xffu, 0x72u, 0x21u, 0xb8u, 0x08u, 0x46u, 0x74u, 0xf7u, 0x43u, 0x24u, 0x8eu, 0xe0u, 0x35u, 0x90u, 0xe6u, 0x81u, 0x3au, 0x26u, 0x4cu,
		0x3cu, 0x28u, 0x52u, 0xbbu, 0x91u, 0xc3u, 0x00u, 0xcbu, 0x88u, 0xd0u, 0x65u, 0x8bu, 0x1bu, 0x53u, 0x2eu, 0xa3u, 0x71u, 0x64u, 0x48u, 0x97u, 0xa2u, 0x0du, 0xf9u, 0x4eu,
		0x38u, 0x19u, 0xefu, 0x46u, 0xa9u, 0xdeu, 0xacu, 0xd8u, 0xa8u, 0xfau, 0x76u, 0x3fu, 0xe3u, 0x9cu, 0x34u, 0x3fu, 0xf9u, 0xdcu, 0xbbu, 0xc7u, 0xc7u, 0x0bu, 0x4fu, 0x1du,
		0x8au, 0x51u, 0xe0u, 0x4bu, 0xcdu, 0xb4u, 0x59u, 0x31u, 0xc8u, 0x9fu, 0x7eu, 0xc9u, 0xd9u, 0x78u, 0x73u, 0x64u, 0xeau, 0xc5u, 0xacu, 0x83u, 0x34u, 0xd3u, 0xebu, 0xc3u,
		0xc5u, 0x81u, 0xa0u, 0xffu, 0xfau, 0x13u, 0x63u, 0xebu, 0x17u, 0x0du, 0xddu, 0x51u, 0xb7u, 0xf0u, 0xdau, 0x49u, 0xd3u, 0x16u, 0x55u, 0x26u, 0x29u, 0xd4u, 0x68u, 0x9eu,
		0x2bu, 0x16u, 0xbeu, 0x58u, 0x7du, 0x47u, 0xa1u, 0xfcu, 0x8fu, 0xf8u, 0xb8u, 0xd1u, 0x7au, 0xd0u, 0x31u, 0xceu, 0x45u, 0xcbu, 0x3au, 0x8fu, 0x95u, 0x16u, 0x04u, 0x28u,
		0xafu, 0xd7u, 0xfbu, 0xcau, 0xbbu, 0x4bu, 0x40u, 0x7eu };

#if JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_AVX512)

	JSONIFIER_INLINE void accumulateSimdRt(uint64_t* acc, const char* value, const uint8_t* secret) {
		{
			const jsonifier_simd_int_512 dataVec   = _mm512_loadu_si512(value);
			const jsonifier_simd_int_512 keyVec	   = _mm512_load_si512(secret);
			const jsonifier_simd_int_512 dataKey   = _mm512_xor_si512(dataVec, keyVec);
			const jsonifier_simd_int_512 dataKeyLo = _mm512_srli_epi64(dataKey, 32);
			const jsonifier_simd_int_512 product   = _mm512_mul_epu32(dataKey, dataKeyLo);
			const jsonifier_simd_int_512 dataSwap  = _mm512_shuffle_epi32(dataVec, ( _MM_PERM_ENUM )_MM_SHUFFLE(1, 0, 3, 2));
			const jsonifier_simd_int_512 sum	   = _mm512_add_epi64(_mm512_load_si512(acc), dataSwap);
			_mm512_store_si512(acc, _mm512_add_epi64(product, sum));
		}
	}

	constexpr void accumulateSimdCt(uint64_t* acc, const char* value, const uint8_t* secret) {
		{
			const jsonifier_simd_fb_type dataVec   = mm512LoadUSi512(value);
			const jsonifier_simd_fb_type keyVec	   = mm512LoadUSi512(secret);
			const jsonifier_simd_fb_type dataKey   = mm512XorSi512(dataVec, keyVec);
			const jsonifier_simd_fb_type dataKeyLo = mm512SrliEpi64(dataKey, 32);
			const jsonifier_simd_fb_type product   = mm512MulEpi32(dataKey, dataKeyLo);
			const jsonifier_simd_fb_type dataSwap  = mm512ShuffleEpi32(dataVec, ( _MM_PERM_ENUM )_MM_SHUFFLE(1, 0, 3, 2));
			const jsonifier_simd_fb_type sum	   = mm512AddEpi64(mm512LoadUSi512(acc), dataSwap);
			mm512StoreUSi512(acc, mm512AddEpi64(product, sum));
		}
	}

	JSONIFIER_INLINE void scrambleSimdRt(uint64_t* acc, const uint8_t* secret) {
		{
			const jsonifier_simd_int_512 accVec	   = _mm512_load_si512(acc);
			const jsonifier_simd_int_512 prime32   = _mm512_set1_epi32(( int32_t )prime321);
			const jsonifier_simd_int_512 shifted   = _mm512_srli_epi64(accVec, 47);
			const jsonifier_simd_int_512 keyVec	   = _mm512_load_si512(secret);
			const jsonifier_simd_int_512 dataKey   = _mm512_ternarylogic_epi32(keyVec, accVec, shifted, 0x96);
			const jsonifier_simd_int_512 dataKeyHi = _mm512_srli_epi64(dataKey, 32);
			const jsonifier_simd_int_512 prodLo	   = _mm512_mul_epu32(dataKey, prime32);
			const jsonifier_simd_int_512 prodHi	   = _mm512_mul_epu32(dataKeyHi, prime32);
			_mm512_store_si512(acc, _mm512_add_epi64(prodLo, _mm512_slli_epi64(prodHi, 32)));
		}
	}

	constexpr void scrambleSimdCt(uint64_t* acc, const uint8_t* secret) {
		{
			const jsonifier_simd_fb_type prime32   = mm512Set1Epi32(( int32_t )prime321);
			const jsonifier_simd_fb_type accVec	   = mm512LoadUSi512(acc);
			const jsonifier_simd_fb_type shifted   = mm512SrliEpi64(accVec, 47);
			const jsonifier_simd_fb_type keyVec	   = mm512LoadUSi512(secret);
			const jsonifier_simd_fb_type dataKey   = mm512TernarylogicEpi32(keyVec, accVec, shifted, 0x96);
			const jsonifier_simd_fb_type dataKeyHi = mm512SrliEpi64(dataKey, 32);
			const jsonifier_simd_fb_type prodLo	   = mm512MulEpi32(dataKey, prime32);
			const jsonifier_simd_fb_type prodHi	   = mm512MulEpi32(dataKeyHi, prime32);
			mm512StoreUSi512(acc, mm512AddEpi64(prodLo, mm512SlliEpi64(prodHi, 32)));
		}
	}

	JSONIFIER_INLINE void initCustomSecretRt(uint64_t seedNew, uint8_t* secret) {
		jsonifierPrefetchInternal(secret);
		{
			int32_t const nbRounds				 = secretDefaultSize / sizeof(jsonifier_simd_int_512);
			const jsonifier_simd_int_512 seedPos = _mm512_set1_epi64(seedNew);
			const jsonifier_simd_int_512 seed64	 = _mm512_mask_sub_epi64(seedPos, 0xAA, _mm512_set1_epi8(0), seedPos);
			for (int32_t i = 0; i < nbRounds; ++i) {
				_mm512_store_si512(secret + i * sizeof(jsonifier_simd_int_512), _mm512_add_epi64(_mm512_load_si512(kSecret + i * sizeof(jsonifier_simd_int_512)), seed64));
			}
		}
	}

	constexpr void initCustomSecretCt(uint64_t seedNew, uint8_t* secret) {
		{
			int32_t const nbRounds				 = secretDefaultSize / sizeof(jsonifier_simd_fb_type);
			const jsonifier_simd_fb_type seedPos = mm512Set1Epi64(seedNew);
			const jsonifier_simd_fb_type seed64	 = mm512MaskSubEpi64(seedPos, 0xAA, mm512Set1Epi8(0), seedPos);
			for (int32_t i = 0; i < nbRounds; ++i) {
				mm512StoreUSi512(secret + i * sizeof(jsonifier_simd_fb_type), mm512AddEpi64(mm512LoadUSi512(kSecret + i * sizeof(jsonifier_simd_fb_type)), seed64));
			}
		}
	}

#elif JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_AVX2)

	JSONIFIER_INLINE void accumulateSimdRt(uint64_t* acc, const char* value, const uint8_t* secret) {
		{
			for (size_t i = 0; i < stripeLength / sizeof(jsonifier_simd_int_256); ++i) {
				const jsonifier_simd_int_256 dataVec   = _mm256_loadu_si256(reinterpret_cast<const jsonifier_simd_int_256*>(value));
				const jsonifier_simd_int_256 keyVec	   = _mm256_loadu_si256(reinterpret_cast<const jsonifier_simd_int_256*>(secret));
				const jsonifier_simd_int_256 dataKey   = _mm256_xor_si256(dataVec, keyVec);
				const jsonifier_simd_int_256 dataKeyLo = _mm256_srli_epi64(dataKey, 32);
				const jsonifier_simd_int_256 product   = _mm256_mul_epi32(dataKey, dataKeyLo);
				const jsonifier_simd_int_256 dataSwap  = _mm256_shuffle_epi32(dataVec, _MM_SHUFFLE(1, 0, 3, 2));
				const jsonifier_simd_int_256 sum	   = _mm256_add_epi64(_mm256_load_si256(reinterpret_cast<const jsonifier_simd_int_256*>(acc)), dataSwap);
				_mm256_store_si256(reinterpret_cast<jsonifier_simd_int_256*>(acc), _mm256_add_epi64(dataKeyLo, sum));
				acc += sizeof(jsonifier_simd_int_256) / sizeof(uint64_t);
				secret += sizeof(jsonifier_simd_int_256);
				value += sizeof(jsonifier_simd_int_256);
			}
		}
	}

	constexpr void accumulateSimdCt(uint64_t* acc, const char* value, const uint8_t* secret) {
		{
			for (size_t i = 0; i < stripeLength / sizeof(jsonifier_simd_int_256); ++i) {
				const jsonifier_simd_fb_type dataVec   = mm256LoadUSi256(value);
				const jsonifier_simd_fb_type keyVec	   = mm256LoadUSi256(secret);
				const jsonifier_simd_fb_type dataKey   = mm256XorSi256(dataVec, keyVec);
				const jsonifier_simd_fb_type dataKeyLo = mm256SrliEpi64(dataKey, 32);
				const jsonifier_simd_fb_type product   = mm256MulEpi32(dataKey, dataKeyLo);
				const jsonifier_simd_fb_type dataSwap  = mm256ShuffleEpi32(dataVec, _MM_SHUFFLE(1, 0, 3, 2));
				const jsonifier_simd_fb_type sum	   = mm256AddEpi64(mm256LoadUSi256(acc), dataSwap);
				mm256StoreUSi256(acc, mm256AddEpi64(dataKeyLo, sum));
				acc += sizeof(jsonifier_simd_int_256) / sizeof(uint64_t);
				secret += sizeof(jsonifier_simd_int_256);
				value += sizeof(jsonifier_simd_int_256);
			}
		}
	}

	JSONIFIER_INLINE void scrambleSimdRt(uint64_t* acc, const uint8_t* secret) {
		{
			const jsonifier_simd_int_256 prime32 = _mm256_set1_epi32(( int32_t )prime321);
			for (size_t i = 0; i < stripeLength / sizeof(jsonifier_simd_int_256); i++) {
				const jsonifier_simd_int_256 accVec	   = _mm256_load_si256(reinterpret_cast<const jsonifier_simd_int_256*>(acc));
				const jsonifier_simd_int_256 shifted   = _mm256_srli_epi64(accVec, 47);
				const jsonifier_simd_int_256 dataVec   = _mm256_xor_si256(accVec, shifted);
				const jsonifier_simd_int_256 keyVec	   = _mm256_loadu_si256(reinterpret_cast<const jsonifier_simd_int_256*>(secret));
				const jsonifier_simd_int_256 dataKey   = _mm256_xor_si256(dataVec, keyVec);
				const jsonifier_simd_int_256 dataKeyHi = _mm256_srli_epi64(dataKey, 32);
				const jsonifier_simd_int_256 prodLo	   = _mm256_mul_epu32(dataKey, prime32);
				const jsonifier_simd_int_256 prodHi	   = _mm256_mul_epu32(dataKeyHi, prime32);
				_mm256_store_si256(reinterpret_cast<jsonifier_simd_int_256*>(acc), _mm256_add_epi64(prodLo, _mm256_slli_epi64(prodHi, 32)));
				acc += sizeof(jsonifier_simd_int_256) / sizeof(uint64_t);
				secret += sizeof(jsonifier_simd_int_256);
			}
		}
	}

	constexpr void scrambleSimdCt(uint64_t* acc, const uint8_t* secret) {
		{
			const jsonifier_simd_fb_type prime32 = mm256Set1Epi32(prime321);
			for (size_t i = 0; i < stripeLength / sizeof(jsonifier_simd_int_256); i++) {
				const jsonifier_simd_fb_type accVec	   = mm256LoadUSi256(acc);
				const jsonifier_simd_fb_type shifted   = mm256SrliEpi64(accVec, 47);
				const jsonifier_simd_fb_type dataVec   = mm256XorSi256(accVec, shifted);
				const jsonifier_simd_fb_type keyVec	   = mm256LoadUSi256(secret);
				const jsonifier_simd_fb_type dataKey   = mm256XorSi256(dataVec, keyVec);
				const jsonifier_simd_fb_type dataKeyHi = mm256SrliEpi64(dataKey, 32);
				const jsonifier_simd_fb_type prodLo	   = mm256MulEpi32(dataKey, prime32);
				const jsonifier_simd_fb_type prodHi	   = mm256MulEpi32(dataKeyHi, prime32);
				mm256StoreUSi256(acc, mm256AddEpi64(prodLo, mm256SlliEpi64(prodHi, 32)));
				acc += sizeof(jsonifier_simd_int_256) / sizeof(uint64_t);
				secret += sizeof(jsonifier_simd_int_256);
			}
		}
	}

	JSONIFIER_INLINE void initCustomSecretRt(uint64_t seedNew, uint8_t* secret) {
		jsonifierPrefetchInternal(secret);
		{
			jsonifier_simd_int_256 const seed64 =
				_mm256_set_epi64x(static_cast<int64_t>(0U - seedNew), static_cast<int64_t>(seedNew), static_cast<int64_t>(0U - seedNew), static_cast<int64_t>(seedNew));
			_mm256_store_si256(reinterpret_cast<jsonifier_simd_int_256*>(secret),
				_mm256_add_epi64(_mm256_load_si256(reinterpret_cast<const jsonifier_simd_int_256*>(kSecret)), seed64));
			_mm256_store_si256(reinterpret_cast<jsonifier_simd_int_256*>(secret + 32),
				_mm256_add_epi64(_mm256_load_si256(reinterpret_cast<const jsonifier_simd_int_256*>(kSecret + 32)), seed64));
			_mm256_store_si256(reinterpret_cast<jsonifier_simd_int_256*>(secret + 64),
				_mm256_add_epi64(_mm256_load_si256(reinterpret_cast<const jsonifier_simd_int_256*>(kSecret + 64)), seed64));
			_mm256_store_si256(reinterpret_cast<jsonifier_simd_int_256*>(secret + 96),
				_mm256_add_epi64(_mm256_load_si256(reinterpret_cast<const jsonifier_simd_int_256*>(kSecret + 96)), seed64));
			_mm256_store_si256(reinterpret_cast<jsonifier_simd_int_256*>(secret + 128),
				_mm256_add_epi64(_mm256_load_si256(reinterpret_cast<const jsonifier_simd_int_256*>(kSecret + 128)), seed64));
			_mm256_store_si256(reinterpret_cast<jsonifier_simd_int_256*>(secret + 160),
				_mm256_add_epi64(_mm256_load_si256(reinterpret_cast<const jsonifier_simd_int_256*>(kSecret + 160)), seed64));
		}
	}

	constexpr void initCustomSecretCt(uint64_t seedNew, uint8_t* secret) {
		{
			const jsonifier_simd_fb_type seed64 = mm256SetrEpi64x((0U - seedNew), seedNew, (0U - seedNew), seedNew);
			mm256StoreUSi256(secret, mm256AddEpi64(mm256LoadUSi256(kSecret), seed64));
			mm256StoreUSi256(secret + 32, mm256AddEpi64(mm256LoadUSi256(kSecret + 32), seed64));
			mm256StoreUSi256(secret + 64, mm256AddEpi64(mm256LoadUSi256(kSecret + 64), seed64));
			mm256StoreUSi256(secret + 96, mm256AddEpi64(mm256LoadUSi256(kSecret + 96), seed64));
			mm256StoreUSi256(secret + 128, mm256AddEpi64(mm256LoadUSi256(kSecret + 128), seed64));
			mm256StoreUSi256(secret + 160, mm256AddEpi64(mm256LoadUSi256(kSecret + 160), seed64));
		}
	}

#elif JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_AVX)

	JSONIFIER_INLINE void accumulateSimdRt(uint64_t* acc, const char* value, const uint8_t* secret) {
		{
			for (size_t i = 0; i < stripeLength / sizeof(jsonifier_simd_int_128); ++i) {
				const jsonifier_simd_int_128 dataVec   = _mm_loadu_si128(reinterpret_cast<const jsonifier_simd_int_128*>(value));
				const jsonifier_simd_int_128 keyVec	   = _mm_load_si128(reinterpret_cast<const jsonifier_simd_int_128*>(secret));
				const jsonifier_simd_int_128 dataKey   = _mm_xor_si128(dataVec, keyVec);
				const jsonifier_simd_int_128 dataKeyLo = _mm_srli_epi64(dataKey, 32);
				const jsonifier_simd_int_128 product   = _mm_mul_epi32(dataKey, dataKeyLo);
				const jsonifier_simd_int_128 dataSwap  = _mm_shuffle_epi32(dataVec, _MM_SHUFFLE(1, 0, 3, 2));
				const jsonifier_simd_int_128 sum	   = _mm_add_epi64(_mm_load_si128(reinterpret_cast<const jsonifier_simd_int_128*>(acc)), dataSwap);
				_mm_store_si128(reinterpret_cast<jsonifier_simd_int_128*>(acc), _mm_add_epi64(dataKeyLo, sum));
				acc += sizeof(jsonifier_simd_int_128) / sizeof(uint64_t);
				secret += sizeof(jsonifier_simd_int_128);
				value += sizeof(jsonifier_simd_int_128);
			}
		}
	}

	constexpr void accumulateSimdCt(uint64_t* acc, const char* value, const uint8_t* secret) {
		{
			for (size_t i = 0; i < stripeLength / sizeof(jsonifier_simd_int_128); ++i) {
				const jsonifier_simd_fb_type dataVec   = mm128LoadUSi128(value);
				const jsonifier_simd_fb_type keyVec	   = mm128LoadUSi128(secret);
				const jsonifier_simd_fb_type dataKey   = mm128XorSi128(dataVec, keyVec);
				const jsonifier_simd_fb_type dataKeyLo = mm128SrliEpi64(dataKey, 32);
				const jsonifier_simd_fb_type product   = mm128MulEpi32(dataKey, dataKeyLo);
				const jsonifier_simd_fb_type dataSwap  = mm128ShuffleEpi32(dataVec, _MM_SHUFFLE(1, 0, 3, 2));
				jsonifier_simd_fb_type sum			   = mm128AddEpi64(mm128LoadUSi128(acc), dataSwap);
				mm128StoreUSi128(acc + (i / 8), mm128AddEpi64(dataKeyLo, sum));
				acc += sizeof(jsonifier_simd_int_128) / sizeof(uint64_t);
				secret += sizeof(jsonifier_simd_int_128);
				value += sizeof(jsonifier_simd_int_128);
			}
		}
	}

	JSONIFIER_INLINE void scrambleSimdRt(uint64_t* acc, const uint8_t* secret) {
		{
			const jsonifier_simd_int_128 prime32 = _mm_set1_epi32(( int32_t )prime321);
			for (size_t i = 0; i < stripeLength / sizeof(jsonifier_simd_int_128); ++i) {
				const jsonifier_simd_int_128 accVec	   = _mm_load_si128(reinterpret_cast<const jsonifier_simd_int_128*>(acc));
				const jsonifier_simd_int_128 shifted   = _mm_srli_epi64(accVec, 47);
				const jsonifier_simd_int_128 dataVec   = _mm_xor_si128(accVec, shifted);
				const jsonifier_simd_int_128 keyVec	   = _mm_load_si128(reinterpret_cast<const jsonifier_simd_int_128*>(secret));
				const jsonifier_simd_int_128 dataKey   = _mm_xor_si128(dataVec, keyVec);
				const jsonifier_simd_int_128 dataKeyHi = _mm_srli_epi64(dataKey, 32);
				const jsonifier_simd_int_128 prodLo	   = _mm_mul_epi32(dataKey, prime32);
				const jsonifier_simd_int_128 prodHi	   = _mm_mul_epi32(dataKeyHi, prime32);
				_mm_store_si128(reinterpret_cast<jsonifier_simd_int_128*>(acc), _mm_add_epi64(prodLo, _mm_slli_epi64(prodHi, 32)));
				acc += sizeof(jsonifier_simd_int_128) / sizeof(uint64_t);
				secret += sizeof(jsonifier_simd_int_128);
			}
		}
	}

	constexpr void scrambleSimdCt(uint64_t* acc, const uint8_t* secret) {
		{
			const jsonifier_simd_fb_type prime32 = mm128Set1Epi32(prime321);

			for (size_t i = 0; i < stripeLength / sizeof(jsonifier_simd_int_128); ++i) {
				const jsonifier_simd_fb_type accVec	   = mm128LoadUSi128(acc);
				const jsonifier_simd_fb_type shifted   = mm128SrliEpi64(accVec, 47);
				const jsonifier_simd_fb_type dataVec   = mm128XorSi128(accVec, shifted);
				const jsonifier_simd_fb_type keyVec	   = mm128LoadUSi128(secret);
				const jsonifier_simd_fb_type dataKey   = mm128XorSi128(dataVec, keyVec);
				const jsonifier_simd_fb_type dataKeyHi = mm128SrliEpi64(dataKey, 32);
				const jsonifier_simd_fb_type prodLo	   = mm128MulEpi32(dataKey, prime32);
				jsonifier_simd_fb_type prodHi		   = mm128MulEpi32(dataKeyHi, prime32);
				mm128StoreUSi128(acc, mm128AddEpi64(prodLo, mm128SlliEpi64(prodHi, 32)));
				acc += sizeof(jsonifier_simd_int_128) / sizeof(uint64_t);
				secret += sizeof(jsonifier_simd_int_128);
			}
		}
	}

	JSONIFIER_INLINE void initCustomSecretRt(uint64_t seedNew, uint8_t* secret) {
		static constexpr int32_t nbRounds	= secretDefaultSize / sizeof(jsonifier_simd_int_128);
		jsonifier_simd_int_128 const seed64 = _mm_set_epi64x((0U - seedNew), seedNew);
		const char* src16					= kSecret;
		for (int32_t i = 0; i < nbRounds; ++i) {
			_mm_store_si128(reinterpret_cast<jsonifier_simd_int_128*>(secret), _mm_add_epi64(_mm_load_si128(reinterpret_cast<const jsonifier_simd_int_128*>(src16)), seed64));
			secret += sizeof(jsonifier_simd_int_128);
			src16 += sizeof(jsonifier_simd_int_128);
		}
	}

	constexpr void initCustomSecretCt(uint64_t seedNew, uint8_t* secret) {
		constexpr int32_t nbRounds			= secretDefaultSize / sizeof(__m128x);
		jsonifier_simd_fb_type const seed64 = mm128SetrEpi64x((0U - seedNew), seedNew);
		const char* src16					= kSecret;
		for (int32_t i = 0; i < nbRounds; ++i) {
			mm128StoreUSi128(secret, mm128AddEpi64(mm128LoadUSi128(src16), seed64));
			secret += sizeof(jsonifier_simd_int_128);
			src16 += sizeof(jsonifier_simd_int_128);
		}
	}

#elif JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_NEON)

	JSONIFIER_INLINE void accumulateSimdRt(uint64_t* acc, const char* value, const uint8_t* secret) {
		for (size_t i = 0; i < stripeLength / sizeof(uint64x2_t); ++i) {
			const uint64x2_t dataVec   = vld1q_u64(reinterpret_cast<const uint64_t*>(value));
			const uint64x2_t keyVec	   = vld1q_u64(reinterpret_cast<const uint64_t*>(secret));
			const uint64x2_t dataKey   = veorq_u64(dataVec, keyVec);
			const uint32x4_t dataKeyLo = vreinterpretq_u32_u64(vshrq_n_u64(dataKey, 32));
			const uint32x4_t product   = vmulq_u32(vreinterpretq_u32_u64(dataKey), dataKeyLo);
			const uint32x4_t dataSwap  = vextq_u32(vreinterpretq_u32_u64(dataVec), vreinterpretq_u32_u64(dataVec), 2);
			const uint64x2_t accVec	   = vld1q_u64(acc);
			const uint64x2_t sum	   = vaddq_u64(accVec, vreinterpretq_u64_u32(dataSwap));
			const uint64x2_t newSum	   = vaddq_u64(vreinterpretq_u64_u32(dataKeyLo), sum);
			vst1q_u64(acc, newSum);
			acc += sizeof(uint64x2_t) / sizeof(uint64_t);
			secret += sizeof(uint64x2_t);
			value += sizeof(uint64x2_t);
		}
	}

	constexpr void accumulateSimdCt(uint64_t* acc, const char* value, const uint8_t* secret) {
		{
			for (size_t i = 0; i < stripeLength / sizeof(uint64x2_t); ++i) {
				const jsonifier_simd_fb_type dataVec   = mm128LoadUSi128(value);
				const jsonifier_simd_fb_type keyVec	   = mm128LoadUSi128(secret);
				const jsonifier_simd_fb_type dataKey   = mm128XorSi128(dataVec, keyVec);
				const jsonifier_simd_fb_type dataKeyLo = mm128SrliEpi64(dataKey, 32);
				const jsonifier_simd_fb_type product   = mm128MulEpi32(dataKey, dataKeyLo);
				const jsonifier_simd_fb_type dataSwap  = mm128ShuffleEpi32(dataVec, mmShuffle(1, 0, 3, 2));
				const jsonifier_simd_fb_type xacc	   = mm128LoadUSi128(acc);
				jsonifier_simd_fb_type sum			   = mm128AddEpi64(xacc, dataSwap);
				sum									   = mm128AddEpi64(dataKeyLo, sum);
				mm128StoreUSi128(acc, sum);
				acc += sizeof(uint64x2_t) / sizeof(uint64_t);
				secret += sizeof(uint64x2_t);
				value += sizeof(uint64x2_t);
			}
		}
	}

	JSONIFIER_INLINE void scrambleSimdRt(uint64_t* acc, const uint8_t* secret) {
		const uint32x4_t prime32 = vdupq_n_u32(static_cast<uint32_t>(prime321));

		for (size_t i = 0; i < stripeLength / sizeof(uint64x2_t); ++i) {
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
			vst1q_u64(acc, vaddq_u64(vreinterpretq_u64_u32(prodLo), prodHi_shifted));
			acc += sizeof(uint64x2_t) / sizeof(uint64_t);
			secret += sizeof(uint64x2_t);
		}
	}

	constexpr void scrambleSimdCt(uint64_t* acc, const uint8_t* secret) {
		{
			const jsonifier_simd_fb_type prime32 = mm128Set1Epi32(prime321);

			for (size_t i = 0; i < stripeLength / sizeof(uint64x2_t); ++i) {
				const jsonifier_simd_fb_type accVec	   = mm128LoadUSi128(acc);
				const jsonifier_simd_fb_type shifted   = mm128SrliEpi64(accVec, 47);
				const jsonifier_simd_fb_type dataVec   = mm128XorSi128(accVec, shifted);
				const jsonifier_simd_fb_type keyVec	   = mm128LoadUSi128(secret);
				const jsonifier_simd_fb_type dataKey   = mm128XorSi128(dataVec, keyVec);
				const jsonifier_simd_fb_type dataKeyHi = mm128SrliEpi64(dataKey, 32);
				const jsonifier_simd_fb_type prodLo	   = mm128MulEpi32(dataKey, prime32);
				jsonifier_simd_fb_type prodHi		   = mm128MulEpi32(dataKeyHi, prime32);
				mm128StoreUSi128(reinterpret_cast<size_t*>(acc), mm128AddEpi64(prodLo, mm128SlliEpi64(prodHi, 32)));
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

		uint8x16x2_t kSecret[nbRounds];

		for (uint64_t i = 0; i < nbRounds; ++i) {
			uint8x16x2_t newSource = kSecret[i];
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
		constexpr auto nbRounds = secretDefaultSize / sizeof(jsonifier_simd_int_128);
		const auto seedPos		= mm128Set1Epi64x(seedNew);

		const auto mask = mm128SetrEpi64x(0ull, std::numeric_limits<uint64_t>::max());

		const __m128x zeros{};

		const auto negSeedPos = mm128SubEpi64(zeros, seedPos);
		const auto seedNewer  = mm128BlendVEpi8(seedPos, negSeedPos, mask);
		for (uint64_t i = 0; i < nbRounds; ++i) {
			auto newSource = mm128LoadUSi128(kSecret + (sizeof(jsonifier_simd_fb_type) * i));
			auto newValue  = mm128AddEpi64(newSource, seedNewer);
			mm128StoreUSi128(secret + (sizeof(jsonifier_simd_fb_type) * i), newValue);
		}
	}

#else

	JSONIFIER_INLINE void accumulateSimdRt(uint64_t* acc, const char* value, const uint8_t* secret) {
		{
			for (size_t i = 0; i < stripeLength / sizeof(jsonifier_simd_int_128); ++i) {
				const jsonifier_simd_fb_type dataVec   = mm128LoadUSi128(value);
				const jsonifier_simd_fb_type keyVec	   = mm128LoadUSi128(secret);
				const jsonifier_simd_fb_type dataKey   = mm128XorSi128(dataVec, keyVec);
				const jsonifier_simd_fb_type dataKeyLo = mm128SrliEpi64(dataKey, 32);
				const jsonifier_simd_fb_type product   = mm128MulEpi32(dataKey, dataKeyLo);
				const jsonifier_simd_fb_type dataSwap  = mm128ShuffleEpi32(dataVec, _MM_SHUFFLE(1, 0, 3, 2));
				const jsonifier_simd_fb_type sum	   = mm128AddEpi64(mm128LoadUSi128(acc), dataSwap);
				mm128StoreUSi128(acc + (i / 8), mm128AddEpi64(dataKeyLo, sum));
				acc += sizeof(jsonifier_simd_int_128) / sizeof(uint64_t);
				secret += sizeof(jsonifier_simd_int_128);
				value += sizeof(jsonifier_simd_int_128);
			}
		}
	}

	constexpr void accumulateSimdCt(uint64_t* acc, const char* value, const uint8_t* secret) {
		{
			for (size_t i = 0; i < stripeLength / sizeof(jsonifier_simd_int_128); ++i) {
				const jsonifier_simd_fb_type dataVec   = mm128LoadUSi128(value);
				const jsonifier_simd_fb_type keyVec	   = mm128LoadUSi128(secret);
				const jsonifier_simd_fb_type dataKey   = mm128XorSi128(dataVec, keyVec);
				const jsonifier_simd_fb_type dataKeyLo = mm128SrliEpi64(dataKey, 32);
				const jsonifier_simd_fb_type product   = mm128MulEpi32(dataKey, dataKeyLo);
				const jsonifier_simd_fb_type dataSwap  = mm128ShuffleEpi32(dataVec, _MM_SHUFFLE(1, 0, 3, 2));
				jsonifier_simd_fb_type sum			   = mm128AddEpi64(mm128LoadUSi128(acc), dataSwap);
				mm128StoreUSi128(acc + (i / 8), mm128AddEpi64(dataKeyLo, sum));
				acc += sizeof(jsonifier_simd_int_128) / sizeof(uint64_t);
				secret += sizeof(jsonifier_simd_int_128);
				value += sizeof(jsonifier_simd_int_128);
			}
		}
	}

	JSONIFIER_INLINE void scrambleSimdRt(uint64_t* acc, const uint8_t* secret) {
		{
			const jsonifier_simd_fb_type prime32 = mm128Set1Epi32(prime321);

			for (size_t i = 0; i < stripeLength; i += 16) {
				const jsonifier_simd_fb_type xacc	   = mm128LoadUSi128(acc + (i / 8));
				const jsonifier_simd_fb_type accVec	   = xacc;
				const jsonifier_simd_fb_type shifted   = mm128SrliEpi64(accVec, 47);
				const jsonifier_simd_fb_type dataVec   = mm128XorSi128(accVec, shifted);
				const jsonifier_simd_fb_type keyVec	   = mm128LoadUSi128(secret + i);
				const jsonifier_simd_fb_type dataKey   = mm128XorSi128(dataVec, keyVec);
				const jsonifier_simd_fb_type dataKeyHi = mm128SrliEpi64(dataKey, 32);
				const jsonifier_simd_fb_type prodLo	   = mm128MulEpi32(dataKey, prime32);
				jsonifier_simd_fb_type prodHi		   = mm128MulEpi32(dataKeyHi, prime32);
				prodHi								   = mm128AddEpi64(prodLo, mm128SlliEpi64(prodHi, 32));
				mm128StoreUSi128(acc + (i / 8), prodHi);
			}
		}
	}

	constexpr void scrambleSimdCt(uint64_t* acc, const uint8_t* secret) {
		{
			const jsonifier_simd_fb_type prime32 = mm128Set1Epi32(prime321);

			for (size_t i = 0; i < stripeLength; i += 16) {
				const jsonifier_simd_fb_type xacc	   = mm128LoadUSi128(acc + (i / 8));
				const jsonifier_simd_fb_type accVec	   = xacc;
				const jsonifier_simd_fb_type shifted   = mm128SrliEpi64(accVec, 47);
				const jsonifier_simd_fb_type dataVec   = mm128XorSi128(accVec, shifted);
				const jsonifier_simd_fb_type keyVec	   = mm128LoadUSi128(secret + i);
				const jsonifier_simd_fb_type dataKey   = mm128XorSi128(dataVec, keyVec);
				const jsonifier_simd_fb_type dataKeyHi = mm128SrliEpi64(dataKey, 32);
				const jsonifier_simd_fb_type prodLo	   = mm128MulEpi32(dataKey, prime32);
				jsonifier_simd_fb_type prodHi		   = mm128MulEpi32(dataKeyHi, prime32);
				prodHi								   = mm128AddEpi64(prodLo, mm128SlliEpi64(prodHi, 32));
				mm128StoreUSi128(acc + (i / 8), prodHi);
			}
		}
	}

	JSONIFIER_INLINE void initCustomSecretRt(uint64_t seedNew, uint8_t* secret) {
		constexpr int32_t nbRounds			= secretDefaultSize / sizeof(__m128x);
		jsonifier_simd_fb_type const seed64 = mm128SetrEpi64x((0U - seedNew), seedNew);
		const char* src16					= kSecret;
		for (int32_t i = 0; i < nbRounds; ++i) {
			mm128StoreUSi128(secret, mm128AddEpi64(mm128LoadUSi128(src16), seed64));
			secret += sizeof(jsonifier_simd_int_128);
			src16 += sizeof(jsonifier_simd_int_128);
		}
	}

	constexpr void initCustomSecretCt(uint64_t seedNew, uint8_t* secret) {
		constexpr int32_t nbRounds			= secretDefaultSize / sizeof(__m128x);
		jsonifier_simd_fb_type const seed64 = mm128SetrEpi64x((0U - seedNew), seedNew);
		const char* src16					= kSecret;
		for (int32_t i = 0; i < nbRounds; ++i) {
			mm128StoreUSi128(secret, mm128AddEpi64(mm128LoadUSi128(src16), seed64));
			secret += sizeof(jsonifier_simd_int_128);
			src16 += sizeof(jsonifier_simd_int_128);
		}
	}

#endif
	JSONIFIER_INLINE constexpr uint32_t rotl32Ct(uint32_t x, uint32_t r) {
		return (((x) << (r)) | ((x) >> (32 - (r))));
	};

	JSONIFIER_INLINE constexpr uint64_t rotl64Ct(uint64_t x, uint64_t r) {
		return (((x) << (r)) | ((x) >> (64 - (r))));
	};

#if !defined(NO_CLANG_BUILTIN) && JSONIFIER_HAS_BUILTIN(__builtin_rotateleft32) && JSONIFIER_HAS_BUILTIN(__builtin_rotateleft64)
	#define rotl32Rt __builtin_rotateleft32
	#define rotl64Rt __builtin_rotateleft64
#elif defined(JSONIFIER_MSVC)
	#define rotl32Rt(x, r) _rotl(x, r)
	#define rotl64Rt(x, r) _rotl64(x, r)
#else
	#define rotl32Rt(x, r) rotl32Ct(x, r)
	#define rotl64Rt(x, r) rotl64Ct(x, r)
#endif

	JSONIFIER_INLINE constexpr uint32_t swap32Ct(uint32_t x) {
		return ((x << 24) & 0xff000000) | ((x << 8) & 0x00ff0000) | ((x >> 8) & 0x0000ff00) | ((x >> 24) & 0x000000ff);
	}

#if defined(JSONIFIER_MSVC)
	#define swap32Rt _byteswap_ulong
#elif JSONIFIER_GCC_VERSION >= 403 || defined(JSONIFIER_CLANG)
	#define swap32Rt __builtin_bswap32
#else
	#define swap32Rt swap32Ct
#endif

	JSONIFIER_INLINE constexpr uint64_t swap64Ct(uint64_t x) {
		return ((x << 56) & 0xff00000000000000ull) | ((x << 40) & 0x00ff000000000000ull) | ((x << 24) & 0x0000ff0000000000ull) | ((x << 8) & 0x000000ff00000000ull) |
			((x >> 8) & 0x00000000ff000000ull) | ((x >> 24) & 0x0000000000ff0000ull) | ((x >> 40) & 0x000000000000ff00ull) | ((x >> 56) & 0x00000000000000ffull);
	}

#if defined(JSONIFIER_MSVC)
	#define swap64Rt _byteswap_uint64
#elif JSONIFIER_GCC_VERSION >= 403 || defined(JSONIFIER_CLANG)
	#define swap64Rt __builtin_bswap64
#else
	#define swap64Rt swap64Ct
#endif

	struct xoshiro256 {
		size_t state[4]{};

		constexpr xoshiro256() {
			constexpr auto x   = 7185499250578500046ull >> 12ull;
			constexpr auto x01 = x ^ x << 25ull;
			constexpr auto x02 = x01 ^ x01 >> 27ull;
			size_t s		   = x02 * 0x2545F4914F6CDD1Dull;
			for (uint64_t y = 0; y < 4; ++y) {
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

		constexpr size_t splitmix64(size_t& seed64) const {
			size_t result = seed64 += 0x9E3779B97F4A7C15ull;
			result		  = (result ^ (result >> 30ull)) * 0xBF58476D1CE4E5B9ull;
			result		  = (result ^ (result >> 27ull)) * 0x94D049BB133111EBull;
			return result ^ (result >> 31ull);
		}
	};

	JSONIFIER_INLINE void accumulateExternalRt(uint64_t* acc, const char* value, const uint8_t* secret, size_t nbStripes) {
		size_t n;
		for (n = 0; n < nbStripes; n++) {
			const char* const in = value + n * 64;
			jsonifierPrefetchInternal(in);
			accumulateSimdRt(acc, in, secret + n * 8);
		}
	}

	constexpr void accumulateExternalCt(uint64_t* acc, const char* value, const uint8_t* secret, size_t nbStripes) {
		size_t n;
		for (n = 0; n < nbStripes; n++) {
			const char* const in = value + n * 64;
			accumulateSimdCt(acc, in, secret + n * 8);
		}
	}

	template<typename value_type> JSONIFIER_INLINE value_type readBitsRt(const void* ptr) {
		value_type returnValue{};
		std::memcpy(&returnValue, ptr, sizeof(value_type));
		return returnValue;
	}

	template<typename value_type, typename char_type> constexpr value_type readBitsCt(const char_type* ptr) {
		value_type returnValue{};
		for (uint64_t x = 0; x < sizeof(value_type); ++x) {
			returnValue |= static_cast<value_type>(static_cast<uint8_t>(ptr[x])) << (x * 8);
		}
		return returnValue;
	}

	template<uint64_t shiftNew> JSONIFIER_INLINE constexpr uint64_t xorShift64(uint64_t v64) {
		return v64 ^ (v64 >> shiftNew);
	}

	JSONIFIER_INLINE constexpr uint64_t avalanche(uint64_t hash) {
		hash ^= hash >> 33;
		hash *= prime641;
		hash ^= hash >> 29;
		hash *= prime643;
		hash ^= hash >> 32;
		return hash;
	}

	JSONIFIER_INLINE __m128x mult64To128Rt(uint64_t lhs, uint64_t rhs) {
		{
#if (defined(JSONIFIER_GNUCXX) || defined(JSONIFIER_CLANG)) && !defined(__wasm__) && defined(__SIZEOF_INT128__) || (defined(_INTEGRAL_MAX_BITS) && _INTEGRAL_MAX_BITS >= 128)
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
			uint64_t const loLo	 = mult32To64(lhs & 0xFFFFFFFF, rhs & 0xFFFFFFFF);
			uint64_t const hiLo	 = mult32To64(lhs >> 32, rhs & 0xFFFFFFFF);
			uint64_t const loHi	 = mult32To64(lhs & 0xFFFFFFFF, rhs >> 32);
			uint64_t const hiHi	 = mult32To64(lhs >> 32, rhs >> 32);
			uint64_t const cross = (loLo >> 32) + (hiLo & 0xFFFFFFFF) + loHi;
			uint64_t const upper = (hiLo >> 32) + (cross >> 32) + hiHi;
			uint64_t const lower = (cross << 32) | (loLo & 0xFFFFFFFF);

			__m128x r128;
			r128.m128x_uint64[0] = lower;
			r128.m128x_uint64[1] = upper;
			return r128;
#endif
		}
	}

	JSONIFIER_INLINE constexpr uint64_t mult32To64(uint64_t x, uint64_t y) {
		return (x & 0xFFFFFFFF) * (y & 0xFFFFFFFF);
	}

	constexpr __m128x mult64To128Ct(uint64_t lhs, uint64_t rhs) {
		uint64_t const loLo	 = mult32To64(lhs & 0xFFFFFFFF, rhs & 0xFFFFFFFF);
		uint64_t const hiLo	 = mult32To64(lhs >> 32, rhs & 0xFFFFFFFF);
		uint64_t const loHi	 = mult32To64(lhs & 0xFFFFFFFF, rhs >> 32);
		uint64_t const hiHi	 = mult32To64(lhs >> 32, rhs >> 32);
		uint64_t const cross = (loLo >> 32) + (hiLo & 0xFFFFFFFF) + loHi;
		uint64_t const upper = (hiLo >> 32) + (cross >> 32) + hiHi;
		uint64_t const lower = (cross << 32) | (loLo & 0xFFFFFFFF);

		__m128x r128;
		r128.m128x_uint64[0] = lower;
		r128.m128x_uint64[1] = upper;
		return r128;
	}

	JSONIFIER_INLINE uint64_t mul128Fold64Rt(uint64_t lhs, uint64_t rhs) {
		{
			__m128x product = mult64To128Rt(lhs, rhs);
			return product.m128x_uint64[0] ^ product.m128x_uint64[1];
		}
	}

	constexpr uint64_t mul128Fold64Ct(uint64_t lhs, uint64_t rhs) {
		__m128x product = mult64To128Ct(lhs, rhs);
		return product.m128x_uint64[0] ^ product.m128x_uint64[1];
	}

	JSONIFIER_INLINE uint64_t mix2AccsRt(const uint64_t* acc, const uint8_t* secret) {
		return mul128Fold64Rt(acc[0] ^ readBitsRt<uint64_t>(secret), acc[1] ^ readBitsRt<uint64_t>(secret + 8));
	}

	constexpr uint64_t mix2AccsCt(const uint64_t* acc, const uint8_t* secret) {
		return mul128Fold64Ct(acc[0] ^ readBitsCt<uint64_t>(secret), acc[1] ^ readBitsCt<uint64_t>(secret + 8));
	}

	JSONIFIER_INLINE uint64_t mergeAccsRt(const uint64_t* acc, const uint8_t* secret, uint64_t start) {
		uint64_t result64 = start;
		size_t i		  = 0;

		for (i = 0; i < 4; i++) {
			result64 += mix2AccsRt(acc + 2 * i, secret + 16 * i);
		}

		return avalanche(result64);
	}

	constexpr uint64_t mergeAccsCt(const uint64_t* acc, const uint8_t* secret, uint64_t start) {
		uint64_t result64 = start;
		size_t i		  = 0;

		for (i = 0; i < 4; i++) {
			result64 += mix2AccsCt(acc + 2 * i, secret + 16 * i);
		}

		return avalanche(result64);
	}

	JSONIFIER_INLINE void hashLongInternalLoopRt(uint64_t* acc, const char* value, size_t length, const uint8_t* secret) {
		{
			static constexpr size_t nbStripesPerBlock = (secretDefaultSize - stripeLength) / secretConsumeRate;
			static constexpr size_t block_len		  = stripeLength * nbStripesPerBlock;
			const size_t nb_blocks					  = (length - 1) / block_len;

			for (size_t n = 0; n < nb_blocks; n++) {
				jsonifierPrefetchInternal(value + n * block_len);
				accumulateExternalRt(acc, value + n * block_len, secret, nbStripesPerBlock);
				scrambleSimdRt(acc, secret + secretDefaultSize - stripeLength);
			}

			const size_t nbStripes = ((length - 1) - (block_len * nb_blocks)) / stripeLength;
			accumulateExternalRt(acc, value + nb_blocks * block_len, secret, nbStripes);
			{
				const char* const p = value + length - stripeLength;
				static constexpr uint64_t SecretLastAccStart{ 7 };
				accumulateSimdRt(acc, p, secret + secretDefaultSize - stripeLength - SecretLastAccStart);
			}
		}
	}

	constexpr void hashLongInternalLoopCt(uint64_t* acc, const char* value, size_t length, const uint8_t* secret) {
		constexpr size_t nbStripesPerBlock = (secretDefaultSize - stripeLength) / secretConsumeRate;
		constexpr size_t block_len		   = stripeLength * nbStripesPerBlock;
		const size_t nb_blocks			   = (length - 1) / block_len;

		for (size_t n = 0; n < nb_blocks; n++) {
			accumulateExternalCt(acc, value + n * block_len, secret, nbStripesPerBlock);
			scrambleSimdCt(acc, secret + secretDefaultSize - stripeLength);
		}

		const size_t nbStripes = ((length - 1) - (block_len * nb_blocks)) / stripeLength;
		accumulateExternalCt(acc, value + nb_blocks * block_len, secret, nbStripes);
		{
			const char* const p = value + length - stripeLength;
			constexpr uint64_t SecretLastAccStart{ 7 };
			accumulateSimdCt(acc, p, secret + secretDefaultSize - stripeLength - SecretLastAccStart);
		}
	}

	template<uint64_t length> JSONIFIER_INLINE uint64_t len1To3Rt(const char* value, const uint8_t* secret, uint64_t seed64) {
		{
			uint8_t const c1 = static_cast<uint8_t>(value[0]);
			uint8_t const c2 = static_cast<uint8_t>(value[length >> 1]);
			uint8_t const c3 = static_cast<uint8_t>(value[length - 1]);
			uint32_t const combined =
				(static_cast<uint32_t>(c1) << 16) | (static_cast<uint32_t>(c2) << 24) | (static_cast<uint32_t>(c3) << 0) | (static_cast<uint32_t>(length) << 8);
			uint64_t const bitflip = (readBitsRt<uint32_t>(secret) ^ readBitsRt<uint32_t>(secret + 4)) + seed64;
			uint64_t const keyed   = static_cast<uint64_t>(combined) ^ bitflip;
			return avalanche(keyed);
		}
	}

	template<uint64_t length> constexpr uint64_t len1To3Ct(const char* value, const uint8_t* secret, uint64_t seed64) {
		{
			uint8_t const c1 = static_cast<uint8_t>(value[0]);
			uint8_t const c2 = static_cast<uint8_t>(value[length >> 1]);
			uint8_t const c3 = static_cast<uint8_t>(value[length - 1]);
			uint32_t const combined =
				(static_cast<uint32_t>(c1) << 16) | (static_cast<uint32_t>(c2) << 24) | (static_cast<uint32_t>(c3) << 0) | (static_cast<uint32_t>(length) << 8);
			uint64_t const bitflip = (readBitsCt<uint32_t>(secret) ^ readBitsCt<uint32_t>(secret + 4)) + seed64;
			uint64_t const keyed   = static_cast<uint64_t>(combined) ^ bitflip;
			return avalanche(keyed);
		}
	}

	JSONIFIER_INLINE uint64_t rrmxmxRt(uint64_t h64, uint64_t length) {
		h64 ^= rotl64Rt(h64, 49) ^ rotl64Rt(h64, 24);
		h64 *= primeMx2;
		h64 ^= (h64 >> 35) + length;
		h64 *= primeMx2;
		return xorShift64<28>(h64);
	}

	constexpr uint64_t rrmxmxCt(uint64_t h64, uint64_t length) {
		h64 ^= rotl64Ct(h64, 49) ^ rotl64Ct(h64, 24);
		h64 *= primeMx2;
		h64 ^= (h64 >> 35) + length;
		h64 *= primeMx2;
		return xorShift64<28>(h64);
	}

	template<uint64_t length> JSONIFIER_INLINE uint64_t len4To8Rt(const char* value, const uint8_t* secret, uint64_t seed64) {
		seed64 ^= ( uint64_t )swap32Rt(( uint32_t )seed64) << 32;
		{
			uint32_t const input1  = readBitsRt<uint32_t>(value);
			uint32_t const input2  = readBitsRt<uint32_t>(value + length - 4);
			uint64_t const bitflip = (readBitsRt<uint64_t>(secret + 8) ^ readBitsRt<uint64_t>(secret + 16)) - seed64;
			uint64_t const input64 = input2 + ((static_cast<uint64_t>(input1)) << 32);
			uint64_t const keyed   = input64 ^ bitflip;
			return rrmxmxRt(keyed, length);
		}
	}

	template<uint64_t length> constexpr uint64_t len4To8Ct(const char* value, const uint8_t* secret, uint64_t seed64) {
		seed64 ^= ( uint64_t )swap32Ct(( uint32_t )seed64) << 32;
		{
			uint32_t const input1  = readBitsCt<uint32_t>(value);
			uint32_t const input2  = readBitsCt<uint32_t>(value + length - 4);
			uint64_t const bitflip = (readBitsCt<uint64_t>(secret + 8) ^ readBitsCt<uint64_t>(secret + 16)) - seed64;
			uint64_t const input64 = input2 + ((static_cast<uint64_t>(input1)) << 32);
			uint64_t const keyed   = input64 ^ bitflip;
			return rrmxmxCt(keyed, length);
		}
	}

	template<uint64_t length> JSONIFIER_INLINE uint64_t len9To16Rt(const char* value, const uint8_t* secret, uint64_t seed64) {
		{
			uint64_t const bitflip1 = (readBitsRt<uint64_t>(secret + 24) ^ readBitsRt<uint64_t>(secret + 32)) + seed64;
			uint64_t const bitflip2 = (readBitsRt<uint64_t>(secret + 40) ^ readBitsRt<uint64_t>(secret + 48)) - seed64;
			uint64_t const input_lo = readBitsRt<uint64_t>(value) ^ bitflip1;
			uint64_t const input_hi = readBitsRt<uint64_t>(value + length - 8) ^ bitflip2;
			uint64_t const acc		= length + swap64Rt(input_lo) + input_hi + mul128Fold64Rt(input_lo, input_hi);
			return avalanche(acc);
		}
	}

	template<uint64_t length> constexpr uint64_t len9To16Ct(const char* value, const uint8_t* secret, uint64_t seed64) {
		{
			uint64_t const bitflip1 = (readBitsCt<uint64_t>(secret + 24) ^ readBitsCt<uint64_t>(secret + 32)) + seed64;
			uint64_t const bitflip2 = (readBitsCt<uint64_t>(secret + 40) ^ readBitsCt<uint64_t>(secret + 48)) - seed64;
			uint64_t const input_lo = readBitsCt<uint64_t>(value) ^ bitflip1;
			uint64_t const input_hi = readBitsCt<uint64_t>(value + length - 8) ^ bitflip2;
			uint64_t const acc		= length + swap64Ct(input_lo) + input_hi + mul128Fold64Ct(input_lo, input_hi);
			return avalanche(acc);
		}
	}

	JSONIFIER_INLINE uint64_t mix16BRt(const char* value, const uint8_t* secret, uint64_t seed64) {
		{
			uint64_t const input_lo = readBitsRt<uint64_t>(value);
			uint64_t const input_hi = readBitsRt<uint64_t>(value + 8);
			return mul128Fold64Rt(input_lo ^ (readBitsRt<uint64_t>(secret) + seed64), input_hi ^ (readBitsRt<uint64_t>(secret + 8) - seed64));
		}
	}

	constexpr uint64_t mix16BCt(const char* value, const uint8_t* secret, uint64_t seed64) {
		{
			uint64_t const input_lo = readBitsCt<uint64_t>(value);
			uint64_t const input_hi = readBitsCt<uint64_t>(value + 8);
			return mul128Fold64Ct(input_lo ^ (readBitsCt<uint64_t>(secret) + seed64), input_hi ^ (readBitsCt<uint64_t>(secret + 8) - seed64));
		}
	}

	template<uint64_t length> JSONIFIER_INLINE uint64_t len17To128Rt(const char* value, const uint8_t* secret, uint64_t seed64) {
		{
			uint64_t acc = length * prime641;
			if constexpr (length > 32) {
				if constexpr (length > 64) {
					if constexpr (length > 96) {
						acc += mix16BRt(value + 48, secret + 96, seed64);
						acc += mix16BRt(value + length - 64, secret + 112, seed64);
					}
					acc += mix16BRt(value + 32, secret + 64, seed64);
					acc += mix16BRt(value + length - 48, secret + 80, seed64);
				}
				acc += mix16BRt(value + 16, secret + 32, seed64);
				acc += mix16BRt(value + length - 32, secret + 48, seed64);
			}
			acc += mix16BRt(value + 0, secret + 0, seed64);
			acc += mix16BRt(value + length - 16, secret + 16, seed64);
			return avalanche(acc);
		}
	}

	template<uint64_t length> constexpr uint64_t len17To128Ct(const char* value, const uint8_t* secret, uint64_t seed64) {
		{
			uint64_t acc = length * prime641;
			if constexpr (length > 32) {
				if constexpr (length > 64) {
					if constexpr (length > 96) {
						acc += mix16BCt(value + 48, secret + 96, seed64);
						acc += mix16BCt(value + length - 64, secret + 112, seed64);
					}
					acc += mix16BCt(value + 32, secret + 64, seed64);
					acc += mix16BCt(value + length - 48, secret + 80, seed64);
				}
				acc += mix16BCt(value + 16, secret + 32, seed64);
				acc += mix16BCt(value + length - 32, secret + 48, seed64);
			}
			acc += mix16BCt(value + 0, secret + 0, seed64);
			acc += mix16BCt(value + length - 16, secret + 16, seed64);
			return avalanche(acc);
		}
	}

	template<uint64_t currentIndex, uint64_t maxIndex> JSONIFIER_INLINE uint64_t len129To240Internal01Rt(uint64_t& acc, const char* value, const uint8_t* secret, uint64_t seed64) {
		if constexpr (currentIndex < maxIndex) {
			acc += mix16BRt(value + (16 * currentIndex), secret + (16 * currentIndex), seed64);
			return len129To240Internal01Rt<currentIndex + 1, maxIndex>(acc, value, secret, seed64);
		} else {
			return acc;
		}
	}

	template<uint64_t currentIndex, uint64_t maxIndex> JSONIFIER_INLINE uint64_t len129To240Internal02Rt(uint64_t& acc, const char* value, const uint8_t* secret, uint64_t seed64) {
		if constexpr (currentIndex < maxIndex) {
			acc += mix16BRt(value + (16 * currentIndex), secret + (16 * (currentIndex - 8)) + midSizeStartOffset, seed64);
			return len129To240Internal02Rt<currentIndex + 1, maxIndex>(acc, value, secret, seed64);
		} else {
			return acc;
		}
	}

	template<uint64_t length> JSONIFIER_INLINE uint64_t len129To240Rt(const char* value, const uint8_t* secret, uint64_t seed64) {
		{
			uint64_t acc = length * prime641;
			uint64_t acc_end;
			constexpr uint32_t nbRounds = ( uint32_t )length / 16;
			acc							= len129To240Internal01Rt<0, 8>(acc, value, secret, seed64);
			acc_end						= mix16BRt(value + length - 16, secret + secretSizeMin - midSizeLastOffset, seed64);
			acc							= avalanche(acc);
			acc_end						= len129To240Internal02Rt<8, nbRounds>(acc_end, value, secret, seed64);
			return avalanche(acc + acc_end);
		}
	}

	template<uint64_t currentIndex, uint64_t maxIndex> constexpr uint64_t len129To240Internal01Ct(uint64_t& acc, const char* value, const uint8_t* secret, uint64_t seed64) {
		if constexpr (currentIndex < maxIndex) {
			acc += mix16BCt(value + (16 * currentIndex), secret + (16 * currentIndex), seed64);
			return len129To240Internal01Ct<currentIndex + 1, maxIndex>(acc, value, secret, seed64);
		} else {
			return acc;
		}
	}

	template<uint64_t currentIndex, uint64_t maxIndex> constexpr uint64_t len129To240Internal02Ct(uint64_t& acc, const char* value, const uint8_t* secret, uint64_t seed64) {
		if constexpr (currentIndex < maxIndex) {
			acc += mix16BCt(value + (16 * currentIndex), secret + (16 * (currentIndex - 8)) + midSizeStartOffset, seed64);
			return len129To240Internal02Ct<currentIndex + 1, maxIndex>(acc, value, secret, seed64);
		} else {
			return acc;
		}
	}

	template<uint64_t length> constexpr uint64_t len129To240Ct(const char* value, const uint8_t* secret, uint64_t seed64) {
		{
			uint64_t acc = length * prime641;
			uint64_t acc_end;
			constexpr uint32_t nbRounds = ( uint32_t )length / 16;
			acc							= len129To240Internal01Ct<0, 8>(acc, value, secret, seed64);
			acc_end						= mix16BCt(value + length - 16, secret + secretSizeMin - midSizeLastOffset, seed64);
			acc							= avalanche(acc);
			acc_end						= len129To240Internal02Ct<8, nbRounds>(acc_end, value, secret, seed64);
			return avalanche(acc + acc_end);
		}
	}

	template<uint64_t length> JSONIFIER_INLINE uint64_t hashxBytesRt(const char* value, const uint8_t* secret, uint64_t seed64) {
		if constexpr (length == 0) {
			return avalanche(seed64 ^ (readBitsRt<uint64_t>(secret + 56) ^ readBitsRt<uint64_t>(secret + 64)));
		} else if constexpr (length <= 3) {
			return len1To3Rt<length>(value, secret, seed64);
		} else if constexpr (length <= 8) {
			return len4To8Rt<length>(value, secret, seed64);
		} else if constexpr (length <= 16) {
			return len9To16Rt<length>(value, secret, seed64);
		} else if constexpr (length <= 128) {
			return len17To128Rt<length>(value, secret, seed64);
		} else {
			return len129To240Rt<length>(value, secret, seed64);
		}
	}

	template<uint64_t length> constexpr uint64_t hashxBytesCt(const char* value, const uint8_t* secret, uint64_t seed64) {
		if constexpr (length == 0) {
			return avalanche(seed64 ^ (readBitsCt<uint64_t>(secret + 56) ^ readBitsCt<uint64_t>(secret + 64)));
		} else if constexpr (length <= 3) {
			return len1To3Ct<length>(value, secret, seed64);
		} else if constexpr (length <= 8) {
			return len4To8Ct<length>(value, secret, seed64);
		} else if constexpr (length <= 16) {
			return len9To16Ct<length>(value, secret, seed64);
		} else if constexpr (length <= 128) {
			return len17To128Ct<length>(value, secret, seed64);
		} else {
			return len129To240Ct<length>(value, secret, seed64);
		}
	}

	template<uint64_t N> struct hash_rt_wrapper {
		JSONIFIER_INLINE static uint64_t op(const char* data, const uint8_t* secret, uint64_t seed64) {
			return hashxBytesRt<N>(data, secret, seed64);
		}
	};

	template<uint64_t N> struct hash_ct_wrapper {
		static constexpr uint64_t op(const char* data, const uint8_t* secret, uint64_t seed64) {
			return hashxBytesCt<N>(data, secret, seed64);
		}
	};

	constexpr auto arrayOfRtFunctionPtrs = generateArrayOfFunctionPtrs<hash_rt_wrapper>(std::make_index_sequence<241>{});

	constexpr auto arrayOfCtFunctionPtrs = generateArrayOfFunctionPtrs<hash_ct_wrapper>(std::make_index_sequence<241>{});

	/**
	 * @brief Structure to hold an array that can be modified at compile-time.
	 */
	struct mutable_constexpr_array {
		/// Array of accumulated values.
		constexpr mutable_constexpr_array(const uint64_t (&ptr)[8]) {
			std::copy(ptr, ptr + 8, acc.data());
		}
		JSONIFIER_ALIGN mutable std::array<uint64_t, jsonifier_internal::accNb> acc{};
	};

	struct key_hasher {
		/**
		 * @brief Default constructor that initializes the seed using a random_num value.
		 */
		constexpr key_hasher(size_t seedNew = 0) {
			setSeedCt(seedNew);
		}

		/**
		 * @brief Sets the seed value at runtime.
		 *
		 * @param seedNew The new seed value.
		 */
		JSONIFIER_INLINE void setSeedRt(uint64_t seedNew) {
			seed = seedNew;
			jsonifier_internal::initCustomSecretRt(seedNew, secret);
		}

		/**
		 * @brief Sets the seed value at compile-time.
		 *
		 * @param seedNew The new seed value.
		 */
		constexpr void setSeedCt(uint64_t seedNew) {
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
		JSONIFIER_INLINE uint64_t hashKeyRt(const char* value, uint64_t length) const {
			if (length <= 240) {
				return jsonifier_internal::arrayOfRtFunctionPtrs[length](value, secret, seed);
			} else {
				return len241ToAnyRt(value, length);
			}
			return {};
		}

		/**
		 * @brief Hashes a key at compile-time.
		 *
		 * @param value The value to be hashed.
		 * @param length The length of the value.
		 * @return The hashed value.
		 */
		constexpr uint64_t hashKeyCt(const char* value, uint64_t length) const {
			////std::cout << "NEW HASH STARTING: " << std::endl;
			if (length <= 240) {
				return (jsonifier_internal::arrayOfCtFunctionPtrs[length])(value, secret, seed);
			} else {
				return len241ToAnyCt(value, length);
			}
		}

	  protected:
		uint8_t secret[jsonifier_internal::secretDefaultSize]{};///< Secret key used for hashing.
		uint64_t seed{};///< Seed value for the hashing algorithm.

		/**
		 * @brief Hashes value longer than 240 bytes at runtime.
		 *
		 * @param value The value data to be hashed.
		 * @param length The length of the value data.
		 * @return The hashed value.
		 */
		JSONIFIER_INLINE uint64_t len241ToAnyRt(const char* value, size_t length) const {
			JSONIFIER_ALIGN constexpr mutable_constexpr_array acc{ jsonifier_internal::initAcc };
			jsonifier_internal::hashLongInternalLoopRt(acc.acc.data(), value, length, secret);
			static constexpr uint64_t SecretMergeAccsStart{ 11 };
			return jsonifier_internal::mergeAccsRt(acc.acc.data(), secret + SecretMergeAccsStart, length * jsonifier_internal::prime641);
		}

		/**
		 * @brief Hashes value longer than 240 bytes at compile-time.
		 *
		 * @param value The value data to be hashed.
		 * @param length The length of the value data.
		 * @return The hashed value.
		 */
		constexpr uint64_t len241ToAnyCt(const char* value, size_t length) const {
			JSONIFIER_ALIGN constexpr mutable_constexpr_array acc{ jsonifier_internal::initAcc };
			jsonifier_internal::hashLongInternalLoopCt(acc.acc.data(), value, length, secret);
			constexpr uint64_t SecretMergeAccsStart{ 11 };
			return jsonifier_internal::mergeAccsCt(acc.acc.data(), secret + SecretMergeAccsStart, length * jsonifier_internal::prime641);
		}
	};
}
