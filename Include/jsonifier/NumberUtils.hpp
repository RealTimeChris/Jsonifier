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
/// https://github.com/RealTimeChris/jsonifier
/// Feb 3, 2023
#pragma once

#include <jsonifier/Allocator.hpp>
#include <jsonifier/String.hpp>

#include <jsonifier/IToStr.hpp>
#include <jsonifier/DToStr.hpp>
#include <jsonifier/StrToD.hpp>
#include <jsonifier/StrToI.hpp>
#include <jsonifier/Parser.hpp>

namespace jsonifier {

	template<bool> class jsonifier_core;

	template<typename value_type_new = char, jsonifier::concepts::num_t value_type01>
	JSONIFIER_ALWAYS_INLINE jsonifier::string_base<value_type_new> toString(const value_type01& value) {
		string_base<value_type_new> returnstring{};
		returnstring.resize(64);
		if constexpr (jsonifier::concepts::unsigned_type<value_type01> && sizeof(value) < 8) {
			uint64_t newValue{ static_cast<uint64_t>(value) };
			auto newPtr = jsonifier_internal::toChars(returnstring.data(), newValue);
			returnstring.resize(static_cast<uint64_t>(newPtr - returnstring.data()));
		} else {
			if constexpr (jsonifier::concepts::signed_type<value_type01> && sizeof(value) < 8) {
				int64_t newValue{ static_cast<int64_t>(value) };
				auto newPtr = jsonifier_internal::toChars(returnstring.data(), newValue);
				returnstring.resize(static_cast<uint64_t>(newPtr - returnstring.data()));
			} else {
				auto newPtr = jsonifier_internal::toChars(returnstring.data(), value);
				returnstring.resize(static_cast<uint64_t>(newPtr - returnstring.data()));
			}
		}
		return returnstring;
	}

	template<uint64_t base = 10> JSONIFIER_ALWAYS_INLINE double strToDouble(const jsonifier::string& string) noexcept {
		double newValue{};
		if (string.size() > 0) [[likely]] {
			auto currentIter = static_cast<const char*>(string.data());
			auto endIter	 = static_cast<const char*>(string.data()) + string.size();
			jsonifier_internal::parseFloat(currentIter, endIter, newValue);
		}
		return newValue;
	}

	template<> JSONIFIER_ALWAYS_INLINE double strToDouble<16>(const jsonifier::string& string) noexcept {
		double newValue{};
		if (string.size() > 0) [[likely]] {
			newValue = std::strtod(string.data(), nullptr);
		}
		return newValue;
	}

	template<uint64_t base = 10> JSONIFIER_ALWAYS_INLINE int64_t strToInt64(const jsonifier::string& string) noexcept {
		int64_t newValue{};
		if (string.size() > 0) [[likely]] {
			auto newPtr = string.data();
			jsonifier_internal::stoui64(newValue, newPtr);
		}
		return newValue;
	}

	template<> JSONIFIER_ALWAYS_INLINE int64_t strToInt64<16>(const jsonifier::string& string) noexcept {
		int64_t newValue{};
		if (string.size() > 0) [[likely]] {
			newValue = std::strtoll(string.data(), nullptr, 16);
		}
		return newValue;
	}

	template<uint64_t base = 10> JSONIFIER_ALWAYS_INLINE uint64_t strToUint64(const jsonifier::string& string) noexcept {
		uint64_t newValue{};
		if (string.size() > 0) [[likely]] {
			auto newPtr = string.data();
			jsonifier_internal::stoui64(newValue, newPtr);
		}
		return newValue;
	}

	template<> JSONIFIER_ALWAYS_INLINE uint64_t strToUint64<16>(const jsonifier::string& string) noexcept {
		uint64_t newValue{};
		if (string.size() > 0) [[likely]] {
			newValue = std::strtoull(string.data(), nullptr, 16);
		}
		return newValue;
	}
}

namespace jsonifier_internal {

	template<typename value_type_new, typename iterator> JSONIFIER_ALWAYS_INLINE bool parseNumber(value_type_new&& value, iterator&& iter, iterator&& end) noexcept {
		using value_type			  = unwrap_t<value_type_new>;

		if constexpr (jsonifier::concepts::integer_t<value_type>) {
			if constexpr (std::is_unsigned_v<value_type>) {
				static constexpr auto maximum{ std::numeric_limits<uint64_t>::max() };
				if (*iter == '-') [[unlikely]] {
					return false;
				}
				if constexpr (sizeof(value_type) == 8) {
					return parseInt(value, iter) && value <= maximum;
				} else {
					uint64_t i;
					return parseInt(i, iter) && i <= maximum ? (value = static_cast<value_type>(i), true) : false;
				}
			} else {
				static constexpr auto maximum{ std::numeric_limits<int64_t>::max() };
				static constexpr auto minAbs{ std::numeric_limits<int64_t>::min() };
				int64_t sign{ (*iter == '-') ? -1ll : 1ll };
				iter += (*iter == '-');
				if constexpr (sizeof(value_type) == 8) {
					return (parseInt(value, iter)) ? (sign == -1ll) ? (value >= minAbs) ? (value *= sign, true) : false : (value <= maximum) ? true : false : false;
				} else {
					int64_t i;
					return (parseInt(i, iter)) ? (sign == -1ll) ? (i >= minAbs) ? (value = i * sign, true) : false : (i <= maximum) ? (value = i, true) : false : false;
				}
			}
		} else {
			if constexpr (std::is_volatile_v<std::remove_reference_t<decltype(value)>>) {
				value_type temp;
				return parseFloat(iter, end, temp) ? (value = temp, true) : false;
			} else {
				return parseFloat(iter, end, value);
			}
		}
		return true;
	}
}