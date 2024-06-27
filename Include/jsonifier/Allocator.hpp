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

	template<auto multiple, typename value_type = decltype(multiple)> constexpr value_type roundUpToMultiple(value_type value) {
		auto remainder = value % multiple;
		return remainder == 0 ? value : value + (multiple - remainder);
	}

	template<auto multiple, typename value_type = decltype(multiple)> constexpr value_type roundDownToMultiple(value_type value) {
		return static_cast<int64_t>(value) >= 0 ? (value / multiple) * multiple : ((value - multiple + 1) / multiple) * multiple;
	}

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
#if defined(JSONIFIER_MSVC)
			return static_cast<value_type*>(_aligned_malloc(roundUpToMultiple<bytesPerStep>(count * sizeof(value_type)), bytesPerStep));
#else
			return static_cast<value_type*>(std::aligned_alloc(bytesPerStep, roundUpToMultiple<bytesPerStep>(count * sizeof(value_type))));
#endif
		}

		JSONIFIER_INLINE void deallocate(pointer ptr) {
			if (ptr) [[likely]] {
#if defined(JSONIFIER_MSVC)
				_aligned_free(ptr);
#else
				free(ptr);
#endif
			}
		}

		template<typename... arg_types> JSONIFIER_INLINE void construct(pointer ptr, arg_types&&... args) {
			new (ptr) value_type(std::forward<arg_types>(args)...);
		}

		JSONIFIER_INLINE size_type maxSize() {
			return allocator_traits::max_size(alloc_wrapper{});
		}

		JSONIFIER_INLINE void destroy(pointer ptr) {
			ptr->~value_type();
		}
	};

}// namespace jsonifier_internal