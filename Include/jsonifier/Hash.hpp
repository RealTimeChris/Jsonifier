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

#include <jsonifier/ISA/CTimeSimdTypes.hpp>
#include <source_location>
#include <unordered_map>
#include <exception>
#include <cstring>
#include <string>
#include <iostream>
#include <array>
#include <bit>

namespace jsonifier_internal {

	JSONIFIER_ALWAYS_INLINE constexpr size_t rotrCt(size_t x, size_t r) {
		return (((x) >> (r)) | ((x) << (64 - (r))));
	}

#if JSONIFIER_HAS_BUILTIN(__builtin_rotateright64)
	#define rotrRt __builtin_rotateright64
#elif defined(_MSC_VER)
	#define rotrRt(x, r) _rotr64(x, r)
#else
	#define rotrRt(x, r) rotrCt(x, r)
#endif

	struct xoshiro256 {
		size_t state[4]{};

		constexpr xoshiro256() noexcept {
			constexpr auto x   = 0x9E3779B185EBCA87ull >> 12ull;
			constexpr auto x01 = x ^ x << 25ull;
			constexpr auto x02 = x01 ^ x01 >> 27ull;
			size_t s		   = x02 * 0x2545F4914F6CDD1Dull;
			for (size_t y = 0; y < 4; ++y) {
				state[y] = splitmix64(s);
			}
		}

		constexpr size_t operator()() noexcept {
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
		constexpr size_t rotl(const size_t x, size_t k) const noexcept {
			return (x << k) | (x >> (64ull - k));
		}

		constexpr size_t splitmix64(size_t& seed64) const noexcept {
			size_t result = seed64 += 0x9E3779B97F4A7C15ull;
			result		  = (result ^ (result >> 30ull)) * 0xBF58476D1CE4E5B9ull;
			result		  = (result ^ (result >> 27ull)) * 0x94D049BB133111EBull;
			return result ^ (result >> 31ull);
		}
	};

	template<typename value_type, typename char_type> constexpr value_type readBitsCt(const char_type* ptr) {
		value_type returnValue{};
		for (size_t x = 0; x < sizeof(value_type); ++x) {
			returnValue |= static_cast<value_type>(static_cast<uint8_t>(ptr[x])) << (x * 8);
		}
		return returnValue;
	}

	struct key_hasher : public xoshiro256 {
		size_t seed{};///< Seed value for the hashing algorithm.
		/**
		 * @brief Default constructor that initializes the seed using a random_num value.
		 */
		constexpr key_hasher() {
			updateSeed();
		}

		/**
		 * @brief Sets the seed value at compile-time.
		 *
		 * @param seedNew The new seed value.
		 */
		JSONIFIER_ALWAYS_INLINE constexpr void updateSeed() {
			seed = xoshiro256::operator()();
		}

		JSONIFIER_ALWAYS_INLINE size_t mixBitsRt(size_t inputValue) const {
			inputValue *= seed;
			return inputValue ^ rotrRt(inputValue, 49);
		}

		JSONIFIER_ALWAYS_INLINE constexpr size_t mixBitsCt(size_t inputValue) const {
			inputValue *= seed;
			return inputValue ^ rotrCt(inputValue, 49);
		}

		/**
		 * @brief Hashes a key at runtime.
		 *
		 * @param value The value to be hashed.
		 * @param length The length of the value.
		 * @return The hashed value.
		 */
		JSONIFIER_ALWAYS_INLINE constexpr size_t hashKeyRt(const char* value, size_t length) const {
			size_t seed64{ seed };
			while (length >= 8) {
				std::memcpy(&returnValue64, value, 8);
				seed64 ^= returnValue64;
				value += 8;
				length -= 8;
			}

			if (length >= 4) {
				std::memcpy(&returnValue32, value, 4);
				seed64 ^= returnValue32;
				value += 4;
				length -= 4;
			}

			if (length >= 2) {
				std::memcpy(&returnValue16, value, 2);
				seed64 ^= returnValue16;
				value += 2;
				length -= 2;
			}

			if (length == 1) {
				seed64 ^= *value;
			}
			return seed64;
		}

		template<size_t length, typename char_type> JSONIFIER_ALWAYS_INLINE constexpr size_t hashKeyRt(const char_type* value) const {
			size_t seed64{ seed };
			constexpr size_t lengthNewer01{ length % 8 };
			if constexpr (length >= 8) {
				for (size_t lengthNew = length; lengthNew >= 8; lengthNew -= 8) {
					std::memcpy(&returnValue64, value, 8);
					seed64 ^= returnValue64;
					value += 8;
				}
			}

			constexpr size_t lengthNewer02{ lengthNewer01 >= 4 ? lengthNewer01 - 4 : lengthNewer01 };
			if constexpr (lengthNewer01 >= 4) {
				std::memcpy(&returnValue32, value, 4);
				seed64 ^= returnValue32;
				value += 4;
			}

			constexpr size_t lengthNewer03{ lengthNewer02 >= 2 ? lengthNewer02 - 2 : lengthNewer02 };
			if constexpr (lengthNewer02 >= 2) {
				std::memcpy(&returnValue16, value, 2);
				seed64 ^= returnValue16;
				value += 2;
			}

			if constexpr (lengthNewer03 == 1) {
				seed64 ^= *value;
			}
			return seed64;
		}

		/**
		 * @brief Hashes a key at compile-time.
		 *
		 * @param value The value to be hashed.
		 * @param length The length of the value.
		 * @return The hashed value.
		 */
		template<typename char_type> JSONIFIER_ALWAYS_INLINE constexpr size_t hashKeyCt(const char_type* value, size_t length) const {
			size_t seed64{ seed };
			while (length >= 8) {
				seed64 ^= readBitsCt<size_t>(value);
				value += 8;
				length -= 8;
			}

			if (length >= 4) {
				seed64 ^= readBitsCt<uint32_t>(value);
				value += 4;
				length -= 4;
			}

			if (length >= 2) {
				seed64 ^= readBitsCt<uint16_t>(value);
				value += 2;
				length -= 2;
			}

			if (length == 1) {
				seed64 ^= *value;
			}
			return seed64;
		}

		template<typename char_type> JSONIFIER_ALWAYS_INLINE constexpr size_t hashKeyCt02(const char_type* value, size_t length) const {
			size_t seed64{ seed };
			size_t lengthNewer01{ length % 8 };
			if (length >= 8) {
				for (size_t lengthNew = length; lengthNew >= 8; lengthNew -= 8) {
					seed64 ^= readBitsCt<size_t>(value);
					value += 8;
				}
			}

			size_t lengthNewer02{ lengthNewer01 >= 4 ? lengthNewer01 - 4 : lengthNewer01 };
			if (lengthNewer01 >= 4) {
				seed64 ^= readBitsCt<uint32_t>(value);
				value += 4;
			}

			size_t lengthNewer03{ lengthNewer02 >= 2 ? lengthNewer02 - 2 : lengthNewer02 };
			if (lengthNewer02 >= 2) {
				seed64 ^= readBitsCt<uint16_t>(value);
				value += 2;
			}

			if (lengthNewer03 == 1) {
				seed64 ^= *value;
			}
			return seed64;
		}

	  protected:
		mutable uint64_t returnValue64{};
		mutable uint32_t returnValue32{};
		mutable uint16_t returnValue16{};
	};


}