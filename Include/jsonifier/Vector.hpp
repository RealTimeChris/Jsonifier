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
	inline void swapF(ValueType& lhs, ValueType& rhs) noexcept(
		std::is_nothrow_move_constructible_v<ValueType>&& std::is_nothrow_move_assignable_v<ValueType>) {
		ValueType temp = std::move(lhs);
		lhs = std::move(rhs);
		rhs = std::move(temp);
	}
}

namespace Jsonifier {

	template<typename ValueType> class Vector : protected std::equal_to<ValueType> {
	  public:
		using value_type			 = ValueType;
		using pointer				 = value_type*;
		using const_pointer			 = const value_type*;
		using reference				 = value_type&;
		using const_reference		 = const value_type&;
		using iterator               = JsonifierInternal::Iterator<Vector>;
		using const_iterator         = JsonifierInternal::ConstIterator<Vector>;
		using reverse_iterator       = std::reverse_iterator<iterator>;
		using difference_type		 = std::ptrdiff_t;
		using const_reverse_iterator = std::reverse_iterator<const_iterator>;
		using object_compare		 = std::equal_to<value_type>;
		using size_type				 = uint64_t;
		using allocator				 = JsonifierInternal::AllocWrapper<value_type>;

		inline Vector() noexcept = default;

		inline Vector& operator=(Vector&& other) noexcept {
			if (this != &other) {
				reset();
				swap(other);
			}
			return *this;
		}

		inline Vector(Vector&& other) noexcept {
			*this = std::move(other);
		}

		inline Vector& operator=(const Vector& other) {
			if (this != &other) {
				reset();
				resize(other.size());
				for (size_type x = 0; x < capacityVal; ++x) {
					dataVal[x] = other[x];
				}
			}
			return *this;
		}

		inline Vector(const Vector& other) {
			*this = other;
		}

		inline Vector& operator=(std::vector<value_type>&& initList) {
			reset();
			resize(initList.size());
			for (size_type x = 0; x < sizeVal; ++x) {
				dataVal[x] = std::move(initList[x]);
			}
			return *this;
		}

		inline explicit Vector(std::vector<value_type>&& initList) {
			*this = std::move(initList);
		}

		inline Vector& operator=(const std::vector<value_type>& initList) {
			reset();
			resize(initList.size());
			for (size_type x = 0; x < sizeVal; ++x) {
				dataVal[x] = initList[x];
			}
			return *this;
		}

		inline explicit Vector(const std::vector<value_type>& initList) {
			*this = initList;
		}

		inline Vector& operator=(std::initializer_list<value_type> initList) {
			reset();
			resize(initList.size());
			for (size_type x = 0; x < sizeVal; ++x) {
				dataVal[x] = std::move(initList.begin()[x]);
			}
			return *this;
		}

		template<typename InputIterator> void insert(iterator where, InputIterator first, InputIterator last) {
			size_type insertCount = std::distance(first, last);

			if (insertCount == 0) {
				return;
			}

			size_type insertPosIndex = where - begin();
			size_type newSize = sizeVal + insertCount;

			if (newSize > capacityVal) {
				reserve(newSize);
			}

			pointer insertPos = dataVal + insertPosIndex;

			std::memmove(insertPos + insertCount, insertPos, (sizeVal - insertPosIndex) * sizeof(value_type));

			for (InputIterator it = first; it != last; ++it) {
				alloc.construct(insertPos++, *it);
			}

			sizeVal = newSize;
		}

		inline explicit Vector(std::initializer_list<value_type> initList) {
			*this = initList;
		}

		inline explicit Vector(value_type&& other, size_type sizeNew) {
			reset();
			resize(sizeNew);
			std::fill(data(), data() + sizeVal, other);
		}

		inline Vector& operator=(value_type other) {
			emplace_back(other);
			return *this;
		}

		inline Vector(value_type other) {
			*this = other;
		}

		 inline iterator begin() noexcept {
			return iterator(dataVal);
		}

		inline const_iterator begin() const noexcept {
			return const_iterator(dataVal);
		}

		inline iterator end() noexcept {
			return iterator(dataVal + sizeVal);
		}

		inline const_iterator end() const noexcept {
			return const_iterator(dataVal + sizeVal);
		}

		inline reverse_iterator rbegin() noexcept {
			return reverse_iterator(end());
		}

		inline const_reverse_iterator rbegin() const noexcept {
			return const_reverse_iterator(end());
		}

		inline reverse_iterator rend() noexcept {
			return reverse_iterator(begin());
		}

		inline const_reverse_iterator rend() const noexcept {
			return const_reverse_iterator(begin());
		}

		inline const_iterator cbegin() const noexcept {
			return begin();
		}

		inline const_iterator cend() const noexcept {
			return end();
		}

		inline const_reverse_iterator crbegin() const noexcept {
			return rbegin();
		}

		inline const_reverse_iterator crend() const noexcept {
			return rend();
		}

		inline reference front() {
			return dataVal[0];
		}

		inline reference back() {
			return dataVal[sizeVal - 1];
		}

		inline reference at(size_type index) {
			if (index >= sizeVal) {
				throw std::out_of_range{ "Sorry, but that index is beyong the end of this Vector's bounds." };
			}
			return dataVal[index];
		}

		inline const_reference at(size_type index) const {
			if (index >= sizeVal) {
				throw std::out_of_range{ "Sorry, but that index is beyong the end of this Vector's bounds." };
			}
			return dataVal[index];
		}

		inline const_reference operator[](size_type index) const {
			return dataVal[index];
		}

		inline reference operator[](size_type index) {
			return dataVal[index];
		}

		inline size_type capacity() const {
			return capacityVal;
		}

		inline size_type size() const {
			return sizeVal;
		}

		inline pointer data() const {
			return dataVal;
		}

		inline pointer data() {
			return dataVal;
		}

		inline explicit operator std::vector<value_type>() const {
			std::vector<value_type> returnValue{};
			for (auto& value: *this) {
				returnValue.emplace_back(value);
			}
			return returnValue;
		}

		inline void erase(size_type count) {
			if (count > sizeVal || count == 0) {
				return;
			}
			std::memmove(dataVal, dataVal + count, sizeVal - count);
			sizeVal -= count;
		}

		inline void erase(iterator iter) {
			if (iter >= end()) {
				return;
			}

			size_type eraseIndex = iter - begin();
			size_type newSize = sizeVal - 1;

			std::memmove(dataVal + eraseIndex, dataVal + eraseIndex + 1, (sizeVal - eraseIndex - 1) * sizeof(value_type));

			sizeVal = newSize;
		}


		inline void erase(iterator iter01, iterator iter02) {
			if (iter01 >= iter02 || iter01 < begin() || iter02 > end()) {
				return;
			}

			size_type sizeCount = iter02 - iter01;
			size_type pos1 = iter01 - begin();
			size_type elementsAfterErase = sizeVal - (pos1 + sizeCount);

			if (sizeCount > sizeVal || sizeCount == 0) {
				return;
			}

			if (elementsAfterErase < sizeCount) {
				std::copy(dataVal + pos1 + sizeCount, dataVal + sizeVal, dataVal + pos1);
			} else {
				std::copy(dataVal + pos1 + sizeCount, dataVal + pos1 + sizeCount + elementsAfterErase, dataVal + pos1);
			}

			sizeVal -= sizeCount;
		}

		inline size_type maxSize() const {
			return std::numeric_limits<size_type>::max() / sizeof(value_type);
		}

		template<typename... ValueTypes> inline reference emplace_back(ValueTypes&&... c) {
			if (sizeVal + 1 >= capacityVal) {
				reserve(capacityVal * 2 + 2);
			}
			alloc.construct(&dataVal[sizeVal++], std::forward<ValueTypes>(c)...);

			return dataVal[sizeVal - 1];
		}

		inline void shrinkToFit() {
			if (sizeVal == capacityVal) {
				return;
			}

			capacityVal = sizeVal;
			pointer newValues = alloc.allocate(capacityVal);
			std::uninitialized_move(dataVal, dataVal + sizeVal, newValues);
			alloc.deallocate(dataVal, sizeVal);
			dataVal = newValues;
		}

		inline void resize(size_type sizeNew) {
			if (sizeNew > 0) {
				if (sizeNew > capacityVal) {
					pointer newPtr = alloc.allocate(sizeNew);
					try {
						if (dataVal) {
							std::uninitialized_move(dataVal, dataVal + sizeVal, newPtr);
							alloc.deallocate(dataVal, capacityVal);
						}
					} catch (...) {
						alloc.deallocate(newPtr, sizeNew);
						throw;
					}
					capacityVal = sizeNew;
					dataVal = newPtr;
				}
				std::uninitialized_value_construct(dataVal + sizeVal, dataVal + capacityVal);
				sizeVal = sizeNew;
			} else {
				sizeVal = 0;
			}
		}

		inline void reserve(size_type capacityNew) {
			if (capacityNew > capacityVal) {
				pointer newPtr = alloc.allocate(capacityNew);
				try {
					if (dataVal) {
						std::uninitialized_move(dataVal, dataVal + sizeVal, newPtr);
						alloc.deallocate(dataVal, capacityVal);
					}
				} catch (...) {
					alloc.deallocate(newPtr, capacityNew);
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

		inline void swap(Vector& other) {
			std::swap(capacityVal, other.capacityVal);
			std::swap(sizeVal, other.sizeVal);
			std::swap(dataVal, other.dataVal);
		}

		inline bool operator==(const Vector<value_type>& rhs) const {
			if (rhs.size() != size()) {
				return false;
			}
			if constexpr (std::is_fundamental_v<value_type>) {
				return JsonifierInternal::JsonifierCoreInternal::compare(rhs.data(), data(), size());
			} else {
				for (size_type x = 0; x < sizeVal; ++x) {
					if (!getObjectComparitor()(rhs.dataVal[x], dataVal[x])) {
						return false;
					}
				}
			}
			return true;
		}

		inline ~Vector() {
			reset();
		};

	  protected:
		size_type capacityVal{};
		size_type sizeVal{};
		pointer dataVal{};

		const object_compare& getObjectComparitor() const {
			return *this;
		}

		inline void reset() {
			if (dataVal && capacityVal) {
				if (sizeVal) {
					std::destroy(dataVal, dataVal + sizeVal);
					sizeVal = 0;
				}
				alloc.deallocate(dataVal, capacityVal);
				dataVal = nullptr;
				capacityVal = 0;
			}
		}

		static constexpr allocator alloc{};
	};

}
