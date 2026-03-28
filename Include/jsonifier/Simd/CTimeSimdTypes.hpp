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

#include <jsonifier/Utilities/TypeTraits.hpp>
#include <jsonifier/Core/Config.hpp>
#include <type_traits>
#include <algorithm>
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

		template<uint64_t... indices> struct pack_values<std::integer_sequence<uint64_t, indices...>> {
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
			pack_values<std::make_integer_sequence<uint64_t, sizeof...(value_type)>>::impl(values, std::forward<value_type>(val_01)...);
		}

		template<typename value_type, uint64_t size>
			requires(size == (static_cast<uint64_t>(simd_class) / sizeof(value_type)))
		JSONIFIER_INLINE explicit constexpr simd_x(const value_type (&val_01)[size]) noexcept {
			pack_values<std::make_integer_sequence<uint64_t, size>>::impl(values, val_01);
		}

		JSONIFIER_INLINE constexpr simd_x() noexcept {
		}
	};

	template<uint64_t size> struct get_int_from_size {
		using type = std::conditional_t<size == 16, uint16_t, std::conditional_t<size == 32, uint32_t, uint64_t>>;
	};

	template<uint64_t size> using get_int_from_size_t = typename get_int_from_size<size>::type;

	template<typename integer_sequence> struct shuffle_epi8_impl;

	template<uint64_t... indices> struct shuffle_epi8_impl<std::integer_sequence<uint64_t, indices...>> {
		template<typename simd_int_t01> JSONIFIER_INLINE static constexpr simd_int_t01 impl(const simd_int_t01& a, const simd_int_t01& b) noexcept {
			simd_int_t01 result{};
			uint8_t control{};
			((control = b.values.x_uint8[indices], result.values.x_uint8[indices] = (control & 0x80) ? 0 : a.values.x_uint8[(indices & ~0x0F) | (control & 0x0F)]), ...);
			return result;
		}
	};

	template<typename simd_class> JSONIFIER_INLINE static constexpr auto op_shuffle(const simd_class& a, const simd_class& b) noexcept {
		return shuffle_epi8_impl<std::make_integer_sequence<uint64_t, static_cast<uint64_t>(simd_class::simd_class)>>::impl(a, b);
	}

	template<typename integer_sequence> struct op_cmp_eq_bitmask_impl;

	template<uint64_t... indices> struct op_cmp_eq_bitmask_impl<std::integer_sequence<uint64_t, indices...>> {
		template<typename simd_int_t01> JSONIFIER_INLINE static constexpr decltype(auto) impl(const simd_int_t01& a, const simd_int_t01& b) noexcept {
			get_int_from_size_t<sizeof...(indices)> mask{};
			((mask |= (a.values.x_uint8[indices] == b.values.x_uint8[indices] ? (static_cast<decltype(mask)>(1) << indices) : 0)), ...);
			return mask;
		}
	};

	template<typename simd_class> JSONIFIER_INLINE static constexpr auto op_cmp_eq_bitmask(const simd_class& a, const simd_class& b) noexcept {
		return op_cmp_eq_bitmask_impl<std::make_integer_sequence<uint64_t, static_cast<uint64_t>(simd_class::simd_class)>>::impl(a, b);
	}

	template<typename integer_sequence> struct movemask_epi8_impl;

	template<uint64_t... indices> struct movemask_epi8_impl<std::integer_sequence<uint64_t, indices...>> {
		template<typename simd_int_t01> JSONIFIER_INLINE static constexpr auto impl(const simd_int_t01& a) noexcept {
			get_int_from_size_t<sizeof...(indices)> mask{};
			((mask |= (a.values.x_uint8[indices] & 0x80) ? (static_cast<decltype(mask)>(1) << indices) : 0), ...);
			return mask;
		}
	};

	template<typename simd_class> JSONIFIER_INLINE static constexpr auto op_bitmask(const simd_class& a) noexcept {
		return movemask_epi8_impl<std::make_integer_sequence<uint64_t, static_cast<uint64_t>(simd_class::simd_class)>>::impl(a);
	}

	template<typename simd_class> JSONIFIER_INLINE static constexpr auto gather_values(const void* str) noexcept {
		simd_class result{};
		std::memcpy(&result, str, static_cast<uint64_t>(simd_class::simd_class));
		return result;
	}

	template<typename simd_class> JSONIFIER_INLINE static constexpr auto gather_values_u(const void* str) noexcept {
		simd_class result{};
		std::memcpy(&result, str, static_cast<uint64_t>(simd_class::simd_class));
		return result;
	}

	template<typename simd_class> JSONIFIER_INLINE static constexpr void store_values(const simd_class& simd_value, void* dst) noexcept {
		std::memcpy(dst, &simd_value, static_cast<uint64_t>(simd_class::simd_class));
	}

	template<typename simd_class> JSONIFIER_INLINE static constexpr void store_values_u(const simd_class& simd_value, void* dst) noexcept {
		std::memcpy(dst, &simd_value, static_cast<uint64_t>(simd_class::simd_class));
	}

	template<typename integer_sequence> struct op_set1_impl;

	template<uint64_t... indices> struct op_set1_impl<std::integer_sequence<uint64_t, indices...>> {
		template<typename simd_int_t01, typename char_type>
			requires(sizeof(char_type) == 1)
		JSONIFIER_INLINE static constexpr decltype(auto) impl(char_type other) noexcept {
			simd_int_t01 result{};
			((result.values.x_int8[indices] = static_cast<int8_t>(other)), ...);
			return result;
		}

		template<typename simd_int_t01, typename char_type>
			requires(sizeof(char_type) == 8)
		JSONIFIER_INLINE static constexpr decltype(auto) impl(char_type other) noexcept {
			simd_int_t01 result{};
			((result.values.x_uint64[indices] = static_cast<uint64_t>(other)), ...);
			return result;
		}
	};

	template<typename simd_class, typename char_type> JSONIFIER_INLINE static constexpr decltype(auto) op_set1_epi8(const char_type& other) noexcept {
		return op_set1_impl<std::make_integer_sequence<uint64_t, static_cast<uint64_t>(simd_class::simd_class)>>::template impl<simd_class>(other);
	}

	template<typename simd_class> JSONIFIER_INLINE static constexpr decltype(auto) op_set1_epi64(const uint64_t& other) noexcept {
		return op_set1_impl<std::make_integer_sequence<uint64_t, static_cast<uint64_t>(simd_class::sixty_four_per)>>::template impl<simd_class>(other);
	}

	template<typename simd_class, typename... value_types>
		requires(sizeof(first_t<value_types...>) == 8 && sizeof...(value_types) == simd_class::sixty_four_per)
	JSONIFIER_INLINE static constexpr decltype(auto) op_set_epi64(const value_types&... vals) noexcept {
		return simd_class{ vals... };
	}

	template<typename integer_sequence> struct op_xor_impl;

	template<uint64_t... indices> struct op_xor_impl<std::integer_sequence<uint64_t, indices...>> {
		template<typename simd_int_t01> JSONIFIER_INLINE static constexpr simd_int_t01 impl(const simd_int_t01& a, const simd_int_t01& b) noexcept {
			simd_int_t01 result{};
			((result.values.x_uint64[indices] = a.values.x_uint64[indices] ^ b.values.x_uint64[indices]), ...);
			return result;
		}
	};

	template<typename simd_class> JSONIFIER_INLINE static constexpr auto op_xor(const simd_class& a, const simd_class& b) noexcept {
		return op_xor_impl<std::make_integer_sequence<uint64_t, static_cast<uint64_t>(simd_class::sixty_four_per)>>::impl(a, b);
	}

	template<typename integer_sequence> struct op_sub_impl;

	template<uint64_t... indices> struct op_sub_impl<std::integer_sequence<uint64_t, indices...>> {
		template<typename simd_int_t01> JSONIFIER_INLINE static constexpr decltype(auto) impl(const simd_int_t01& a, const simd_int_t01& b) noexcept {
			simd_int_t01 result{};
			((result.values.x_uint8[indices] = a.values.x_uint8[indices] - b.values.x_uint8[indices]), ...);
			return result;
		}
	};

	template<typename simd_class> JSONIFIER_INLINE static constexpr decltype(auto) op_sub(const simd_class& a, const simd_class& b) noexcept {
		return op_sub_impl<std::make_integer_sequence<uint64_t, static_cast<uint64_t>(simd_class::simd_class)>>::impl(a, b);
	}

	template<typename integer_sequence> struct op_cmp_gt_impl;

	template<uint64_t... indices> struct op_cmp_gt_impl<std::integer_sequence<uint64_t, indices...>> {
		template<typename simd_int_t01> JSONIFIER_INLINE static constexpr simd_int_t01 impl(const simd_int_t01& a, const simd_int_t01& b) noexcept {
			simd_int_t01 result{};
			((result.values.x_int8[indices] = (static_cast<int8_t>(a.values.x_int8[indices]) > static_cast<int8_t>(b.values.x_int8[indices])) ? 0x80 : 0x00), ...);
			return result;
		}
	};

	template<typename simd_class> JSONIFIER_INLINE static constexpr auto op_cmp_gt(const simd_class& a, const simd_class& b) noexcept {
		return op_cmp_gt_impl<std::make_integer_sequence<uint64_t, static_cast<uint64_t>(simd_class::simd_class)>>::impl(a, b);
	}

	template<typename integer_sequence> struct op_add_impl;

	template<uint64_t... indices> struct op_add_impl<std::integer_sequence<uint64_t, indices...>> {
		template<typename simd_int_t01> JSONIFIER_INLINE static constexpr decltype(auto) impl(const simd_int_t01& a, const simd_int_t01& b) noexcept {
			simd_int_t01 result{};
			((result.values.x_uint8[indices] = a.values.x_uint8[indices] + b.values.x_uint8[indices]), ...);
			return result;
		}
	};

	template<typename simd_class> JSONIFIER_INLINE static constexpr decltype(auto) op_add(const simd_class& a, const simd_class& b) noexcept {
		return op_add_impl<std::make_integer_sequence<uint64_t, static_cast<uint64_t>(simd_class::simd_class)>>::impl(a, b);
	}

	template<typename integer_sequence> struct op_or_impl;

	template<uint64_t... indices> struct op_or_impl<std::integer_sequence<uint64_t, indices...>> {
		template<typename simd_int_t01> JSONIFIER_INLINE static constexpr simd_int_t01 impl(const simd_int_t01& a, const simd_int_t01& b) noexcept {
			simd_int_t01 result{};
			((result.values.x_uint64[indices] = a.values.x_uint64[indices] | b.values.x_uint64[indices]), ...);
			return result;
		}
	};

	template<typename simd_class> JSONIFIER_INLINE static constexpr auto op_or(const simd_class& a, const simd_class& b) noexcept {
		return op_or_impl<std::make_integer_sequence<uint64_t, static_cast<uint64_t>(simd_class::sixty_four_per)>>::impl(a, b);
	}

	template<typename integer_sequence> struct op_and_impl;

	template<uint64_t... indices> struct op_and_impl<std::integer_sequence<uint64_t, indices...>> {
		template<typename simd_int_t01> JSONIFIER_INLINE static constexpr simd_int_t01 impl(const simd_int_t01& a, const simd_int_t01& b) noexcept {
			simd_int_t01 result{};
			((result.values.x_uint64[indices] = a.values.x_uint64[indices] & b.values.x_uint64[indices]), ...);
			return result;
		}
	};

	template<typename simd_class> JSONIFIER_INLINE static constexpr auto op_and(const simd_class& a, const simd_class& b) noexcept {
		return op_and_impl<std::make_integer_sequence<uint64_t, static_cast<uint64_t>(simd_class::sixty_four_per)>>::impl(a, b);
	}

	template<typename integer_sequence> struct op_test_impl;

	template<uint64_t... indices> struct op_test_impl<std::integer_sequence<uint64_t, indices...>> {
		template<typename simd_int_t01> JSONIFIER_INLINE static constexpr bool impl(const simd_int_t01& value) noexcept {
			return ((value.values.x_uint64[indices] != 0ULL) || ...);
		}
	};

	template<typename simd_class> JSONIFIER_INLINE static constexpr bool op_test(const simd_class& value) noexcept {
		return !op_test_impl<std::make_integer_sequence<uint64_t, static_cast<uint64_t>(simd_class::sixty_four_per)>>::impl(value);
	}

	template<typename integer_sequence> struct op_andnot_impl;

	template<uint64_t... indices> struct op_andnot_impl<std::integer_sequence<uint64_t, indices...>> {
		template<typename simd_int_t01> JSONIFIER_INLINE static constexpr simd_int_t01 impl(const simd_int_t01& a, const simd_int_t01& b) noexcept {
			simd_int_t01 result{};
			((result.values.x_uint64[indices] = (~a.values.x_uint64[indices]) & b.values.x_uint64[indices]), ...);
			return result;
		}
	};

	template<typename simd_class> JSONIFIER_INLINE static constexpr auto op_andnot(const simd_class& a, const simd_class& b) noexcept {
		return op_andnot_impl<std::make_integer_sequence<uint64_t, static_cast<uint64_t>(simd_class::sixty_four_per)>>::impl(a, b);
	}

	template<typename integer_sequence> struct op_cmp_eq_impl;

	template<uint64_t... indices> struct op_cmp_eq_impl<std::integer_sequence<uint64_t, indices...>> {
		template<typename simd_int_t01> JSONIFIER_INLINE static constexpr simd_int_t01 impl(const simd_int_t01& a, const simd_int_t01& b) noexcept {
			simd_int_t01 result{};
			((result.values.x_int8[indices] = (a.values.x_int8[indices] == b.values.x_int8[indices]) ? 0x80 : 0x00), ...);
			return result;
		}
	};

	template<typename simd_class> JSONIFIER_INLINE static constexpr auto op_cmp_eq_epi8(const simd_class& a, const simd_class& b) noexcept {
		return op_cmp_eq_impl<std::make_integer_sequence<uint64_t, static_cast<uint64_t>(simd_class::simd_class)>>::impl(a, b);
	}

}