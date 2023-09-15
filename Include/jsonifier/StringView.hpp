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
		using pointer				 = value_type*;
		using const_pointer			 = const value_type*;
		using reference				 = value_type&;
		using const_reference		 = const value_type&;
		using iterator				 = jsonifier_internal::iterator<value_type>;
		using const_iterator		 = jsonifier_internal::iterator<const value_type>;
		using reverse_iterator		 = std::reverse_iterator<iterator>;
		using const_reverse_iterator = std::reverse_iterator<const_iterator>;
		using size_type				 = uint64_t;
		using allocator				 = jsonifier_internal::alloc_wrapper<value_type>;
		using traits_type			 = jsonifier_internal::char_traits<value_type>;

		const_pointer dataVal{};
		size_type sizeVal{};

		static constexpr size_type npos{ std::numeric_limits<size_type>::max() };

		constexpr string_view_base() : dataVal(), sizeVal(0) {
		}

		template<jsonifier_internal::string_t value_type_newer> constexpr string_view_base& operator=(const value_type_newer& stringNew) {
			dataVal = stringNew.data();
			sizeVal = stringNew.size();
			return *this;
		}

		template<jsonifier_internal::string_t value_type_newer> constexpr string_view_base(const value_type_newer& stringNew) {
			*this = stringNew;
		}

		template<uint64_t strLength> constexpr string_view_base(const value_type (&other)[strLength]) {
			dataVal = other;
			sizeVal = strLength;
		}

		constexpr string_view_base(const_pointer pointerNew) : dataVal(pointerNew), sizeVal(traits_type::length(pointerNew)) {
		}

		constexpr string_view_base(const_pointer pointerNew, const size_type countNew) : dataVal(pointerNew), sizeVal(countNew){};

		constexpr const_iterator begin() const {
			return const_iterator(dataVal);
		}

		constexpr const_iterator end() const {
			return const_iterator(dataVal + sizeVal);
		}

		constexpr const_iterator cbegin() const {
			return const_iterator(begin());
		}

		constexpr const_iterator cend() const {
			return const_iterator(end());
		}

		constexpr const_reverse_iterator crbegin() const {
			return const_reverse_iterator(cend());
		}

		constexpr const_reverse_iterator crend() const {
			return const_reverse_iterator(cbegin());
		}

		constexpr size_type size() const {
			return sizeVal;
		}

		constexpr size_type length() const {
			return sizeVal;
		}

		constexpr bool empty() const {
			return sizeVal == 0;
		}

		constexpr const_pointer data() const {
			return dataVal;
		}

		constexpr size_type max_size() const {
			return std::min(static_cast<uint64_t>(std::numeric_limits<std::ptrdiff_t>::max()), static_cast<uint64_t>(-1) / sizeof(value_type));
		}

		constexpr const_reference at(const size_type offsetNew) const {
			if (offsetNew >= sizeVal) {
				throw std::out_of_range{ "Sorry, but that index is beyond the end of this string_view instance." };
			}
			return dataVal[offsetNew];
		}

		constexpr const_reference operator[](const size_type offsetNew) const {
			return dataVal[offsetNew];
		}

		constexpr const_reference front() const {
			return dataVal[0];
		}

		constexpr const_reference back() const {
			return dataVal[sizeVal - 1];
		}

		template<typename... ArgTypes> constexpr size_type find(ArgTypes&&... args) const {
			return this->operator std::basic_string_view<value_type>().find(std::forward<ArgTypes>(args)...);
		}

		template<typename... ArgTypes> constexpr size_type findFirstOf(ArgTypes&&... args) const {
			return this->operator std::basic_string_view<value_type>().find_first_of(std::forward<ArgTypes>(args)...);
		}

		template<typename... ArgTypes> constexpr size_type findLastOf(ArgTypes&&... args) const {
			return this->operator std::basic_string_view<value_type>().find_last_of(std::forward<ArgTypes>(args)...);
		}

		template<typename... ArgTypes> constexpr size_type findFirstNotOf(ArgTypes&&... args) const {
			return this->operator std::basic_string_view<value_type>().find_first_not_of(std::forward<ArgTypes>(args)...);
		}

		template<typename... ArgTypes> constexpr size_type findLastNotOf(ArgTypes&&... args) const {
			return this->operator std::basic_string_view<value_type>().find_last_not_of(std::forward<ArgTypes>(args)...);
		}

		constexpr void swap(string_view_base& other) {
			const string_view_base temp{ other };
			other = *this;
			*this = temp;
		}

		constexpr string_view_base substr(const size_type offsetNew = 0, size_type countNew = npos) const {
			if (offsetNew >= sizeVal) {
				throw std::out_of_range("Substring position is out of range.");
			}

			countNew = std::min(countNew, sizeVal - offsetNew);
			return string_view_base(dataVal + offsetNew, countNew);
		}

		template<typename value_type_newer = value_type> inline explicit operator std::basic_string<value_type_newer>() const {
			std::basic_string<value_type_newer> returnValue{};
			returnValue.resize(sizeVal);
			std::memcpy(returnValue.data(), data(), returnValue.size());
			return returnValue;
		}

		template<typename value_type_newer = value_type> inline explicit operator string_base<value_type_newer>() const {
			string_base<value_type_newer> returnValue{};
			returnValue.resize(sizeVal);
			std::memcpy(returnValue.data(), data(), returnValue.size());
			return returnValue;
		}

		constexpr explicit operator std::basic_string_view<value_type>() const {
			return { data(), size() };
		}

		template<jsonifier_internal::pointer_t value_type_newer>
		inline friend std::enable_if_t<!std::is_array_v<value_type_newer>, bool> operator==(const string_view_base& lhs, const value_type_newer& rhs) {
			auto rhsLength = traits_type::length(rhs);
			if (lhs.size() != rhsLength) {
				return false;
			}
			return jsonifier_internal::jsonifier_core_internal::compare(rhs, lhs.data(), rhsLength);
		}

		template<jsonifier_internal::char_array_t value_type_newer> inline bool operator==(const value_type_newer& rhs) {
			auto rhsLength = std::size(rhs) - 1;
			if (size() != rhsLength) {
				return false;
			}
			return jsonifier_internal::jsonifier_core_internal::compare(rhs, data(), rhsLength);
		}

		template<jsonifier_internal::string_t value_type_newer> inline friend bool operator==(const string_view_base& lhs, const value_type_newer& rhs) {
			if (rhs.size() != lhs.size()) {
				return false;
			}
			return jsonifier_internal::jsonifier_core_internal::compare(rhs.data(), lhs.data(), rhs.size());
		}

		template<typename value_type_newer, size_type N> inline friend string_base<value_type_new> operator+(const value_type_newer (&lhs)[N], const string_view_base& rhs) {
			string_base<value_type_new> newLhs{ lhs };
			newLhs += rhs;
			return newLhs;
		}

		template<typename value_type_newer, size_type N> inline friend string_base<value_type_new> operator+=(const value_type_newer (&lhs)[N], const string_view_base& rhs) {
			string_base<value_type_new> newLhs{ lhs };
			newLhs += rhs;
			return newLhs;
		}

		template<jsonifier_internal::pointer_t string_type_new> inline friend string_base<value_type_new> operator+(string_type_new&& lhs, const string_view_base& rhs) {
			string_base<value_type_new> newLhs{ lhs };
			newLhs += rhs;
			return newLhs;
		}

		template<jsonifier_internal::pointer_t string_type_new> inline friend string_base<value_type_new> operator+=(string_type_new&& lhs, const string_view_base& rhs) {
			string_base<value_type_new> newLhs{ lhs };
			newLhs += rhs;
			return newLhs;
		}

		inline string_base<value_type_new> operator+(const value_type& rhs) {
			string_base<value_type_new> newLhs{ *this };
			newLhs.pushBack(rhs);
			return newLhs;
		}

		inline string_base<value_type_new> operator+=(const value_type& rhs) {
			string_base<value_type_new> newLhs{ *this };
			newLhs.pushBack(rhs);
			return newLhs;
		}

		template<jsonifier_internal::string_t string_type_new> inline string_base<value_type_new> operator+(const string_type_new& rhs) const {
			string_base<value_type_new> newLhs{ *this };
			newLhs += rhs;
			return newLhs;
		}

		template<jsonifier_internal::string_t string_type_new> inline string_base<value_type_new> operator+=(const string_type_new& rhs) {
			string_base<value_type_new> newLhs{ *this };
			newLhs.append(rhs.data(), rhs.size());
			return newLhs;
		}

		template<typename value_type_newer, size_type N> inline string_base<value_type_new> operator+(const value_type_newer (&rhs)[N]) const {
			string_base<value_type_new> newLhs{ *this };
			newLhs += rhs;
			return newLhs;
		}

		template<typename value_type_newer, size_type N> inline string_base<value_type_new> operator+=(const value_type_newer (&rhs)[N]) {
			string_base newLhs{ *this };
			newLhs += rhs;
			return newLhs;
		}
	};

	using string_view = string_view_base<char>;

	inline std::ostream& operator<<(std::ostream& oStream, const jsonifier::string_view_base<char>& string) {
		oStream << string.operator jsonifier::string();
		return oStream;
	}

}// namespace jsonifier
