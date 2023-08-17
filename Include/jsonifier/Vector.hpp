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

#include <jsonifier/Allocator.hpp>
#include <jsonifier/Iterator.hpp>
#include <jsonifier/Compare.hpp>
#include <memory_resource>
#include <source_location>

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
}

namespace Jsonifier {

	template<typename ValueType> class Vector : public JsonifierInternal::AllocWrapper<ValueType> {
	  public:
		using value_type = ValueType;
		using pointer = value_type*;
		using const_pointer = const value_type*;
		using reference = value_type&;
		using const_reference = const value_type&;
		using iterator = JsonifierInternal::Iterator<value_type>;
		using const_iterator = const iterator;
		using size_type = size_t;
		using allocator = JsonifierInternal::AllocWrapper<value_type>;

		constexpr Vector() noexcept = default;

		inline Vector& operator=(Vector&& other) noexcept {
			JsonifierInternal::swapF(capacityVal, other.capacityVal);
			JsonifierInternal::swapF(sizeVal, other.sizeVal);
			JsonifierInternal::swapF(values, other.values);
			return *this;
		}

		inline Vector(Vector&& other) noexcept {
			*this = std::move(other);
		}

		constexpr Vector& operator=(std::initializer_list<value_type> initList) {
			resize(initList.size());
			for (size_type x = 0; x < sizeVal; ++x) {
				values[x] = std::move(initList.begin()[x]);
			}
			return *this;
		}

		constexpr explicit Vector(std::initializer_list<value_type> initList) {
			*this = initList;
		}

		inline explicit Vector(value_type&& other, size_type sizeNew) noexcept {
			resize(sizeNew);
			std::fill(data(), data() + sizeVal, other);
		}

		inline Vector& operator=(const Vector& other) noexcept {
			if (this != &other) {
				reserve(other.size());
				for (size_type x = 0; x < capacityVal; ++x) {
					emplace_back(other[x]);
				}
			}
			return *this;
		}

		inline Vector(const Vector& other) noexcept {
			*this = other;
		}

		constexpr Vector& operator=(value_type other) noexcept {
			emplace_back(other);
			return *this;
		}

		inline Vector(value_type other) noexcept {
			*this = other;
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

		constexpr reference front() noexcept {
			return values[0];
		}

		constexpr reference back() noexcept {
			return values[sizeVal - 1];
		}

		constexpr reference operator[](size_type index) const noexcept {
			return values[index];
		}

		constexpr size_type capacity() const noexcept {
			return capacityVal;
		}

		constexpr size_type size() const noexcept {
			return sizeVal;
		}

		constexpr pointer data() const noexcept {
			return values;
		}

		constexpr size_type maxSize() const noexcept {
			return std::numeric_limits<size_type>::max() / sizeof(value_type);
		}

		constexpr void emplace_back(const_reference c) {
			if (sizeVal + 1 >= capacityVal) {
				reserve(capacityVal * 8 + 1);
			}
			allocator::construct(&values[sizeVal++], c);
		}

		constexpr void emplace_back(value_type&& c) {
			if (sizeVal + 1 >= capacityVal) {
				reserve(capacityVal * 8 + 1);
			}
			allocator::construct(&values[sizeVal++], std::forward<value_type>(c));
		}

		inline void resize(size_type sizeNew) {
			if (sizeNew > 0) {
				if (sizeNew > capacityVal) {
					pointer newPtr = allocator::allocate(sizeNew);
					try {
						if (values) {
							std::uninitialized_move(values, values + sizeVal, newPtr);
							allocator::deallocate(values, capacityVal);
						}
					} catch (...) {
						allocator::deallocate(newPtr, sizeNew);
						throw;
					}
					capacityVal = sizeNew;
					values = newPtr;
					std::uninitialized_default_construct(values + sizeVal, values + sizeVal + (sizeNew - sizeVal));
				} else if (sizeNew > sizeVal) {
					std::uninitialized_default_construct(values + sizeVal, values + sizeVal + (sizeNew - sizeVal));
				}
				sizeVal = sizeNew;
			} else {
				sizeVal = 0;
			}
		}

		inline void reserve(size_type capacityNew) {
			if (capacityNew > capacityVal) {
				pointer newPtr = allocator::allocate(capacityNew);
				try {
					if (values) {
						std::uninitialized_move(values, values + sizeVal, newPtr);
						allocator::deallocate(values, capacityVal);
					}
				} catch (...) {
					allocator::deallocate(newPtr, capacityNew);
					throw;
				}
				capacityVal = capacityNew;
				values = newPtr;
			}
		}

		constexpr void reset() {
			std::fill(values, values + capacityVal, value_type{});
		}

		constexpr bool operator==(const Vector<value_type>& rhs) const noexcept {
			if (rhs.size() != size()) {
				return false;
			}
			return JsonifierInternal::JsonifierCoreInternal::compare(rhs.data(), this->data(), this->size());
		}

		constexpr ~Vector() {
			if (values && capacityVal) {
				allocator::deallocate(values, capacityVal);
			}
		};

	  protected:
		size_type capacityVal{};
		size_type sizeVal{};
		pointer values{};
	};

}
