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
#pragma once

#include <functional>
#include <optional>
#include <atomic>

namespace jsonifier_internal {

	template<typename string_t> jsonifier_constexpr uint64_t fnv1aHash(const string_t& value) {
		uint64_t d = 5381;
		for (const auto& c: value)
			d = d * 33 + static_cast<uint64_t>(c);
		return d;
	}

	template<typename value_type> struct hash;

	// https://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function
	// With the lowest bits removed, based on experimental setup.
	template<typename string_t> jsonifier_constexpr uint64_t fnv1aHash(const string_t& value, uint64_t seed) {
		uint64_t d = (0x811c9dc5 ^ seed) * static_cast<uint64_t>(0x01000193);
		for (const auto& c: value)
			d = (d ^ static_cast<uint64_t>(c)) * static_cast<uint64_t>(0x01000193);
		return d >> 8;
	}

	template<typename value_type01, typename value_type02> jsonifier_constexpr bool stringConstCompare(const value_type01& string01, const value_type02& string02) {
		if (string01.size() != string02.size()) [[unlikely]] {
			return false;
		}
		using char_t = value_type01::value_type;
		for (uint64_t x = 0; x < string01.size(); ++x) {
			if (string01[x] != static_cast<char_t>(string02[x])) [[unlikely]] {
				return false;
			}
		}
		return true;
	}

	template<typename value_type> class char_traits : public std::char_traits<value_type> {};

	template<> class char_traits<uint8_t> {
	  public:
		using value_type	= uint8_t;
		using pointer		= value_type*;
		using const_pointer = const value_type*;
		using size_type		= uint64_t;

		static jsonifier_constexpr void move(pointer firstNew, pointer first2, size_type count) {
			if (std::is_constant_evaluated()) {
				bool loopForward = true;

				for (pointer source = first2; source != first2 + count; ++source) {
					if (firstNew == source) {
						loopForward = false;
					}
				}

				if (loopForward) {
					for (uint64_t index = 0; index != count; ++index) {
						firstNew[index] = first2[index];
					}
				} else {
					for (uint64_t index = count; index != 0; --index) {
						firstNew[index - 1] = first2[index - 1];
					}
				}

				return;
			}
			std::memmove(firstNew, first2, count * sizeof(value_type));
		}

		static jsonifier_constexpr size_type length(const_pointer first) {
			const_pointer newPtr = first;
			size_type count		 = 0;
			while (newPtr && *newPtr != value_type{}) {
				++count;
				++newPtr;
			}

			return count;
		}
	};

}// namespace jsonifier_internal