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

#include <jsonifier/Compare.hpp>
#include <memory_resource>

#ifndef NO_UNIQUE_ADDRESS
	#ifdef __has_cpp_attribute
		#if __has_cpp_attribute(no_unique_address)
			#ifdef _MSC_VER
				#define NO_UNIQUE_ADDRESS [[msvc::no_unique_address]]
			#else
				#define NO_UNIQUE_ADDRESS [[no_unique_address]]
			#endif
		#else
			#define NO_UNIQUE_ADDRESS
		#endif
	#else
		#define NO_UNIQUE_ADDRESS
	#endif
#endif

#ifdef __linux__
	#ifndef _tzcnt_u16
		#define _tzcnt_u16 __tzcnt_u16
	#endif
#endif

namespace JsonifierInternal {

	template<typename ValueType, std::enable_if_t<std::is_move_constructible_v<ValueType> && std::is_move_assignable_v<ValueType>, int> = 0>
	constexpr void swapF(ValueType& lhs, ValueType& rhs) noexcept(
		std::is_nothrow_move_constructible_v<ValueType>&& std::is_nothrow_move_assignable_v<ValueType>) {
		ValueType temp = std::move(lhs);
		lhs = std::move(rhs);
		rhs = std::move(temp);
	}

	template<class PtrType> inline constexpr auto unfancy(PtrType ptr) noexcept {// converts from a fancy pointer to a plain pointer
		return std::addressof(*ptr);
	}

	template<class Type> struct IsCharacter : std::false_type {};

	template<> struct IsCharacter<char> : std::true_type {};

	template<> struct IsCharacter<signed char> : std::true_type {};

	template<> struct IsCharacter<unsigned char> : std::true_type {};

#ifdef __cpp_char8_t
	template<> struct IsCharacter<char8_t> : std::true_type {};
#endif

	template<class Type> struct IsCharacterOrBool : IsCharacter<Type>::type {};

	template<> struct IsCharacterOrBool<bool> : std::true_type {};

	template<class Type> struct IsCharacterOrByteOrBool : IsCharacterOrBool<Type>::type {};

}

namespace Jsonifier {

	template<typename ValueType> class Iterator {
	  public:
		using iterator_concept = std::bidirectional_iterator_tag;
		using value_type = ValueType;
		using difference_type = ptrdiff_t;
		using pointer = const value_type*;
		using reference = const value_type&;
		using size_type = size_t;

		inline constexpr Iterator() noexcept = default;

		inline constexpr Iterator(const pointer _Data, const size_type _Size, const size_type offsetNew) noexcept
			: dataVal(_Data), offset(offsetNew) {
		}

		inline constexpr reference operator*() const noexcept {
			return dataVal[offset];
		}

		inline constexpr pointer operator->() const noexcept {
			return dataVal + offset;
		}

		inline constexpr Iterator& operator++() noexcept {
			++offset;
			return *this;
		}

		inline constexpr Iterator operator++(int32_t) noexcept {
			Iterator temp{ *this };
			++*this;
			return temp;
		}

		inline constexpr Iterator& operator--() noexcept {
			--offset;
			return *this;
		}

		inline constexpr Iterator operator--(int32_t) noexcept {
			Iterator temp{ *this };
			--*this;
			return temp;
		}

		inline constexpr Iterator& operator+=(const difference_type offsetNew) noexcept {
			offset += static_cast<size_type>(offsetNew);
			return *this;
		}

		inline constexpr Iterator operator+(const difference_type offsetNew) const noexcept {
			Iterator temp{ *this };
			temp += offsetNew;
			return temp;
		}

		inline constexpr Iterator operator+(const difference_type offsetNew) noexcept {
			*this += offsetNew;
			return *this;
		}

		inline constexpr Iterator& operator-=(const difference_type offsetNew) noexcept {
			offset -= static_cast<size_type>(offsetNew);
			return *this;
		}

		inline constexpr Iterator operator-(const difference_type offsetNew) const noexcept {
			Iterator temp{ *this };
			temp -= offsetNew;
			return temp;
		}

		inline constexpr difference_type operator-(const Iterator& rhs) const noexcept {
			return static_cast<difference_type>(offset - rhs.offset);
		}

		inline constexpr reference operator[](const difference_type offsetNew) const noexcept {
			return *(*this + offsetNew);
		}

		inline constexpr bool operator==(const Iterator& rhs) const noexcept {
			return offset == rhs.offset;
		}

		inline constexpr bool operator!=(const Iterator& rhs) const noexcept {
			return !(*this == rhs);
		}

		inline constexpr bool operator<(const Iterator& rhs) const noexcept {
			return offset < rhs.offset;
		}

		inline constexpr bool operator>(const Iterator& rhs) const noexcept {
			return rhs < *this;
		}

		inline constexpr bool operator<=(const Iterator& rhs) const noexcept {
			return !(rhs < *this);
		}

		inline constexpr bool operator>=(const Iterator& rhs) const noexcept {
			return !(*this < rhs);
		}

	  protected:
		pointer dataVal = nullptr;
		size_type offset = 0;
	};

	template<typename ValueType> class Vector {
	  public:
		using value_type = ValueType;
		using pointer = value_type*;
		using const_pointer = const value_type*;
		using reference = value_type&;
		using iterator = Iterator<value_type>;
		using const_iterator = const iterator;
		using size_type = size_t;
		using allocator_type = std::pmr::polymorphic_allocator<value_type>;

		inline constexpr Vector() noexcept = default;

		inline Vector& operator=(Vector&& other) noexcept {
			capacityVal = other.capacityVal;
			other.capacityVal = 0;
			sizeVal = other.sizeVal;
			other.sizeVal = 0;
			values = other.values;
			other.values = nullptr;
			return *this;
		}

		inline Vector(Vector&& other) noexcept {
			*this = std::move(other);
		}

		inline constexpr Vector& operator=(const std::initializer_list<value_type>& initList) {
			resize(initList.size());
			std::memcpy(data(), initList.begin(), sizeof(value_type) * sizeVal);
			return *this;
		}

		inline constexpr explicit Vector(const std::initializer_list<value_type>& initList) {
			*this = initList;
		}

		inline constexpr Vector& operator=(std::initializer_list<value_type>&& initList) {
			resize(initList.size());
			for (size_t x = 0; x < sizeVal; ++x) {
				values[x] = std::move(initList.begin()[x]);
			}
			return *this;
		}

		inline constexpr explicit Vector(std::initializer_list<value_type>&& initList) {
			*this = std::forward<std::initializer_list<value_type>>(initList);
		}

		inline explicit Vector(value_type&& other, size_type sizeNew) noexcept {
			resize(sizeNew);
			std::fill(data(), data() + sizeVal, other);
		}

		inline Vector& operator=(const Vector& other) noexcept {
			if (this != &other) {
				reserve(other.size());
				for (size_t x = 0; x < capacityVal; ++x) {
					emplace_back(other[x]);
				}
			}
			return *this;
		}

		inline Vector(const Vector& other) noexcept {
			*this = other;
		}

		inline constexpr Vector& operator=(value_type other) noexcept {
			emplace_back(other);
			return *this;
		}

		inline Vector(value_type other) noexcept {
			*this = other;
		}

		inline constexpr const_iterator begin() const noexcept {
			return const_iterator(values, sizeVal, 0);
		}

		inline constexpr const_iterator end() const noexcept {
			return const_iterator(values, sizeVal, sizeVal);
		}

		inline constexpr iterator begin() noexcept {
			return iterator(values, sizeVal, 0);
		}

		inline constexpr iterator end() noexcept {
			return iterator(values, sizeVal, sizeVal);
		}

		inline constexpr reference front() noexcept {
			return values[0];
		}

		inline constexpr reference back() noexcept {
			return values[sizeVal - 1];
		}

		inline constexpr reference operator[](size_type index) const noexcept {
			return values[index];
		}

		inline constexpr size_type capacity() const noexcept {
			return capacityVal;
		}

		inline constexpr size_type size() const noexcept {
			return sizeVal;
		}

		inline constexpr pointer data() const noexcept {
			return values;
		}

		inline constexpr size_type maxSize() const noexcept {
			return std::numeric_limits<size_t>::max() / sizeof(value_type);
		}

		inline constexpr void emplace_back(const value_type& c) {
			if (sizeVal + 1 >= capacityVal) {
				reserve(capacityVal * 8 + 1);
			}
			alloc.construct(&values[sizeVal++], c);
		}

		inline constexpr void emplace_back(value_type&& c) {
			if (sizeVal + 1 >= capacityVal) {
				reserve(capacityVal * 8 + 1);
			}
			alloc.construct(&values[sizeVal++], std::forward<value_type>(c));
		}

		inline constexpr void resize(size_t newSize) {
			value_type fillChar{};
			resize(newSize, fillChar);
		}

		inline constexpr void reserve(size_t newCapacity) {
			if (newCapacity > capacityVal) {
				value_type* newBuffer = alloc.allocate(newCapacity);
				std::memcpy(newBuffer, values, sizeVal);
				alloc.deallocate(values, capacityVal);
				values = newBuffer;
				capacityVal = newCapacity;
			}
		}

		inline constexpr void reset() {
			std::fill(values, values + capacityVal, value_type{});
		}

		inline constexpr bool operator==(const Vector& rhs) noexcept {
			if (rhs.size() != size()) {
				return false;
			}
			for (size_type x = 0; x < rhs.size(); ++x) {
				if (operator[](x) != rhs[x]) {
					return false;
				}
			}
			return true;
		}

		inline constexpr bool operator!=(const Vector& rhs) noexcept {
			return !(rhs == *this);
		}

		inline constexpr ~Vector() {
			if (values && capacityVal) {
				alloc.deallocate(values, capacityVal);
			}
		};

	  protected:
		NO_UNIQUE_ADDRESS allocator_type alloc{};
		size_type capacityVal{};
		size_type sizeVal{};
		pointer values{};

		inline constexpr void destroyRange(pointer firstVal, const_pointer last) noexcept {
			if constexpr (!std::is_trivially_destructible<value_type>::value) {
				for (; firstVal != last; ++firstVal) {
					alloc.destroy(JsonifierInternal::unfancy(firstVal));
				}
			}
		}

		inline constexpr void changeArray(const pointer newVector, const size_type newSize, const size_type newCapacity) {
			if (values) {
				destroyRange(values, values + sizeVal);
				alloc.deallocate(values, sizeVal);
			}
			values = newVector;
			capacityVal = newCapacity;
			sizeVal = newSize;
		}

		inline constexpr auto fillMemset(pointer dest, const value_type newValue, const size_t count) {
			std::memset(static_cast<void*>(dest), static_cast<unsigned char>(newValue), count);
			return dest + count;
		}

		inline constexpr auto fillMemsetZero(pointer dest, const size_t count) {
			std::memset(static_cast<void*>(dest), 0, count);
			return dest + count;
		}

		inline constexpr void resizeReallocate(const size_type newSize) {
			if (newSize > maxSize()) {
				throw std::runtime_error{ "Sorry, but that length is too long." };
			}
			const value_type value{};

			pointer& firstVal = values;

			const auto oldSize = sizeVal;
			const size_type newCapacity = newSize;
			const pointer newVector = alloc.allocate(newCapacity);
			const pointer appendedFirst = newVector + oldSize;
			pointer appendedLast = appendedFirst;

			try {
				if constexpr (JsonifierInternal::IsCharacterOrByteOrBool<value_type>::value) {
					appendedLast = fillMemset(appendedFirst, value, newSize - oldSize);
				} else {
					appendedLast = fillMemsetZero(appendedFirst, newSize - oldSize);
				}

				if constexpr (std::is_nothrow_move_constructible_v<ValueType> || !std::is_copy_constructible_v<ValueType>) {
					std::memmove(newVector, firstVal, sizeVal);
				} else {
					std::memcpy(newVector, firstVal, sizeVal);
				}
			} catch (...) {
				destroyRange(appendedFirst, appendedLast);
				alloc.deallocate(newVector, newCapacity);
			}

			changeArray(newVector, newSize, newCapacity);
		}

		inline constexpr void resize(const size_type newSize, const value_type& value) {
			pointer& firstVal = values;
			const auto oldSize = sizeVal;
			if (newSize < oldSize) {
				const pointer newLast = firstVal + newSize;
				destroyRange(newLast, (firstVal + sizeVal));
				sizeVal = newSize;
				return;
			}

			if (newSize > oldSize) {
				const auto oldCapacity = capacityVal;
				if (newSize > oldCapacity) {
					resizeReallocate(newSize);
					sizeVal = newSize;
					return;
				}
				sizeVal = newSize;
			}
		}
	};

}
