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

#include <jsonifier-incl/simd/c_time_simd_types.hpp>
#include <jsonifier-incl/simd/simd_types.hpp>
#include <jsonifier-incl/simd/bmi.hpp>

namespace jsonifier::simd {

#if !JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_AVX) && !JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_AVX2) && !JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_AVX512) && \
	!JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_NEON)

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

	template<typename value_type> JSONIFIER_INLINE static value_type postCmpTzcnt(value_type value) noexcept {
		return tzcnt(value);
	}

	template<simd_int_128_type simd_int_type_new> JSONIFIER_INLINE static simd_int_type_new gatherValues(const void* str) noexcept {
		return gather_values<simd_int_type_new>(static_cast<const simd_x<simd_classes::x_128>*>(str));
	}

	template<simd_int_128_type simd_int_type_new> JSONIFIER_INLINE static simd_int_type_new gatherValuesU(const void* str, void*) noexcept {
		return gather_values_u<simd_int_type_new>(static_cast<const simd_x<simd_classes::x_128>*>(str));
	}

	template<simd_int_128_type simd_int_type_new, typename char_t>
		requires(sizeof(char_t) == 8)
	JSONIFIER_INLINE static simd_int_type_new gatherValue(char_t str) noexcept {
		return op_set1_epi64<simd_int_type_new>(static_cast<int64_t>(str));
	}

	template<simd_int_128_type simd_int_type_new, typename char_t>
		requires(sizeof(char_t) == 1)
	JSONIFIER_INLINE static simd_int_type_new gatherValue(char_t str, std::source_location location = std::source_location::current()) noexcept {
		return op_set1_epi8<simd_int_type_new>(static_cast<char>(str));
	}

	template<simd_int_128_type simd_int_type_new> JSONIFIER_INLINE static void store(const simd_int_type_new& value, void* storageLocation) noexcept {
		store_values(value, storageLocation);
	}

	template<simd_int_128_type simd_int_type_new> JSONIFIER_INLINE static void storeU(const simd_int_type_new& value, void* storageLocation, void*) noexcept {
		store_values_u(value, storageLocation);
	}

	template<simd_int_128_type simd_int_t01, simd_int_128_type simd_int_t02> JSONIFIER_INLINE static auto opCmpEq(const simd_int_t01& value, const simd_int_t02& other) noexcept {
		return static_cast<uint16_t>(op_cmp_eq_bitmask<simd_int_t01>(value, other));
	}

	template<simd_int_128_type simd_int_t01, simd_int_128_type simd_int_t02> JSONIFIER_INLINE static auto opCmpLt(const simd_int_t01& value, const simd_int_t02& other) noexcept {
		static constexpr jsonifier_simd_int_128 offset = op_set1_epi8<simd_int_t01>(static_cast<char>(0x80));
		return static_cast<uint32_t>(op_bitmask<simd_int_t01>(op_cmp_gt<simd_int_t01>(op_add<simd_int_t01>(other, offset), op_add<simd_int_t01>(value, offset))));
	}

	template<simd_int_128_type simd_int_t01, simd_int_128_type simd_int_t02>
	JSONIFIER_INLINE static auto opCmpEqRaw(const simd_int_t01& value, const simd_int_t02& other) noexcept {
		return op_cmp_eq_epi8<simd_int_t01>(value, other);
	}

	template<simd_int_128_type simd_int_t01, simd_int_128_type simd_int_t02>
	JSONIFIER_INLINE static auto opCmpLtRaw(const simd_int_t01& value, const simd_int_t02& other) noexcept {
		static constexpr jsonifier_simd_int_128 offset = op_set1_epi8<simd_int_t01>(static_cast<char>(0x80));
		return op_cmp_gt<simd_int_t01>(op_add<simd_int_t01>(other, offset), op_add<simd_int_t01>(value, offset));
	}

	template<simd_int_128_type simd_int_t01, simd_int_128_type simd_int_t02>
	JSONIFIER_INLINE static auto opCmpEqBitMask(const simd_int_t01& value, const simd_int_t02& other) noexcept {
		return static_cast<uint16_t>(op_cmp_eq_bitmask<simd_int_t01>(value, other));
	}

	template<simd_int_128_type simd_int_t01> JSONIFIER_INLINE static auto opBitMaskRaw(const simd_int_t01& value) noexcept {
		return op_bitmask<simd_int_t01>(value);
	}

	template<simd_int_128_type simd_int_t01, simd_int_128_type simd_int_t02> JSONIFIER_INLINE static auto opShuffle(const simd_int_t01& value, const simd_int_t02& other) noexcept {
		return op_shuffle<simd_int_t01>(value, other);
	}

	template<simd_int_128_type simd_int_t01, simd_int_128_type simd_int_t02> JSONIFIER_INLINE static auto opXor(const simd_int_t01& value, const simd_int_t02& other) noexcept {
		return op_xor<simd_int_t01>(value, other);
	}

	template<simd_int_128_type simd_int_t01, simd_int_128_type simd_int_t02> JSONIFIER_INLINE static auto opAnd(const simd_int_t01& value, const simd_int_t02& other) noexcept {
		return op_and<simd_int_t01>(value, other);
	}

	template<simd_int_128_type simd_int_t01, simd_int_128_type simd_int_t02> JSONIFIER_INLINE static auto opOr(const simd_int_t01& value, const simd_int_t02& other) noexcept {
		return op_or<simd_int_t01>(value, other);
	}

	template<simd_int_128_type simd_int_t01, simd_int_128_type simd_int_t02> JSONIFIER_INLINE static auto opAndNot(const simd_int_t01& value, const simd_int_t02& other) noexcept {
		return op_andnot<simd_int_t01>(other, value);
	}

	template<simd_int_128_type simd_int_t01> JSONIFIER_INLINE static auto opTest(const simd_int_t01& value) noexcept {
		return !op_test<simd_int_t01>(value);
	}

	template<simd_int_128_type simd_int_t01> JSONIFIER_INLINE static auto opNot(const simd_int_t01& value) noexcept {
		return op_xor<simd_int_t01>(value, op_set1_epi64<simd_int_t01>(0xFFFFFFFFFFFFFFFFll));
	}

	template<simd_int_128_type simd_type> JSONIFIER_INLINE static jsonifier_simd_int_128 opSetLSB(const simd_type& value, bool valueNew) noexcept {
		static constexpr jsonifier_simd_int_128 mask{ op_set_epi64<simd_type>(0x01uLL, 0ULL) };
		return valueNew ? op_or<jsonifier_simd_int_128>(value, mask) : op_andnot<jsonifier_simd_int_128>(mask, value);
	}

	template<simd_int_128_type simd_type> JSONIFIER_INLINE static bool opGetMSB(const simd_type& value) noexcept {
		const simd_type result = op_and<simd_type>(value, op_set_epi64<simd_type>(0x00ll, 0x8000000000000000ll));
		return !op_test<simd_type>(result);
	}

#endif

}
