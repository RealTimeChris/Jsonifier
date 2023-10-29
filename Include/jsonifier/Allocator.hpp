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

namespace jsonifier_internal {

	template<typename value_type_new> class aligned_allocator : public std::pmr::polymorphic_allocator<value_type_new> {
	  public:
		using value_type = value_type_new;
		using pointer	 = value_type*;
		using size_type	 = uint64_t;
		using allocator	 = std::pmr::polymorphic_allocator<value_type_new>;

		jsonifier_inline pointer allocate(size_type n) {
			if (n == 0) {
				return nullptr;
			}
			return static_cast<pointer>(allocator::allocate_bytes(roundUpToMultiple<BytesPerStep>(n * sizeof(value_type)), BytesPerStep));
		}

		jsonifier_inline void deallocate(pointer ptr, size_type n) {
			if (ptr) {
				allocator::deallocate_bytes(ptr, roundUpToMultiple<BytesPerStep>(n * sizeof(value_type)), BytesPerStep);
			}
		}

		template<typename... Args> jsonifier_inline void construct(pointer p, Args&&... args) {
			new (p) value_type(std::forward<Args>(args)...);
		}

		jsonifier_inline void destroy(pointer p) {
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

		jsonifier_inline pointer allocate(size_type count) {
			return allocator_traits::allocate(*this, count);
		}

		jsonifier_inline void deallocate(pointer ptr, size_type count) {
			allocator_traits::deallocate(*this, ptr, count);
		}

		template<typename... Args> jsonifier_inline void construct(pointer ptr, Args&&... args) {
			allocator_traits::construct(*this, ptr, std::forward<Args>(args)...);
		}

		jsonifier_inline size_type maxSize() {
			return allocator_traits::max_size(*this);
		}

		jsonifier_inline void destroy(pointer ptr) {
			allocator_traits::destroy(*this, ptr);
		}
	};

}// namespace jsonifier_internal
