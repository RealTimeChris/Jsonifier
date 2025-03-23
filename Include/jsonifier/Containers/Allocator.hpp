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

#if JSONIFIER_PLATFORM_WINDOWS
	#define WIN32_LEAN_AND_MEAN
	#include <windows.h>
#elif JSONIFIER_PLATFORM_LINUX || JSONIFIER_PLATFORM_MAC
	#include <sys/mman.h>
#endif

namespace jsonifier::internal {

	template<typename value_type> JSONIFIER_INLINE constexpr value_type&& forward(remove_reference_t<value_type>& t) noexcept {
		return static_cast<value_type&&>(t);
	}

	template<typename value_type>
		requires(std::is_rvalue_reference_v<value_type>)
	JSONIFIER_INLINE constexpr value_type&& forward(remove_reference_t<value_type>&& t) noexcept {
		static_assert(!std::is_lvalue_reference_v<value_type>, "value_type cannot be an lvalue reference (e.g., U&).");
		return static_cast<value_type&&>(t);
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
		using const_pointer	   = const value_type*;
		using size_type		   = size_t;
		using difference_type  = ptrdiff_t;
		using allocator_traits = std::allocator_traits<alloc_wrapper<value_type>>;

		template<typename U> struct rebind {
			using other = alloc_wrapper<U>;
		};

		static constexpr size_t alignment = bytesPerStep;

		alloc_wrapper() noexcept = default;

		template<typename U> alloc_wrapper(const alloc_wrapper<U>&) noexcept {
		}

		JSONIFIER_INLINE static pointer allocate(size_type count) noexcept {
			if JSONIFIER_UNLIKELY (count == 0) {
				return nullptr;
			}

			const size_type bytes		  = count * sizeof(value_type);
			const size_type aligned_bytes = roundUpToMultiple<alignment>(bytes);

			if (aligned_bytes >= huge_page_threshold) {
				const size_type hp_bytes = roundUpToMultiple<huge_page_size>(aligned_bytes);

#if JSONIFIER_PLATFORM_WINDOWS
				void* p = VirtualAlloc(nullptr, hp_bytes, MEM_RESERVE | MEM_COMMIT | MEM_LARGE_PAGES, PAGE_READWRITE);
				if (!p) {
					p = VirtualAlloc(nullptr, hp_bytes, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
				}
				if (p != nullptr) {
					return static_cast<pointer>(p);
				}

#elif JSONIFIER_PLATFORM_LINUX
				void* p = mmap(nullptr, hp_bytes, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_HUGETLB, -1, 0);
				if (p != MAP_FAILED) {
					return static_cast<pointer>(p);
				}

				p = mmap(nullptr, hp_bytes, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
				if (p != MAP_FAILED) {
					madvise(p, hp_bytes, MADV_HUGEPAGE);
					return static_cast<pointer>(p);
				}

#elif JSONIFIER_PLATFORM_MAC
				void* p = mmap(nullptr, hp_bytes, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
				if (p != MAP_FAILED) {
					return static_cast<pointer>(p);
				}
#endif
			}

#if JSONIFIER_PLATFORM_WINDOWS || JSONIFIER_PLATFORM_LINUX
			void* p = _mm_malloc(aligned_bytes, alignment);
#else
			void* p = aligned_alloc(alignment, aligned_bytes);
#endif

			return static_cast<pointer>(p);
		}

		JSONIFIER_INLINE void deallocate(pointer p, size_type count) noexcept {
			if JSONIFIER_LIKELY (p) {
				const size_type bytes		  = count * sizeof(value_type);
				const size_type aligned_bytes = roundUpToMultiple<alignment>(bytes);

				if (aligned_bytes >= huge_page_threshold) {
#if JSONIFIER_PLATFORM_WINDOWS
					VirtualFree(p, 0, MEM_RELEASE);
#elif JSONIFIER_PLATFORM_LINUX || JSONIFIER_PLATFORM_MAC
					const size_type hp_bytes = roundUpToMultiple<huge_page_size>(aligned_bytes);
					munmap(p, hp_bytes);
#else
					free(p);
#endif
				} else {
#if JSONIFIER_PLATFORM_WINDOWS || JSONIFIER_PLATFORM_LINUX
					_mm_free(p);
#else
					free(p);
#endif
				}
			}
		}

		template<typename... arg_types> JSONIFIER_INLINE static void construct(pointer p, arg_types&&... args) noexcept {
			new (p) value_type(internal::forward<arg_types>(args)...);
		}

		JSONIFIER_INLINE static size_type maxSize() noexcept {
			return allocator_traits::max_size(alloc_wrapper{});
		}

		JSONIFIER_INLINE static void destroy(pointer p) noexcept {
			if constexpr (!std::is_trivially_destructible_v<value_type>) {
				p->~value_type();
			}
		}

		JSONIFIER_INLINE constexpr bool operator==(const alloc_wrapper&) const noexcept {
			return true;
		}

		JSONIFIER_INLINE constexpr bool operator!=(const alloc_wrapper& other) const noexcept {
			return !(*this == other);
		}

	  private:
		static constexpr size_t huge_page_size		= 2 * 1024 * 1024ULL;
		static constexpr size_t huge_page_threshold = huge_page_size * 2;
	};

}// namespace internal