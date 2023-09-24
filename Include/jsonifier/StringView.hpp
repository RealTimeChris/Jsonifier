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

	template<typename value_type_new> class string_view_base : public string_op_base<string_view_base<value_type_new>, value_type_new> {
	  public:
		using value_type			 = value_type_new;
		using pointer				 = value_type*;
		using const_pointer			 = const value_type*;
		using reference				 = value_type&;
		using const_reference		 = const value_type&;
		using iterator				 = jsonifier_internal::iterator<string_view_base::value_type>;
		using const_iterator		 = jsonifier_internal::iterator<const string_view_base::value_type>;
		using reverse_iterator		 = std::reverse_iterator<iterator>;
		using const_reverse_iterator = std::reverse_iterator<const_iterator>;
		using size_type				 = uint64_t;
		using allocator				 = jsonifier_internal::alloc_wrapper<value_type>;
		using traits_type			 = jsonifier_internal::char_traits<value_type>;

		const_pointer dataVal{};
		size_type sizeVal{};

		friend string_op_base<string_view_base, value_type>;

		static constexpr auto npos{ std::numeric_limits<size_type>::max() };

		constexpr string_view_base() noexcept : dataVal(), sizeVal(0) {
		}

		template<jsonifier_internal::has_data_and_size value_type_newer> constexpr string_view_base& operator=(const value_type_newer& stringNew) {
			dataVal = stringNew.data();
			sizeVal = stringNew.size();
			return *this;
		}

		template<jsonifier_internal::has_data_and_size value_type_newer> constexpr string_view_base(const value_type_newer& stringNew) {
			*this = stringNew;
		}

		template<uint64_t strLength> constexpr string_view_base(const value_type (&other)[strLength]) {
			dataVal = other;
			sizeVal = strLength;
		}

		constexpr string_view_base(const_pointer pointerNew) noexcept : dataVal(pointerNew), sizeVal(traits_type::length(pointerNew)) {
		}

		constexpr string_view_base(const_pointer pointerNew, const size_type countNew) noexcept : dataVal(pointerNew), sizeVal(countNew){};

		constexpr const_iterator begin() noexcept {
			return const_iterator(dataVal);
		}

		constexpr const_iterator begin() const noexcept {
			return const_iterator(dataVal);
		}

		constexpr const_iterator end() noexcept {
			return const_iterator(dataVal + sizeVal);
		}

		constexpr const_iterator end() const noexcept {
			return const_iterator(dataVal + sizeVal);
		}

		constexpr reverse_iterator rbegin() noexcept {
			return reverse_iterator(end());
		}

		constexpr const_reverse_iterator rbegin() const noexcept {
			return const_reverse_iterator(end());
		}

		constexpr reverse_iterator rend() noexcept {
			return reverse_iterator(begin());
		}

		constexpr const_reverse_iterator rend() const noexcept {
			return const_reverse_iterator(begin());
		}

		constexpr const_iterator cbegin() const noexcept {
			return begin();
		}

		constexpr const_iterator cend() const noexcept {
			return end();
		}

		constexpr const_reverse_iterator crbegin() const noexcept {
			return rbegin();
		}

		constexpr const_reverse_iterator crend() const noexcept {
			return rend();
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

		constexpr void swap(string_view_base& other) {
			const string_view_base temp{ other };
			other = *this;
			*this = temp;
		}

		constexpr string_view_base substr(const size_type offsetNew = 0, size_type countNew = npos) const {
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

		constexpr explicit operator std::string_view() const {
			return { data(), size() };
		}

		template<jsonifier_internal::has_data_and_size stringType> constexpr bool operator==(const stringType& rhs) const {
			if (rhs.size() != size()) {
				return false;
			}
			return jsonifier_internal::jsonifier_core_internal::compare(rhs.data(), data(), size());
		}

		template<size_type StrLength> constexpr bool operator==(const value_type (&rhs)[StrLength]) const {
			if (StrLength - 1 != size()) {
				return false;
			}
			return jsonifier_internal::jsonifier_core_internal::compare(rhs, data(), size());
		}

		template<jsonifier_internal::string_t stringTypeNew> inline friend string_base<value_type> operator+(const stringTypeNew& lhs, const string_view_base<value_type>& rhs) {
			string_base<value_type> newLhs{ lhs };
			newLhs += rhs;
			return newLhs;
		}

		template<jsonifier_internal::string_t stringTypeNew> inline friend string_base<value_type> operator+=(const stringTypeNew& lhs, const string_view_base<value_type>& rhs) {
			string_base<value_type> newLhs{ lhs };
			newLhs += rhs;
			return newLhs;
		}

		template<size_type size> inline friend string_base<value_type> operator+(const value_type (&lhs)[size], const string_view_base<value_type>& rhs) {
			string_base<value_type> newLhs{ lhs };
			newLhs += rhs;
			return newLhs;
		}

		template<size_type size> inline friend string_base<value_type> operator+=(const value_type (&lhs)[size], const string_view_base<value_type>& rhs) {
			string_base<value_type> newLhs{ lhs };
			newLhs += rhs;
			return newLhs;
		}

		template<uint64_t strLength> inline string_base<value_type> operator+(const value_type (&rhs)[strLength]) {
			string_base<value_type> newLhs{ *this };
			newLhs += rhs;
			return newLhs;
		}

		template<uint64_t strLength> inline string_base<value_type> operator+=(const value_type (&rhs)[strLength]) {
			string_base<value_type> newLhs{ *this };
			newLhs += rhs;
			return newLhs;
		}

		inline string_base<value_type> operator+(const value_type& rhs) {
			string_base<value_type> newLhs{ *this };
			newLhs.pushBack(rhs);
			return newLhs;
		}

		inline string_base<value_type> operator+=(const value_type& rhs) {
			string_base<value_type> newLhs{ *this };
			newLhs.pushBack(rhs);
			return newLhs;
		}

		template<jsonifier_internal::string_t stringTypeNew> inline string_base<value_type> operator+(const stringTypeNew& rhs) noexcept {
			string_base<value_type> newLhs{ *this };
			newLhs += rhs;
			return newLhs;
		}

		template<jsonifier_internal::string_t stringTypeNew> inline string_base<value_type> operator+=(const stringTypeNew& rhs) noexcept {
			string_base<value_type> newLhs{ *this };
			newLhs.append(rhs.data(), rhs.size());
			return newLhs;
		}
	};

	using string_view = string_view_base<char>;

	inline std::ostream& operator<<(std::ostream& oStream, const jsonifier::string_view_base<char>& string) {
		oStream << string.operator jsonifier::string();
		return oStream;
	}

}// namespace jsonifier
