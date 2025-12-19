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

#include <jsonifier/Containers/Allocator.hpp>
#include <jsonifier/Utilities/String.hpp>
#include <jsonifier/Utilities/IToStr.hpp>
#include <jsonifier/Utilities/DToStr.hpp>
#include <jsonifier/Utilities/StrToD.hpp>
#include <jsonifier/Utilities/StrToI.hpp>
#include <jsonifier/Parsing/Parser.hpp>

namespace jsonifier {

	template<bool> class jsonifier_core;

	template<typename value_type_new = char, concepts::num_t value_type01> JSONIFIER_INLINE static string_base<value_type_new> toString(const value_type01& value) noexcept {
		string_base<value_type_new> returnstring{};
		returnstring.resize(64);
		if constexpr (sizeof(value_type01) == 8) {
			auto newPtr = internal::to_chars<value_type_new>::impl(returnstring.data(), value);
			returnstring.resize(static_cast<uint64_t>(newPtr - returnstring.data()));
		} else {
			if constexpr (concepts::unsigned_t<value_type01>) {
				uint64_t newValue{ static_cast<uint64_t>(value) };
				auto newPtr = internal::to_chars<value_type_new>::impl(returnstring.data(), newValue);
				returnstring.resize(static_cast<uint64_t>(newPtr - returnstring.data()));
			} else if constexpr (concepts::signed_t<value_type01>) {
				int64_t newValue{ static_cast<int64_t>(value) };
				auto newPtr = internal::to_chars<value_type_new>::impl(returnstring.data(), newValue);
				returnstring.resize(static_cast<uint64_t>(newPtr - returnstring.data()));
			} else {
				double newValue{ static_cast<double>(value) };
				auto newPtr = internal::to_chars<value_type_new>::impl(returnstring.data(), newValue);
				returnstring.resize(static_cast<uint64_t>(newPtr - returnstring.data()));
			}
		}
		return returnstring;
	}

	template<uint64_t base = 10> JSONIFIER_INLINE static double strToDouble(const string& stringNew) noexcept {
		double newValue{};
		if JSONIFIER_LIKELY (stringNew.size() > 0) {
			auto iter = static_cast<string_view_ptr>(stringNew.data());
			auto end  = static_cast<string_view_ptr>(stringNew.data()) + stringNew.size();
			internal::parseFloat(newValue, iter, end);
		}
		return newValue;
	}

	template<> JSONIFIER_INLINE double strToDouble<16>(const string& stringNew) noexcept {
		double newValue{};
		if JSONIFIER_LIKELY (stringNew.size() > 0) {
			newValue = std::strtod(stringNew.data(), nullptr);
		}
		return newValue;
	}

	template<uint64_t base = 10> JSONIFIER_INLINE static int64_t strToInt64(const string& stringNew) noexcept {
		int64_t newValue{};
		if JSONIFIER_LIKELY (stringNew.size() > 0) {
			auto iter = static_cast<string_view_ptr>(stringNew.data());
			auto end  = static_cast<string_view_ptr>(stringNew.data()) + stringNew.size();
			internal::integer_parser<int64_t>::parseInt(newValue, iter, end);
		}
		return newValue;
	}

	template<> JSONIFIER_INLINE int64_t strToInt64<16>(const string& stringNew) noexcept {
		int64_t newValue{};
		if JSONIFIER_LIKELY (stringNew.size() > 0) {
			newValue = std::strtoll(stringNew.data(), nullptr, 16);
		}
		return newValue;
	}

	template<uint64_t base = 10> JSONIFIER_INLINE static uint64_t strToUint64(const string& stringNew) noexcept {
		uint64_t newValue{};
		if JSONIFIER_LIKELY (stringNew.size() > 0) {
			auto iter = static_cast<string_view_ptr>(stringNew.data());
			auto end  = static_cast<string_view_ptr>(stringNew.data()) + stringNew.size();
			internal::integer_parser<uint64_t>::parseInt(newValue, iter, end);
		}
		return newValue;
	}

	template<> JSONIFIER_INLINE uint64_t strToUint64<16>(const string& stringNew) noexcept {
		uint64_t newValue{};
		if JSONIFIER_LIKELY (stringNew.size() > 0) {
			newValue = std::strtoull(stringNew.data(), nullptr, 16);
		}
		return newValue;
	}
}

namespace jsonifier::internal {

	template<typename value_type_new, typename iterator> JSONIFIER_INLINE static bool parseNumber(value_type_new& value, iterator&& iter, iterator&& end) noexcept {
		using value_type = value_type_new;

		if constexpr (concepts::integer_t<value_type>) {
			if constexpr (concepts::unsigned_t<value_type>) {
				if constexpr (concepts::uns64_t<value_type>) {
					return integer_parser<value_type>::parseInt(value, iter, end);
				} else {
					uint64_t i;
					return integer_parser<uint64_t>::parseInt(i, iter, end) ? (value = static_cast<value_type>(i), true) : false;
				}
			} else {
				if constexpr (concepts::sig64_t<value_type>) {
					return integer_parser<value_type>::parseInt(value, iter, end);
				} else {
					int64_t i;
					return integer_parser<int64_t>::parseInt(i, iter, end) ? (value = static_cast<value_type>(i), true) : false;
				}
			}
		} else {
			if constexpr (std::is_volatile_v<jsonifier::internal::remove_reference_t<decltype(value)>>) {
				double temp;
				return parseFloat(temp, iter, end) ? (value = static_cast<value_type>(temp), true) : false;
			} else {
				return parseFloat(value, iter, end);
			}
		}
	}
}