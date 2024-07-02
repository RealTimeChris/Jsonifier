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

#include <source_location>
#include <unordered_map>
#include <exception>
#include <string>
#include <array>
#include <bit>

namespace jsonifier_internal {

	struct __m128x {
		uint64_t values[2]{};
	};

	constexpr __m128x set1Epi64(uint64_t value) {
		__m128x returnValue{};
		returnValue.values[0] = value;
		returnValue.values[1] = value;
		return returnValue;
	}

	constexpr __m128x setREpi64(uint64_t value01, uint64_t value02) {
		__m128x returnValue{};
		returnValue.values[0] = value02;
		returnValue.values[1] = value01;
		return returnValue;
	}

	constexpr __m128x setZero() {
		return {};
	}

	constexpr __m128x blendvEpi8(const __m128x& a, const __m128x& b, const __m128x& mask) {
		__m128x result;
		for (int32_t i = 0; i < 2; ++i) {
			result.values[i] = 0;
			for (int32_t j = 0; j < 8; ++j) {
				uint8_t maskByte	= (mask.values[1 - i] >> (j * 8)) & 0xFF;
				uint8_t aByte		= (a.values[1 - i] >> (j * 8)) & 0xFF;
				uint8_t bByte		= (b.values[1 - i] >> (j * 8)) & 0xFF;
				uint8_t blendedByte = (maskByte ? bByte : aByte);
				result.values[i] |= (static_cast<uint64_t>(blendedByte) << (j * 8));
			}
		}
		return result;
	}

	constexpr __m128x addEpi64(const __m128x& value01, const __m128x& value02) {
		__m128x returnValue{};
		returnValue.values[0] = value01.values[0] + value02.values[0];
		returnValue.values[1] = value01.values[1] + value02.values[1];
		return returnValue;
	}

	constexpr __m128x loaduSi128(const uint8_t* ptr) {
		uint64_t low  = 0;
		uint64_t high = 0;

		for (int32_t i = 0; i < 8; ++i) {
			low |= static_cast<uint64_t>(ptr[i]) << (i * 8);
		}

		for (int32_t i = 0; i < 8; ++i) {
			high |= static_cast<uint64_t>(ptr[i + 8]) << (i * 8);
		}

		return __m128x{ low, high };
	}

	constexpr void storeuSi128(uint8_t* ptr, const __m128x& data) {
		for (int32_t i = 0; i < 8; ++i) {
			ptr[i] = static_cast<uint8_t>(data.values[0] >> (i * 8));
		}

		for (int32_t i = 0; i < 8; ++i) {
			ptr[i + 8] = static_cast<uint8_t>(data.values[1] >> (i * 8));
		}
	}

	constexpr __m128x subEpi64(const __m128x& value01, const __m128x& value02) {
		__m128x returnValue{};
		returnValue.values[0] = value01.values[0] - value02.values[0];
		returnValue.values[1] = value01.values[1] - value02.values[1];
		return returnValue;
	}

	constexpr uint64_t xxhPrime641{ 0x9E3779B185EBCA87ull };
	constexpr uint64_t xxhPrime642{ 0xC2B2AE3D27D4EB4FULL };
	constexpr uint64_t primeMx1{ 0x165667919E3779F9ull };
	constexpr uint64_t primeMx2{ 0x9FB21C651E98DF25ULL };
	constexpr uint64_t secretDefaultSize{ 192 };

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

	JSONIFIER_INLINE constexpr uint64_t xorShift64(uint64_t v64, int32_t shift) {
		return v64 ^ (v64 >> shift);
	}

	JSONIFIER_INLINE constexpr uint64_t avalanche(uint64_t h64) {
		h64 = xorShift64(h64, 37);
		h64 *= primeMx1;
		h64 = xorShift64(h64, 32);
		return h64;
	}

	JSONIFIER_INLINE __m128x mult64To128Rt(uint64_t lhs, uint64_t rhs) {
#if (defined(__GNUC__) || defined(__clang__)) && !defined(__wasm__) && defined(__SIZEOF_INT128__) || (defined(_INTEGRAL_MAX_BITS) && _INTEGRAL_MAX_BITS >= 128)
		__uint128_t const product = ( __uint128_t )lhs * ( __uint128_t )rhs;
		__m128x r128;
		r128.values[0] = ( uint64_t )(product);
		r128.values[1] = ( uint64_t )(product >> 64);
		return r128;
#elif (defined(_M_X64) || defined(_M_IA64)) && !defined(_M_ARM64EC)

	#if !defined(JSONIFIER_MSVC)
		#pragma intrinsic(_umul128)
	#endif
		uint64_t productHigh;
		uint64_t const productLow = _umul128(lhs, rhs, &productHigh);
		__m128x r128;
		r128.values[0] = productLow;
		r128.values[1] = productHigh;
		return r128;
#elif defined(_M_ARM64) || defined(_M_ARM64EC)

	#if !defined(JSONIFIER_MSVC)
		#pragma intrinsic(__umulh)
	#endif
		__m128x r128;
		r128.values[0] = lhs * rhs;
		r128.values[1] = __umulh(lhs, rhs);
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
		r128.values[0] = lower;
		r128.values[1] = upper;
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
		r128.values[0] = lower;
		r128.values[1] = upper;
		return r128;
	}

	JSONIFIER_INLINE uint64_t mul128Fold64Rt(uint64_t lhs, uint64_t rhs) {
		__m128x product = mult64To128Rt(lhs, rhs);
		return product.values[0] ^ product.values[1];
	}

	constexpr uint64_t mul128Fold64Ct(uint64_t lhs, uint64_t rhs) {
		__m128x product = mult64To128Ct(lhs, rhs);
		return product.values[0] ^ product.values[1];
	}

	constexpr uint64_t fnvOffsetBasis{ 0xcbf29ce484222325ull };
	constexpr uint64_t fnvPrime{ 0x00000100000001B3ull };

	struct key_hasher {
		constexpr void setSeed(uint64_t seedNew) {
			seed = seedNew;
			initCustomSecretCt();
		}

		constexpr operator uint64_t() const {
			return seed;
		}

		JSONIFIER_INLINE uint64_t hashKeyRtSingle(const char* value, uint64_t length) const {
			uint64_t hashValue{ seed };
			if (length) {
				hashValue ^= static_cast<uint64_t>(value[0]);
				hashValue *= fnvPrime;
			}
			return hashValue;
		}

		constexpr uint64_t hashKeyCtSingle(const char* value, uint64_t length) const {
			uint64_t hashValue{ seed };
			if (length) {
				hashValue ^= static_cast<uint64_t>(value[0]);
				hashValue *= fnvPrime;
			}
			return hashValue;
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

		constexpr void initCustomSecretCt() {
			constexpr auto nbRounds = secretDefaultSize / sizeof(simd_int_128);
			const auto seedPos		= set1Epi64(seed);

			const auto mask = setREpi64(0ull, std::numeric_limits<uint64_t>::max());

			const auto zeros = setZero();

			const auto negSeedPos = subEpi64(zeros, seedPos);
			const auto seedNew	  = blendvEpi8(seedPos, negSeedPos, mask);
			for (uint64_t i = 0; i < nbRounds; ++i) {
				auto newSource = loaduSi128(xxh3KSecret + (sizeof(__m128x) * i));
				auto newValue  = addEpi64(newSource, seedNew);
				storeuSi128(secret + (sizeof(__m128x) * i), newValue);
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

		JSONIFIER_INLINE uint64_t len129ToAnyRt(const char* input, size_t length) const {
			uint64_t acc = length * xxhPrime641;
			uint64_t nBlocks{ (length - 128) / 16 };
			for (uint64_t x = 0; x < nBlocks; ++x) {
				acc += mix16BRt(input + (x * 16), secret + ((x * 16) % secretDefaultSize));
				acc += mix16BRt(input + length - (x * 16), secret + ((x * 16) % secretDefaultSize));
				length -= 16;
				input += 16;
			}
			return avalanche(acc) ^ len17To12864bRt(input, length);
		}

		constexpr uint64_t len129ToAnyCt(const char* input, size_t length) const {
			uint64_t acc = length * xxhPrime641;
			uint64_t nBlocks{ (length - 128) / 16 };
			for (uint64_t x = 0; x < nBlocks; ++x) {
				acc += mix16BCt(input + (x * 16), secret + ((x * 16) % secretDefaultSize));
				acc += mix16BCt(input + length - (x * 16), secret + ((x * 16) % secretDefaultSize));
				length -= 16;
				input += 16;
			}
			return avalanche(acc) ^ len17To12864bCt(input, length);
		}
	};
}