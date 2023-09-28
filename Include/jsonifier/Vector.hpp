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

#include <jsonifier/Allocator.hpp>
#include <jsonifier/Compare.hpp>
#include <jsonifier/Iterator.hpp>
#include <source_location>
#include <cstring>

namespace jsonifier {

	template<typename value_type_new> class vector : protected std::equal_to<value_type_new>, protected jsonifier_internal::alloc_wrapper<value_type_new> {
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
		using object_compare		 = std::equal_to<value_type>;
		using size_type				 = uint64_t;
		using allocator				 = jsonifier_internal::alloc_wrapper<value_type>;

		inline vector() noexcept = default;

		inline vector& operator=(vector&& other) noexcept {
			if (this != &other && dataVal != other.dataVal) {
				reset();
				swap(other);
			}
			return *this;
		}

		inline vector(vector&& other) noexcept : capacityVal{}, sizeVal{}, dataVal{} {
			*this = std::move(other);
		}

		inline vector& operator=(const vector& other) {
			if (this != &other) {
				reset();
				auto sizeValNew = other.size();
				reserve(sizeValNew);
				std::uninitialized_copy(other.data(), other.data() + sizeValNew, dataVal);
				sizeVal = sizeValNew;
			}
			return *this;
		}

		inline vector(const vector& other) : capacityVal{}, sizeVal{}, dataVal{} {
			*this = other;
		}

		inline vector& operator=(std::vector<value_type>&& other) {
			reset();
			auto sizeValNew = other.size();
			reserve(sizeValNew);
			std::uninitialized_move(other.data(), other.data() + sizeValNew, dataVal);
			sizeVal = sizeValNew;
			return *this;
		}

		inline explicit vector(std::vector<value_type>&& other) : capacityVal{}, sizeVal{}, dataVal{} {
			*this = std::move(other);
		}

		inline vector& operator=(const std::vector<value_type>& other) {
			reset();
			auto sizeValNew = other.size();
			reserve(sizeValNew);
			std::uninitialized_copy(other.data(), other.data() + sizeValNew, dataVal);
			sizeVal = sizeValNew;
			return *this;
		}

		inline explicit vector(const std::vector<value_type>& other) : capacityVal{}, sizeVal{}, dataVal{} {
			*this = other;
		}

		inline vector& operator=(std::initializer_list<value_type> other) {
			reset();
			auto sizeValNew = other.size();
			reserve(sizeValNew);
			std::uninitialized_move(other.begin(), other.begin() + sizeValNew, dataVal);
			sizeVal = sizeValNew;
			return *this;
		}

		inline explicit vector(std::initializer_list<value_type> other) : capacityVal{}, sizeVal{}, dataVal{} {
			*this = other;
		}

		inline explicit vector(value_type&& other, size_type sizeNew) : capacityVal{}, sizeVal{}, dataVal{} {
			reset();
			auto sizeValNew = sizeNew;
			reserve(sizeValNew);
			std::uninitialized_fill(data(), data() + sizeValNew, other);
			sizeVal = sizeValNew;
		}

		inline vector& operator=(value_type other) {
			emplace_back(other);
			return *this;
		}

		inline vector(value_type other) : capacityVal{}, sizeVal{}, dataVal{} {
			*this = other;
		}

		template<typename InputIterator> inline void insert(iterator where, InputIterator first, InputIterator last) {
			size_type insertCount = std::distance(first, last);

			if (insertCount == 0) {
				return;
			}

			size_type insertPosIndex = where - begin();
			size_type newSize		 = sizeVal + insertCount;

			if (newSize > capacityVal) {
				reserve(newSize);
			}

			pointer insertPos = dataVal + insertPosIndex;

			for (InputIterator it = first; it != last; ++it) {
				getAlloc().construct(insertPos++, *it);
			}

			sizeVal = newSize;
		}

		template<typename value_type_newer> inline void insert(iterator where, value_type_newer&& value) {
			size_type insertCount = 1;

			if (insertCount == 0) {
				return;
			}

			size_type insertPosIndex = where - begin();
			size_type newSize		 = sizeVal + insertCount;

			if (newSize > capacityVal) {
				reserve(newSize);
			}

			pointer insertPos = dataVal + insertPosIndex;

			getAlloc().construct(insertPos++, value);

			sizeVal = newSize;
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
				throw std::out_of_range{ "Sorry, but that index is beyond the end of this vector's bounds." };
			}
			return dataVal[index];
		}

		inline const_reference at(size_type index) const {
			if (index >= sizeVal) {
				throw std::out_of_range{ "Sorry, but that index is beyond the end of this vector's bounds." };
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

		inline size_type maxSize() const {
			return std::numeric_limits<size_type>::max() / sizeof(value_type);
		}

		template<typename... value_types> inline reference emplace_back(value_types&&... c) {
			if (sizeVal + 1 >= capacityVal) {
				reserve(capacityVal * 2 + 2);
			}
			getAlloc().construct(&dataVal[sizeVal++], std::forward<value_types>(c)...);

			return dataVal[sizeVal - 1];
		}

		inline void erase(size_type count) {
			if (count >= sizeVal) {
				return;
			}
			size_type newSize = sizeVal - count;

			std::destroy(dataVal, dataVal + count);

			for (size_type i = 0; i < newSize; ++i) {
				dataVal[i] = std::move(dataVal[i + count]);
			}

			sizeVal = newSize;
		}

		inline void erase(iterator iter) {
			if (iter < begin() || iter >= end()) {
				return;
			}

			size_type eraseIndex = static_cast<size_type>(iter - begin());
			size_type newSize	 = sizeVal - 1;

			getAlloc().destroy(dataVal + eraseIndex);

			for (size_type i = eraseIndex; i < newSize; ++i) {
				dataVal[i] = std::move(dataVal[i + 1]);
			}

			sizeVal = newSize;
		}

		inline void shrinkToFit() {
			if (sizeVal == capacityVal) {
				return;
			}

			auto oldCapacity  = capacityVal;
			capacityVal		  = sizeVal;
			pointer newValues = getAlloc().allocate(capacityVal);
			std::uninitialized_move(dataVal, dataVal + sizeVal, newValues);
			getAlloc().deallocate(dataVal, oldCapacity);
			dataVal = newValues;
		}

		inline void resize(size_type sizeNew) {
			if (sizeNew > 0) [[likely]] {
				if (sizeNew > capacityVal) {
					pointer newPtr = getAlloc().allocate(sizeNew);
					try {
						if (dataVal) {
							std::uninitialized_move(dataVal, dataVal + sizeVal, newPtr);
							getAlloc().deallocate(dataVal, capacityVal);
						}
					} catch (...) {
						getAlloc().deallocate(newPtr, sizeNew);
						throw;
					}
					capacityVal = sizeNew;
					dataVal		= newPtr;
					std::uninitialized_value_construct(dataVal + sizeVal, dataVal + capacityVal);
				} else if (sizeNew > sizeVal) {
					std::uninitialized_value_construct(dataVal + sizeVal, dataVal + capacityVal);
				} else if (sizeNew < sizeVal) {
					std::destroy(dataVal + sizeNew, dataVal + sizeVal);
				}
				sizeVal = sizeNew;
			} else {
				sizeVal = 0;
			}
		}

		inline void reserve(size_t capacityValNew) {
			if (capacityValNew > capacityVal) [[likely]] {
				pointer newPtr = getAlloc().allocate(capacityValNew);
				try {
					if (dataVal && sizeVal) [[likely]] {
						std::uninitialized_move(dataVal, dataVal + sizeVal, newPtr);
						getAlloc().deallocate(dataVal, capacityVal);
					}
				} catch (...) {
					getAlloc().deallocate(newPtr, capacityValNew);
					throw;
				}

				capacityVal = capacityValNew;
				dataVal		= newPtr;
			}
		}

		inline void clear() {
			std::destroy(dataVal, dataVal + sizeVal);
			sizeVal = 0;
		}

		inline void swap(vector& other) {
			std::swap(capacityVal, other.capacityVal);
			std::swap(sizeVal, other.sizeVal);
			std::swap(dataVal, other.dataVal);
		}

		inline bool operator==(const vector<value_type>& rhs) const {
			if (rhs.size() != size()) {
				return false;
			}
			if constexpr (!std::is_fundamental_v<value_type>) {
				for (size_type x = 0; x < sizeVal; ++x) {
					if (!getObjectComparitor()(rhs.dataVal[x], dataVal[x])) {
						return false;
					}
				}
				return true;
			} else {
				return jsonifier_internal::jsonifier_core_internal::compare(rhs.data(), data(), size());
			}
		}

		inline ~vector() {
			reset();
		};

	  protected:
		size_type capacityVal{};
		size_type sizeVal{};
		pointer dataVal{};

		inline const object_compare& getObjectComparitor() const {
			return *this;
		}

		inline allocator& getAlloc() {
			return *this;
		}

		inline void reset() {
			if (dataVal && capacityVal) {
				if (sizeVal) {
					std::destroy(dataVal, dataVal + sizeVal);
					sizeVal = 0;
				}
				getAlloc().deallocate(dataVal, capacityVal);
				dataVal		= nullptr;
				capacityVal = 0;
			}
		}
	};

}// namespace jsonifier
