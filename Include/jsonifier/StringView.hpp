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
/// Feb 20, 2023
#pragma once

#include <jsonifier/String.hpp>

namespace jsonifier {

	template<typename value_type_new> class string_view_base {
	  public:
		using value_type			 = value_type_new;
		using const_pointer			 = const value_type*;
		using const_reference		 = const value_type&;
		using iterator				 = jsonifier_internal::iterator<value_type>;
		using const_iterator		 = jsonifier_internal::iterator<const value_type>;
		using difference_type		 = std::ptrdiff_t;
		using const_reverse_iterator = std::reverse_iterator<const_iterator>;
		using size_type				 = uint64_t;
		using traits_type			 = jsonifier_internal::char_traits<value_type>;

		static constexpr size_type npos{ std::numeric_limits<size_type>::max() };

		JSONIFIER_ALWAYS_INLINE constexpr string_view_base() noexcept : dataVal(), sizeVal(0) {
		}

		template<typename value_type_newer> JSONIFIER_ALWAYS_INLINE constexpr string_view_base& operator=(const string_base<value_type_newer>& stringNew) noexcept {
			dataVal = stringNew.data();
			sizeVal = stringNew.size();
			return *this;
		}

		template<typename value_type_newer> JSONIFIER_ALWAYS_INLINE constexpr string_view_base(const string_base<value_type_newer>& stringNew) noexcept {
			*this = stringNew;
		}

		template<jsonifier::concepts::string_t value_type_newer> JSONIFIER_ALWAYS_INLINE constexpr string_view_base& operator=(value_type_newer&& stringNew) noexcept {
			dataVal = stringNew.data();
			sizeVal = stringNew.size();
			return *this;
		}

		template<jsonifier::concepts::string_t value_type_newer> JSONIFIER_ALWAYS_INLINE constexpr string_view_base(value_type_newer&& stringNew) noexcept {
			*this = stringNew;
		}

		template<typename value_type_newer, jsonifier::concepts::same_character_size<value_type>>
		JSONIFIER_ALWAYS_INLINE constexpr string_view_base& operator=(const value_type_newer& stringNew) noexcept {
			dataVal = stringNew.data();
			sizeVal = stringNew.size();
			return *this;
		}

		template<typename value_type_newer, jsonifier::concepts::same_character_size<value_type>>
		JSONIFIER_ALWAYS_INLINE constexpr string_view_base(const value_type_newer& stringNew) noexcept {
			*this = stringNew;
		}

		JSONIFIER_ALWAYS_INLINE constexpr string_view_base(const_pointer pointerNew, const size_type countNew) noexcept : dataVal(pointerNew), sizeVal(countNew){};

		JSONIFIER_ALWAYS_INLINE constexpr string_view_base(const_pointer pointerNew) noexcept : dataVal(pointerNew), sizeVal(std::char_traits<value_type>::length(pointerNew)){};

		JSONIFIER_ALWAYS_INLINE constexpr const_iterator begin() noexcept {
			return const_iterator{ dataVal };
		}

		JSONIFIER_ALWAYS_INLINE constexpr const_iterator end() noexcept {
			return const_iterator{ dataVal + sizeVal };
		}

		JSONIFIER_ALWAYS_INLINE constexpr const_reverse_iterator rbegin() noexcept {
			return const_reverse_iterator{ end() };
		}

		JSONIFIER_ALWAYS_INLINE constexpr const_reverse_iterator rend() noexcept {
			return const_reverse_iterator{ begin() };
		}

		JSONIFIER_ALWAYS_INLINE constexpr const_iterator begin() const noexcept {
			return const_iterator{ dataVal };
		}

		JSONIFIER_ALWAYS_INLINE constexpr const_iterator end() const noexcept {
			return const_iterator{ dataVal + sizeVal };
		}

		JSONIFIER_ALWAYS_INLINE constexpr const_reverse_iterator rbegin() const noexcept {
			return const_reverse_iterator{ end() };
		}

		JSONIFIER_ALWAYS_INLINE constexpr const_reverse_iterator rend() const noexcept {
			return const_reverse_iterator{ begin() };
		}

		JSONIFIER_ALWAYS_INLINE constexpr size_type size() const noexcept {
			return sizeVal;
		}

		JSONIFIER_ALWAYS_INLINE constexpr size_type length() const noexcept {
			return sizeVal;
		}

		JSONIFIER_ALWAYS_INLINE constexpr bool empty() const noexcept {
			return sizeVal == 0;
		}

		JSONIFIER_ALWAYS_INLINE constexpr const_pointer data() const noexcept {
			return dataVal;
		}

		JSONIFIER_ALWAYS_INLINE constexpr size_type maxSize() const noexcept {
			return std::min(static_cast<uint64_t>(std::numeric_limits<std::ptrdiff_t>::max()), static_cast<uint64_t>(-1) / sizeof(value_type));
		}

		JSONIFIER_ALWAYS_INLINE constexpr const_reference at(const size_type offsetNew) const noexcept {
			if (offsetNew >= sizeVal) [[unlikely]] {
				throw std::out_of_range{ "Sorry, but that index is beyond the end of this string_view instance." };
			}
			return dataVal[offsetNew];
		}

		JSONIFIER_ALWAYS_INLINE constexpr const_reference operator[](const size_type offsetNew) const noexcept {
			return dataVal[offsetNew];
		}

		JSONIFIER_ALWAYS_INLINE constexpr const_reference front() const noexcept {
			return dataVal[0];
		}

		JSONIFIER_ALWAYS_INLINE constexpr const_reference back() const noexcept {
			return dataVal[sizeVal - 1];
		}

		template<typename... arg_types> JSONIFIER_ALWAYS_INLINE constexpr size_type rfind(arg_types&&... args) const noexcept {
			return this->operator std::basic_string_view<value_type>().rfind(std::forward<arg_types>(args)...);
		}

		template<typename... arg_types> JSONIFIER_ALWAYS_INLINE constexpr size_type find(arg_types&&... args) const noexcept {
			return this->operator std::basic_string_view<value_type>().find(std::forward<arg_types>(args)...);
		}

		template<typename... arg_types> JSONIFIER_ALWAYS_INLINE constexpr size_type findFirstOf(arg_types&&... args) const noexcept {
			return this->operator std::basic_string_view<value_type>().find_first_of(std::forward<arg_types>(args)...);
		}

		template<typename... arg_types> JSONIFIER_ALWAYS_INLINE constexpr size_type findLastOf(arg_types&&... args) const noexcept {
			return this->operator std::basic_string_view<value_type>().find_last_of(std::forward<arg_types>(args)...);
		}

		template<typename... arg_types> JSONIFIER_ALWAYS_INLINE constexpr size_type findFirstNotOf(arg_types&&... args) const noexcept {
			return this->operator std::basic_string_view<value_type>().find_first_not_of(std::forward<arg_types>(args)...);
		}

		template<typename... arg_types> JSONIFIER_ALWAYS_INLINE constexpr size_type findLastNotOf(arg_types&&... args) const noexcept {
			return this->operator std::basic_string_view<value_type>().find_last_not_of(std::forward<arg_types>(args)...);
		}

		JSONIFIER_ALWAYS_INLINE constexpr void swap(string_view_base& other) noexcept {
			std::swap(dataVal, other.dataVal);
			std::swap(sizeVal, other.sizeVal);
		}

		JSONIFIER_ALWAYS_INLINE constexpr string_view_base substr(const size_type offsetNew = 0, size_type countNew = npos) const {
			if (offsetNew > sizeVal) [[unlikely]] {
				throw std::out_of_range("Substring position is out of range.");
			}

			countNew = std::min(countNew, sizeVal - offsetNew);
			return string_view_base(dataVal + offsetNew, countNew);
		}

		template<typename value_type_newer = value_type> JSONIFIER_ALWAYS_INLINE constexpr explicit operator jsonifier::string_base<value_type_newer>() const noexcept {
			jsonifier::string_base<value_type_newer> returnValue{};
			returnValue.resize(sizeVal);
			if (sizeVal > 0 && dataVal) [[likely]] {
				std::memcpy(returnValue.data(), data(), returnValue.size());
			}
			return returnValue;
		}

		template<typename value_type_newer = value_type> JSONIFIER_ALWAYS_INLINE constexpr explicit operator std::basic_string<value_type_newer>() const noexcept {
			std::basic_string<value_type_newer> returnValue{};
			returnValue.resize(sizeVal);
			if (sizeVal > 0 && dataVal) [[likely]] {
				std::memcpy(returnValue.data(), data(), returnValue.size());
			}
			return returnValue;
		}

		template<typename value_type_newer = value_type> JSONIFIER_ALWAYS_INLINE constexpr explicit operator std::basic_string_view<value_type_newer>() const noexcept {
			return { data(), size() };
		}

		template<jsonifier::concepts::pointer_t value_type_newer> JSONIFIER_ALWAYS_INLINE constexpr friend std::enable_if_t<!std::is_array_v<value_type_newer>, bool> operator==(
			const string_view_base& lhs, const value_type_newer& rhs) noexcept {
			auto rhsLength = jsonifier_internal::char_traits<std::remove_pointer_t<value_type_newer>>::length(rhs);
			return rhsLength == lhs.size() && jsonifier_internal::compare(lhs.data(), rhs, rhsLength);
		}

		template<jsonifier::concepts::string_t value_type_newer>
		JSONIFIER_ALWAYS_INLINE constexpr friend bool operator==(const string_view_base& lhs, const value_type_newer& rhs) noexcept {
			if (std::is_constant_evaluated()) {
				auto compareValues = [=]() -> bool {
					for (uint64_t x = 0; x < rhs.size(); ++x) {
						if (rhs[x] != lhs[x]) {
							return false;
						}
					}
					return true;
				}();
				return lhs.size() == rhs.size() && compareValues;
			} else {
				return rhs.size() == lhs.size() && jsonifier_internal::compare(lhs.data(), rhs.data(), rhs.size());
			}
		}

		template<typename value_type_newer, size_type size>
		JSONIFIER_ALWAYS_INLINE constexpr friend string_base<value_type_newer> operator+(const value_type_newer (&lhs)[size], const string_view_base& rhs) noexcept {
			string_base<value_type_newer> newLhs{ lhs };
			newLhs += rhs;
			return newLhs;
		}

		template<typename value_type_newer, size_type size>
		JSONIFIER_ALWAYS_INLINE constexpr friend string_base<value_type_newer> operator+=(const value_type_newer (&lhs)[size], const string_view_base& rhs) noexcept {
			string_base<value_type_newer> newLhs{ lhs };
			newLhs += rhs;
			return newLhs;
		}

		template<jsonifier::concepts::pointer_t string_type_newer>
		JSONIFIER_ALWAYS_INLINE constexpr friend string_base<string_type_newer> operator+(string_type_newer&& lhs, const string_view_base& rhs) noexcept {
			string_base<std::remove_pointer_t<string_type_newer>> newLhs{ lhs };
			newLhs += rhs;
			return newLhs;
		}

		template<jsonifier::concepts::pointer_t string_type_newer>
		JSONIFIER_ALWAYS_INLINE constexpr friend string_base<string_type_newer> operator+=(string_type_newer&& lhs, const string_view_base& rhs) noexcept {
			string_base<std::remove_pointer_t<string_type_newer>> newLhs{ lhs };
			newLhs += rhs;
			return newLhs;
		}

		JSONIFIER_ALWAYS_INLINE constexpr string_base<value_type_new> operator+(const value_type& rhs) noexcept {
			string_base<value_type_new> newLhs{ *this };
			newLhs.emplace_back(rhs);
			return newLhs;
		}

		JSONIFIER_ALWAYS_INLINE constexpr string_base<value_type_new> operator+=(const value_type& rhs) noexcept {
			string_base<value_type_new> newLhs{ *this };
			newLhs.emplace_back(rhs);
			return newLhs;
		}

		template<jsonifier::concepts::string_t string_type_new> JSONIFIER_ALWAYS_INLINE constexpr string_base<value_type_new> operator+(const string_type_new& rhs) const noexcept {
			string_base<value_type_new> newLhs{ *this };
			newLhs += rhs;
			return newLhs;
		}

		template<jsonifier::concepts::string_t string_type_new> JSONIFIER_ALWAYS_INLINE constexpr string_base<value_type_new> operator+=(const string_type_new& rhs) noexcept {
			string_base<value_type_new> newLhs{ *this };
			newLhs.append(rhs.data(), rhs.size());
			return newLhs;
		}

		template<typename value_type_newer, size_type size>
		JSONIFIER_ALWAYS_INLINE constexpr string_base<value_type_newer> operator+(const value_type_newer (&rhs)[size]) const noexcept {
			string_base<value_type_newer> newLhs{ *this };
			newLhs += rhs;
			return newLhs;
		}

		template<typename value_type_newer, size_type size>
		JSONIFIER_ALWAYS_INLINE constexpr string_base<value_type_newer> operator+=(const value_type_newer (&rhs)[size]) noexcept {
			string_base<value_type_newer> newLhs{ *this };
			newLhs += rhs;
			return newLhs;
		}

		const_pointer dataVal{};
		size_type sizeVal{};
	};

	using string_view = string_view_base<char>;

	JSONIFIER_ALWAYS_INLINE std::ostream& operator<<(std::ostream& oStream, const jsonifier::string_view& string) noexcept {
		oStream << string.operator jsonifier::string_base<char>();
		return oStream;
	}

	[[nodiscard]] constexpr string_view operator""_sv(const char* stringNew, size_t lengthNew) noexcept {
		return string_view(stringNew, lengthNew);
	}

}// namespace jsonifier
