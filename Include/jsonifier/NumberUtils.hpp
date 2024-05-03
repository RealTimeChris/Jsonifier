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
/// https://github.com/RealTimeChris/jsonifier
/// Feb 3, 2023
/// Most of the code in this header was sampled from Glaze library - https://github.com/stephenberry/glaze
#pragma once

#include <jsonifier/Allocator.hpp>
#include <jsonifier/String.hpp>

#include <jsonifier/IToStr.hpp>
#include <jsonifier/DToStr.hpp>
#include <jsonifier/StrToD.hpp>
#include <jsonifier/StrToI.hpp>

namespace jsonifier {

	template<typename value_type = char, jsonifier::concepts::num_t value_type01> JSONIFIER_INLINE jsonifier::string_base<value_type> toString(const value_type01& value) {
		string_base<value_type> returnstring{};
		returnstring.resize(64);
		if constexpr (jsonifier::concepts::unsigned_type<value_type01> && sizeof(value) < 8) {
			uint64_t newValue{ static_cast<uint64_t>(value) };
			auto newPtr = jsonifier_internal::toChars(returnstring.data(), newValue);
			returnstring.resize(static_cast<uint64_t>(newPtr - returnstring.data()));
		} else if constexpr (jsonifier::concepts::signed_type<value_type01> && sizeof(value) < 8) {
			int64_t newValue{ static_cast<int64_t>(value) };
			auto newPtr = jsonifier_internal::toChars(returnstring.data(), newValue);
			returnstring.resize(static_cast<uint64_t>(newPtr - returnstring.data()));
		} else {
			auto newPtr = jsonifier_internal::toChars(returnstring.data(), value);
			returnstring.resize(static_cast<uint64_t>(newPtr - returnstring.data()));
		}
		return returnstring;
	}

	template<uint64_t base = 10> JSONIFIER_INLINE double strToDouble(const jsonifier::string& string) {
		double newValue{};
		if (string.size() > 0) [[likely]] {
			auto newPtr = string.data();
			jsonifier_internal::parseNumber(newValue, newPtr, string.size());
		}
		return newValue;
	}

	template<> JSONIFIER_INLINE double strToDouble<16>(const jsonifier::string& string) {
		double newValue{};
		if (string.size() > 0) [[likely]] {
			newValue = std::strtod(string.data(), nullptr);
		}
		return newValue;
	}

	template<uint64_t base = 10> JSONIFIER_INLINE int64_t strToInt64(const jsonifier::string& string) {
		int64_t newValue{};
		if (string.size() > 0) [[likely]] {
			auto newPtr = string.data();
			jsonifier_internal::stoui64(newValue, newPtr);
		}
		return newValue;
	}

	template<> JSONIFIER_INLINE int64_t strToInt64<16>(const jsonifier::string& string) {
		int64_t newValue{};
		if (string.size() > 0) [[likely]] {
			newValue = std::strtoll(string.data(), nullptr, 16);
		}
		return newValue;
	}

	template<uint64_t base = 10> JSONIFIER_INLINE uint64_t strToUint64(const jsonifier::string& string) {
		uint64_t newValue{};
		if (string.size() > 0) [[likely]] {
			auto newPtr = string.data();
			jsonifier_internal::stoui64(newValue, newPtr);
		}
		return newValue;
	}

	template<> JSONIFIER_INLINE uint64_t strToUint64<16>(const jsonifier::string& string) {
		uint64_t newValue{};
		if (string.size() > 0) [[likely]] {
			newValue = std::strtoull(string.data(), nullptr, 16);
		}
		return newValue;
	}
}