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
		using const_iterator		 = jsonifier_internal::iterator<const value_type>;
		using const_reverse_iterator = std::reverse_iterator<const_iterator>;
		using size_type				 = uint64_t;
		using traits_type			 = jsonifier_internal::char_traits<value_type>;

		static constexpr size_type npos{ std::numeric_limits<size_type>::max() };

		constexpr string_view_base() : dataVal(), sizeVal(0) {
		}

		template<typename value_type_newer> constexpr string_view_base& operator=(const string_base<value_type_newer>& stringNew) {
			dataVal = stringNew.data();
			sizeVal = stringNew.size();
			return *this;
		}

		template<typename value_type_newer> constexpr string_view_base(const string_base<value_type_newer>& stringNew) {
			*this = stringNew;
		}

		template<typename value_type_newer, jsonifier::concepts::same_character_size<value_type>> constexpr string_view_base& operator=(const value_type_newer& stringNew) {
			dataVal = stringNew.data();
			sizeVal = stringNew.size();
			return *this;
		}

		template<typename value_type_newer, jsonifier::concepts::same_character_size<value_type>> constexpr string_view_base(const value_type_newer& stringNew) {
			*this = stringNew;
		}

		constexpr string_view_base(const_pointer pointerNew) : dataVal(pointerNew), sizeVal(traits_type::length(pointerNew)) {
		}

		constexpr string_view_base(const_pointer pointerNew, const size_type countNew) : dataVal(pointerNew), sizeVal(countNew){};

		constexpr const_iterator begin() noexcept {
			return const_iterator{ dataVal };
		}

		constexpr const_iterator end() noexcept {
			return const_iterator{ dataVal + sizeVal };
		}

		constexpr const_reverse_iterator rbegin() noexcept {
			return const_reverse_iterator{ end() };
		}

		constexpr const_reverse_iterator rend() noexcept {
			return const_reverse_iterator{ begin() };
		}

		constexpr const_iterator begin() const noexcept {
			return const_iterator{ dataVal };
		}

		constexpr const_iterator end() const noexcept {
			return const_iterator{ dataVal + sizeVal };
		}

		constexpr const_reverse_iterator rbegin() const noexcept {
			return const_reverse_iterator{ end() };
		}

		constexpr const_reverse_iterator rend() const noexcept {
			return const_reverse_iterator{ begin() };
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

		inline size_type find(const_pointer args, size_type position = 0) const {
			auto newSize = traits_type::length(args);
			if (position + newSize > sizeVal) {
				return npos;
			}
			auto foundValue = jsonifier_internal::find(dataVal + position, sizeVal - position, args, newSize);
			return foundValue == npos ? npos : foundValue + position;
		}

		template<jsonifier::concepts::char_type value_type_newer> inline size_type find(value_type_newer args, size_type position = 0) const {
			value_type newValue{ static_cast<value_type>(args) };
			if (position + 1 > sizeVal) {
				return npos;
			}
			auto foundValue = jsonifier_internal::find(dataVal + position, sizeVal - position, &newValue, 1);
			return foundValue == npos ? npos : foundValue + position;
		}

		template<jsonifier::concepts::string_t value_type_newer> inline size_type find(const value_type_newer& args, size_type position = 0) const {
			if (position + args.size() > sizeVal) {
				return npos;
			}
			auto foundValue = jsonifier_internal::find(dataVal + position, sizeVal - position, args.data(), args.size());
			return foundValue == npos ? npos : foundValue + position;
		}

		template<typename... arg_types> constexpr size_type findFirstOf(arg_types&&... args) const {
			return operator std::basic_string_view<value_type>().find_first_of(std::forward<arg_types>(args)...);
		}

		template<typename... arg_types> constexpr size_type findLastOf(arg_types&&... args) const {
			return operator std::basic_string_view<value_type>().find_last_of(std::forward<arg_types>(args)...);
		}

		template<typename... arg_types> constexpr size_type findFirstNotOf(arg_types&&... args) const {
			return operator std::basic_string_view<value_type>().find_first_not_of(std::forward<arg_types>(args)...);
		}

		template<typename... arg_types> constexpr size_type findLastNotOf(arg_types&&... args) const {
			return operator std::basic_string_view<value_type>().find_last_not_of(std::forward<arg_types>(args)...);
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
			if (sizeVal > 0) {
				std::memcpy(returnValue.data(), data(), returnValue.size());
			}
			return returnValue;
		}

		template<typename value_type_newer = value_type> inline explicit operator string_base<value_type_newer>() const {
			string_base<value_type_newer> returnValue{};
			returnValue.resize(sizeVal);
			if (sizeVal > 0) {
				std::memcpy(returnValue.data(), data(), returnValue.size());
			}
			return returnValue;
		}

		constexpr explicit operator std::basic_string_view<value_type>() const {
			return { data(), size() };
		}

		template<jsonifier::concepts::pointer_t value_type_newer>
		inline friend std::enable_if_t<!std::is_array_v<value_type_newer>, bool> operator==(const string_view_base& lhs, const value_type_newer& rhs) {
			auto rhsLength = traits_type::length(rhs);
			if (lhs.size() != rhsLength) {
				return false;
			}
			return jsonifier_internal::jsonifier_core_internal::compare(rhs, lhs.data(), rhsLength);
		}

		template<jsonifier::concepts::string_t value_type_newer> inline friend bool operator==(const string_view_base& lhs, const value_type_newer& rhs) {
			if (rhs.size() != lhs.size()) {
				return false;
			}
			return jsonifier_internal::jsonifier_core_internal::compare(rhs.data(), lhs.data(), rhs.size());
		}

		template<typename value_type_newer, size_type size>
		inline friend string_base<value_type_newer> operator+(const value_type_newer (&lhs)[size], const string_view_base& rhs) {
			string_base<value_type_newer> newLhs{ lhs };
			newLhs += rhs;
			return newLhs;
		}

		template<typename value_type_newer, size_type size>
		inline friend string_base<value_type_newer> operator+=(const value_type_newer (&lhs)[size], const string_view_base& rhs) {
			string_base<value_type_newer> newLhs{ lhs };
			newLhs += rhs;
			return newLhs;
		}

		template<jsonifier::concepts::pointer_t string_type_newer> inline friend string_base<string_type_newer> operator+(string_type_newer&& lhs, const string_view_base& rhs) {
			string_base<std::remove_pointer_t<string_type_newer>> newLhs{ lhs };
			newLhs += rhs;
			return newLhs;
		}

		template<jsonifier::concepts::pointer_t string_type_newer> inline friend string_base<string_type_newer> operator+=(string_type_newer&& lhs, const string_view_base& rhs) {
			string_base<std::remove_pointer_t<string_type_newer>> newLhs{ lhs };
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

		template<jsonifier::concepts::string_t string_type_new> inline string_base<value_type_new> operator+(const string_type_new& rhs) const {
			string_base<value_type_new> newLhs{ *this };
			newLhs += rhs;
			return newLhs;
		}

		template<jsonifier::concepts::string_t string_type_new> inline string_base<value_type_new> operator+=(const string_type_new& rhs) {
			string_base<value_type_new> newLhs{ *this };
			newLhs.append(rhs.data(), rhs.size());
			return newLhs;
		}

		template<typename value_type_newer, size_type size> inline string_base<value_type_newer> operator+(const value_type_newer (&rhs)[size]) const {
			string_base<value_type_newer> newLhs{ *this };
			newLhs += rhs;
			return newLhs;
		}

		template<typename value_type_newer, size_type size> inline string_base<value_type_newer> operator+=(const value_type_newer (&rhs)[size]) {
			string_base<value_type_newer> newLhs{ *this };
			newLhs += rhs;
			return newLhs;
		}

	  protected:
		const_pointer dataVal{};
		size_type sizeVal{};
	};

	using string_view = string_view_base<char>;

	inline std::ostream& operator<<(std::ostream& oStream, const jsonifier::string_view_base<char>& string) {
		oStream << string.operator jsonifier::string();
		return oStream;
	}

}// namespace jsonifier

namespace jsonifier_internal {

	template<typename value_type> struct hash<jsonifier::string_view_base<value_type>> {
		constexpr uint64_t operator()(jsonifier::string_view_base<uint8_t> value) const {
			return fnv1aHash(value);
		}

		constexpr uint64_t operator()(jsonifier::string_view_base<char> value) const {
			return fnv1aHash(value);
		}

		constexpr uint64_t operator()(jsonifier::string_view_base<uint8_t> value, uint64_t seed) const {
			return fnv1aHash(value, seed);
		}

		constexpr uint64_t operator()(jsonifier::string_view_base<char> value, uint64_t seed) const {
			return fnv1aHash(value, seed);
		}
	};
}

namespace std {

	template<> struct hash<jsonifier::string_view> {
		inline size_t operator()(jsonifier ::string_view lhs) const {
			return jsonifier_internal::fnv1aHash(lhs);
		}
	};
}
