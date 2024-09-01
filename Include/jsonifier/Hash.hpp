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

	constexpr std::array<uint64_t, 32> primeSeeds{ 0x9E3779B97F4A7C15ull, 0xBB67AE8584CAA73Bull, 0xC6EF372FE94F82BEull, 0x3243F6A8885A308Dull, 0x3C6EF372FE94F82Aull,
		0x41C6D48ED781DADBull, 0x607F38003DE7B62Cull, 0x4A7484AA03ED3DA8ull, 0x789B0FBD5FEFCA10ull, 0x6A09E667F3BCC908ull, 0x510E527FADE682D1ull, 0x1F83D9ABFB41BD6Bull,
		0x5BE0CD19137E2179ull, 0xC6BC279692B5CC83ull, 0x142C8D844E02E21Cull, 0x3A84E481C3382C15ull, 0xB5C0FBCFEC4D3B2Full, 0x563CBC5B5E7E7CFAull, 0x6374618B44433E23ull,
		0x3916A2E04642E281ull, 0x487C1FEBC93E7521ull, 0xDAA66B36681D81A1ull, 0x923F82A4AF193BEBull, 0x8C4751B1F6C6226Dull, 0xAB1C5ED5DA0B09E1ull, 0xD807AA98A3030242ull,
		0x12835B01C4576ACDull, 0x243185BE4EE4B28Cull, 0x550C7DC3D5FFB4E2ull, 0x72BE5D74F27B896Full, 0x80DEB1FE3B1696B1ull, 0x9BDC06A725C71235ull };

	template<typename value_type, typename char_type> constexpr value_type readBitsCt(const char_type* ptr) noexcept {
		value_type returnValue{};
		for (size_t x = 0; x < sizeof(value_type); ++x) {
			returnValue |= static_cast<value_type>(static_cast<uint8_t>(ptr[x])) << (x * 8);
		}
		return returnValue;
	}

	struct key_hasher {
		size_t seedIndex{};///< Seed value for the hashing algorithm.
		size_t seed{};///< Seed value for the hashing algorithm.
		/**
		 * @brief Default constructor that initializes the seed using a random_num value.
		 */
		constexpr key_hasher() noexcept {
			updateSeed();
		}

		/**
		 * @brief Sets the seed value at compile-time.
		 *
		 * @param seedNew The new seed value.
		 */
		JSONIFIER_ALWAYS_INLINE constexpr void updateSeed() noexcept {
			seed = primeSeeds[(seedIndex++) % 32];
		}

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

	  protected:
		mutable uint64_t returnValue64{};
		mutable uint32_t returnValue32{};
		mutable uint16_t returnValue16{};
	};


}