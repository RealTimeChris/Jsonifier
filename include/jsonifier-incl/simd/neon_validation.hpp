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
/// The code below drew heavy inspiration from Dr. Lemire's library, simdjson (https://github.com/simdjson/simdjson)
/// https://github.com/RealTimeChris/jsonifier
#pragma once

#include <jsonifier-incl/containers/array.hpp>
#include <jsonifier-incl/simd/avx.hpp>

namespace jsonifier {

#if JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_NEON)

	static constexpr uint8_t byte1HighTableRaw[]{ tooLong, tooLong, tooLong, tooLong, tooLong, tooLong, tooLong, tooLong, twoConts, twoConts, twoConts, twoConts,
		tooShort | overlong2, tooShort, tooShort | overlong3 | surrogate, tooShort | tooLarge | tooLarge1000 | overlong4 };

	JSONIFIER_ALIGN(bytesPerStep)
	static constexpr std::array<uint8_t, bytesPerStep> byte1HighTable{ [] {
		std::array<uint8_t, bytesPerStep> returnValue{};
		for (uint64_t x = 0; x < bytesPerStep; ++x) {
			returnValue[x] = byte1HighTableRaw[x % std::size(byte1HighTableRaw)];
		}
		return returnValue;
	}() };

	static constexpr uint8_t byte1LowTableRaw[]{ carry | overlong3 | overlong2 | overlong4, carry | overlong2, carry, carry, carry | tooLarge, carry | tooLarge | tooLarge1000,
		carry | tooLarge | tooLarge1000, carry | tooLarge | tooLarge1000, carry | tooLarge | tooLarge1000, carry | tooLarge | tooLarge1000, carry | tooLarge | tooLarge1000,
		carry | tooLarge | tooLarge1000, carry | tooLarge | tooLarge1000, carry | tooLarge | tooLarge1000 | surrogate, carry | tooLarge | tooLarge1000,
		carry | tooLarge | tooLarge1000 };

	JSONIFIER_ALIGN(bytesPerStep)
	static constexpr std::array<uint8_t, bytesPerStep> byte1LowTable{ [] {
		std::array<uint8_t, bytesPerStep> returnValue{};
		for (uint64_t x = 0; x < bytesPerStep; ++x) {
			returnValue[x] = byte1LowTableRaw[x % std::size(byte1LowTableRaw)];
		}
		return returnValue;
	}() };

	static constexpr uint8_t byte2HighTableRaw[]{ tooShort, tooShort, tooShort, tooShort, tooShort, tooShort, tooShort, tooShort,
		tooLong | overlong2 | twoConts | overlong3 | tooLarge1000 | overlong4, tooLong | overlong2 | twoConts | overlong3 | tooLarge,
		tooLong | overlong2 | twoConts | surrogate | tooLarge, tooLong | overlong2 | twoConts | surrogate | tooLarge, tooShort, tooShort, tooShort, tooShort };

	JSONIFIER_ALIGN(bytesPerStep)
	static constexpr std::array<uint8_t, bytesPerStep> byte2HighTable{ [] {
		std::array<uint8_t, bytesPerStep> return_value{};
		for (uint64_t x = 0; x < bytesPerStep; ++x) {
			return_value[x] = byte2HighTableRaw[x % std::size(byte2HighTableRaw)];
		}
		return return_value;
	}() };

	inline bool validateUtf8Ascii([[maybe_unused]] const uint8_t* src, [[maybe_unused]] uint64_t len) {
		return true;
	}

	inline bool validateUtf8Mixed([[maybe_unused]] const uint8_t* src, [[maybe_unused]] uint64_t len) {
		return true;
	}

	inline bool validateUtf8Multibyte([[maybe_unused]] const uint8_t* src, [[maybe_unused]] uint64_t len) {
		return true;
	}

	inline bool validateUtf8([[maybe_unused]] const uint8_t* src, [[maybe_unused]] uint64_t len) {
		return true;
	}

#endif

}
