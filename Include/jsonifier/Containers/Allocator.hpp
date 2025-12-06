/*
	MIT License

	Copyright (c) 2024 RealTimeChris

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

#include <jsonifier/Simd/SimdTypes.hpp>
#include <memory_resource>
#include <stdlib.h>

namespace jsonifier::internal {

	template<typename value_type> JSONIFIER_INLINE constexpr value_type&& forward(jsonifier::internal::remove_reference_t<value_type>& value) noexcept {
		return static_cast<value_type&&>(value);
	}

	template<typename value_type>
	concept r_value_reference = std::is_rvalue_reference_v<value_type>;

	template<r_value_reference value_type> JSONIFIER_INLINE constexpr value_type forward(value_type value) noexcept {
		return value;
	}

	template<auto multiple, typename value_type = decltype(multiple)> JSONIFIER_INLINE constexpr value_type roundUpToMultiple(value_type value) noexcept {
		if constexpr ((multiple & (multiple - 1)) == 0) {
			constexpr auto mulSub1{ multiple - 1 };
			constexpr auto notMulSub1{ ~mulSub1 };
			return (value + (mulSub1)) & notMulSub1;
		} else {
			const auto remainder = value % multiple;
			return remainder == 0 ? value : value + (multiple - remainder);
		}
	}

	template<auto multiple, typename value_type = decltype(multiple)> JSONIFIER_INLINE constexpr value_type roundDownToMultiple(value_type value) noexcept {
		if constexpr ((multiple & (multiple - 1)) == 0) {
			constexpr auto notMulSub1{ ~(multiple - 1) };
			return value & notMulSub1;
		} else {
			return static_cast<int64_t>(value) >= 0 ? (value / multiple) * multiple : ((value - multiple + 1) / multiple) * multiple;
		}
	}

	template<typename value_type_new> class alloc_wrapper {
	  public:
		using value_type	   = value_type_new;
		using pointer		   = value_type*;
		using size_type		   = size_t;
		using allocator_traits = std::allocator_traits<alloc_wrapper<value_type>>;

		JSONIFIER_INLINE static pointer allocate(size_type count) noexcept {
			if JSONIFIER_UNLIKELY (count == 0) {
				return nullptr;
			}
#if JSONIFIER_PLATFORM_WINDOWS || JSONIFIER_PLATFORM_LINUX
			return static_cast<value_type*>(_mm_malloc(roundUpToMultiple<bytesPerStep>(count * sizeof(value_type)), bytesPerStep));
#else
			return static_cast<value_type*>(aligned_alloc(bytesPerStep, roundUpToMultiple<bytesPerStep>(count * sizeof(value_type))));
#endif
		}

		JSONIFIER_INLINE void deallocate(pointer ptr, size_t = 0) noexcept {
			if JSONIFIER_LIKELY (ptr) {
#if JSONIFIER_PLATFORM_WINDOWS || JSONIFIER_PLATFORM_LINUX
				_mm_free(ptr);
#else
				free(ptr);
#endif
			}
		}

		template<typename... arg_types> JSONIFIER_INLINE static void construct(pointer ptr, arg_types&&... args) noexcept {
			new (ptr) value_type(internal::forward<arg_types>(args)...);
		}

		JSONIFIER_INLINE static size_type maxSize() noexcept {
			return allocator_traits::max_size(alloc_wrapper{});
		}

		JSONIFIER_INLINE static void destroy(pointer ptr) noexcept {
			ptr->~value_type();
		}
	};

}// namespace internal