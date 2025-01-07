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
/// Feb 20, 2023
#pragma once

#include <jsonifier/Allocator.hpp>
#include <jsonifier/Iterator.hpp>
#include <jsonifier/Compare.hpp>

namespace jsonifier_internal {

	template<typename char_type, size_t initialSize> struct string_base;

	template<typename value_type>
	concept not_uint8_t = !std::same_as<std::remove_cvref_t<value_type>, uint8_t>;

	template<typename value_type> class char_traits;

	template<not_uint8_t value_type> class char_traits<value_type> : public std::char_traits<std::remove_cvref_t<value_type>> {};

	template<jsonifier::concepts::uns8_t value_type_new> class char_traits<value_type_new> {
	  public:
		using value_type	= uint8_t;
		using pointer		= value_type*;
		using const_pointer = const value_type*;
		using size_type		= uint64_t;

		static constexpr void move(pointer firstNew, pointer first2, size_type count) noexcept {
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
			std::memmove(firstNew, first2, count * sizeof(value_type_new));
		}

		static constexpr size_type length(const_pointer first) noexcept {
			const_pointer newPtr = first;
			size_type count		 = 0;
			while (newPtr && *newPtr != static_cast<uint8_t>(0x00u)) {
				++count;
				++newPtr;
			}

			return count;
		}
	};

}

namespace jsonifier {

	template<typename value_type> class string_view_base;

	template<typename value_type_new, uint64_t newerSize = 0> class string_base : public std::string {
	  public:

		string_base() noexcept {
			if constexpr (newerSize > 0) {
				std::string::resize(newerSize);
			}
		};

		string_base& operator=(string_base&& other) noexcept {
			if JSONIFIER_LIKELY (this != &other) {
				string_base newValue{ other };
				swap(newValue);
			}
			return *this;
		}

		explicit string_base(string_base&& other) noexcept {
			swap(other);
		};

		string_base& operator=(const string_base& other) noexcept {
			if JSONIFIER_LIKELY (this != &other) {
				string_base newValue{ other };
				swap(newValue);
			}
			return *this;
		}

		template<typename... arg_types> string_base substr(arg_types&&... args) {
			return std::string::substr(jsonifier_internal::forward<arg_types>(args)...);
		}

		string_base(const string_base& other) noexcept {
			size_type newSize = other.size();
			if JSONIFIER_LIKELY (newSize > 0 && newSize < max_size()) {
				resize(newSize);
				std::uninitialized_copy(other.data(), other.data() + newSize, data());
			}
		}

		template<jsonifier::concepts::string_t value_type_newer> string_base& operator=(value_type_newer&& other) noexcept {
			string_base newValue{ other };
			swap(newValue);
			return *this;
		}

		template<jsonifier::concepts::string_t value_type_newer> string_base(value_type_newer&& other) noexcept {
			size_type newSize = other.size() * (sizeof(typename std::remove_cvref_t<value_type_newer>::value_type) / sizeof(value_type));
			if JSONIFIER_LIKELY (newSize > 0 && newSize < max_size()) {
				resize(newSize);
				std::memcpy(data(), other.data(), newSize);
			}
		}

		template<jsonifier::concepts::pointer_t value_type_newer> string_base& operator=(value_type_newer other) noexcept {
			string_base newValue{ jsonifier_internal::forward<value_type_newer>(other) };
			swap(newValue);
			return *this;
		}

		template<jsonifier::concepts::pointer_t value_type_newer> string_base(value_type_newer other) noexcept {
			if (other) {
				const auto newSize = jsonifier_internal::char_traits<std::remove_pointer_t<value_type_newer>>::length(other) *
					(sizeof(std::remove_pointer_t<value_type_newer>) / sizeof(value_type));
				if JSONIFIER_LIKELY (newSize > 0 && newSize < max_size()) {
					resize(newSize);
					std::memcpy(data(), other, newSize);
				}
			}
		}

		template<jsonifier::concepts::char_t value_type_newer> string_base& operator=(value_type_newer other) noexcept {
			std::string::push_back(static_cast<value_type>(other));
			return *this;
		}

		template<jsonifier::concepts::char_t value_type_newer> string_base(value_type_newer other) noexcept {
			*this = other;
		}

		string_base(const_pointer other, uint64_t newSize) noexcept {
			if JSONIFIER_LIKELY (newSize > 0 && newSize < max_size()) {
				resize(newSize);
				std::memcpy(data(), other, newSize);
			}
		}

		template<std::forward_iterator iterator_new>
		string_base(iterator_new other, uint64_t newSize) noexcept {
			if JSONIFIER_LIKELY (newSize > 0 && newSize < max_size()) {
				resize(newSize);
				std::memcpy(data(), &*other, newSize);
			}
		}
	};

	using string = string_base<char>;

	template<typename value_type> std::ostream& operator<<(std::ostream& os, const string_base<value_type>& string) noexcept {
		os << string.data();
		return os;
	}
}// namespace jsonifier

namespace jsonifier_internal {

	static thread_local jsonifier::string_base<char, 1024 * 1024> stringBuffer{};

}