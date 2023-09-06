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
/// https://github.com/RealTimeChris/Jsonifier
/// Feb 20, 2023
#pragma once

#include <jsonifier/String.hpp>

namespace Jsonifier {

	template<typename ValueType> class StringViewBase : public StringOpBase<StringViewBase<ValueType>, ValueType> {
	  public:
		using value_type			 = ValueType;
		using pointer				 = value_type*;
		using const_pointer			 = const value_type*;
		using reference				 = value_type&;
		using const_reference		 = const value_type&;
		using iterator				 = JsonifierInternal::Iterator<StringViewBase::value_type>;
		using const_iterator		 = JsonifierInternal::Iterator<const StringViewBase::value_type>;
		using reverse_iterator		 = std::reverse_iterator<iterator>;
		using const_reverse_iterator = std::reverse_iterator<const_iterator>;
		using size_type				 = uint64_t;
		using allocator				 = JsonifierInternal::AllocWrapper<value_type>;
		using traits_type			 = JsonifierInternal::CharTraits<value_type>;

		const_pointer dataVal{};
		size_type sizeVal{};

		friend StringOpBase<StringViewBase, value_type>;

		static constexpr auto npos{ std::numeric_limits<size_type>::max() };

		constexpr StringViewBase() noexcept : dataVal(), sizeVal(0) {
		}

		template<JsonifierInternal::HasDataAndSize ValueTypeNew> constexpr StringViewBase& operator=(const ValueTypeNew& stringNew) {
			dataVal = stringNew.data();
			sizeVal = stringNew.size();
			return *this;
		}

		template<JsonifierInternal::HasDataAndSize ValueTypeNew> constexpr StringViewBase(const ValueTypeNew& stringNew) {
			*this = stringNew;
		}

		template<uint64_t strLength> constexpr StringViewBase(const value_type (&other)[strLength]) {
			dataVal = other;
			sizeVal = strLength;
		}

		constexpr StringViewBase(const_pointer pointerNew) noexcept : dataVal(pointerNew), sizeVal(traits_type::length(pointerNew)) {
		}

		constexpr StringViewBase(const_pointer pointerNew, const size_type countNew) noexcept : dataVal(pointerNew), sizeVal(countNew){};

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
				throw std::out_of_range{ "Sorry, but that index is beyond the end of this StringView instance." };
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

		constexpr void swap(StringViewBase& other) {
			const StringViewBase temp{ other };
			other = *this;
			*this = temp;
		}

		constexpr StringViewBase substr(const size_type offsetNew = 0, size_type countNew = npos) const {
			return StringViewBase(dataVal + offsetNew, countNew);
		}

		template<typename ValueTypeNew = value_type> inline explicit operator std::basic_string<ValueTypeNew>() const {
			std::basic_string<ValueTypeNew> returnValue{};
			returnValue.resize(sizeVal);
			std::memcpy(returnValue.data(), data(), returnValue.size());
			return returnValue;
		}

		template<typename ValueTypeNew = value_type> inline explicit operator StringBase<ValueTypeNew>() const {
			StringBase<ValueTypeNew> returnValue{};
			returnValue.resize(sizeVal);
			std::memcpy(returnValue.data(), data(), returnValue.size());
			return returnValue;
		}

		constexpr explicit operator std::string_view() const {
			return { data(), size() };
		}

		template<JsonifierInternal::HasDataAndSize StringType> constexpr bool operator==(const StringType& rhs) const {
			if (rhs.size() != size()) {
				return false;
			}
			return JsonifierInternal::JsonifierCoreInternal::compare(rhs.data(), data(), size());
		}

		template<size_type StrLength> constexpr bool operator==(const value_type (&rhs)[StrLength]) const {
			if (StrLength - 1 != size()) {
				return false;
			}
			return JsonifierInternal::JsonifierCoreInternal::compare(rhs, data(), size());
		}

		template<JsonifierInternal::StringT StringTypeNew> inline friend StringBase<value_type> operator+(const StringTypeNew& lhs, const StringViewBase<value_type>& rhs) {
			StringBase<value_type> newLhs{ lhs };
			newLhs += rhs;
			return newLhs;
		}

		template<JsonifierInternal::StringT StringTypeNew> inline friend StringBase<value_type> operator+=(const StringTypeNew& lhs, const StringViewBase<value_type>& rhs) {
			StringBase<value_type> newLhs{ lhs };
			newLhs += rhs;
			return newLhs;
		}

		template<size_type size> inline friend StringBase<value_type> operator+(const value_type (&lhs)[size], const StringViewBase<value_type>& rhs) {
			StringBase<value_type> newLhs{ lhs };
			newLhs += rhs;
			return newLhs;
		}

		template<size_type size> inline friend StringBase<value_type> operator+=(const value_type (&lhs)[size], const StringViewBase<value_type>& rhs) {
			StringBase<value_type> newLhs{ lhs };
			newLhs += rhs;
			return newLhs;
		}

		template<uint64_t strLength> inline StringBase<value_type> operator+(const value_type (&rhs)[strLength]) {
			StringBase<value_type> newLhs{ *this };
			newLhs += rhs;
			return newLhs;
		}

		template<uint64_t strLength> inline StringBase<value_type> operator+=(const value_type (&rhs)[strLength]) {
			StringBase<value_type> newLhs{ *this };
			newLhs += rhs;
			return newLhs;
		}

		inline StringBase<value_type> operator+(const value_type& rhs) {
			StringBase<value_type> newLhs{ *this };
			newLhs.pushBack(rhs);
			return newLhs;
		}

		inline StringBase<value_type> operator+=(const value_type& rhs) {
			StringBase<value_type> newLhs{ *this };
			newLhs.pushBack(rhs);
			return newLhs;
		}

		template<JsonifierInternal::StringT StringTypeNew> inline StringBase<value_type> operator+(const StringTypeNew& rhs) noexcept {
			StringBase<value_type> newLhs{ *this };
			newLhs += rhs;
			return newLhs;
		}

		template<JsonifierInternal::StringT StringTypeNew> inline StringBase<value_type> operator+=(const StringTypeNew& rhs) noexcept {
			StringBase<value_type> newLhs{ *this };
			newLhs.append(rhs.data(), rhs.size());
			return newLhs;
		}

		constexpr ~StringViewBase() noexcept = default;
	};

	using StringView = StringViewBase<char>;

	inline std::ostream& operator<<(std::ostream& oStream, const Jsonifier::StringViewBase<char>& string) {
		oStream << string.operator Jsonifier::String();
		return oStream;
	}

}// namespace Jsonifier
