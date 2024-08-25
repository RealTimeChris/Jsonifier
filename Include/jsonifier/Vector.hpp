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
#include <jsonifier/Iterator.hpp>
#include <jsonifier/Compare.hpp>

namespace jsonifier {

	template<typename value_type_new, uint64_t sizeValNewer> class vector : protected std::equal_to<value_type_new>, protected jsonifier_internal::alloc_wrapper<value_type_new> {
	  public:
		using value_type			 = value_type_new;
		using pointer				 = value_type*;
		using const_pointer			 = const pointer;
		using reference				 = value_type&;
		using const_reference		 = const value_type&;
		using iterator				 = jsonifier_internal::iterator<value_type>;
		using const_iterator		 = jsonifier_internal::iterator<const value_type>;
		using difference_type		 = std::ptrdiff_t;
		using reverse_iterator		 = std::reverse_iterator<iterator>;
		using const_reverse_iterator = std::reverse_iterator<const_iterator>;
		using object_compare		 = std::equal_to<value_type>;
		using size_type				 = uint64_t;
		using allocator				 = jsonifier_internal::alloc_wrapper<value_type>;

		JSONIFIER_ALWAYS_INLINE vector() noexcept {
			if constexpr (sizeValNewer > 0) {
				reserve(sizeValNewer);
			}
		}

		template<typename value_type_newer> JSONIFIER_ALWAYS_INLINE vector(size_type sizeNew, value_type_newer&& values) : capacityVal{}, sizeVal{}, dataVal{} {
			auto sizeValNew = sizeNew;
			if (sizeValNew > 0 && sizeValNew < maxSize()) {
				reserve(sizeValNew);
				std::uninitialized_fill(data(), data() + sizeValNew, values);
				sizeVal = sizeValNew;
			}
		}

		JSONIFIER_ALWAYS_INLINE vector& operator=(vector&& other) noexcept {
			if (this != &other && dataVal != other.dataVal) {
				vector{ other }.swap(*this);
			}
			return *this;
		}

		JSONIFIER_ALWAYS_INLINE vector(vector&& other) noexcept : capacityVal{}, sizeVal{}, dataVal{} {
			swap(other);
		}

		JSONIFIER_ALWAYS_INLINE vector& operator=(const vector& other) noexcept {
			if (this != &other) {
				vector{ other }.swap(*this);
			}
			return *this;
		}

		JSONIFIER_ALWAYS_INLINE vector(const vector& other) : capacityVal{}, sizeVal{}, dataVal{} {
			auto sizeValNew = other.size();
			if (sizeValNew > 0 && sizeValNew < maxSize()) {
				reserve(sizeValNew);
				std::uninitialized_copy(other.data(), other.data() + sizeValNew, dataVal);
				sizeVal = sizeValNew;
			}
		}

		JSONIFIER_ALWAYS_INLINE vector& operator=(std::vector<value_type>&& other) noexcept {
			vector{ other }.swap(*this);
			return *this;
		}

		JSONIFIER_ALWAYS_INLINE explicit vector(std::vector<value_type>&& other) noexcept : capacityVal{}, sizeVal{}, dataVal{} {
			auto sizeValNew = other.size();
			if (sizeValNew > 0 && sizeValNew < maxSize()) {
				reserve(sizeValNew);
				std::uninitialized_move(other.data(), other.data() + sizeValNew, dataVal);
				sizeVal = sizeValNew;
			}
		}

		JSONIFIER_ALWAYS_INLINE vector& operator=(const std::vector<value_type>& other) noexcept {
			vector{ other }.swap(*this);
			return *this;
		}

		JSONIFIER_ALWAYS_INLINE explicit vector(const std::vector<value_type>& other) : capacityVal{}, sizeVal{}, dataVal{} {
			auto sizeValNew = other.size();
			if (sizeValNew > 0 && sizeValNew < maxSize()) {
				reserve(sizeValNew);
				std::uninitialized_copy(other.data(), other.data() + sizeValNew, dataVal);
				sizeVal = sizeValNew;
			}
		}

		JSONIFIER_ALWAYS_INLINE vector(iterator first, iterator last) noexcept {
			difference_type insertCount = std::distance(first, last);
			if (!first) {
				return;
			}
			if (insertCount == 0) {
				return;
			}

			size_type insertPosIndex = 0;
			size_type newSize		 = sizeVal + insertCount;

			if (newSize > capacityVal) {
				reserve(newSize);
			}

			pointer insertPos = dataVal + insertPosIndex;

			for (iterator iter = first; iter < last; ++iter) {
				allocator::construct(insertPos++, *iter);
			}

			sizeVal = newSize;
		}

		JSONIFIER_ALWAYS_INLINE vector& operator=(std::initializer_list<value_type> other) noexcept {
			vector{ other }.swap(*this);
			return *this;
		}

		JSONIFIER_ALWAYS_INLINE explicit vector(std::initializer_list<value_type> other) : capacityVal{}, sizeVal{}, dataVal{} {
			auto sizeValNew = other.size();
			if (sizeValNew > 0 && sizeValNew < maxSize()) {
				reserve(sizeValNew);
				std::uninitialized_move(other.begin(), other.begin() + sizeValNew, dataVal);
				sizeVal = sizeValNew;
			}
		}

		JSONIFIER_ALWAYS_INLINE vector& operator=(const value_type& other) noexcept {
			emplace_back(other);
			return *this;
		}

		JSONIFIER_ALWAYS_INLINE vector(const value_type& other) : capacityVal{}, sizeVal{}, dataVal{} {
			*this = other;
		}

		template<typename... arg_types> JSONIFIER_ALWAYS_INLINE void assign(size_type count, arg_types&&... newValue) noexcept {
			size_type newSize = sizeVal + count;

			if (newSize > capacityVal) {
				reserve(newSize);
			}

			pointer insertPos = dataVal + sizeVal;
			std::uninitialized_fill(insertPos, insertPos + count, newValue...);
			sizeVal = newSize;
		}

		JSONIFIER_ALWAYS_INLINE void insert(iterator where, iterator first, iterator last) noexcept {
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

			for (iterator iter = first; iter != last; ++iter) {
				allocator::construct(insertPos++, *iter);
			}

			sizeVal = newSize;
		}

		template<typename value_type_newer> JSONIFIER_ALWAYS_INLINE void insert(iterator where, value_type_newer&& value) {
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

			allocator::construct(insertPos++, value);

			sizeVal = newSize;
		}

		JSONIFIER_ALWAYS_INLINE iterator begin() noexcept {
			return iterator{ dataVal };
		}

		JSONIFIER_ALWAYS_INLINE iterator end() noexcept {
			return iterator{ dataVal + sizeVal };
		}

		JSONIFIER_ALWAYS_INLINE reverse_iterator rbegin() noexcept {
			return reverse_iterator{ end() };
		}

		JSONIFIER_ALWAYS_INLINE reverse_iterator rend() noexcept {
			return reverse_iterator{ begin() };
		}

		JSONIFIER_ALWAYS_INLINE const_iterator begin() const noexcept {
			return const_iterator{ dataVal };
		}

		JSONIFIER_ALWAYS_INLINE const_iterator end() const noexcept {
			return const_iterator{ dataVal + sizeVal };
		}

		JSONIFIER_ALWAYS_INLINE const_reverse_iterator rbegin() const noexcept {
			return const_reverse_iterator{ end() };
		}

		JSONIFIER_ALWAYS_INLINE const_reverse_iterator rend() const noexcept {
			return const_reverse_iterator{ begin() };
		}

		JSONIFIER_ALWAYS_INLINE reference front() noexcept {
			return dataVal[0];
		}

		JSONIFIER_ALWAYS_INLINE reference back() noexcept {
			return dataVal[sizeVal - 1];
		}

		JSONIFIER_ALWAYS_INLINE const_reference front() const {
			return dataVal[0];
		}

		JSONIFIER_ALWAYS_INLINE const_reference back() const {
			return dataVal[sizeVal - 1];
		}

		JSONIFIER_ALWAYS_INLINE reference at(size_type index) noexcept {
			if (index >= sizeVal) {
				throw std::out_of_range{ "Sorry, but that index is beyond the end of this vector's bounds." };
			}
			return dataVal[index];
		}

		JSONIFIER_ALWAYS_INLINE const_reference at(size_type index) const {
			if (index >= sizeVal) {
				throw std::out_of_range{ "Sorry, but that index is beyond the end of this vector's bounds." };
			}
			return dataVal[index];
		}

		JSONIFIER_ALWAYS_INLINE const_reference operator[](size_type index) const {
			return dataVal[index];
		}

		JSONIFIER_ALWAYS_INLINE reference operator[](size_type index) noexcept {
			return dataVal[index];
		}

		JSONIFIER_ALWAYS_INLINE size_type capacity() const {
			return capacityVal;
		}

		JSONIFIER_ALWAYS_INLINE size_type size() const {
			return sizeVal;
		}

		JSONIFIER_ALWAYS_INLINE pointer data() const {
			return dataVal;
		}

		JSONIFIER_ALWAYS_INLINE bool empty() const {
			return sizeVal == 0;
		}

		JSONIFIER_ALWAYS_INLINE pointer data() noexcept {
			return dataVal;
		}

		JSONIFIER_ALWAYS_INLINE operator std::vector<value_type>() const {
			std::vector<value_type> returnValue{};
			for (auto& value: *this) {
				returnValue.emplace_back(value);
			}
			return returnValue;
		}

		template<typename... value_types> JSONIFIER_ALWAYS_INLINE reference emplace_back(value_types&&... c) noexcept {
			if (sizeVal + 1 >= capacityVal) {
				reserve(capacityVal * 2 + 2);
			}
			allocator::construct(&dataVal[sizeVal++], std::forward<value_types>(c)...);

			return dataVal[sizeVal - 1];
		}

		JSONIFIER_ALWAYS_INLINE reference emplace_back(value_type&& c) noexcept {
			if (sizeVal + 1 >= capacityVal) {
				reserve(capacityVal * 2 + 2);
			}
			allocator::construct(&dataVal[sizeVal++], std::move(c));

			return dataVal[sizeVal - 1];
		}

		JSONIFIER_ALWAYS_INLINE reference emplace_back(const value_type& c) noexcept {
			if (sizeVal + 1 >= capacityVal) {
				reserve(capacityVal * 2 + 2);
			}
			allocator::construct(&dataVal[sizeVal++], c);

			return dataVal[sizeVal - 1];
		}

		JSONIFIER_ALWAYS_INLINE void erase(size_type count) noexcept {
			if (count >= sizeVal) {
				return;
			}
			size_type newSize = sizeVal - count;

			std::destroy(dataVal, dataVal + count);

			std::uninitialized_move(dataVal + count, dataVal + count + newSize, dataVal);

			sizeVal = newSize;
		}

		JSONIFIER_ALWAYS_INLINE iterator erase(iterator iter) {
			if (iter < begin() || iter > end()) {
				return end();
			}

			size_type eraseIndex = static_cast<size_type>(iter - begin());
			size_type newSize	 = sizeVal - 1;

			allocator::destroy(dataVal + eraseIndex);

			std::uninitialized_move(dataVal + eraseIndex + 1, dataVal + sizeVal, dataVal + eraseIndex);

			sizeVal = newSize;
			return iterator{ dataVal + eraseIndex };
		}

		JSONIFIER_ALWAYS_INLINE void shrink_to_fit() noexcept {
			if (sizeVal == capacityVal) {
				return;
			}

			vector<value_type> newVector{};
			newVector.reserve(sizeVal);
			std::uninitialized_move(dataVal, dataVal + sizeVal, newVector.data());
			newVector.sizeVal = sizeVal;
			swap(newVector);
		}

		constexpr size_type maxSize() noexcept {
			return (std::min)(static_cast<size_type>((std::numeric_limits<size_type>::max)()), allocator::maxSize());
		}

		JSONIFIER_ALWAYS_INLINE void resize(size_type newSize) {
			if (newSize > capacityVal) [[likely]] {
				pointer newPtr = allocator::allocate(newSize);
				try {
					if (sizeVal > 0ull) {
						if constexpr (std::is_copy_constructible_v<value_type>) {
							std::uninitialized_copy(dataVal, dataVal + sizeVal, newPtr);
						} else if constexpr (std::is_move_constructible_v<value_type>) {
							std::uninitialized_move(dataVal, dataVal + sizeVal, newPtr);
						}
					}
					if (dataVal && capacityVal > 0) [[likely]] {
						allocator::deallocate(dataVal);
					}
				} catch (...) {
					allocator::deallocate(newPtr);
					throw;
				}
				capacityVal = newSize;
				dataVal		= newPtr;
				std::uninitialized_value_construct(dataVal + sizeVal, dataVal + capacityVal);
			} else if (newSize > sizeVal) [[unlikely]] {
				std::uninitialized_value_construct(dataVal + sizeVal, dataVal + capacityVal);
			} else if (newSize < sizeVal) {
				std::destroy(dataVal + newSize, dataVal + sizeVal);
			}
			sizeVal = newSize;
		}

		JSONIFIER_ALWAYS_INLINE void reserve(size_type capacityNew) {
			if (capacityNew > capacityVal) [[likely]] {
				pointer newPtr = allocator::allocate(capacityNew);
				try {
					if (dataVal && capacityVal > 0) [[likely]] {
						if (sizeVal > 0) {
							std::uninitialized_move(dataVal, dataVal + sizeVal, newPtr);
						}
						allocator::deallocate(dataVal);
					}
				} catch (...) {
					allocator::deallocate(newPtr);
					throw;
				}
				capacityVal = capacityNew;
				dataVal		= newPtr;
			}
		}

		JSONIFIER_ALWAYS_INLINE void clear() noexcept {
			std::destroy(dataVal, dataVal + sizeVal);
			sizeVal = 0;
		}

		JSONIFIER_ALWAYS_INLINE void swap(vector& other) noexcept {
			std::swap(capacityVal, other.capacityVal);
			std::swap(sizeVal, other.sizeVal);
			std::swap(dataVal, other.dataVal);
		}

		JSONIFIER_ALWAYS_INLINE bool operator==(const vector<value_type>& rhs) const {
			if (rhs.size() != size()) {
				return false;
			}
			if constexpr (!std::is_fundamental_v<value_type>) {
				for (size_type x = 0; x < sizeVal; ++x) {
					if (!object_compare()(rhs.dataVal[x], dataVal[x])) {
						return false;
					}
				}
				return true;
			} else {
				return jsonifier_internal::compare(rhs.data(), data(), size());
			}
		}

		JSONIFIER_ALWAYS_INLINE ~vector() noexcept {
			reset();
		};

	  protected:
		size_type capacityVal{};
		size_type sizeVal{};
		pointer dataVal{};

		JSONIFIER_ALWAYS_INLINE void reset() noexcept {
			if (dataVal && capacityVal) {
				if (sizeVal) {
					std::destroy(dataVal, dataVal + sizeVal);
					sizeVal = 0;
				}
				allocator::deallocate(dataVal);
				dataVal		= nullptr;
				capacityVal = 0;
			}
		}
	};

}// namespace jsonifier