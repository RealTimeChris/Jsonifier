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
#include <source_location>

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

	template<typename OTy> struct Relational {
		inline friend bool operator!=(const OTy& lhs, const OTy& rhs) {
			return !(lhs == rhs);
		}

	  protected:
		Relational() noexcept = default;
	};

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

	template<typename ValueType> class AllocWrapper {
	  public:
		using value_type = ValueType;
		using pointer = value_type*;
		using size_type = size_t;
		using allocator = std::pmr::polymorphic_allocator<value_type>;
		using allocator_traits = std::allocator_traits<allocator>;

		inline constexpr AllocWrapper() noexcept = default;

		inline constexpr AllocWrapper& operator=(AllocWrapper&& other) noexcept {
			return *this;
		};

		inline constexpr AllocWrapper(AllocWrapper&& other) noexcept {};

		inline constexpr AllocWrapper& operator=(const AllocWrapper& other) noexcept {
			return *this;
		};

		inline constexpr AllocWrapper(const AllocWrapper& other) noexcept {};

		inline constexpr pointer allocate(size_type count) noexcept {
			return allocTraits.allocate(alloc, count);
		}

		inline constexpr void deallocate(pointer ptr, size_type count) noexcept {
			allocTraits.deallocate(alloc, ptr, count);
		}

		template<typename... Args> inline constexpr void construct(pointer ptr, Args... args) noexcept {
			allocTraits.construct(alloc, ptr, args...);
		}

		inline constexpr void destroy(pointer ptr) noexcept {
			allocTraits.destroy(alloc, ptr);
		}

	  protected:
		NO_UNIQUE_ADDRESS allocator_traits allocTraits{};
		NO_UNIQUE_ADDRESS allocator alloc{};
	};
}

namespace Jsonifier {

	template<typename ValueType> class Vector : public JsonifierInternal::Relational<Vector<ValueType>> {
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

		inline constexpr Vector() noexcept = default;

		inline Vector& operator=(Vector&& other) noexcept {
			JsonifierInternal::swapF(capacityVal, other.capacityVal);
			JsonifierInternal::swapF(sizeVal, other.sizeVal);
			JsonifierInternal::swapF(values, other.values);
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
			for (size_type x = 0; x < sizeVal; ++x) {
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
				for (size_type x = 0; x < capacityVal; ++x) {
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
			return std::numeric_limits<size_type>::max() / sizeof(value_type);
		}

		inline constexpr void emplace_back(const_reference c) {
			if (sizeVal + 1 >= capacityVal) {
				reserve(capacityVal * 8 + 1);
			}
			allocator alloc{};
			alloc.construct(&values[sizeVal++], c);
		}

		inline constexpr void emplace_back(value_type&& c) {
			if (sizeVal + 1 >= capacityVal) {
				reserve(capacityVal * 8 + 1);
			}
			allocator alloc{};
			alloc.construct(&values[sizeVal++], std::forward<value_type>(c));
		}

		inline void resize(size_type sizeNew) {
			if (sizeNew > 0) {
				if (sizeNew > capacityVal) {
					allocator alloc{};
					pointer newPtr = alloc.allocate(sizeNew);
					try {
						if (values) {
							std::uninitialized_move(values, values + sizeVal, newPtr);
							alloc.deallocate(values, capacityVal);
						}
					} catch (...) {
						alloc.deallocate(newPtr, sizeNew);
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
				allocator alloc{};
				pointer newPtr = alloc.allocate(capacityNew);
				try {
					if (values) {
						std::uninitialized_move(values, values + sizeVal, newPtr);
						alloc.deallocate(values, capacityVal);
					}
				} catch (...) {
					alloc.deallocate(newPtr, capacityNew);
					throw;
				}
				capacityVal = capacityNew;
				values = newPtr;
			}
		}

		inline constexpr void reset() {
			std::fill(values, values + capacityVal, value_type{});
		}

		inline constexpr bool operator==(const Vector<value_type>& rhs) const noexcept {
			if (rhs.size() != size()) {
				return false;
			}
			return JsonifierInternal::JsonifierCoreInternal::compare(rhs.data(), this->data(), this->size());
		}

		inline constexpr ~Vector() {
			if (values && capacityVal) {
				allocator alloc{};
				alloc.deallocate(values, capacityVal);
			}
		};

	  protected:
		size_type capacityVal{};
		size_type sizeVal{};
		pointer values{};
	};

}
