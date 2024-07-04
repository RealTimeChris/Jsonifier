/*
	MIT License

	Copyright (c) 2023 RealTimeChris

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
/// Feb 3, 2023
#pragma once

#include <jsonifier/ISA/Fallback.hpp>
#include <source_location>
#include <unordered_map>
#include <exception>
#include <string>
#include <array>
#include <bit>

namespace jsonifier_internal {

	JSONIFIER_ALIGN constexpr uint64_t xxhPrime641{ 0x9E3779B185EBCA87ull };
	JSONIFIER_ALIGN constexpr uint64_t xxhPrime642{ 0xC2B2AE3D27D4EB4Full };
	JSONIFIER_ALIGN constexpr uint64_t xxhPrime643{ 0x165667B19E3779F9ull };
	JSONIFIER_ALIGN constexpr uint64_t xxhPrime644{ 0x85EBCA77C2B2AE63ull };
	JSONIFIER_ALIGN constexpr uint64_t xxhPrime645{ 0x27D4EB2F165667C5ull };

	JSONIFIER_ALIGN constexpr uint64_t xxhPrime321{ 0x9E3779B1U };
	JSONIFIER_ALIGN constexpr uint64_t xxhPrime322{ 0x85EBCA77U };
	JSONIFIER_ALIGN constexpr uint64_t xxhPrime323{ 0xC2B2AE3DU };
	JSONIFIER_ALIGN constexpr uint64_t xxhPrime324{ 0x27D4EB2FU };
	JSONIFIER_ALIGN constexpr uint64_t xxhPrime325{ 0x165667B1U };

	JSONIFIER_ALIGN constexpr uint64_t xxhStripeLength{ 64 };
	JSONIFIER_ALIGN constexpr uint64_t xxhSecretConsumeRate{ 8 };
	JSONIFIER_ALIGN constexpr uint64_t xxhAccNb{ xxhStripeLength / sizeof(uint64_t) };
	JSONIFIER_ALIGN constexpr std::array<uint64_t, 8> xxhInitAcc{ xxhPrime323, xxhPrime641, xxhPrime642, xxhPrime643, xxhPrime644, xxhPrime322, xxhPrime645, xxhPrime321 };

	JSONIFIER_INLINE void accumulateAvxRt(uint64_t* acc, const char* input, const uint8_t* secret, uint64_t secretLength) {
		{
			simd_int_128* const xacc		  = ( simd_int_128* )acc;
			const simd_int_128* const xinput  = ( const simd_int_128* )input;
			const simd_int_128* const xsecret = ( const simd_int_128* )secret;
			for (size_t i = 0; i < xxhStripeLength / sizeof(simd_int_128); ++i) {
				const simd_int_128 data_vec	   = simd_internal::gatherValuesU<simd_int_128>(xinput + i);
				const simd_int_128 key_vec	   = simd_internal::gatherValuesU<simd_int_128>(xsecret + i);
				const simd_int_128 data_key	   = simd_internal::opXor(data_vec, key_vec);
				const simd_int_128 data_key_lo = simd_internal::opSrli<32>(data_key);
				const simd_int_128 product	   = simd_internal::opMul(data_key, data_key_lo);
				const simd_int_128 data_swap   = simd_internal::opShuffle(data_vec);
				simd_int_128 sum			   = simd_internal::opAdd(xacc[i], data_swap);
				sum							   = simd_internal::opAdd(data_key_lo, sum);
				xacc[i]						   = sum;
			}
		}
	}

	constexpr void accumulateAvxCt(uint64_t* acc, const char* input, const uint8_t* secret, uint64_t secretLength) {
		{
			for (size_t i = 0; i < xxhStripeLength; i += 16) {
				const __m128x data_vec		   = simd_internal::mm128LoadUSi128(input + i);
				const __m128x key_vec		   = simd_internal::mm128LoadUSi128(secret + i);
				const __m128x data_key		   = simd_internal::mm128XorSi128(data_vec, key_vec);
				const __m128x data_key_lo	   = simd_internal::mm128SrliEpi64(data_key, 32);
				const __m128x product		   = simd_internal::mm128MulEpi32(data_key, data_key_lo);
				const __m128x data_swap		   = simd_internal::mm128ShuffleEpi32(data_vec, mmShuffle(1, 0, 3, 2));
				const __m128x xacc			   = simd_internal::mm128LoadUSi128(acc + (i / 8));
				__m128x sum					   = simd_internal::mm128AddEpi64(xacc, data_swap);
				sum							   = simd_internal::mm128AddEpi64(data_key_lo, sum);
				simd_internal::mm128StoreUSi128(acc + (i / 8), sum);
			}
		}
	}

	JSONIFIER_INLINE void scrambleAvxRt(uint64_t* acc, const uint8_t* secret) {
		{
			simd_int_128* const xacc		  = ( simd_int_128* )acc;
			const simd_int_128* const xsecret = ( const simd_int_128* )secret;
			const simd_int_128 prime32		  = simd_internal::gatherValue<simd_int_128>(( int32_t )xxhPrime321);

			for (size_t i = 0; i < xxhStripeLength / sizeof(simd_int_128); ++i) {
				const simd_int_128 acc_vec	   = xacc[i];
				const simd_int_128 shifted	   = simd_internal::opSrli<47>(acc_vec);
				const simd_int_128 data_vec	   = simd_internal::opXor(acc_vec, shifted);
				const simd_int_128 key_vec	   = simd_internal::gatherValuesU<simd_int_128>(xsecret + i);
				const simd_int_128 data_key	   = simd_internal::opXor(data_vec, key_vec);
				const simd_int_128 data_key_hi = simd_internal::opSrli<32>(data_key);
				const simd_int_128 prod_lo	   = simd_internal::opMul(data_key, prime32);
				const simd_int_128 prod_hi	   = simd_internal::opMul(data_key_hi, prime32);
				xacc[i]						   = simd_internal::opAdd(prod_lo, simd_internal::opSlli<32>(prod_hi));
			}
		}
	}

	constexpr void scrambleAvxCt(uint64_t* acc, const uint8_t* secret) {
		{
			const __m128x prime32		 = simd_internal::mm128Set1Epi32(xxhPrime321);

			for (size_t i = 0; i < xxhStripeLength; i += 16) {
				const __m128x xacc			   = simd_internal::mm128LoadUSi128(acc + (i / 8));
				const __m128x acc_vec		   = xacc;
				const __m128x shifted		   = simd_internal::mm128SrliEpi64(acc_vec, 47);
				const __m128x data_vec		   = simd_internal::mm128XorSi128(acc_vec, shifted);
				const __m128x key_vec		   = simd_internal::mm128LoadUSi128(secret + i);
				const __m128x data_key		   = simd_internal::mm128XorSi128(data_vec, key_vec);
				const __m128x data_key_hi	   = simd_internal::mm128SrliEpi64(data_key, 32);
				const __m128x prod_lo		   = simd_internal::mm128MulEpi32(data_key, prime32);
				__m128x prod_hi				   = simd_internal::mm128MulEpi32(data_key_hi, prime32);
				prod_hi						   = simd_internal::mm128AddEpi64(prod_lo, simd_internal::mm128SlliEpi64(prod_hi, 32));
				simd_internal::mm128StoreUSi128(acc + (i / 8), prod_hi);
			}
		}
	}

	JSONIFIER_ALIGN constexpr uint64_t primeMx1{ 0x165667919E3779F9ull };
	JSONIFIER_ALIGN constexpr uint64_t primeMx2{ 0x9FB21C651E98DF25ULL };
	JSONIFIER_ALIGN constexpr uint64_t secretDefaultSize{ 192 };

	JSONIFIER_ALIGN constexpr uint8_t xxh3KSecret[secretDefaultSize]{ 0xb8u, 0xfeu, 0x6cu, 0x39u, 0x23u, 0xa4u, 0x4bu, 0xbeu, 0x7cu, 0x01u, 0x81u, 0x2cu, 0xf7u, 0x21u, 0xadu,
		0x1cu, 0xdeu, 0xd4u, 0x6du, 0xe9u, 0x83u, 0x90u, 0x97u, 0xdbu, 0x72u, 0x40u, 0xa4u, 0xa4u, 0xb7u, 0xb3u, 0x67u, 0x1fu, 0xcbu, 0x79u, 0xe6u, 0x4eu, 0xccu, 0xc0u, 0xe5u,
		0x78u, 0x82u, 0x5au, 0xd0u, 0x7du, 0xccu, 0xffu, 0x72u, 0x21u, 0xb8u, 0x08u, 0x46u, 0x74u, 0xf7u, 0x43u, 0x24u, 0x8eu, 0xe0u, 0x35u, 0x90u, 0xe6u, 0x81u, 0x3au, 0x26u,
		0x4cu, 0x3cu, 0x28u, 0x52u, 0xbbu, 0x91u, 0xc3u, 0x00u, 0xcbu, 0x88u, 0xd0u, 0x65u, 0x8bu, 0x1bu, 0x53u, 0x2eu, 0xa3u, 0x71u, 0x64u, 0x48u, 0x97u, 0xa2u, 0x0du, 0xf9u,
		0x4eu, 0x38u, 0x19u, 0xefu, 0x46u, 0xa9u, 0xdeu, 0xacu, 0xd8u, 0xa8u, 0xfau, 0x76u, 0x3fu, 0xe3u, 0x9cu, 0x34u, 0x3fu, 0xf9u, 0xdcu, 0xbbu, 0xc7u, 0xc7u, 0x0bu, 0x4fu,
		0x1du, 0x8au, 0x51u, 0xe0u, 0x4bu, 0xcdu, 0xb4u, 0x59u, 0x31u, 0xc8u, 0x9fu, 0x7eu, 0xc9u, 0xd9u, 0x78u, 0x73u, 0x64u, 0xeau, 0xc5u, 0xacu, 0x83u, 0x34u, 0xd3u, 0xebu,
		0xc3u, 0xc5u, 0x81u, 0xa0u, 0xffu, 0xfau, 0x13u, 0x63u, 0xebu, 0x17u, 0x0du, 0xddu, 0x51u, 0xb7u, 0xf0u, 0xdau, 0x49u, 0xd3u, 0x16u, 0x55u, 0x26u, 0x29u, 0xd4u, 0x68u,
		0x9eu, 0x2bu, 0x16u, 0xbeu, 0x58u, 0x7du, 0x47u, 0xa1u, 0xfcu, 0x8fu, 0xf8u, 0xb8u, 0xd1u, 0x7au, 0xd0u, 0x31u, 0xceu, 0x45u, 0xcbu, 0x3au, 0x8fu, 0x95u, 0x16u, 0x04u,
		0x28u, 0xafu, 0xd7u, 0xfbu, 0xcau, 0xbbu, 0x4bu, 0x40u, 0x7eu };

	JSONIFIER_INLINE constexpr uint64_t swap64Ct(uint64_t x) {
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

	JSONIFIER_INLINE uint64_t swap64Rt(uint64_t x) {
		return swap64Internal(x);
	}

	template<typename value_type, typename char_type> JSONIFIER_INLINE value_type readBitsRt(const char_type* ptr) {
		JSONIFIER_ALIGN value_type returnValue{};
		std::memcpy(&returnValue, ptr, sizeof(value_type));
		return returnValue;
	}

	template<typename value_type, typename char_type> constexpr value_type readBitsCt(const char_type* ptr) {
		JSONIFIER_ALIGN value_type returnValue{};
		for (uint64_t x = 0; x < sizeof(value_type); ++x) {
			returnValue |= static_cast<value_type>(static_cast<uint8_t>(ptr[x])) << (x * 8);
		}
		return returnValue;
	}

	template<uint64_t shiftNew> JSONIFIER_INLINE constexpr uint64_t xorShift64(uint64_t v64) {
		constexpr auto shift{ shiftNew };
		return v64 ^ (v64 >> shift);
	}

	JSONIFIER_INLINE constexpr uint64_t avalanche(uint64_t h64) {
		h64 = xorShift64<37>(h64);
		h64 *= primeMx1;
		return h64;
	}

	JSONIFIER_INLINE __m128x mult64To128Rt(uint64_t lhs, uint64_t rhs) {
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
		uint64_t productHigh;
		uint64_t const productLow = _umul128(lhs, rhs, &productHigh);
		__m128x r128;
		r128.m128x_uint64[0] = productLow;
		r128.m128x_uint64[1] = productHigh;
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
		__m128x product = mult64To128Rt(lhs, rhs);
		return product.m128x_uint64[0] ^ product.m128x_uint64[1];
	}

	constexpr uint64_t mul128Fold64Ct(uint64_t lhs, uint64_t rhs) {
		__m128x product = mult64To128Ct(lhs, rhs);
		return product.m128x_uint64[0] ^ product.m128x_uint64[1];
	}

	constexpr uint64_t fnvOffsetBasis{ 0xcbf29ce484222325ull };
	constexpr uint64_t fnvPrime{ 0x00000100000001B3ull };

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

	JSONIFIER_INLINE void hashLongInternalLoopRt(uint64_t* acc, const char* input, size_t len, const uint8_t* secret, uint64_t secretSize) {
		size_t const nbStripesPerBlock = (secretSize - xxhStripeLength) / xxhSecretConsumeRate;
		size_t const blockLength		   = xxhStripeLength * nbStripesPerBlock;
		size_t const nbBlocks		   = (len - 1) / blockLength;

		size_t n;

		for (n = 0; n < nbBlocks; n++) {
			accumulateAvxRt(acc, input + n * blockLength, secret, nbStripesPerBlock);
			scrambleAvxRt(acc, secret + secretSize - xxhStripeLength);
		}

		{
			size_t const nbStripes = ((len - 1) - (blockLength * nbBlocks)) / xxhStripeLength;
			accumulateAvxRt(acc, input + nbBlocks * blockLength, secret, nbStripes);

			{
				const char* const p = input + len - xxhStripeLength;
				accumulateAvxRt(acc, p, secret + secretSize - xxhStripeLength - 7, secretSize);
			}
		}
	}

	constexpr void hashLongInternalLoopCt(uint64_t* acc, const char* input, size_t len, const uint8_t* secret, uint64_t secretSize) {
		size_t const nbStripesPerBlock = (secretSize - xxhStripeLength) / xxhSecretConsumeRate;
		size_t const blockLength	   = xxhStripeLength * nbStripesPerBlock;
		size_t const nbBlocks		   = (len - 1) / blockLength;

		size_t n;

		for (n = 0; n < nbBlocks; n++) {
			accumulateAvxCt(acc, input + n * blockLength, secret, nbStripesPerBlock);
			scrambleAvxCt(acc, secret + secretSize - xxhStripeLength);
		}

		{
			size_t const nbStripes = ((len - 1) - (blockLength * nbBlocks)) / xxhStripeLength;
			accumulateAvxCt(acc, input + nbBlocks * blockLength, secret, nbStripes);

			{
				const char* const p = input + len - xxhStripeLength;
				accumulateAvxCt(acc, p, secret + secretSize - xxhStripeLength - 7, secretSize);
			}
		}
	}

	struct key_hasher {
		constexpr void setSeed(uint64_t seedNew) {
			initCustomSecretCt(seedNew);
		}

		constexpr operator uint64_t() const {
			return seed;
		}

		JSONIFIER_INLINE uint64_t hashKeyRt(const char* value, uint64_t length) const {
			if (length <= 8) {
				uint64_t hashValue{ seed };
				if (length == 8) {
					hashValue ^= readBitsRt<uint64_t>(value);
					hashValue *= fnvPrime;
					value += 8;
					length -= 8;
				}
				if (length >= 4) {
					hashValue ^= static_cast<uint64_t>(readBitsRt<uint32_t>(value));
					hashValue *= fnvPrime;
					value += 4;
					length -= 4;
				}
				if (length >= 2) {
					hashValue ^= static_cast<uint64_t>(readBitsRt<uint16_t>(value));
					hashValue *= fnvPrime;
					value += 2;
					length -= 2;
				}
				if (length) {
					hashValue ^= static_cast<uint64_t>(value[0]);
					hashValue *= fnvPrime;
				}
				return hashValue;
			} else if (length <= 16) {
				return len9To1664bRt(value, length);
			} else if (length <= 128) {
				return len17To12864bRt(value, length);
			} else {
				return len129ToAnyRt(value, length);
			}
		}

		constexpr uint64_t hashKeyCt(const char* value, uint64_t length) const {
			if (length <= 8) {
				uint64_t hashValue{ seed };
				if (length == 8) {
					hashValue ^= readBitsCt<uint64_t>(value);
					hashValue *= fnvPrime;
					value += 8;
					length -= 8;
				}
				if (length >= 4) {
					hashValue ^= static_cast<uint64_t>(readBitsCt<uint32_t>(value));
					hashValue *= fnvPrime;
					value += 4;
					length -= 4;
				}
				if (length >= 2) {
					hashValue ^= static_cast<uint64_t>(readBitsCt<uint16_t>(value));
					hashValue *= fnvPrime;
					value += 2;
					length -= 2;
				}
				if (length) {
					hashValue ^= static_cast<uint64_t>(value[0]);
					hashValue *= fnvPrime;
				}
				return hashValue;
			} else if (length <= 16) {
				return len9To1664bCt(value, length);
			} else if (length <= 128) {
				return len17To12864bCt(value, length);
			} else {
				return len129ToAnyCt(value, length);
			}
		}

	  protected:
		JSONIFIER_ALIGN uint8_t secret[secretDefaultSize]{};
		uint64_t seed{};

		constexpr void initCustomSecretCt(uint64_t seedNew) {
			seed					= seedNew;
			constexpr auto nbRounds = secretDefaultSize / sizeof(simd_int_128);
			const auto seedPos		= simd_internal::mm128Set1Epi64xCt(seed);

			const auto mask = simd_internal::mm128SetrEpi64xCt(0ull, std::numeric_limits<uint64_t>::max());

			const auto zeros = simd_internal::mm128SetZero();

			const auto negSeedPos = simd_internal::mm128SubEpi64(zeros, seedPos);
			const auto seedNewer  = simd_internal::mm128BlendVEpi8(seedPos, negSeedPos, mask);
			for (uint64_t i = 0; i < nbRounds; ++i) {
				auto newSource = simd_internal::mm128LoadUSi128(xxh3KSecret + (sizeof(__m128x) * i));
				auto newValue  = simd_internal::mm128AddEpi64(newSource, seedNewer);
				simd_internal::mm128StoreUSi128(secret + (sizeof(__m128x) * i), newValue);
			}
		}

		JSONIFIER_INLINE uint64_t len9To1664bRt(const char* input, size_t length) const {
			{
				uint64_t const bitflip2 = (readBitsRt<uint64_t>(secret + 40) ^ readBitsRt<uint64_t>(secret + 48)) - seed;
				uint64_t const inputLo	= readBitsRt<uint64_t>(input) ^ bitflip2;
				uint64_t const inputHi	= readBitsRt<uint64_t>(input + length - 8) ^ bitflip2;
				uint64_t const acc		= length + swap64Rt(inputLo) + inputHi + mul128Fold64Rt(inputLo, inputHi);
				return avalanche(acc);
			}
		}

		constexpr uint64_t len9To1664bCt(const char* input, size_t length) const {
			{
				uint64_t const bitflip2 = (readBitsCt<uint64_t>(secret + 40) ^ readBitsCt<uint64_t>(secret + 48)) - seed;
				uint64_t const inputLo	= readBitsCt<uint64_t>(input) ^ bitflip2;
				uint64_t const inputHi	= readBitsCt<uint64_t>(input + length - 8) ^ bitflip2;
				uint64_t const acc		= length + swap64Ct(inputLo) + inputHi + mul128Fold64Ct(inputLo, inputHi);
				return avalanche(acc);
			}
		}

		JSONIFIER_INLINE uint64_t mix16BRt(const char* input, const uint8_t* secretNew) const {
			{
				uint64_t const input_lo = readBitsRt<uint64_t>(input);
				uint64_t const input_hi = readBitsRt<uint64_t>(input + 8);
				return (input_lo ^ (readBitsRt<uint64_t>(secretNew) + seed)) ^ (input_hi ^ (readBitsRt<uint64_t>(secretNew + 8) - seed));
			}
		}

		constexpr uint64_t mix16BCt(const char* input, const uint8_t* secretNew) const {
			{
				uint64_t const input_lo = readBitsCt<uint64_t>(input);
				uint64_t const input_hi = readBitsCt<uint64_t>(input + 8);
				return (input_lo ^ (readBitsCt<uint64_t>(secretNew) + seed)) ^ (input_hi ^ (readBitsCt<uint64_t>(secretNew + 8) - seed));
			}
		}

		JSONIFIER_INLINE uint64_t len17To12864bRt(const char* input, size_t length) const {
			uint64_t acc = length * xxhPrime641;
			if (length > 32) {
				if (length > 64) {
					if (length > 96) {
						acc += mix16BRt(input + 48, secret + 96);
						acc += mix16BRt(input + length - 64, secret + 112);
					}
					acc += mix16BRt(input + 32, secret + 64);
					acc += mix16BRt(input + length - 48, secret + 80);
				}
				acc += mix16BRt(input + 16, secret + 32);
				acc += mix16BRt(input + length - 32, secret + 48);
			}
			acc += mix16BRt(input + 0, secret + 0);
			acc += mix16BRt(input + length - 16, secret + 16);
			return avalanche(acc);
		}

		constexpr uint64_t len17To12864bCt(const char* input, size_t length) const {
			uint64_t acc = length * xxhPrime641;
			if (length > 32) {
				if (length > 64) {
					if (length > 96) {
						acc += mix16BCt(input + 48, secret + 96);
						acc += mix16BCt(input + length - 64, secret + 112);
					}
					acc += mix16BCt(input + 32, secret + 64);
					acc += mix16BCt(input + length - 48, secret + 80);
				}
				acc += mix16BCt(input + 16, secret + 32);
				acc += mix16BCt(input + length - 32, secret + 48);
			}
			acc += mix16BCt(input + 0, secret + 0);
			acc += mix16BCt(input + length - 16, secret + 16);
			return avalanche(acc);
		}

		JSONIFIER_INLINE uint64_t len129ToAnyRt(const char* input, size_t len) const {
			JSONIFIER_ALIGN std::array<uint64_t, xxhAccNb> acc = xxhInitAcc;

			hashLongInternalLoopRt(acc.data(), input, len, secret, secretDefaultSize);
			return mergeAccsRt(acc.data(), secret + 11, len * xxhPrime641);
		}

		constexpr uint64_t len129ToAnyCt(const char* input, size_t len) const {
			JSONIFIER_ALIGN std::array<uint64_t, xxhAccNb> acc = xxhInitAcc;

			hashLongInternalLoopCt(acc.data(), input, len, secret, secretDefaultSize);
			return mergeAccsCt(acc.data(), secret + 11, len * xxhPrime641);
		}
	};
}