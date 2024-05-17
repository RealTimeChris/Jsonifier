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

#include <jsonifier/ISA/SimdCommon.hpp>
#include <memory_resource>

namespace jsonifier_internal {

	template<auto multiple, typename value_type = decltype(multiple)> constexpr value_type roundUpToMultiple(value_type val) {
		auto remainder = val % multiple;
		return remainder == 0 ? val : val + (multiple - remainder);
	}

	template<auto multiple, typename value_type = decltype(multiple)> constexpr value_type roundDownToMultiple(value_type val) {
		return static_cast<int64_t>(val) >= 0 ? (val / multiple) * multiple : ((val - multiple + 1) / multiple) * multiple;
	}

#if defined(_MSC_VER)

	template<typename value_type> JSONIFIER_INLINE value_type* jsonifierAlignedAlloc(uint64_t size) {
		return static_cast<value_type*>(_aligned_malloc(roundUpToMultiple<BytesPerStep>(size * sizeof(value_type)), BytesPerStep));
	}

	JSONIFIER_INLINE void jsonifierFree(void* ptr) {
		_aligned_free(ptr);
	}

#else

	template<typename value_type> JSONIFIER_INLINE value_type* jsonifierAlignedAlloc(uint64_t size) {
		return static_cast<value_type*>(std::aligned_alloc(BytesPerStep, roundUpToMultiple<BytesPerStep>(size * sizeof(value_type))));
	}

	JSONIFIER_INLINE void jsonifierFree(void* ptr) {
		free(ptr);
	}

#endif

	template<typename value_type_new> class alloc_wrapper {
	  public:
		using value_type	   = value_type_new;
		using pointer		   = value_type*;
		using size_type		   = uint64_t;
		using allocator_traits = std::allocator_traits<alloc_wrapper<value_type>>;

		JSONIFIER_INLINE pointer allocate(size_type count) {
			if (count == 0) [[unlikely]] {
				return nullptr;
			}
			return jsonifierAlignedAlloc<value_type>(count);
		}

		JSONIFIER_INLINE void deallocate(pointer ptr, size_type) {
			if (ptr) [[likely]] {
				jsonifierFree(ptr);
			}
		}

		template<typename... arg_types> JSONIFIER_INLINE void construct(pointer ptr, arg_types&&... args) {
			new (ptr) value_type(std::forward<arg_types>(args)...);
		}

		JSONIFIER_INLINE size_type maxSize() {
			return allocator_traits::max_size(*this);
		}

		JSONIFIER_INLINE void destroy(pointer ptr) {
			ptr->~value_type();
		}
	};

}// namespace jsonifier_internal
