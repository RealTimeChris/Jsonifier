/*
	Jsonifier - For parsing and serializing Json - very rapidly.
	Copyright (C) 2023 Chris M. (RealTimeChris)

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
	Lesser General Public License for more details.

	You should have received a copy of the GNU Lesser General Public
	License along with this library; if not, write to the Free Software
	Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301
	USA
*/
/// https://github.com/RealTimeChris/Jsonifier
/// Feb 20, 2023
#pragma once

#include <jsonifier/Compare.hpp>
#include <memory_resource>

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

#ifdef __linux__
	#ifndef _tzcnt_u16
		#define _tzcnt_u16 __tzcnt_u16
	#endif
#endif

namespace JsonifierInternal {

	template<typename OTy, std::enable_if_t<std::is_move_constructible_v<OTy> && std::is_move_assignable_v<OTy>, int> = 0>
	constexpr void swapF(OTy& lhs, OTy& rhs) noexcept(std::is_nothrow_move_constructible_v<OTy>&& std::is_nothrow_move_assignable_v<OTy>) {
		OTy temp = std::move(lhs);
		lhs = std::move(rhs);
		rhs = std::move(temp);
	}

}

namespace Jsonifier {

	template<typename OTy> class Iterator {
	  public:
		using iterator_concept = std::bidirectional_iterator_tag;
		using value_type = OTy;
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

	template<typename OTy> class Vector {
	  public:
		using value_type = OTy;
		using pointer = value_type*;
		using const_pointer = const value_type*;
		using reference = value_type&;
		using const_reference = const value_type&;
		using iterator = Iterator<value_type>;
		using const_iterator = const iterator;
		using reverse_iterator = std::reverse_iterator<iterator>;
		using const_reverse_iterator = const reverse_iterator;
		using size_type = size_t;
		using difference_type = ptrdiff_t;
		using allocator = std::pmr::polymorphic_allocator<value_type>;

		inline constexpr Vector() noexcept = default;

		inline Vector& operator=(Vector&& other) noexcept {
			this->capacityVal = other.capacityVal;
			other.capacityVal = 0;
			this->sizeVal = other.sizeVal;
			other.sizeVal = 0;
			this->values = other.values;
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
				this->values[x] = std::move(initList.begin()[x]);
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

		inline constexpr void emplace_back(const value_type& c) {
			if (sizeVal + 1 >= capacityVal) {
				reserve(capacityVal * 2 + 2);
			}
			allocator{}.construct(&values[sizeVal++], c);
		}

		inline constexpr void emplace_back(value_type&& c) {
			if (sizeVal + 1 >= capacityVal) {
				reserve(capacityVal * 2 + 2);
			}
			allocator{}.construct(&values[sizeVal++], std::forward<value_type>(c));
		}

		inline constexpr void resize(size_t newSize, value_type fillChar = value_type{}) {
			if (newSize > sizeVal) {
				if (newSize > capacityVal) {
					reserve(newSize);
				}
				for (size_t x = sizeVal; x < newSize; ++x) {
					allocator{}.construct(&values[x], fillChar);
				}
				sizeVal = newSize;
			} else if (newSize < sizeVal) {
				sizeVal = newSize;
			}
		}

		inline constexpr void reserve(size_t newCapacity) {
			if (newCapacity > capacityVal) {
				value_type* newBuffer = allocator{}.allocate(newCapacity);
				std::copy(values, values + sizeVal, newBuffer);
				allocator{}.deallocate(values, capacityVal);
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
				allocator{}.deallocate(values, capacityVal);
			}
		};

	  protected:
		size_type capacityVal{};
		value_type* values{};
		size_type sizeVal{};
	};

}
