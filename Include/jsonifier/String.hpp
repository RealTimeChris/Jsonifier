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

#include <jsonifier/Vector.hpp>

#include <algorithm>
#include <string>

namespace JsonifierInternal {

	template<typename OTy1, typename OTy2> constexpr bool stringConstCompare(const OTy1& S0, const OTy2& S1) noexcept {
		if (S0.size() != S1.size()) [[unlikely]] {
			return false;
		}
		using CharType = OTy1::value_type;
		for (size_t x = 0; x < S0.size(); ++x) {
			if (S0[x] != static_cast<CharType>(S1[x])) [[unlikely]] {
				return false;
			}
		}
		return true;
	}

	template<typename ValueType>
	concept HasSize = requires(ValueType value) {
		{ value.size() };
	};

	template<typename ValueType>
	concept HasData = requires(ValueType value) {
		{ value.data() };
	};

	template<typename ValueType>
	concept HasDataAndSize = HasData<ValueType> && HasSize<ValueType>;
}

namespace Jsonifier {


	template<typename ValueType> class StringViewBase;

	template<typename ValueType> class StringBase : public JsonifierInternal::AllocWrapper<ValueType> {
	  public:
		using value_type = ValueType;
		using traits_type = std::char_traits<char>;
		using pointer = value_type*;
		using const_pointer = const value_type*;
		using reference = value_type&;
		using const_reference = const value_type&;
		using iterator = JsonifierInternal::Iterator<value_type>;
		using const_iterator = const iterator;
		using reverse_iterator = std::reverse_iterator<iterator>;
		using const_reverse_iterator = const reverse_iterator;
		using size_type = size_t;
		using difference_type = ptrdiff_t;
		using allocator = JsonifierInternal::AllocWrapper<value_type>;

		constexpr StringBase() noexcept : capacityVal{ 0 }, sizeVal{ 0 }, values{ nullptr } {};

		inline static constexpr size_type npos{ std::numeric_limits<size_type>::max() };

		constexpr StringBase& operator=(StringBase&& other) noexcept {
			if (this != &other) {
				JsonifierInternal::swapF(capacityVal, other.capacityVal);
				JsonifierInternal::swapF(sizeVal, other.sizeVal);
				JsonifierInternal::swapF(values, other.values);
			}
			return *this;
		}

		inline explicit StringBase(StringBase&& other) noexcept {
			*this = std::move(other);
		}

		constexpr StringBase& operator=(const StringBase& other) noexcept {
			if (this != &other) {
				auto sizeNew = other.size();
				if (sizeNew) [[likely]] {
					reserve(sizeNew);
					std::copy(other.data(), other.data() + sizeNew, values);
					sizeVal = sizeNew;
				}
			}
			return *this;
		}

		constexpr StringBase(const StringBase& other) noexcept {
			*this = other;
		}

		constexpr StringBase(value_type other) noexcept {
			*this = other;
		}

		constexpr StringBase& operator=(const std::string& other) noexcept {
			auto sizeNew = other.size();
			if (sizeNew) [[likely]] {
				reserve(sizeNew);
				std::copy(other.data(), other.data() + sizeNew, values);
				sizeVal = sizeNew;
			}
			return *this;
		}

		constexpr StringBase(const std::string& other) noexcept {
			*this = other;
		}

		constexpr StringBase& operator=(const std::string_view& other) noexcept {
			auto sizeNew = other.size();
			if (sizeNew) [[likely]] {
				reserve(sizeNew);
				std::copy(other.data(), other.data() + sizeNew, values);
				sizeVal = sizeNew;
			}
			return *this;
		}

		constexpr StringBase(const std::string_view& other) noexcept {
			*this = other;
		}

		template<size_t strLength> constexpr StringBase(const value_type (&other)[strLength]) {
			if (strLength) [[likely]] {
				reserve(strLength);
				std::copy(other, other + strLength, values);
				sizeVal = strLength;
			}
		}

		constexpr StringBase& operator=(const_pointer other) noexcept {
			auto sizeNew = traits_type::length(other);
			if (sizeNew) [[likely]] {
				reserve(sizeNew);
				std::copy(other, other + sizeNew, values);
				sizeVal = sizeNew;
			}
			return *this;
		}

		constexpr StringViewBase<value_type> stringView(size_type offSet, size_type count) noexcept {
			return { values + offSet, count };
		}

		constexpr StringBase(const_pointer other) noexcept {
			*this = other;
		}

		constexpr StringBase(const_pointer other, size_t sizeNew) noexcept {
			if (sizeNew) [[likely]] {
				reserve(sizeNew);
				std::copy(other, other + sizeNew, values);
				sizeVal = sizeNew;
			}
		}

		constexpr StringBase substr(size_type pos, size_type count) {
			if (pos + count >= sizeVal) {
				return StringBase{};
			}
			StringBase result{};
			result.reserve(count);
			std::copy(values + pos, values + pos + count * sizeof(value_type), result.values);
			result.sizeVal = count;
			allocator::construct(&result.values[count], '\0');
			return result;
		}

		inline void erase(size_type count) {
			if (count > sizeVal || count == 0) {
				return;
			}
			traits_type::move(values, values + count, sizeVal - count);
			sizeVal -= count;
			values[sizeVal] = '\0';
		}

		constexpr auto& back() const noexcept {
			return *(values + sizeVal);
		}

		constexpr auto& front() const noexcept {
			return *values;
		}

		constexpr const_iterator begin() const noexcept {
			return const_iterator(values, 0);
		}

		constexpr const_iterator end() const noexcept {
			return const_iterator(values, sizeVal);
		}

		constexpr iterator begin() noexcept {
			return iterator(values, 0);
		}

		constexpr iterator end() noexcept {
			return iterator(values, sizeVal);
		}

		constexpr void pushBack(value_type value) {
			if (sizeVal + 1 >= capacityVal) {
				reserve((sizeVal + 2) * 4);
			}
			allocator::construct(&values[sizeVal++], value);
			allocator::construct(&values[sizeVal], '\0');
		}

		constexpr value_type& operator[](size_type index) noexcept {
			return values[index];
		}

		constexpr value_type& operator[](size_type index) const noexcept {
			return values[index];
		}

		constexpr operator std::string() const {
			std::string newString{};
			newString.resize(size());
			std::memcpy(newString.data(), data(), size());
			return newString;
		}

		constexpr void clear() noexcept {
			sizeVal = 0;
		}

		constexpr size_type maxSize() const {
			return std::numeric_limits<size_type>::max();
		}

		inline void resize(size_type sizeNew) {
			if (sizeNew > 0) [[likely]] {
				if (sizeNew > capacityVal) [[likely]] {
					pointer newPtr = allocator::allocate(sizeNew + 1);
					try {
						if (values) [[likely]] {
							std::uninitialized_move(values, values + sizeVal, newPtr);
							allocator::deallocate(values, capacityVal + 1);
						}
					} catch (...) {
						allocator::deallocate(newPtr, sizeNew + 1);
						throw;
					}
					capacityVal = sizeNew;
					values = newPtr;
					std::uninitialized_default_construct(values + sizeVal, values + capacityVal);
				} else if (sizeNew > sizeVal) [[unlikely]] {
					std::uninitialized_default_construct(values + sizeVal, values + capacityVal);
				}
				sizeVal = sizeNew;
				allocator::construct(&values[sizeVal], '\0');
			} else {
				sizeVal = 0;
			}
		}

		inline void reserve(size_type capacityNew) {
			if (capacityNew > capacityVal) [[likely]] {
				pointer newPtr = allocator::allocate(capacityNew + 1);
				try {
					if (values) [[likely]] {
						std::uninitialized_move(values, values + sizeVal, newPtr);
						allocator::deallocate(values, capacityVal + 1);
					}
				} catch (...) {
					allocator::deallocate(newPtr, capacityNew + 1);
					throw;
				}
				capacityVal = capacityNew;
				values = newPtr;
			}
		}

		constexpr size_type capacity() const noexcept {
			return capacityVal;
		}

		constexpr size_type size() const noexcept {
			return sizeVal;
		}

		constexpr bool empty() const noexcept {
			return sizeVal == 0;
		}

		constexpr operator StringViewBase<value_type>() {
			return { values, sizeVal };
		}

		constexpr pointer data() const noexcept {
			return values;
		}

		template<JsonifierInternal::HasDataAndSize StringType> constexpr bool operator==(const StringType& rhs) const noexcept {
			if (rhs.size() != size()) {
				return false;
			}
			return JsonifierInternal::JsonifierCoreInternal::compare(rhs.data(), data(), rhs.size());
		}

		template<JsonifierInternal::HasDataAndSize StringType> constexpr bool operator!=(const StringType& rhs) const noexcept {
			return !(*this == rhs);
		}

		template<size_type StrLength> constexpr bool operator==(const value_type (&rhs)[StrLength]) const noexcept {
			if (StrLength != size()) {
				return false;
			}
			return JsonifierInternal::JsonifierCoreInternal::compare(rhs, data(), StrLength);
		}

		template<typename StringType> constexpr StringBase& operator+=(const StringType& rhs) noexcept {
			auto oldSize = size();
			resize(oldSize + rhs.size());
			std::copy(rhs.data(), rhs.data() + rhs.size(), data() + oldSize);
			return *this;
		}

		constexpr StringBase& operator+=(const_pointer rhs) noexcept {
			auto oldSize = size();
			auto rhsSize = traits_type::length(rhs);
			resize(oldSize + rhsSize);
			std::copy(rhs, rhs + rhsSize, data() + oldSize);
			return *this;
		}

		constexpr StringBase operator+(const StringBase& rhs) noexcept {
			StringBase newString(*this);
			newString += rhs;
			return newString;
		}

		constexpr StringBase operator+(const_pointer rhs) noexcept {
			StringBase newString(*this);
			newString += rhs;
			return newString;
		}

		constexpr ~StringBase() {
			if (values && capacityVal > 0) {
				allocator::deallocate(values, capacityVal + 1);
				values = nullptr;
				capacityVal = 0;
			}
		};

	  protected:
		size_type capacityVal{};
		size_type sizeVal{};
		pointer values{};
	};

	template<typename ValueType, typename Traits, typename SizeType> std::basic_ostream<ValueType, Traits>& insertString(
		std::basic_ostream<ValueType, Traits>& oStream, const ValueType* const data, const SizeType size) {
		using OstreamType = std::basic_ostream<ValueType, Traits>;
		typename OstreamType::iostate state = OstreamType::goodbit;

		SizeType pad;
		if (oStream.width() <= 0 || static_cast<SizeType>(oStream.width()) <= size) {
			pad = 0;
		} else {
			pad = static_cast<SizeType>(oStream.width()) - size;
		}

		const typename OstreamType::sentry ok(oStream);

		if (!ok) {
			state |= OstreamType::badbit;
		} else {
			try {
				if ((oStream.flags() & OstreamType::adjustfield) != OstreamType::left) {
					for (; 0 < pad; --pad) {
						if (Traits::eq_int_type(Traits::eof(), oStream.rdbuf()->sputc(oStream.fill()))) {
							state |= OstreamType::badbit;
							break;
						}
					}
				}

				if (state == OstreamType::goodbit &&
					oStream.rdbuf()->sputn(data, static_cast<std::streamsize>(size)) != static_cast<std::streamsize>(size)) {
					state |= OstreamType::badbit;
				} else {
					for (; 0 < pad; --pad) {
						if (Traits::eq_int_type(Traits::eof(), oStream.rdbuf()->sputc(oStream.fill()))) {
							state |= OstreamType::badbit;
							break;
						}
					}
				}
				oStream.width(0);
			} catch (...) {
				oStream.setstate(OstreamType::badbit);
			}
		}
		oStream.setstate(state);
		return oStream;
	}

	using String = StringBase<char>;

	template<typename ValueType> inline std::ostream& operator<<(std::ostream& os, const StringBase<ValueType>& string) {
		os << string.operator typename std::string();
		return os;
	}

	inline String operator+(const char* lhs, const String& rhs) noexcept {
		String newString(lhs);
		newString += rhs;
		return newString;
	}

	template<typename ValueType> inline std::string operator+=(const std::string& lhs, const StringBase<ValueType>& rhs) noexcept {
		String newString(lhs);
		newString += rhs;
		return newString;
	}
}
