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

	struct ct_key_hasher : public xoshiro256 {
		size_t seed{};///< seed value for the hashing algorithm.
		/**
		 * @brief Default constructor that initializes the seed using a random_num value.
		 */
		constexpr ct_key_hasher() {
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

		/**
		 * @brief Hashes a key at compile-time.
		 *
		 * @param value The value to be hashed.
		 * @param length The length of the value.
		 * @return The hashed value.
		 */
		template<typename char_type> JSONIFIER_ALWAYS_INLINE constexpr size_t hashKeyCt(const char_type* value, size_t length) const noexcept {
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
	};

	template<size_t seed> struct rt_key_hasher {
		/**
		 * @brief Hashes a key at runtime.
		 *
		 * @param value The value to be hashed.
		 * @param length The length of the value.
		 * @return The hashed value.
		 */
		JSONIFIER_ALWAYS_INLINE constexpr size_t hashKeyRt(const char* value, size_t length) const noexcept {
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

	  protected:
		mutable size_t returnValue64{};
		mutable uint32_t returnValue32{};
		mutable uint16_t returnValue16{};
	};

	template<size_t length, size_t seed> struct bit_mixer;

	template<> struct bit_mixer<0, 0> {
		template<typename char_type> JSONIFIER_ALWAYS_INLINE static constexpr size_t mixBits(char_type* str, size_t seed, size_t length) {
			size_t result = seed;
			switch (length) {
				case 1:
					return result ^ (static_cast<size_t>(str[0]));
				case 2:
					return result ^ (static_cast<size_t>(str[0]) | (static_cast<size_t>(str[1]) << 8));
				case 3:
					return result ^ (static_cast<size_t>(str[0]) | (static_cast<size_t>(str[1]) << 8) | (static_cast<size_t>(str[2]) << 16));
				case 4:
					return result ^ (static_cast<size_t>(str[0]) | (static_cast<size_t>(str[1]) << 8) | (static_cast<size_t>(str[2]) << 16) | (static_cast<size_t>(str[3]) << 24));
				case 5:
					return result ^
						(static_cast<size_t>(str[0]) | (static_cast<size_t>(str[1]) << 8) | (static_cast<size_t>(str[2]) << 16) | (static_cast<size_t>(str[3]) << 24) |
							(static_cast<size_t>(str[4]) << 32));
				case 6:
					return result ^
						(static_cast<size_t>(str[0]) | (static_cast<size_t>(str[1]) << 8) | (static_cast<size_t>(str[2]) << 16) | (static_cast<size_t>(str[3]) << 24) |
							(static_cast<size_t>(str[4]) << 32) | (static_cast<size_t>(str[5]) << 40));
				case 7:
					return result ^
						(static_cast<size_t>(str[0]) | (static_cast<size_t>(str[1]) << 8) | (static_cast<size_t>(str[2]) << 16) | (static_cast<size_t>(str[3]) << 24) |
							(static_cast<size_t>(str[4]) << 32) | (static_cast<size_t>(str[5]) << 40) | (static_cast<size_t>(str[6]) << 48));
				case 8:
					return result ^
						(static_cast<size_t>(str[0]) | (static_cast<size_t>(str[1]) << 8) | (static_cast<size_t>(str[2]) << 16) | (static_cast<size_t>(str[3]) << 24) |
							(static_cast<size_t>(str[4]) << 32) | (static_cast<size_t>(str[5]) << 40) | (static_cast<size_t>(str[6]) << 48) | (static_cast<size_t>(str[7]) << 56));
				case 9:
					return result ^
						(static_cast<size_t>(str[0]) | (static_cast<size_t>(str[1]) << 8) | (static_cast<size_t>(str[2]) << 16) | (static_cast<size_t>(str[3]) << 24) |
							(static_cast<size_t>(str[4]) << 32) | (static_cast<size_t>(str[5]) << 40) | (static_cast<size_t>(str[6]) << 48) | (static_cast<size_t>(str[7]) << 56) |
							(static_cast<size_t>(str[8])));
				case 10:
					return result ^
						(static_cast<size_t>(str[0]) | (static_cast<size_t>(str[1]) << 8) | (static_cast<size_t>(str[2]) << 16) | (static_cast<size_t>(str[3]) << 24) |
							(static_cast<size_t>(str[4]) << 32) | (static_cast<size_t>(str[5]) << 40) | (static_cast<size_t>(str[6]) << 48) | (static_cast<size_t>(str[7]) << 56) |
							((static_cast<size_t>(str[8]) | (static_cast<size_t>(str[9]) << 8))));
				case 11:
					return result ^
						(static_cast<size_t>(str[0]) | (static_cast<size_t>(str[1]) << 8) | (static_cast<size_t>(str[2]) << 16) | (static_cast<size_t>(str[3]) << 24) |
							(static_cast<size_t>(str[4]) << 32) | (static_cast<size_t>(str[5]) << 40) | (static_cast<size_t>(str[6]) << 48) | (static_cast<size_t>(str[7]) << 56) |
							(static_cast<size_t>(str[8]) | (static_cast<size_t>(str[9]) << 8) | (static_cast<size_t>(str[10]) << 16)));
				case 12:
					return result ^
						(static_cast<size_t>(str[0]) | (static_cast<size_t>(str[1]) << 8) | (static_cast<size_t>(str[2]) << 16) | (static_cast<size_t>(str[3]) << 24) |
							(static_cast<size_t>(str[4]) << 32) | (static_cast<size_t>(str[5]) << 40) | (static_cast<size_t>(str[6]) << 48) | (static_cast<size_t>(str[7]) << 56) |
							(static_cast<size_t>(str[8]) | (static_cast<size_t>(str[9]) << 8) | (static_cast<size_t>(str[10]) << 16) | (static_cast<size_t>(str[11]) << 24)));
				case 13:
					return result ^
						(static_cast<size_t>(str[0]) | (static_cast<size_t>(str[1]) << 8) | (static_cast<size_t>(str[2]) << 16) | (static_cast<size_t>(str[3]) << 24) |
							(static_cast<size_t>(str[4]) << 32) | (static_cast<size_t>(str[5]) << 40) | (static_cast<size_t>(str[6]) << 48) | (static_cast<size_t>(str[7]) << 56) |
							(static_cast<size_t>(str[8]) | (static_cast<size_t>(str[9]) << 8) | (static_cast<size_t>(str[10]) << 16) | (static_cast<size_t>(str[11]) << 24) |
								(static_cast<size_t>(str[12]) << 32)));
				case 14:
					return result ^
						(static_cast<size_t>(str[0]) | (static_cast<size_t>(str[1]) << 8) | (static_cast<size_t>(str[2]) << 16) | (static_cast<size_t>(str[3]) << 24) |
							(static_cast<size_t>(str[4]) << 32) | (static_cast<size_t>(str[5]) << 40) | (static_cast<size_t>(str[6]) << 48) | (static_cast<size_t>(str[7]) << 56) |
							(static_cast<size_t>(str[8]) | (static_cast<size_t>(str[9]) << 8) | (static_cast<size_t>(str[10]) << 16) | (static_cast<size_t>(str[11]) << 24) |
								(static_cast<size_t>(str[12]) << 32) | (static_cast<size_t>(str[13]) << 40)));
				case 15:
					return result ^
						(static_cast<size_t>(str[0]) | (static_cast<size_t>(str[1]) << 8) | (static_cast<size_t>(str[2]) << 16) | (static_cast<size_t>(str[3]) << 24) |
							(static_cast<size_t>(str[4]) << 32) | (static_cast<size_t>(str[5]) << 40) | (static_cast<size_t>(str[6]) << 48) | (static_cast<size_t>(str[7]) << 56) |
							(static_cast<size_t>(str[8]) | (static_cast<size_t>(str[9]) << 8) | (static_cast<size_t>(str[10]) << 16) | (static_cast<size_t>(str[11]) << 24) |
								(static_cast<size_t>(str[12]) << 32) | (static_cast<size_t>(str[13]) << 40) | (static_cast<size_t>(str[14]) << 48)));
				case 16:
					return result ^
						(static_cast<size_t>(str[0]) | (static_cast<size_t>(str[1]) << 8) | (static_cast<size_t>(str[2]) << 16) | (static_cast<size_t>(str[3]) << 24) |
							(static_cast<size_t>(str[4]) << 32) | (static_cast<size_t>(str[5]) << 40) | (static_cast<size_t>(str[6]) << 48) | (static_cast<size_t>(str[7]) << 56) |
							(static_cast<size_t>(str[8]) | (static_cast<size_t>(str[9]) << 8) | (static_cast<size_t>(str[10]) << 16) | (static_cast<size_t>(str[11]) << 24) |
								(static_cast<size_t>(str[12]) << 32) | (static_cast<size_t>(str[13]) << 40) | (static_cast<size_t>(str[14]) << 48) |
								(static_cast<size_t>(str[15]) << 56)));
				case 17:
					return result ^
						(static_cast<size_t>(str[0]) | (static_cast<size_t>(str[1]) << 8) | (static_cast<size_t>(str[2]) << 16) | (static_cast<size_t>(str[3]) << 24) |
							(static_cast<size_t>(str[4]) << 32) | (static_cast<size_t>(str[5]) << 40) | (static_cast<size_t>(str[6]) << 48) | (static_cast<size_t>(str[7]) << 56) |
							(static_cast<size_t>(str[8]) | (static_cast<size_t>(str[9]) << 8) | (static_cast<size_t>(str[10]) << 16) | (static_cast<size_t>(str[11]) << 24) |
								(static_cast<size_t>(str[12]) << 32) | (static_cast<size_t>(str[13]) << 40) | (static_cast<size_t>(str[14]) << 48) |
								(static_cast<size_t>(str[15]) << 56) | (static_cast<size_t>(str[16]))));
				case 18:
					return result ^
						(static_cast<size_t>(str[0]) | (static_cast<size_t>(str[1]) << 8) | (static_cast<size_t>(str[2]) << 16) | (static_cast<size_t>(str[3]) << 24) |
							(static_cast<size_t>(str[4]) << 32) | (static_cast<size_t>(str[5]) << 40) | (static_cast<size_t>(str[6]) << 48) | (static_cast<size_t>(str[7]) << 56) |
							(static_cast<size_t>(str[8]) | (static_cast<size_t>(str[9]) << 8) | (static_cast<size_t>(str[10]) << 16) | (static_cast<size_t>(str[11]) << 24) |
								(static_cast<size_t>(str[12]) << 32) | (static_cast<size_t>(str[13]) << 40) | (static_cast<size_t>(str[14]) << 48) |
								(static_cast<size_t>(str[15]) << 56) | (static_cast<size_t>(str[16])) | (static_cast<size_t>(str[17]) << 8)));
				case 19:
					return result ^
						(static_cast<size_t>(str[0]) | (static_cast<size_t>(str[1]) << 8) | (static_cast<size_t>(str[2]) << 16) | (static_cast<size_t>(str[3]) << 24) |
							(static_cast<size_t>(str[4]) << 32) | (static_cast<size_t>(str[5]) << 40) | (static_cast<size_t>(str[6]) << 48) | (static_cast<size_t>(str[7]) << 56) |
							(static_cast<size_t>(str[8]) | (static_cast<size_t>(str[9]) << 8) | (static_cast<size_t>(str[10]) << 16) | (static_cast<size_t>(str[11]) << 24) |
								(static_cast<size_t>(str[12]) << 32) | (static_cast<size_t>(str[13]) << 40) | (static_cast<size_t>(str[14]) << 48) |
								(static_cast<size_t>(str[15]) << 56) | (static_cast<size_t>(str[16])) | (static_cast<size_t>(str[17]) << 8) |
								(static_cast<size_t>(str[18]) << 16)));
				case 20:
					return result ^
						(static_cast<size_t>(str[0]) | (static_cast<size_t>(str[1]) << 8) | (static_cast<size_t>(str[2]) << 16) | (static_cast<size_t>(str[3]) << 24) |
							(static_cast<size_t>(str[4]) << 32) | (static_cast<size_t>(str[5]) << 40) | (static_cast<size_t>(str[6]) << 48) | (static_cast<size_t>(str[7]) << 56) |
							(static_cast<size_t>(str[8]) | (static_cast<size_t>(str[9]) << 8) | (static_cast<size_t>(str[10]) << 16) | (static_cast<size_t>(str[11]) << 24) |
								(static_cast<size_t>(str[12]) << 32) | (static_cast<size_t>(str[13]) << 40) | (static_cast<size_t>(str[14]) << 48) |
								(static_cast<size_t>(str[15]) << 56) | (static_cast<size_t>(str[16])) | (static_cast<size_t>(str[17]) << 8) | (static_cast<size_t>(str[18]) << 16) |
								(static_cast<size_t>(str[19]) << 24)));
				case 21:
					return result ^
						(static_cast<size_t>(str[0]) | (static_cast<size_t>(str[1]) << 8) | (static_cast<size_t>(str[2]) << 16) | (static_cast<size_t>(str[3]) << 24) |
							(static_cast<size_t>(str[4]) << 32) | (static_cast<size_t>(str[5]) << 40) | (static_cast<size_t>(str[6]) << 48) | (static_cast<size_t>(str[7]) << 56) |
							(static_cast<size_t>(str[8]) | (static_cast<size_t>(str[9]) << 8) | (static_cast<size_t>(str[10]) << 16) | (static_cast<size_t>(str[11]) << 24) |
								(static_cast<size_t>(str[12]) << 32) | (static_cast<size_t>(str[13]) << 40) | (static_cast<size_t>(str[14]) << 48) |
								(static_cast<size_t>(str[15]) << 56) | (static_cast<size_t>(str[16])) | (static_cast<size_t>(str[17]) << 8) | (static_cast<size_t>(str[18]) << 16) |
								(static_cast<size_t>(str[19]) << 24) | (static_cast<size_t>(str[20]) << 32)));
				case 22:
					return result ^
						(static_cast<size_t>(str[0]) | (static_cast<size_t>(str[1]) << 8) | (static_cast<size_t>(str[2]) << 16) | (static_cast<size_t>(str[3]) << 24) |
							(static_cast<size_t>(str[4]) << 32) | (static_cast<size_t>(str[5]) << 40) | (static_cast<size_t>(str[6]) << 48) | (static_cast<size_t>(str[7]) << 56) |
							(static_cast<size_t>(str[8]) | (static_cast<size_t>(str[9]) << 8) | (static_cast<size_t>(str[10]) << 16) | (static_cast<size_t>(str[11]) << 24) |
								(static_cast<size_t>(str[12]) << 32) | (static_cast<size_t>(str[13]) << 40) | (static_cast<size_t>(str[14]) << 48) |
								(static_cast<size_t>(str[15]) << 56) | (static_cast<size_t>(str[16])) | (static_cast<size_t>(str[17]) << 8) | (static_cast<size_t>(str[18]) << 16) |
								(static_cast<size_t>(str[19]) << 24) | (static_cast<size_t>(str[20]) << 32) | (static_cast<size_t>(str[21]) << 40)));
				case 23:
					return result ^
						(static_cast<size_t>(str[0]) | (static_cast<size_t>(str[1]) << 8) | (static_cast<size_t>(str[2]) << 16) | (static_cast<size_t>(str[3]) << 24) |
							(static_cast<size_t>(str[4]) << 32) | (static_cast<size_t>(str[5]) << 40) | (static_cast<size_t>(str[6]) << 48) | (static_cast<size_t>(str[7]) << 56) |
							(static_cast<size_t>(str[8]) | (static_cast<size_t>(str[9]) << 8) | (static_cast<size_t>(str[10]) << 16) | (static_cast<size_t>(str[11]) << 24) |
								(static_cast<size_t>(str[12]) << 32) | (static_cast<size_t>(str[13]) << 40) | (static_cast<size_t>(str[14]) << 48) |
								(static_cast<size_t>(str[15]) << 56) | (static_cast<size_t>(str[16])) | (static_cast<size_t>(str[17]) << 8) | (static_cast<size_t>(str[18]) << 16) |
								(static_cast<size_t>(str[19]) << 24) | (static_cast<size_t>(str[20]) << 32) | (static_cast<size_t>(str[21]) << 40) |
								(static_cast<size_t>(str[22]) << 48)));
				case 24:
					return result ^
						(static_cast<size_t>(str[0]) | (static_cast<size_t>(str[1]) << 8) | (static_cast<size_t>(str[2]) << 16) | (static_cast<size_t>(str[3]) << 24) |
							(static_cast<size_t>(str[4]) << 32) | (static_cast<size_t>(str[5]) << 40) | (static_cast<size_t>(str[6]) << 48) | (static_cast<size_t>(str[7]) << 56) |
							(static_cast<size_t>(str[8]) | (static_cast<size_t>(str[9]) << 8) | (static_cast<size_t>(str[10]) << 16) | (static_cast<size_t>(str[11]) << 24) |
								(static_cast<size_t>(str[12]) << 32) | (static_cast<size_t>(str[13]) << 40) | (static_cast<size_t>(str[14]) << 48) |
								(static_cast<size_t>(str[15]) << 56) | (static_cast<size_t>(str[16])) | (static_cast<size_t>(str[17]) << 8) | (static_cast<size_t>(str[18]) << 16) |
								(static_cast<size_t>(str[19]) << 24) | (static_cast<size_t>(str[20]) << 32) | (static_cast<size_t>(str[21]) << 40) |
								(static_cast<size_t>(str[22]) << 48) | (static_cast<size_t>(str[23]) << 56)));
				case 25:
					return result ^
						(static_cast<size_t>(str[0]) | (static_cast<size_t>(str[1]) << 8) | (static_cast<size_t>(str[2]) << 16) | (static_cast<size_t>(str[3]) << 24) |
							(static_cast<size_t>(str[4]) << 32) | (static_cast<size_t>(str[5]) << 40) | (static_cast<size_t>(str[6]) << 48) | (static_cast<size_t>(str[7]) << 56) |
							(static_cast<size_t>(str[8]) | (static_cast<size_t>(str[9]) << 8) | (static_cast<size_t>(str[10]) << 16) | (static_cast<size_t>(str[11]) << 24) |
								(static_cast<size_t>(str[12]) << 32) | (static_cast<size_t>(str[13]) << 40) | (static_cast<size_t>(str[14]) << 48) |
								(static_cast<size_t>(str[15]) << 56) | (static_cast<size_t>(str[16])) | (static_cast<size_t>(str[17]) << 8) | (static_cast<size_t>(str[18]) << 16) |
								(static_cast<size_t>(str[19]) << 24) | (static_cast<size_t>(str[20]) << 32) | (static_cast<size_t>(str[21]) << 40) |
								(static_cast<size_t>(str[22]) << 48) | (static_cast<size_t>(str[23]) << 56) | (static_cast<size_t>(str[24]))));
				case 26:
					return result ^
						(static_cast<size_t>(str[0]) | (static_cast<size_t>(str[1]) << 8) | (static_cast<size_t>(str[2]) << 16) | (static_cast<size_t>(str[3]) << 24) |
							(static_cast<size_t>(str[4]) << 32) | (static_cast<size_t>(str[5]) << 40) | (static_cast<size_t>(str[6]) << 48) | (static_cast<size_t>(str[7]) << 56) |
							(static_cast<size_t>(str[8]) | (static_cast<size_t>(str[9]) << 8) | (static_cast<size_t>(str[10]) << 16) | (static_cast<size_t>(str[11]) << 24) |
								(static_cast<size_t>(str[12]) << 32) | (static_cast<size_t>(str[13]) << 40) | (static_cast<size_t>(str[14]) << 48) |
								(static_cast<size_t>(str[15]) << 56) | (static_cast<size_t>(str[16])) | (static_cast<size_t>(str[17]) << 8) | (static_cast<size_t>(str[18]) << 16) |
								(static_cast<size_t>(str[19]) << 24) | (static_cast<size_t>(str[20]) << 32) | (static_cast<size_t>(str[21]) << 40) |
								(static_cast<size_t>(str[22]) << 48) | (static_cast<size_t>(str[23]) << 56) |
								(static_cast<size_t>(str[24]) | (static_cast<size_t>(str[25]) << 8))));
				case 27:
					return result ^
						(static_cast<size_t>(str[0]) | (static_cast<size_t>(str[1]) << 8) | (static_cast<size_t>(str[2]) << 16) | (static_cast<size_t>(str[3]) << 24) |
							(static_cast<size_t>(str[4]) << 32) | (static_cast<size_t>(str[5]) << 40) | (static_cast<size_t>(str[6]) << 48) | (static_cast<size_t>(str[7]) << 56) |
							(static_cast<size_t>(str[8]) | (static_cast<size_t>(str[9]) << 8) | (static_cast<size_t>(str[10]) << 16) | (static_cast<size_t>(str[11]) << 24) |
								(static_cast<size_t>(str[12]) << 32) | (static_cast<size_t>(str[13]) << 40) | (static_cast<size_t>(str[14]) << 48) |
								(static_cast<size_t>(str[15]) << 56) | (static_cast<size_t>(str[16])) | (static_cast<size_t>(str[17]) << 8) | (static_cast<size_t>(str[18]) << 16) |
								(static_cast<size_t>(str[19]) << 24) | (static_cast<size_t>(str[20]) << 32) | (static_cast<size_t>(str[21]) << 40) |
								(static_cast<size_t>(str[22]) << 48) | (static_cast<size_t>(str[23]) << 56) |
								(static_cast<size_t>(str[24]) | (static_cast<size_t>(str[25]) << 8) | (static_cast<size_t>(str[26]) << 16))));
				case 28:
					return result ^
						(static_cast<size_t>(str[0]) | (static_cast<size_t>(str[1]) << 8) | (static_cast<size_t>(str[2]) << 16) | (static_cast<size_t>(str[3]) << 24) |
							(static_cast<size_t>(str[4]) << 32) | (static_cast<size_t>(str[5]) << 40) | (static_cast<size_t>(str[6]) << 48) | (static_cast<size_t>(str[7]) << 56) |
							(static_cast<size_t>(str[8]) | (static_cast<size_t>(str[9]) << 8) | (static_cast<size_t>(str[10]) << 16) | (static_cast<size_t>(str[11]) << 24) |
								(static_cast<size_t>(str[12]) << 32) | (static_cast<size_t>(str[13]) << 40) | (static_cast<size_t>(str[14]) << 48) |
								(static_cast<size_t>(str[15]) << 56) | (static_cast<size_t>(str[16])) | (static_cast<size_t>(str[17]) << 8) | (static_cast<size_t>(str[18]) << 16) |
								(static_cast<size_t>(str[19]) << 24) | (static_cast<size_t>(str[20]) << 32) | (static_cast<size_t>(str[21]) << 40) |
								(static_cast<size_t>(str[22]) << 48) | (static_cast<size_t>(str[23]) << 56) |
								(static_cast<size_t>(str[24]) | (static_cast<size_t>(str[25]) << 8) | (static_cast<size_t>(str[26]) << 16) |
									(static_cast<size_t>(str[27]) << 24))));
				case 29:
					return result ^
						(static_cast<size_t>(str[0]) | (static_cast<size_t>(str[1]) << 8) | (static_cast<size_t>(str[2]) << 16) | (static_cast<size_t>(str[3]) << 24) |
							(static_cast<size_t>(str[4]) << 32) | (static_cast<size_t>(str[5]) << 40) | (static_cast<size_t>(str[6]) << 48) | (static_cast<size_t>(str[7]) << 56) |
							(static_cast<size_t>(str[8]) | (static_cast<size_t>(str[9]) << 8) | (static_cast<size_t>(str[10]) << 16) | (static_cast<size_t>(str[11]) << 24) |
								(static_cast<size_t>(str[12]) << 32) | (static_cast<size_t>(str[13]) << 40) | (static_cast<size_t>(str[14]) << 48) |
								(static_cast<size_t>(str[15]) << 56) | (static_cast<size_t>(str[16])) | (static_cast<size_t>(str[17]) << 8) | (static_cast<size_t>(str[18]) << 16) |
								(static_cast<size_t>(str[19]) << 24) | (static_cast<size_t>(str[20]) << 32) | (static_cast<size_t>(str[21]) << 40) |
								(static_cast<size_t>(str[22]) << 48) | (static_cast<size_t>(str[23]) << 56) |
								(static_cast<size_t>(str[24]) | (static_cast<size_t>(str[25]) << 8) | (static_cast<size_t>(str[26]) << 16) | (static_cast<size_t>(str[27]) << 24) |
									(static_cast<size_t>(str[28]) << 32))));
				case 30:
					return result ^
						(static_cast<size_t>(str[0]) | (static_cast<size_t>(str[1]) << 8) | (static_cast<size_t>(str[2]) << 16) | (static_cast<size_t>(str[3]) << 24) |
							(static_cast<size_t>(str[4]) << 32) | (static_cast<size_t>(str[5]) << 40) | (static_cast<size_t>(str[6]) << 48) | (static_cast<size_t>(str[7]) << 56) |
							(static_cast<size_t>(str[8]) | (static_cast<size_t>(str[9]) << 8) | (static_cast<size_t>(str[10]) << 16) | (static_cast<size_t>(str[11]) << 24) |
								(static_cast<size_t>(str[12]) << 32) | (static_cast<size_t>(str[13]) << 40) | (static_cast<size_t>(str[14]) << 48) |
								(static_cast<size_t>(str[15]) << 56) | (static_cast<size_t>(str[16])) | (static_cast<size_t>(str[17]) << 8) | (static_cast<size_t>(str[18]) << 16) |
								(static_cast<size_t>(str[19]) << 24) | (static_cast<size_t>(str[20]) << 32) | (static_cast<size_t>(str[21]) << 40) |
								(static_cast<size_t>(str[22]) << 48) | (static_cast<size_t>(str[23]) << 56) |
								(static_cast<size_t>(str[24]) | (static_cast<size_t>(str[25]) << 8) | (static_cast<size_t>(str[26]) << 16) | (static_cast<size_t>(str[27]) << 24) |
									(static_cast<size_t>(str[28]) << 32) | (static_cast<size_t>(str[29]) << 40))));
				case 31:
					return result ^
						(static_cast<size_t>(str[0]) | (static_cast<size_t>(str[1]) << 8) | (static_cast<size_t>(str[2]) << 16) | (static_cast<size_t>(str[3]) << 24) |
							(static_cast<size_t>(str[4]) << 32) | (static_cast<size_t>(str[5]) << 40) | (static_cast<size_t>(str[6]) << 48) | (static_cast<size_t>(str[7]) << 56) |
							(static_cast<size_t>(str[8]) | (static_cast<size_t>(str[9]) << 8) | (static_cast<size_t>(str[10]) << 16) | (static_cast<size_t>(str[11]) << 24) |
								(static_cast<size_t>(str[12]) << 32) | (static_cast<size_t>(str[13]) << 40) | (static_cast<size_t>(str[14]) << 48) |
								(static_cast<size_t>(str[15]) << 56) | (static_cast<size_t>(str[16])) | (static_cast<size_t>(str[17]) << 8) | (static_cast<size_t>(str[18]) << 16) |
								(static_cast<size_t>(str[19]) << 24) | (static_cast<size_t>(str[20]) << 32) | (static_cast<size_t>(str[21]) << 40) |
								(static_cast<size_t>(str[22]) << 48) | (static_cast<size_t>(str[23]) << 56) |
								(static_cast<size_t>(str[24]) | (static_cast<size_t>(str[25]) << 8) | (static_cast<size_t>(str[26]) << 16) | (static_cast<size_t>(str[27]) << 24) |
									(static_cast<size_t>(str[28]) << 32) | (static_cast<size_t>(str[29]) << 40) | (static_cast<size_t>(str[30]) << 48))));
				case 32:
					return result ^
						(static_cast<size_t>(str[0]) | (static_cast<size_t>(str[1]) << 8) | (static_cast<size_t>(str[2]) << 16) | (static_cast<size_t>(str[3]) << 24) |
							(static_cast<size_t>(str[4]) << 32) | (static_cast<size_t>(str[5]) << 40) | (static_cast<size_t>(str[6]) << 48) | (static_cast<size_t>(str[7]) << 56) |
							(static_cast<size_t>(str[8]) | (static_cast<size_t>(str[9]) << 8) | (static_cast<size_t>(str[10]) << 16) | (static_cast<size_t>(str[11]) << 24) |
								(static_cast<size_t>(str[12]) << 32) | (static_cast<size_t>(str[13]) << 40) | (static_cast<size_t>(str[14]) << 48) |
								(static_cast<size_t>(str[15]) << 56) | (static_cast<size_t>(str[16])) | (static_cast<size_t>(str[17]) << 8) | (static_cast<size_t>(str[18]) << 16) |
								(static_cast<size_t>(str[19]) << 24) | (static_cast<size_t>(str[20]) << 32) | (static_cast<size_t>(str[21]) << 40) |
								(static_cast<size_t>(str[22]) << 48) | (static_cast<size_t>(str[23]) << 56) |
								(static_cast<size_t>(str[24]) | (static_cast<size_t>(str[25]) << 8) | (static_cast<size_t>(str[26]) << 16) | (static_cast<size_t>(str[27]) << 24) |
									(static_cast<size_t>(str[28]) << 32) | (static_cast<size_t>(str[29]) << 40) | (static_cast<size_t>(str[30]) << 48) |
									(static_cast<size_t>(str[31]) << 56))));
				default:
					return result;
			}
		}
	};

	template<size_t seed> struct bit_mixer<0, seed> {
		template<typename char_type> JSONIFIER_ALWAYS_INLINE static constexpr size_t mixBits(char_type* str) {
			return seed;
		}
	};

	template<size_t seed> struct bit_mixer<1, seed> {
		template<typename char_type> JSONIFIER_ALWAYS_INLINE static constexpr size_t mixBits(char_type* str) {
			return seed ^ (static_cast<size_t>(str[0]));
		}
	};

	template<size_t seed> struct bit_mixer<2, seed> {
		template<typename char_type> JSONIFIER_ALWAYS_INLINE static constexpr size_t mixBits(char_type* str) {
			return seed ^ (static_cast<size_t>(str[0]) | (static_cast<size_t>(str[1]) << 8));
		}
	};

	template<size_t seed> struct bit_mixer<3, seed> {
		template<typename char_type> JSONIFIER_ALWAYS_INLINE static constexpr size_t mixBits(char_type* str) {
			return seed ^ (static_cast<size_t>(str[0]) | (static_cast<size_t>(str[1]) << 8) | (static_cast<size_t>(str[2]) << 16));
		}
	};

	template<size_t seed> struct bit_mixer<4, seed> {
		template<typename char_type> JSONIFIER_ALWAYS_INLINE static constexpr size_t mixBits(char_type* str) {
			return seed ^ (static_cast<size_t>(str[0]) | (static_cast<size_t>(str[1]) << 8) | (static_cast<size_t>(str[2]) << 16) | (static_cast<size_t>(str[3]) << 24));
		}
	};

	template<size_t seed> struct bit_mixer<5, seed> {
		template<typename char_type> JSONIFIER_ALWAYS_INLINE static constexpr size_t mixBits(char_type* str) {
			return seed ^
				(static_cast<size_t>(str[0]) | (static_cast<size_t>(str[1]) << 8) | (static_cast<size_t>(str[2]) << 16) | (static_cast<size_t>(str[3]) << 24) |
					(static_cast<size_t>(str[4]) << 32));
		}
	};

	template<size_t seed> struct bit_mixer<6, seed> {
		template<typename char_type> JSONIFIER_ALWAYS_INLINE static constexpr size_t mixBits(char_type* str) {
			return seed ^
				(static_cast<size_t>(str[0]) | (static_cast<size_t>(str[1]) << 8) | (static_cast<size_t>(str[2]) << 16) | (static_cast<size_t>(str[3]) << 24) |
					(static_cast<size_t>(str[4]) << 32) | (static_cast<size_t>(str[5]) << 40));
		}
	};

	template<size_t seed> struct bit_mixer<7, seed> {
		template<typename char_type> JSONIFIER_ALWAYS_INLINE static constexpr size_t mixBits(char_type* str) {
			return seed ^
				(static_cast<size_t>(str[0]) | (static_cast<size_t>(str[1]) << 8) | (static_cast<size_t>(str[2]) << 16) | (static_cast<size_t>(str[3]) << 24) |
					(static_cast<size_t>(str[4]) << 32) | (static_cast<size_t>(str[5]) << 40) | (static_cast<size_t>(str[6]) << 48));
		}
	};

	template<size_t seed> struct bit_mixer<8, seed> {
		template<typename char_type> JSONIFIER_ALWAYS_INLINE static constexpr size_t mixBits(char_type* str) {
			return seed ^
				(static_cast<size_t>(str[0]) | (static_cast<size_t>(str[1]) << 8) | (static_cast<size_t>(str[2]) << 16) | (static_cast<size_t>(str[3]) << 24) |
					(static_cast<size_t>(str[4]) << 32) | (static_cast<size_t>(str[5]) << 40) | (static_cast<size_t>(str[6]) << 48) | (static_cast<size_t>(str[7]) << 56));
		}
	};

	template<size_t seed> struct bit_mixer<9, seed> {
		template<typename char_type> JSONIFIER_ALWAYS_INLINE static constexpr size_t mixBits(char_type* str) {
			return seed ^
				(static_cast<size_t>(str[0]) | (static_cast<size_t>(str[1]) << 8) | (static_cast<size_t>(str[2]) << 16) | (static_cast<size_t>(str[3]) << 24) |
					(static_cast<size_t>(str[4]) << 32) | (static_cast<size_t>(str[5]) << 40) | (static_cast<size_t>(str[6]) << 48) | (static_cast<size_t>(str[7]) << 56) |
					(static_cast<size_t>(str[8])));
		}
	};

	template<size_t seed> struct bit_mixer<10, seed> {
		template<typename char_type> JSONIFIER_ALWAYS_INLINE static constexpr size_t mixBits(char_type* str) {
			return seed ^
				(static_cast<size_t>(str[0]) | (static_cast<size_t>(str[1]) << 8) | (static_cast<size_t>(str[2]) << 16) | (static_cast<size_t>(str[3]) << 24) |
					(static_cast<size_t>(str[4]) << 32) | (static_cast<size_t>(str[5]) << 40) | (static_cast<size_t>(str[6]) << 48) | (static_cast<size_t>(str[7]) << 56) |
					(static_cast<size_t>(str[8]) | (static_cast<size_t>(str[9]) << 8)));
		}
	};

	template<size_t seed> struct bit_mixer<11, seed> {
		template<typename char_type> JSONIFIER_ALWAYS_INLINE static constexpr size_t mixBits(char_type* str) {
			return seed ^
				(static_cast<size_t>(str[0]) | (static_cast<size_t>(str[1]) << 8) | (static_cast<size_t>(str[2]) << 16) | (static_cast<size_t>(str[3]) << 24) |
					(static_cast<size_t>(str[4]) << 32) | (static_cast<size_t>(str[5]) << 40) | (static_cast<size_t>(str[6]) << 48) | (static_cast<size_t>(str[7]) << 56) |
					(static_cast<size_t>(str[8]) | (static_cast<size_t>(str[9]) << 8) | (static_cast<size_t>(str[10]) << 16)));
		}
	};

	template<size_t seed> struct bit_mixer<12, seed> {
		template<typename char_type> JSONIFIER_ALWAYS_INLINE static constexpr size_t mixBits(char_type* str) {
			return seed ^
				(static_cast<size_t>(str[0]) | (static_cast<size_t>(str[1]) << 8) | (static_cast<size_t>(str[2]) << 16) | (static_cast<size_t>(str[3]) << 24) |
					(static_cast<size_t>(str[4]) << 32) | (static_cast<size_t>(str[5]) << 40) | (static_cast<size_t>(str[6]) << 48) | (static_cast<size_t>(str[7]) << 56) |
					(static_cast<size_t>(str[8]) | (static_cast<size_t>(str[9]) << 8) | (static_cast<size_t>(str[10]) << 16) | (static_cast<size_t>(str[11]) << 24)));
		}
	};

	template<size_t seed> struct bit_mixer<13, seed> {
		template<typename char_type> JSONIFIER_ALWAYS_INLINE static constexpr size_t mixBits(char_type* str) {
			return seed ^
				(static_cast<size_t>(str[0]) | (static_cast<size_t>(str[1]) << 8) | (static_cast<size_t>(str[2]) << 16) | (static_cast<size_t>(str[3]) << 24) |
					(static_cast<size_t>(str[4]) << 32) | (static_cast<size_t>(str[5]) << 40) | (static_cast<size_t>(str[6]) << 48) | (static_cast<size_t>(str[7]) << 56) |
					(static_cast<size_t>(str[8]) | (static_cast<size_t>(str[9]) << 8) | (static_cast<size_t>(str[10]) << 16) | (static_cast<size_t>(str[11]) << 24) |
						(static_cast<size_t>(str[12]) << 32)));
		}
	};

	template<size_t seed> struct bit_mixer<14, seed> {
		template<typename char_type> JSONIFIER_ALWAYS_INLINE static constexpr size_t mixBits(char_type* str) {
			return seed ^
				(static_cast<size_t>(str[0]) | (static_cast<size_t>(str[1]) << 8) | (static_cast<size_t>(str[2]) << 16) | (static_cast<size_t>(str[3]) << 24) |
					(static_cast<size_t>(str[4]) << 32) | (static_cast<size_t>(str[5]) << 40) | (static_cast<size_t>(str[6]) << 48) | (static_cast<size_t>(str[7]) << 56) |
					(static_cast<size_t>(str[8]) | (static_cast<size_t>(str[9]) << 8) | (static_cast<size_t>(str[10]) << 16) | (static_cast<size_t>(str[11]) << 24) |
						(static_cast<size_t>(str[12]) << 32) | (static_cast<size_t>(str[13]) << 40)));
		}
	};

	template<size_t seed> struct bit_mixer<15, seed> {
		template<typename char_type> JSONIFIER_ALWAYS_INLINE static constexpr size_t mixBits(char_type* str) {
			return seed ^
				(static_cast<size_t>(str[0]) | (static_cast<size_t>(str[1]) << 8) | (static_cast<size_t>(str[2]) << 16) | (static_cast<size_t>(str[3]) << 24) |
					(static_cast<size_t>(str[4]) << 32) | (static_cast<size_t>(str[5]) << 40) | (static_cast<size_t>(str[6]) << 48) | (static_cast<size_t>(str[7]) << 56) |
					(static_cast<size_t>(str[8]) | (static_cast<size_t>(str[9]) << 8) | (static_cast<size_t>(str[10]) << 16) | (static_cast<size_t>(str[11]) << 24) |
						(static_cast<size_t>(str[12]) << 32) | (static_cast<size_t>(str[13]) << 40) | (static_cast<size_t>(str[14]) << 48)));
		}
	};

	template<size_t seed> struct bit_mixer<16, seed> {
		template<typename char_type> JSONIFIER_ALWAYS_INLINE static constexpr size_t mixBits(char_type* str) {
			return seed ^
				(static_cast<size_t>(str[0]) | (static_cast<size_t>(str[1]) << 8) | (static_cast<size_t>(str[2]) << 16) | (static_cast<size_t>(str[3]) << 24) |
					(static_cast<size_t>(str[4]) << 32) | (static_cast<size_t>(str[5]) << 40) | (static_cast<size_t>(str[6]) << 48) | (static_cast<size_t>(str[7]) << 56) |
					(static_cast<size_t>(str[8]) | (static_cast<size_t>(str[9]) << 8) | (static_cast<size_t>(str[10]) << 16) | (static_cast<size_t>(str[11]) << 24) |
						(static_cast<size_t>(str[12]) << 32) | (static_cast<size_t>(str[13]) << 40) | (static_cast<size_t>(str[14]) << 48) | (static_cast<size_t>(str[15]) << 56)));
		}
	};

	template<size_t seed> struct bit_mixer<17, seed> {
		template<typename char_type> JSONIFIER_ALWAYS_INLINE static constexpr size_t mixBits(char_type* str) {
			return seed ^
				(static_cast<size_t>(str[0]) | (static_cast<size_t>(str[1]) << 8) | (static_cast<size_t>(str[2]) << 16) | (static_cast<size_t>(str[3]) << 24) |
					(static_cast<size_t>(str[4]) << 32) | (static_cast<size_t>(str[5]) << 40) | (static_cast<size_t>(str[6]) << 48) | (static_cast<size_t>(str[7]) << 56) |
					(static_cast<size_t>(str[8]) | (static_cast<size_t>(str[9]) << 8) | (static_cast<size_t>(str[10]) << 16) | (static_cast<size_t>(str[11]) << 24) |
						(static_cast<size_t>(str[12]) << 32) | (static_cast<size_t>(str[13]) << 40) | (static_cast<size_t>(str[14]) << 48) | (static_cast<size_t>(str[15]) << 56) |
						(static_cast<size_t>(str[16]))));
		}
	};

	template<size_t seed> struct bit_mixer<18, seed> {
		template<typename char_type> JSONIFIER_ALWAYS_INLINE static constexpr size_t mixBits(char_type* str) {
			return seed ^
				(static_cast<size_t>(str[0]) | (static_cast<size_t>(str[1]) << 8) | (static_cast<size_t>(str[2]) << 16) | (static_cast<size_t>(str[3]) << 24) |
					(static_cast<size_t>(str[4]) << 32) | (static_cast<size_t>(str[5]) << 40) | (static_cast<size_t>(str[6]) << 48) | (static_cast<size_t>(str[7]) << 56) |
					(static_cast<size_t>(str[8]) | (static_cast<size_t>(str[9]) << 8) | (static_cast<size_t>(str[10]) << 16) | (static_cast<size_t>(str[11]) << 24) |
						(static_cast<size_t>(str[12]) << 32) | (static_cast<size_t>(str[13]) << 40) | (static_cast<size_t>(str[14]) << 48) | (static_cast<size_t>(str[15]) << 56) |
						(static_cast<size_t>(str[16])) | (static_cast<size_t>(str[17]) << 8)));
		}
	};

	template<size_t seed> struct bit_mixer<19, seed> {
		template<typename char_type> JSONIFIER_ALWAYS_INLINE static constexpr size_t mixBits(char_type* str) {
			return seed ^
				(static_cast<size_t>(str[0]) | (static_cast<size_t>(str[1]) << 8) | (static_cast<size_t>(str[2]) << 16) | (static_cast<size_t>(str[3]) << 24) |
					(static_cast<size_t>(str[4]) << 32) | (static_cast<size_t>(str[5]) << 40) | (static_cast<size_t>(str[6]) << 48) | (static_cast<size_t>(str[7]) << 56) |
					(static_cast<size_t>(str[8]) | (static_cast<size_t>(str[9]) << 8) | (static_cast<size_t>(str[10]) << 16) | (static_cast<size_t>(str[11]) << 24) |
						(static_cast<size_t>(str[12]) << 32) | (static_cast<size_t>(str[13]) << 40) | (static_cast<size_t>(str[14]) << 48) | (static_cast<size_t>(str[15]) << 56) |
						(static_cast<size_t>(str[16])) | (static_cast<size_t>(str[17]) << 8) | (static_cast<size_t>(str[18]) << 16)));
		}
	};

	template<size_t seed> struct bit_mixer<20, seed> {
		template<typename char_type> JSONIFIER_ALWAYS_INLINE static constexpr size_t mixBits(char_type* str) {
			return seed ^
				(static_cast<size_t>(str[0]) | (static_cast<size_t>(str[1]) << 8) | (static_cast<size_t>(str[2]) << 16) | (static_cast<size_t>(str[3]) << 24) |
					(static_cast<size_t>(str[4]) << 32) | (static_cast<size_t>(str[5]) << 40) | (static_cast<size_t>(str[6]) << 48) | (static_cast<size_t>(str[7]) << 56) |
					(static_cast<size_t>(str[8]) | (static_cast<size_t>(str[9]) << 8) | (static_cast<size_t>(str[10]) << 16) | (static_cast<size_t>(str[11]) << 24) |
						(static_cast<size_t>(str[12]) << 32) | (static_cast<size_t>(str[13]) << 40) | (static_cast<size_t>(str[14]) << 48) | (static_cast<size_t>(str[15]) << 56) |
						(static_cast<size_t>(str[16])) | (static_cast<size_t>(str[17]) << 8) | (static_cast<size_t>(str[18]) << 16) | (static_cast<size_t>(str[19]) << 24)));
		}
	};

	template<size_t seed> struct bit_mixer<21, seed> {
		template<typename char_type> JSONIFIER_ALWAYS_INLINE static constexpr size_t mixBits(char_type* str) {
			return seed ^
				(static_cast<size_t>(str[0]) | (static_cast<size_t>(str[1]) << 8) | (static_cast<size_t>(str[2]) << 16) | (static_cast<size_t>(str[3]) << 24) |
					(static_cast<size_t>(str[4]) << 32) | (static_cast<size_t>(str[5]) << 40) | (static_cast<size_t>(str[6]) << 48) | (static_cast<size_t>(str[7]) << 56) |
					(static_cast<size_t>(str[8]) | (static_cast<size_t>(str[9]) << 8) | (static_cast<size_t>(str[10]) << 16) | (static_cast<size_t>(str[11]) << 24) |
						(static_cast<size_t>(str[12]) << 32) | (static_cast<size_t>(str[13]) << 40) | (static_cast<size_t>(str[14]) << 48) | (static_cast<size_t>(str[15]) << 56) |
						(static_cast<size_t>(str[16])) | (static_cast<size_t>(str[17]) << 8) | (static_cast<size_t>(str[18]) << 16) | (static_cast<size_t>(str[19]) << 24) |
						(static_cast<size_t>(str[20]) << 32)));
		}
	};

	template<size_t seed> struct bit_mixer<22, seed> {
		template<typename char_type> JSONIFIER_ALWAYS_INLINE static constexpr size_t mixBits(char_type* str) {
			return seed ^
				(static_cast<size_t>(str[0]) | (static_cast<size_t>(str[1]) << 8) | (static_cast<size_t>(str[2]) << 16) | (static_cast<size_t>(str[3]) << 24) |
					(static_cast<size_t>(str[4]) << 32) | (static_cast<size_t>(str[5]) << 40) | (static_cast<size_t>(str[6]) << 48) | (static_cast<size_t>(str[7]) << 56) |
					(static_cast<size_t>(str[8]) | (static_cast<size_t>(str[9]) << 8) | (static_cast<size_t>(str[10]) << 16) | (static_cast<size_t>(str[11]) << 24) |
						(static_cast<size_t>(str[12]) << 32) | (static_cast<size_t>(str[13]) << 40) | (static_cast<size_t>(str[14]) << 48) | (static_cast<size_t>(str[15]) << 56) |
						(static_cast<size_t>(str[16])) | (static_cast<size_t>(str[17]) << 8) | (static_cast<size_t>(str[18]) << 16) | (static_cast<size_t>(str[19]) << 24) |
						(static_cast<size_t>(str[20]) << 32) | (static_cast<size_t>(str[21]) << 40)));
		}
	};

	template<size_t seed> struct bit_mixer<23, seed> {
		template<typename char_type> JSONIFIER_ALWAYS_INLINE static constexpr size_t mixBits(char_type* str) {
			return seed ^
				(static_cast<size_t>(str[0]) | (static_cast<size_t>(str[1]) << 8) | (static_cast<size_t>(str[2]) << 16) | (static_cast<size_t>(str[3]) << 24) |
					(static_cast<size_t>(str[4]) << 32) | (static_cast<size_t>(str[5]) << 40) | (static_cast<size_t>(str[6]) << 48) | (static_cast<size_t>(str[7]) << 56) |
					(static_cast<size_t>(str[8]) | (static_cast<size_t>(str[9]) << 8) | (static_cast<size_t>(str[10]) << 16) | (static_cast<size_t>(str[11]) << 24) |
						(static_cast<size_t>(str[12]) << 32) | (static_cast<size_t>(str[13]) << 40) | (static_cast<size_t>(str[14]) << 48) | (static_cast<size_t>(str[15]) << 56) |
						(static_cast<size_t>(str[16])) | (static_cast<size_t>(str[17]) << 8) | (static_cast<size_t>(str[18]) << 16) | (static_cast<size_t>(str[19]) << 24) |
						(static_cast<size_t>(str[20]) << 32) | (static_cast<size_t>(str[21]) << 40) | (static_cast<size_t>(str[22]) << 48)));
		}
	};

	template<size_t seed> struct bit_mixer<24, seed> {
		template<typename char_type> JSONIFIER_ALWAYS_INLINE static constexpr size_t mixBits(char_type* str) {
			return seed ^
				(static_cast<size_t>(str[0]) | (static_cast<size_t>(str[1]) << 8) | (static_cast<size_t>(str[2]) << 16) | (static_cast<size_t>(str[3]) << 24) |
					(static_cast<size_t>(str[4]) << 32) | (static_cast<size_t>(str[5]) << 40) | (static_cast<size_t>(str[6]) << 48) | (static_cast<size_t>(str[7]) << 56) |
					(static_cast<size_t>(str[8]) | (static_cast<size_t>(str[9]) << 8) | (static_cast<size_t>(str[10]) << 16) | (static_cast<size_t>(str[11]) << 24) |
						(static_cast<size_t>(str[12]) << 32) | (static_cast<size_t>(str[13]) << 40) | (static_cast<size_t>(str[14]) << 48) | (static_cast<size_t>(str[15]) << 56) |
						(static_cast<size_t>(str[16])) | (static_cast<size_t>(str[17]) << 8) | (static_cast<size_t>(str[18]) << 16) | (static_cast<size_t>(str[19]) << 24) |
						(static_cast<size_t>(str[20]) << 32) | (static_cast<size_t>(str[21]) << 40) | (static_cast<size_t>(str[22]) << 48) | (static_cast<size_t>(str[23]) << 56)));
		}
	};

	template<size_t seed> struct bit_mixer<25, seed> {
		template<typename char_type> JSONIFIER_ALWAYS_INLINE static constexpr size_t mixBits(char_type* str) {
			return seed ^
				(static_cast<size_t>(str[0]) | (static_cast<size_t>(str[1]) << 8) | (static_cast<size_t>(str[2]) << 16) | (static_cast<size_t>(str[3]) << 24) |
					(static_cast<size_t>(str[4]) << 32) | (static_cast<size_t>(str[5]) << 40) | (static_cast<size_t>(str[6]) << 48) | (static_cast<size_t>(str[7]) << 56) |
					(static_cast<size_t>(str[8]) | (static_cast<size_t>(str[9]) << 8) | (static_cast<size_t>(str[10]) << 16) | (static_cast<size_t>(str[11]) << 24) |
						(static_cast<size_t>(str[12]) << 32) | (static_cast<size_t>(str[13]) << 40) | (static_cast<size_t>(str[14]) << 48) | (static_cast<size_t>(str[15]) << 56) |
						(static_cast<size_t>(str[16])) | (static_cast<size_t>(str[17]) << 8) | (static_cast<size_t>(str[18]) << 16) | (static_cast<size_t>(str[19]) << 24) |
						(static_cast<size_t>(str[20]) << 32) | (static_cast<size_t>(str[21]) << 40) | (static_cast<size_t>(str[22]) << 48) | (static_cast<size_t>(str[23]) << 56) |
						(static_cast<size_t>(str[24]))));
		}
	};

	template<size_t seed> struct bit_mixer<26, seed> {
		template<typename char_type> JSONIFIER_ALWAYS_INLINE static constexpr size_t mixBits(char_type* str) {
			return seed ^
				(static_cast<size_t>(str[0]) | (static_cast<size_t>(str[1]) << 8) | (static_cast<size_t>(str[2]) << 16) | (static_cast<size_t>(str[3]) << 24) |
					(static_cast<size_t>(str[4]) << 32) | (static_cast<size_t>(str[5]) << 40) | (static_cast<size_t>(str[6]) << 48) | (static_cast<size_t>(str[7]) << 56) |
					(static_cast<size_t>(str[8]) | (static_cast<size_t>(str[9]) << 8) | (static_cast<size_t>(str[10]) << 16) | (static_cast<size_t>(str[11]) << 24) |
						(static_cast<size_t>(str[12]) << 32) | (static_cast<size_t>(str[13]) << 40) | (static_cast<size_t>(str[14]) << 48) | (static_cast<size_t>(str[15]) << 56) |
						(static_cast<size_t>(str[16])) | (static_cast<size_t>(str[17]) << 8) | (static_cast<size_t>(str[18]) << 16) | (static_cast<size_t>(str[19]) << 24) |
						(static_cast<size_t>(str[20]) << 32) | (static_cast<size_t>(str[21]) << 40) | (static_cast<size_t>(str[22]) << 48) | (static_cast<size_t>(str[23]) << 56) |
						(static_cast<size_t>(str[24])) | (static_cast<size_t>(str[25]) << 8)));
		}
	};

	template<size_t seed> struct bit_mixer<27, seed> {
		template<typename char_type> JSONIFIER_ALWAYS_INLINE static constexpr size_t mixBits(char_type* str) {
			return seed ^
				(static_cast<size_t>(str[0]) | (static_cast<size_t>(str[1]) << 8) | (static_cast<size_t>(str[2]) << 16) | (static_cast<size_t>(str[3]) << 24) |
					(static_cast<size_t>(str[4]) << 32) | (static_cast<size_t>(str[5]) << 40) | (static_cast<size_t>(str[6]) << 48) | (static_cast<size_t>(str[7]) << 56) |
					(static_cast<size_t>(str[8]) | (static_cast<size_t>(str[9]) << 8) | (static_cast<size_t>(str[10]) << 16) | (static_cast<size_t>(str[11]) << 24) |
						(static_cast<size_t>(str[12]) << 32) | (static_cast<size_t>(str[13]) << 40) | (static_cast<size_t>(str[14]) << 48) | (static_cast<size_t>(str[15]) << 56) |
						(static_cast<size_t>(str[16])) | (static_cast<size_t>(str[17]) << 8) | (static_cast<size_t>(str[18]) << 16) | (static_cast<size_t>(str[19]) << 24) |
						(static_cast<size_t>(str[20]) << 32) | (static_cast<size_t>(str[21]) << 40) | (static_cast<size_t>(str[22]) << 48) | (static_cast<size_t>(str[23]) << 56) |
						(static_cast<size_t>(str[24])) | (static_cast<size_t>(str[25]) << 8) | (static_cast<size_t>(str[26]) << 16)));
		}
	};

	template<size_t seed> struct bit_mixer<28, seed> {
		template<typename char_type> JSONIFIER_ALWAYS_INLINE static constexpr size_t mixBits(char_type* str) {
			return seed ^
				(static_cast<size_t>(str[0]) | (static_cast<size_t>(str[1]) << 8) | (static_cast<size_t>(str[2]) << 16) | (static_cast<size_t>(str[3]) << 24) |
					(static_cast<size_t>(str[4]) << 32) | (static_cast<size_t>(str[5]) << 40) | (static_cast<size_t>(str[6]) << 48) | (static_cast<size_t>(str[7]) << 56) |
					(static_cast<size_t>(str[8]) | (static_cast<size_t>(str[9]) << 8) | (static_cast<size_t>(str[10]) << 16) | (static_cast<size_t>(str[11]) << 24) |
						(static_cast<size_t>(str[12]) << 32) | (static_cast<size_t>(str[13]) << 40) | (static_cast<size_t>(str[14]) << 48) | (static_cast<size_t>(str[15]) << 56) |
						(static_cast<size_t>(str[16])) | (static_cast<size_t>(str[17]) << 8) | (static_cast<size_t>(str[18]) << 16) | (static_cast<size_t>(str[19]) << 24) |
						(static_cast<size_t>(str[20]) << 32) | (static_cast<size_t>(str[21]) << 40) | (static_cast<size_t>(str[22]) << 48) | (static_cast<size_t>(str[23]) << 56) |
						(static_cast<size_t>(str[24])) | (static_cast<size_t>(str[25]) << 8) | (static_cast<size_t>(str[26]) << 16) | (static_cast<size_t>(str[27]) << 24)));
		}
	};

	template<size_t seed> struct bit_mixer<29, seed> {
		template<typename char_type> JSONIFIER_ALWAYS_INLINE static constexpr size_t mixBits(char_type* str) {
			return seed ^
				(static_cast<size_t>(str[0]) | (static_cast<size_t>(str[1]) << 8) | (static_cast<size_t>(str[2]) << 16) | (static_cast<size_t>(str[3]) << 24) |
					(static_cast<size_t>(str[4]) << 32) | (static_cast<size_t>(str[5]) << 40) | (static_cast<size_t>(str[6]) << 48) | (static_cast<size_t>(str[7]) << 56) |
					(static_cast<size_t>(str[8]) | (static_cast<size_t>(str[9]) << 8) | (static_cast<size_t>(str[10]) << 16) | (static_cast<size_t>(str[11]) << 24) |
						(static_cast<size_t>(str[12]) << 32) | (static_cast<size_t>(str[13]) << 40) | (static_cast<size_t>(str[14]) << 48) | (static_cast<size_t>(str[15]) << 56) |
						(static_cast<size_t>(str[16])) | (static_cast<size_t>(str[17]) << 8) | (static_cast<size_t>(str[18]) << 16) | (static_cast<size_t>(str[19]) << 24) |
						(static_cast<size_t>(str[20]) << 32) | (static_cast<size_t>(str[21]) << 40) | (static_cast<size_t>(str[22]) << 48) | (static_cast<size_t>(str[23]) << 56) |
						(static_cast<size_t>(str[24])) | (static_cast<size_t>(str[25]) << 8) | (static_cast<size_t>(str[26]) << 16) | (static_cast<size_t>(str[27]) << 24) |
						(static_cast<size_t>(str[28]) << 32)));
		}
	};

	template<size_t seed> struct bit_mixer<30, seed> {
		template<typename char_type> JSONIFIER_ALWAYS_INLINE static constexpr size_t mixBits(char_type* str) {
			return seed ^
				(static_cast<size_t>(str[0]) | (static_cast<size_t>(str[1]) << 8) | (static_cast<size_t>(str[2]) << 16) | (static_cast<size_t>(str[3]) << 24) |
					(static_cast<size_t>(str[4]) << 32) | (static_cast<size_t>(str[5]) << 40) | (static_cast<size_t>(str[6]) << 48) | (static_cast<size_t>(str[7]) << 56) |
					(static_cast<size_t>(str[8]) | (static_cast<size_t>(str[9]) << 8) | (static_cast<size_t>(str[10]) << 16) | (static_cast<size_t>(str[11]) << 24) |
						(static_cast<size_t>(str[12]) << 32) | (static_cast<size_t>(str[13]) << 40) | (static_cast<size_t>(str[14]) << 48) | (static_cast<size_t>(str[15]) << 56) |
						(static_cast<size_t>(str[16])) | (static_cast<size_t>(str[17]) << 8) | (static_cast<size_t>(str[18]) << 16) | (static_cast<size_t>(str[19]) << 24) |
						(static_cast<size_t>(str[20]) << 32) | (static_cast<size_t>(str[21]) << 40) | (static_cast<size_t>(str[22]) << 48) | (static_cast<size_t>(str[23]) << 56) |
						(static_cast<size_t>(str[24])) | (static_cast<size_t>(str[25]) << 8) | (static_cast<size_t>(str[26]) << 16) | (static_cast<size_t>(str[27]) << 24) |
						(static_cast<size_t>(str[28]) << 32) | (static_cast<size_t>(str[29]) << 40)));
		}
	};

	template<size_t seed> struct bit_mixer<31, seed> {
		template<typename char_type> JSONIFIER_ALWAYS_INLINE static constexpr size_t mixBits(char_type* str) {
			return seed ^
				(static_cast<size_t>(str[0]) | (static_cast<size_t>(str[1]) << 8) | (static_cast<size_t>(str[2]) << 16) | (static_cast<size_t>(str[3]) << 24) |
					(static_cast<size_t>(str[4]) << 32) | (static_cast<size_t>(str[5]) << 40) | (static_cast<size_t>(str[6]) << 48) | (static_cast<size_t>(str[7]) << 56) |
					(static_cast<size_t>(str[8]) | (static_cast<size_t>(str[9]) << 8) | (static_cast<size_t>(str[10]) << 16) | (static_cast<size_t>(str[11]) << 24) |
						(static_cast<size_t>(str[12]) << 32) | (static_cast<size_t>(str[13]) << 40) | (static_cast<size_t>(str[14]) << 48) | (static_cast<size_t>(str[15]) << 56) |
						(static_cast<size_t>(str[16])) | (static_cast<size_t>(str[17]) << 8) | (static_cast<size_t>(str[18]) << 16) | (static_cast<size_t>(str[19]) << 24) |
						(static_cast<size_t>(str[20]) << 32) | (static_cast<size_t>(str[21]) << 40) | (static_cast<size_t>(str[22]) << 48) | (static_cast<size_t>(str[23]) << 56) |
						(static_cast<size_t>(str[24])) | (static_cast<size_t>(str[25]) << 8) | (static_cast<size_t>(str[26]) << 16) | (static_cast<size_t>(str[27]) << 24) |
						(static_cast<size_t>(str[28]) << 32) | (static_cast<size_t>(str[29]) << 40) | (static_cast<size_t>(str[30]) << 48)));
		}
	};

	template<size_t seed> struct bit_mixer<32, seed> {
		template<typename char_type> JSONIFIER_ALWAYS_INLINE static constexpr size_t mixBits(char_type* str) {
			return seed ^
				(static_cast<size_t>(str[0]) | (static_cast<size_t>(str[1]) << 8) | (static_cast<size_t>(str[2]) << 16) | (static_cast<size_t>(str[3]) << 24) |
					(static_cast<size_t>(str[4]) << 32) | (static_cast<size_t>(str[5]) << 40) | (static_cast<size_t>(str[6]) << 48) | (static_cast<size_t>(str[7]) << 56) |
					(static_cast<size_t>(str[8]) | (static_cast<size_t>(str[9]) << 8) | (static_cast<size_t>(str[10]) << 16) | (static_cast<size_t>(str[11]) << 24) |
						(static_cast<size_t>(str[12]) << 32) | (static_cast<size_t>(str[13]) << 40) | (static_cast<size_t>(str[14]) << 48) | (static_cast<size_t>(str[15]) << 56) |
						(static_cast<size_t>(str[16])) | (static_cast<size_t>(str[17]) << 8) | (static_cast<size_t>(str[18]) << 16) | (static_cast<size_t>(str[19]) << 24) |
						(static_cast<size_t>(str[20]) << 32) | (static_cast<size_t>(str[21]) << 40) | (static_cast<size_t>(str[22]) << 48) | (static_cast<size_t>(str[23]) << 56) |
						(static_cast<size_t>(str[24])) | (static_cast<size_t>(str[25]) << 8) | (static_cast<size_t>(str[26]) << 16) | (static_cast<size_t>(str[27]) << 24) |
						(static_cast<size_t>(str[28]) << 32) | (static_cast<size_t>(str[29]) << 40) | (static_cast<size_t>(str[30]) << 48) | (static_cast<size_t>(str[31]) << 56)));
		}
	};

}