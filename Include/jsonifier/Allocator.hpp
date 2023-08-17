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
/// Feb 3, 2023
#pragma once

#include <memory_resource>
#include <cstdint>
#include <utility>
#include <memory>

#if defined(_MSC_VER)
	#define ALIGNED_ALLOC(size, alignment) _aligned_malloc(size, alignment)
	#define ALIGNED_FREE(ptr) _aligned_free(ptr)
#else
	#define ALIGNED_ALLOC(size, alignment) aligned_alloc(alignment, size)
	#define ALIGNED_FREE(ptr) free(ptr)
#endif

#ifdef T_AVX512
	#define ALIGNMENT 64
#elif defined T_AVX2
	#define ALIGNMENT 32
#elif defined T_AVX
	#define ALIGNMENT 16
#else
	#define ALIGNMENT 8
#endif

namespace JsonifierInternal {

	template<typename ValueType> class AlignedAllocator {
	  public:
		using value_type = ValueType;
		using pointer = value_type*;
		using size_type = size_t;
		using allocator = std::pmr::polymorphic_allocator<value_type>;

		constexpr pointer allocate(size_type n) {
			if (n == 0) {
				return nullptr;
			}

			return static_cast<pointer>(ALIGNED_ALLOC(n * sizeof(ValueType), ALIGNMENT));
		}

		constexpr void deallocate(pointer p, size_type) {
			if (p) {
				return ALIGNED_FREE(p);
			}
		}

		template<typename... Args> constexpr void construct(pointer p, Args&&... args) {
			new (p) value_type(std::forward<Args>(args)...);
		}

		constexpr void destroy(pointer p) {
			p->~value_type();
		}
	};

	template<typename ValueType> class AllocWrapper : public AlignedAllocator<ValueType> {
	  public:
		using value_type = ValueType;
		using pointer = value_type*;
		using size_type = size_t;
		using allocator = AlignedAllocator<value_type>;
		using allocator_traits = std::allocator_traits<allocator>;

		inline pointer allocate(size_type count) noexcept {
			return allocator_traits::allocate(*this, count);
		}

		inline void deallocate(pointer ptr, size_type count) noexcept {
			allocator_traits::deallocate(*this, ptr, count);
		}

		template<typename... Args> inline void construct(pointer ptr, Args&&... args) noexcept {
			allocator_traits::construct(*this, ptr, std::forward<Args>(args)...);
		}

		inline void destroy(pointer ptr) noexcept {
			allocator_traits::destroy(*this, ptr);
		}
	};

}
