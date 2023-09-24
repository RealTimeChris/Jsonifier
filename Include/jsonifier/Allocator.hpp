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
/// Feb 3, 2023
#pragma once

#include <jsonifier/ISADetection/ISADetectionBase.hpp>
#include <memory_resource>

inline uint64_t findNextClosestMultiple(uint64_t number) {
	if constexpr (ALIGNMENT == 0) {
		return 0;
	}

	uint64_t remainder = number % ALIGNMENT;
	if (remainder == 0) {
		return number;
	}

	uint64_t nextMultiple = number + (ALIGNMENT - remainder);
	return nextMultiple;
}

namespace jsonifier_internal {

	template<typename value_type_new> class aligned_allocator : public std::pmr::polymorphic_allocator<value_type_new> {
	  public:
		using value_type = value_type_new;
		using pointer	 = value_type*;
		using size_type	 = uint64_t;
		using allocator	 = std::pmr::polymorphic_allocator<value_type>;

		inline pointer allocate(size_type n) {
			if (n == 0) {
				return nullptr;
			}
			return static_cast<value_type*>(allocator::allocate_bytes(findNextClosestMultiple(n * sizeof(value_type)), ALIGNMENT));
		}

		inline void deallocate(pointer ptr, size_type n) {
			if (ptr) {
				allocator::deallocate_bytes(ptr, findNextClosestMultiple(n * sizeof(value_type)), ALIGNMENT);
			}
		}

		template<typename... Args> inline void construct(pointer p, Args&&... args) {
			new (p) value_type(std::forward<Args>(args)...);
		}

		inline void destroy(pointer p) {
			p->~value_type();
		}
	};

	template<typename value_type_new> class alloc_wrapper : public aligned_allocator<value_type_new> {
	  public:
		using value_type	   = value_type_new;
		using pointer		   = value_type*;
		using size_type		   = uint64_t;
		using allocator		   = aligned_allocator<value_type>;
		using allocator_traits = std::allocator_traits<allocator>;

		inline pointer allocate(size_type count) {
			return allocator_traits::allocate(*this, count);
		}

		inline void deallocate(pointer ptr, size_type count) {
			allocator_traits::deallocate(*this, ptr, count);
		}

		template<typename... Args> inline void construct(pointer ptr, Args&&... args) {
			allocator_traits::construct(*this, ptr, std::forward<Args>(args)...);
		}

		inline void destroy(pointer ptr) {
			allocator_traits::destroy(*this, ptr);
		}
	};

}// namespace jsonifier_internal
