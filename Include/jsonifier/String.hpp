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

	template<typename OTy1, typename OTy2> constexpr bool stringConstCompare(const OTy1& S0, const OTy2& S1) {
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

	template<typename ValueType> class StringBase {
	  public:
		using value_type			 = ValueType;
		using pointer				 = value_type*;
		using const_pointer			 = const value_type*;
		using reference				 = value_type&;
		using const_reference		 = const value_type&;
		using iterator               = JsonifierInternal::Iterator<StringBase>;
		using const_iterator         = JsonifierInternal::ConstIterator<StringBase>;
		using reverse_iterator       = std::reverse_iterator<iterator>;
		using difference_type		 = std::ptrdiff_t;
		using const_reverse_iterator = std::reverse_iterator<const_iterator>;
		using object_compare		 = std::equal_to<value_type>;
		using size_type				 = uint64_t;
		using allocator				 = JsonifierInternal::AllocWrapper<value_type>;
		using traits_type			 = std::char_traits<value_type>;

		inline StringBase() noexcept : capacityVal{ 0 }, sizeVal{ 0 }, dataVal{ nullptr } {};

		inline static size_type npos{ std::numeric_limits<size_type>::max() };

		inline StringBase& operator=(StringBase&& other) noexcept {
			if (this != &other) {
				JsonifierInternal::swapF(capacityVal, other.capacityVal);
				JsonifierInternal::swapF(sizeVal, other.sizeVal);
				JsonifierInternal::swapF(dataVal, other.dataVal);
			}
			return *this;
		}

		inline explicit StringBase(StringBase&& other) noexcept {
			*this = std::move(other);
		}

		inline StringBase& operator=(const StringBase& other) {
			if (this != &other) {
				auto sizeNew = other.size();
				if (sizeNew) [[likely]] {
					reserve(sizeNew);
					std::copy(other.data(), other.data() + sizeNew, dataVal);
					sizeVal = sizeNew;
				}
			}
			return *this;
		}

		inline StringBase(const StringBase& other) {
			*this = other;
		}

		inline StringBase(value_type other) {
			*this = other;
		}

		inline StringBase& operator=(const std::string& other) {
			auto sizeNew = other.size();
			if (sizeNew) [[likely]] {
				reserve(sizeNew);
				std::copy(other.data(), other.data() + sizeNew, dataVal);
				sizeVal = sizeNew;
			}
			return *this;
		}

		inline StringBase(const std::string& other) {
			*this = other;
		}

		inline StringBase& operator=(const std::string_view& other) {
			auto sizeNew = other.size();
			if (sizeNew) [[likely]] {
				reserve(sizeNew);
				std::copy(other.data(), other.data() + sizeNew, dataVal);
				sizeVal = sizeNew;
			}
			return *this;
		}

		inline StringBase(const std::string_view& other) {
			*this = other;
		}

		template<size_t strLength> inline StringBase(const value_type (&other)[strLength]) {
			if (strLength) [[likely]] {
				reserve(strLength);
				std::copy(other, other + strLength, dataVal);
				sizeVal = strLength;
			}
		}

		inline StringBase& operator=(const_pointer other) {
			auto sizeNew = traits_type::length(other);
			if (sizeNew) [[likely]] {
				reserve(sizeNew);
				std::copy(other, other + sizeNew, dataVal);
				sizeVal = sizeNew;
			}
			return *this;
		}

		inline StringBase(const_pointer other) {
			*this = other;
		}

		inline StringBase(const_pointer other, size_t sizeNew) {
			if (sizeNew) [[likely]] {
				reserve(sizeNew);
				std::copy(other, other + sizeNew, dataVal);
				sizeVal = sizeNew;
			}
		}

		inline const_iterator cbegin() const {
			return const_iterator(dataVal);
		}

		inline const_iterator cend() const {
			return const_iterator(dataVal + sizeVal);
		}

		inline iterator begin() const {
			return iterator(dataVal);
		}

		inline iterator end() const {
			return iterator(dataVal + sizeVal);
		}

		inline const_reverse_iterator crbegin() const {
			return reverse_iterator(end());
		}

		inline const_reverse_iterator crend() const {
			return reverse_iterator(begin());
		}

		inline reverse_iterator rbegin() const {
			return reverse_iterator(end());
		}

		inline reverse_iterator rend() const {
			return reverse_iterator(begin());
		}

		inline StringViewBase<value_type> stringView(size_type offSet, size_type count) {
			return { dataVal + offSet, count };
		}

		inline StringBase substr(size_type pos, size_type count) {
			if (pos + count >= sizeVal) {
				return StringBase{};
			}
			StringBase result{};
			result.reserve(count);
			std::copy(dataVal + pos, dataVal + pos + count * sizeof(value_type), result.dataVal);
			result.sizeVal = count;
			alloc.construct(&result.dataVal[count], '\0');
			return result;
		}

		inline void erase(size_type count) {
			if (count > sizeVal || count == 0) {
				return;
			}
			traits_type::move(dataVal, dataVal + count, sizeVal - count);
			sizeVal -= count;
			alloc.construct(&dataVal[sizeVal], '\0');
		}

		inline auto& back() const {
			return *(dataVal + sizeVal);
		}

		inline auto& front() const {
			return *dataVal;
		}

		inline void pushBack(value_type value) {
			if (sizeVal + 1 >= capacityVal) {
				reserve((sizeVal + 2) * 4);
			}
			alloc.construct(&dataVal[sizeVal++], value);
			alloc.construct(&dataVal[sizeVal], '\0');
		}

		inline value_type& operator[](size_type index) {
			return dataVal[index];
		}

		inline value_type& operator[](size_type index) const {
			return dataVal[index];
		}

		inline operator std::string() const {
			std::string newString{};
			newString.resize(size());
			std::memcpy(newString.data(), data(), size());
			return newString;
		}

		inline size_type maxSize() const {
			return std::numeric_limits<size_type>::max() / sizeof(value_type);
		}

		inline void resize(size_type sizeNew) {
			if (sizeNew > 0) {
				if (sizeNew > capacityVal) {
					pointer newPtr = alloc.allocate(sizeNew + 1);
					try {
						if (dataVal) {
							std::uninitialized_move(dataVal, dataVal + sizeVal, newPtr);
							alloc.deallocate(dataVal, capacityVal + 1);
						}
					} catch (...) {
						alloc.deallocate(newPtr, sizeNew + 1);
						throw;
					}
					capacityVal = sizeNew;
					dataVal = newPtr;
				}
				std::uninitialized_value_construct(dataVal + sizeVal, dataVal + capacityVal);
				sizeVal = sizeNew;
				alloc.construct(&dataVal[sizeVal], '\0');
			} else {
				sizeVal = 0;
			}
		}

		inline void reserve(size_type capacityNew) {
			if (capacityNew > capacityVal) {
				pointer newPtr = alloc.allocate(capacityNew + 1);
				try {
					if (dataVal) {
						std::uninitialized_move(dataVal, dataVal + sizeVal, newPtr);
						alloc.deallocate(dataVal, capacityVal + 1);
					}
				} catch (...) {
					alloc.deallocate(newPtr, capacityNew + 1);
					throw;
				}
				capacityVal = capacityNew;
				dataVal = newPtr;
			}
		}

		inline void clear() {
			std::destroy(dataVal, dataVal + sizeVal);
			sizeVal = 0;
		}

		inline size_type capacity() const {
			return capacityVal;
		}

		inline size_type size() const {
			return sizeVal;
		}

		inline bool empty() const {
			return sizeVal == 0;
		}

		inline operator StringViewBase<value_type>() const {
			return { dataVal, sizeVal };
		}

		inline pointer data() const {
			return dataVal;
		}

		template<JsonifierInternal::HasDataAndSize StringType> inline bool operator==(const StringType& rhs) const {
			if (rhs.size() != size()) {
				return false;
			}
			return JsonifierInternal::JsonifierCoreInternal::compare(rhs.data(), data(), rhs.size());
		}

		template<JsonifierInternal::HasDataAndSize StringType> inline bool operator!=(const StringType& rhs) const {
			return !(*this == rhs);
		}

		template<size_type StrLength> inline bool operator==(const value_type (&rhs)[StrLength]) const {
			if (StrLength != size()) {
				return false;
			}
			return JsonifierInternal::JsonifierCoreInternal::compare(rhs, data(), StrLength);
		}

		template<typename StringType> inline StringBase& operator+=(const StringType& rhs) {
			auto oldSize = size();
			resize(oldSize + rhs.size());
			std::copy(rhs.data(), rhs.data() + rhs.size(), data() + oldSize);
			return *this;
		}

		inline StringBase& operator+=(const_pointer rhs) {
			auto oldSize = size();
			auto rhsSize = traits_type::length(rhs);
			resize(oldSize + rhsSize);
			std::copy(rhs, rhs + rhsSize, data() + oldSize);
			return *this;
		}

		inline StringBase operator+(const StringBase& rhs) {
			StringBase newString(*this);
			newString += rhs;
			return newString;
		}

		inline StringBase operator+(const_pointer rhs) {
			StringBase newString(*this);
			newString += rhs;
			return newString;
		}

		inline ~StringBase() {
			if (dataVal && capacityVal > 0) {
				alloc.deallocate(dataVal, capacityVal + 1);
				dataVal = nullptr;
				capacityVal = 0;
			}
		};

	  protected:
		size_type capacityVal{};
		size_type sizeVal{};
		pointer dataVal{};

		static constexpr allocator alloc{};
	};

	template<typename ValueType, typename Traits, typename SizeType> std::basic_ostream<ValueType, Traits>& insertString(
		std::basic_ostream<ValueType, Traits>& oStream, const ValueType* const dataVal, const SizeType size) {
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
					oStream.rdbuf()->sputn(dataVal, static_cast<std::streamsize>(size)) != static_cast<std::streamsize>(size)) {
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

	inline String operator+(const char* lhs, const String& rhs) {
		String newString(lhs);
		newString += rhs;
		return newString;
	}

	template<typename ValueType> inline std::string operator+=(const std::string& lhs, const StringBase<ValueType>& rhs) {
		String newString(lhs);
		newString += rhs;
		return newString;
	}
}
