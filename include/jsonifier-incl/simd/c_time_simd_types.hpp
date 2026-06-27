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

#include <jsonifier-incl/utilities/type_entities.hpp>
#include <jsonifier-incl/core/config.hpp>
#include <type_traits>
#include <algorithm>
#include <cstring>
#include <cstdint>
#include <utility>

namespace jsonifier::simd {

	enum class simd_classes {
		x_128 = 16,
		x_256 = 32,
		x_512 = 64,
	};

	template<typename... value_types> struct first;

	template<typename value_type, typename... value_types> struct first<value_type, value_types...> {
		using type = value_type;
	};

	template<typename... value_types> using first_t = typename first<value_types...>::type;

	template<simd_classes simd_class_new> struct simd_x {
	  public:
		static constexpr simd_classes simd_class{ simd_class_new };
		static constexpr uint64_t sixty_four_per{ static_cast<uint64_t>(simd_class) / sizeof(uint64_t) };
		static constexpr uint64_t thirty_two_per{ static_cast<uint64_t>(simd_class) / sizeof(uint32_t) };
		static constexpr uint64_t sixteen_per{ static_cast<uint64_t>(simd_class) / sizeof(uint16_t) };
		static constexpr uint64_t eight_per{ static_cast<uint64_t>(simd_class) / sizeof(uint8_t) };

		union JSONIFIER_ALIGN(static_cast<uint64_t>(simd_class)) storage_type {
			uint64_t x_uint64[sixty_four_per];
			uint32_t x_uint32[thirty_two_per];
			uint16_t x_uint16[sixteen_per];
			uint8_t x_uint8[eight_per];
			int64_t x_int64[sixty_four_per];
			int32_t x_int32[thirty_two_per];
			int16_t x_int16[sixteen_per];
			int8_t x_int8[eight_per];
		} values{};

		template<typename integer_sequence> struct pack_values;

		template<uint64_t... indices> struct pack_values<jsonifier::internal::integer_sequence<indices...>> {
			template<typename... value_type>
				requires(sizeof(first_t<value_type...>) == 8 && sizeof...(value_type) == sixty_four_per)
			JSONIFIER_INLINE static constexpr void impl(storage_type& values, value_type&&... args) {
				((values.x_uint64[indices] = args), ...);
			}
			template<typename... value_type>
				requires(sizeof(first_t<value_type...>) == 4 && sizeof...(value_type) == thirty_two_per)
			JSONIFIER_INLINE static constexpr void impl(storage_type& values, value_type&&... args) {
				((values.x_uint32[indices] = args), ...);
			}

			template<typename... value_type>
				requires(sizeof(first_t<value_type...>) == 2 && sizeof...(value_type) == sixteen_per)
			JSONIFIER_INLINE static constexpr void impl(storage_type& values, value_type&&... args) {
				((values.x_uint16[indices] = args), ...);
			}

			template<typename... value_type>
				requires(sizeof(first_t<value_type...>) == 1 && sizeof...(value_type) == eight_per)
			JSONIFIER_INLINE static constexpr void impl(storage_type& values, value_type&&... args) {
				((values.x_uint8[indices] = args), ...);
			}

			JSONIFIER_INLINE static constexpr void impl(storage_type& values, const uint64_t (&args)[sizeof...(indices)]) {
				((values.x_uint64[indices] = args[indices]), ...);
			}

			JSONIFIER_INLINE static constexpr void impl(storage_type& values, const uint32_t (&args)[sizeof...(indices)]) {
				((values.x_uint32[indices] = args[indices]), ...);
			}

			JSONIFIER_INLINE static constexpr void impl(storage_type& values, const uint16_t (&args)[sizeof...(indices)]) {
				((values.x_uint16[indices] = args[indices]), ...);
			}

			JSONIFIER_INLINE static constexpr void impl(storage_type& values, const uint8_t (&args)[sizeof...(indices)]) {
				((values.x_uint8[indices] = args[indices]), ...);
			}
		};

		template<typename... value_type>
			requires(sizeof...(value_type) == (static_cast<uint64_t>(simd_class) / sizeof(first_t<value_type...>)))
		JSONIFIER_INLINE explicit constexpr simd_x(value_type&&... val_01) noexcept {
			pack_values<jsonifier::internal::make_integer_sequence<sizeof...(value_type)>>::impl(values, std::forward<value_type>(val_01)...);
		}

		template<typename value_type, uint64_t size>
			requires(size == (static_cast<uint64_t>(simd_class) / sizeof(value_type)))
		JSONIFIER_INLINE explicit constexpr simd_x(const value_type (&val_01)[size]) noexcept {
			pack_values<jsonifier::internal::make_integer_sequence<size>>::impl(values, val_01);
		}

		JSONIFIER_INLINE constexpr simd_x() noexcept {
		}
	};

}
