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
#include <jsonifier-incl/simd/simd_common.hpp>
#include <jsonifier-incl/utilities/string_literal.hpp>

namespace jsonifier::internal {

	template<const uint8_t repeat, concepts::unsigned_t return_type> static constexpr return_type repeatByte() noexcept {
		return static_cast<return_type>(0x0101010101010101ull) * static_cast<return_type>(repeat);
	}

	template<char valueNewer, typename char_type> struct char_comparison {
		static constexpr char value{ valueNewer };
		JSONIFIER_INLINE static const char_type* memchar(const char_type* data, uint64_t lengthNew) noexcept {
#if JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_AVX512)
			if (lengthNew >= 64) {
				using simd_type						 = typename get_type_at_index<simd::avx_list, 0>::type::type::type;
				using integer_type					 = typename get_type_at_index<simd::avx_list, 0>::type::integer_type;
				static constexpr uint64_t vectorSize = get_type_at_index<simd::avx_list, 0>::type::bytesProcessed;
				const simd_type search_value		 = simd::gatherValue<simd_type>(static_cast<uint8_t>(value));
				JSONIFIER_ALIGN(64) char valuesToLoad[64];
				integer_type mask;
				simd_type chunk;
				while (lengthNew >= vectorSize) {
					std::memcpy(valuesToLoad, data, 64);
					chunk = simd::gatherValues<simd_type>(valuesToLoad);
					mask  = simd::opCmpEq(chunk, search_value);
					if JSONIFIER_UNLIKELY (mask != 0) {
						data += simd::postCmpTzcnt(mask);
						return data;
					}
					lengthNew -= vectorSize;
					data += vectorSize;
				}
			}
#endif

#if JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_AVX512) || JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_AVX2)
			if (lengthNew >= 32) {
				using simd_type						 = typename get_type_at_index<simd::avx_list, 1>::type::type::type;
				using integer_type					 = typename get_type_at_index<simd::avx_list, 1>::type::integer_type;
				static constexpr uint64_t vectorSize = get_type_at_index<simd::avx_list, 1>::type::bytesProcessed;
				const simd_type search_value		 = simd::gatherValue<simd_type>(static_cast<uint8_t>(value));
				JSONIFIER_ALIGN(32) char valuesToLoad[32];
				integer_type mask;
				simd_type chunk;
				while (lengthNew >= vectorSize) {
					std::memcpy(valuesToLoad, data, 32);
					chunk = simd::gatherValues<simd_type>(valuesToLoad);
					mask  = simd::opCmpEq(chunk, search_value);
					if JSONIFIER_UNLIKELY (mask != 0) {
						data += simd::postCmpTzcnt(mask);
						return data;
					}
					lengthNew -= vectorSize;
					data += vectorSize;
				}
			}
#endif
			if (lengthNew >= 16) {
				using simd_type						 = typename get_type_at_index<simd::avx_list, 2>::type::type::type;
				using integer_type					 = typename get_type_at_index<simd::avx_list, 2>::type::integer_type;
				static constexpr uint64_t vectorSize = get_type_at_index<simd::avx_list, 2>::type::bytesProcessed;
				const simd_type search_value		 = simd::gatherValue<simd_type>(static_cast<uint8_t>(value));
				JSONIFIER_ALIGN(16) char valuesToLoad[16];
				integer_type mask;
				simd_type chunk;
				while (lengthNew >= vectorSize) {
					std::memcpy(valuesToLoad, data, 16);
					chunk = simd::gatherValues<simd_type>(valuesToLoad);
					mask  = static_cast<integer_type>(simd::opCmpEq(chunk, search_value));
					if JSONIFIER_UNLIKELY (mask != 0) {
						data += simd::postCmpTzcnt(mask);
						return data;
					}
					lengthNew -= vectorSize;
					data += vectorSize;
				}
			}
			{
				static constexpr uint64_t nBytes{ sizeof(uint64_t) };
				if (lengthNew >= nBytes) {
					static constexpr auto valueNew{ repeatByte<value, uint64_t>() };
					static constexpr auto highBits{ repeatByte<0x80, uint64_t>() };
					static constexpr auto lowBits{ repeatByte<0x01, uint64_t>() };
					uint64_t simdValue;
					std::memcpy(&simdValue, data, sizeof(uint64_t));
					const auto chunk = simdValue ^ valueNew;
					auto next		 = ((chunk - lowBits) & ~chunk) & highBits;
					if (next) {
						next = simd::tzcnt(static_cast<uint64_t>(next)) >> 3u;
						data += next;
						return data;
					} else {
						lengthNew -= nBytes;
						data += nBytes;
					}
				}
			}
			{
				static constexpr uint64_t nBytes{ sizeof(uint32_t) };
				if (lengthNew >= nBytes) {
					static constexpr auto valueNew{ repeatByte<value, uint32_t>() };
					static constexpr auto highBits{ repeatByte<0x80, uint32_t>() };
					static constexpr auto lowBits{ repeatByte<0x01, uint32_t>() };
					uint32_t simdValue;
					std::memcpy(&simdValue, data, sizeof(uint32_t));
					const auto chunk = simdValue ^ valueNew;
					auto next		 = ((chunk - lowBits) & ~chunk) & highBits;
					if (next) {
						next = simd::tzcnt(static_cast<uint32_t>(next)) >> 3u;
						data += next;
						return data;
					} else {
						lengthNew -= nBytes;
						data += nBytes;
					}
				}
			}
			{
				static constexpr uint64_t nBytes{ sizeof(uint16_t) };
				if (lengthNew >= nBytes) {
					static constexpr auto valueNew{ repeatByte<value, uint16_t>() };
					static constexpr auto highBits{ repeatByte<0x80, uint16_t>() };
					static constexpr auto lowBits{ repeatByte<0x01, uint16_t>() };
					uint16_t simdValue;
					std::memcpy(&simdValue, data, sizeof(uint16_t));
					const auto chunk = simdValue ^ valueNew;
					auto next		 = ((chunk - lowBits) & ~chunk) & highBits;
					if (next) {
						next = simd::tzcnt(static_cast<uint16_t>(next)) >> 3u;
						data += next;
						return data;
					} else {
						lengthNew -= nBytes;
						data += nBytes;
					}
				}
			}

			if (lengthNew > 0 && *data == value) {
				++data;
				return data;
			}
			return static_cast<decltype(data)>(nullptr);
		}
	};

	struct comparison {
		template<typename char_type01, typename char_type02> JSONIFIER_INLINE static bool compare(const char_type01* lhs, char_type02* rhs, uint64_t lengthNew) noexcept {
#if JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_AVX512)
			if (lengthNew >= 64) {
				using simd_type						 = typename get_type_at_index<simd::avx_list, 0>::type::type::type;
				static constexpr uint64_t vectorSize = get_type_at_index<simd::avx_list, 0>::type::bytesProcessed;
				static constexpr uint64_t mask		 = get_type_at_index<simd::avx_list, 0>::type::mask;
				JSONIFIER_ALIGN(64) char valuesToLoad[64];
				simd_type value01, value02;
				while (lengthNew >= vectorSize) {
					std::memcpy(valuesToLoad, lhs, 64);
					value01 = simd::gatherValues<simd_type>(valuesToLoad);
					std::memcpy(valuesToLoad, rhs, 64);
					value02 = simd::gatherValues<simd_type>(valuesToLoad);
					if (simd::opCmpEq(value01, value02) != mask) {
						return false;
					}
					lengthNew -= vectorSize;
					lhs += vectorSize;
					rhs += vectorSize;
				}
			}
#endif
#if JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_AVX512) || JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_AVX2)
			if (lengthNew >= 32) {
				using simd_type						 = typename get_type_at_index<simd::avx_list, 1>::type::type::type;
				static constexpr uint64_t vectorSize = get_type_at_index<simd::avx_list, 1>::type::bytesProcessed;
				static constexpr uint64_t mask		 = get_type_at_index<simd::avx_list, 1>::type::mask;
				JSONIFIER_ALIGN(32) char valuesToLoad[32];
				simd_type value01, value02;
				while (lengthNew >= vectorSize) {
					std::memcpy(valuesToLoad, lhs, 32);
					value01 = simd::gatherValues<simd_type>(valuesToLoad);
					std::memcpy(valuesToLoad, rhs, 32);
					value02 = simd::gatherValues<simd_type>(valuesToLoad);
					if (simd::opCmpEq(value01, value02) != mask) {
						return false;
					}
					lengthNew -= vectorSize;
					lhs += vectorSize;
					rhs += vectorSize;
				}
			}
#endif
			if (lengthNew >= 16) {
				using simd_type						 = typename get_type_at_index<simd::avx_list, 2>::type::type::type;
				static constexpr uint64_t vectorSize = get_type_at_index<simd::avx_list, 2>::type::bytesProcessed;
				static constexpr uint64_t mask		 = get_type_at_index<simd::avx_list, 2>::type::mask;
				JSONIFIER_ALIGN(16) char valuesToLoad[16];
				simd_type value01, value02;
				while (lengthNew >= vectorSize) {
					std::memcpy(valuesToLoad, lhs, 16);
					value01 = simd::gatherValues<simd_type>(valuesToLoad);
					std::memcpy(valuesToLoad, rhs, 16);
					value02 = simd::gatherValues<simd_type>(valuesToLoad);
					if (simd::opCmpEq(value01, value02) != mask) {
						return false;
					}
					lengthNew -= vectorSize;
					lhs += vectorSize;
					rhs += vectorSize;
				}
			}
			{
				static constexpr uint64_t nBytes{ sizeof(uint64_t) };
				if (lengthNew >= nBytes) {
					uint64_t v1, v2;
					std::memcpy(&v1, lhs, nBytes);
					std::memcpy(&v2, rhs, nBytes);
					if ((v1 ^ v2) != 0) {
						return false;
					}
					lengthNew -= nBytes;
					lhs += nBytes;
					rhs += nBytes;
				}
			}
			{
				static constexpr uint64_t nBytes{ sizeof(uint32_t) };
				if (lengthNew >= nBytes) {
					uint32_t v1, v2;
					std::memcpy(&v1, lhs, nBytes);
					std::memcpy(&v2, rhs, nBytes);
					if ((v1 ^ v2) != 0) {
						return false;
					}
					lengthNew -= nBytes;
					lhs += nBytes;
					rhs += nBytes;
				}
			}
			{
				static constexpr uint64_t nBytes{ sizeof(uint16_t) };
				if (lengthNew >= nBytes) {
					uint16_t v1, v2;
					std::memcpy(&v1, lhs, nBytes);
					std::memcpy(&v2, rhs, nBytes);
					if ((v1 ^ v2) != 0) {
						return false;
					}
					lengthNew -= nBytes;
					lhs += nBytes;
					rhs += nBytes;
				}
			}
			if (lengthNew && *lhs != *rhs) {
				return false;
			}
			return true;
		}
	};

	template<uint64_t length> struct convert_length_to_int {
		static_assert(length <= 8, "Sorry, but that string is too long!");
		using type = jsonifier::internal::conditional_t<length == 1, uint8_t,
			jsonifier::internal::conditional_t<length <= 2, uint16_t,
				jsonifier::internal::conditional_t<length <= 4, uint32_t, jsonifier::internal::conditional_t<length <= 8, uint64_t, void>>>>;
	};

	template<uint64_t length> using convert_length_to_int_t = typename convert_length_to_int<length>::type;

	template<string_literal string>
		requires(string.length == 0)
	static constexpr auto packValues() {
		return uint8_t{};
	}

	template<string_literal string>
		requires(string.length > 0 && string.length <= 8)
	static constexpr auto packValues() {
		convert_length_to_int_t<string.length> returnValues{};
		for (uint64_t x = 0; x < string.length; ++x) {
			returnValues |= static_cast<convert_length_to_int_t<string.length>>(static_cast<uint64_t>(string[x]) << ((x % 8) * 8));
		}
		return returnValues;
	}

	template<uint64_t size> static constexpr uint64_t getPackingSize() {
		if constexpr (size >= 64) {
			return 64;
		} else if constexpr (size >= 32) {
			return 32;
		} else {
			return 16;
		}
	}

	template<string_literal string>
		requires(string.length != 0 && string.length > 8)
	static constexpr auto packValues() {
		JSONIFIER_ALIGN(16) array<uint64_t, roundUpToMultiple<16>(getPackingSize<string.length>())> returnValues{};
		for (uint64_t x = 0; x < string.length; ++x) {
			if (x / 8 < (string.length / 8) + 1) {
				returnValues[x / 8] |= (static_cast<uint64_t>(string[x]) << ((x % 8) * 8));
			}
		}
		return returnValues;
	}

	template<typename value_type>
	concept equals_0 = value_type::length == 0;

	template<typename value_type>
	concept gt_0_lt_16 = value_type::length > 0 && value_type::length < 16;

	template<typename value_type>
	concept eq_16 = value_type::length == 16 && bytesPerStep >= 16;

	template<typename value_type>
	concept eq_32 = value_type::length == 32 && bytesPerStep >= 32;

	template<typename value_type>
	concept eq_64 = value_type::length == 64 && bytesPerStep >= 64;

	template<typename value_type>
	concept gt_16 = value_type::length > 16 && !eq_16<value_type> && !eq_32<value_type> && !eq_64<value_type>;

	struct string_status {
		uint64_t remaining_length{};
		uint64_t current_index{};
	};

	template<auto string, uint64_t offset> static constexpr string_literal<((offset >= string.length) ? string.length : offset) + 1> offSetIntoLiteral() noexcept {
		constexpr uint64_t newSize		= ((offset >= string.length) ? string.length : offset);
		string_literal<newSize + 1> sl{};
		if constexpr (newSize > 0) {
			std::copy_n(string.data(), newSize, sl.values);
			sl.values[newSize] = '\0';
		}
		return sl;
	}

	template<auto string, uint64_t offset> static constexpr string_literal<((offset >= string.length) ? 0 : string.length - offset) + 1> offSetNewLiteral() noexcept {
		constexpr uint64_t newSize		= (offset >= string.length) ? 0 : string.length - offset;
		string_literal<newSize + 1> sl{};
		if constexpr (newSize > 0) {
			std::copy_n(string.data() + offset, newSize, sl.values);
			sl.values[newSize] = '\0';
		}
		return sl;
	}

	template<uint64_t split_mod, uint64_t size> static consteval uint64_t get_split_string_count(const string_literal<size>&) noexcept {
		constexpr uint64_t split_count = (size + split_mod - 1ull) / split_mod;
		return split_count == 0ull ? 1ull : split_count;
	}

	template<string_status status, auto string_new, uint64_t split_mod> struct string_literal_splitter_impl {
		static constexpr uint64_t current_length{ split_mod < status.remaining_length ? split_mod : status.remaining_length };
		static constexpr auto string{ offSetIntoLiteral<offSetNewLiteral<string_new, status.current_index>(), current_length>() };
	};

	template<typename sequence, auto string_new, uint64_t split_mod> struct string_literal_splitter;

	template<uint64_t index, typename string_type> static constexpr string_literal<index + 1> stringLiteralFromView(string_type str) noexcept {
		string_literal<index + 1> sl{};
		std::copy_n(str.data(), str.size(), sl.values);
		sl[index] = '\0';
		return sl;
	}

	static constexpr auto getOffsetIntoLiteralSize(uint64_t inputSize) noexcept {
		if (inputSize >= 64ULL && bytesPerStep >= 64ULL) {
			return 64ULL;
		} else if (inputSize >= 32ULL && bytesPerStep >= 32ULL) {
			return 32ULL;
		} else {
			return 16ULL;
		}
	}

	template<typename sl_type, jsonifier::internal::remove_cvref_t<sl_type> stringNew, typename = void> struct string_literal_comparitor;

	template<uint64_t... indices, auto string_new, uint64_t split_mod> struct string_literal_splitter<integer_sequence<indices...>, string_new, split_mod> {
		static consteval string_status get_split_string_status(uint64_t index, uint64_t total_length) noexcept {
			uint64_t current_index{ split_mod * index };
			string_status status{ total_length, current_index };
			status.remaining_length -= current_index;
			return status;
		}

		JSONIFIER_INLINE static string_view_ptr impl(string_view_ptr str) noexcept {
			return (( void )((
						(str = string_literal_comparitor<decltype(string_literal_splitter_impl<get_split_string_status(indices, string_new.size()), string_new, split_mod>::string),
							 string_literal_splitter_impl<get_split_string_status(indices, string_new.size()), string_new, split_mod>::string>::impl(str),
							str != nullptr) &&
						...)),
				str);
		}
	};

	template<equals_0 sl_type, jsonifier::internal::remove_cvref_t<sl_type> stringNew> struct string_literal_comparitor<sl_type, stringNew, void> {
		JSONIFIER_INLINE static string_view_ptr impl(string_view_ptr str) noexcept {
			return str;
		}
	};

	template<gt_0_lt_16 sl_type, jsonifier::internal::remove_cvref_t<sl_type> stringNew> struct string_literal_comparitor<sl_type, stringNew, void> {
		JSONIFIER_INLINE static string_view_ptr impl(string_view_ptr str) noexcept {
			static constexpr auto stringLiteral{ stringNew };
			static constexpr auto newCount{ stringLiteral.size() };
			if constexpr (newCount > 8) {
				JSONIFIER_ALIGN(16) static constexpr auto valuesNew{ packValues<stringLiteral>() };
				jsonifier_simd_int_128 data1{};
				std::memcpy(&data1, str, newCount);
				const jsonifier_simd_int_128 data2{ simd::gatherValues<jsonifier_simd_int_128>(valuesNew.data()) };
				return !simd::opTest(simd::opXor(data1, data2)) ? str + newCount : nullptr;
			} else if constexpr (newCount == 8) {
				static constexpr auto valuesNew{ packValues<stringLiteral>() };
				uint64_t l;
				std::memcpy(&l, str, 8);
				return !(l ^ valuesNew) ? str + newCount : nullptr;
			} else if constexpr (newCount == 7) {
				static constexpr auto valuesNew{ packValues<stringLiteral>() };
				uint64_t l{};
				std::memcpy(&l, str, 7);
				return !(l ^ valuesNew) ? str + newCount : nullptr;
			} else if constexpr (newCount == 6) {
				static constexpr auto valuesNew{ packValues<stringLiteral>() };
				uint64_t l{};
				std::memcpy(&l, str, 6);
				return !(l ^ valuesNew) ? str + newCount : nullptr;
			} else if constexpr (newCount == 5) {
				static constexpr uint32_t valuesNew{ static_cast<uint32_t>(packValues<stringLiteral>()) };
				uint32_t l;
				std::memcpy(&l, str, 4);
				return (!(l ^ valuesNew) && (str[4] == stringLiteral[4])) ? str + newCount : nullptr;
			} else if constexpr (newCount == 4) {
				static constexpr uint32_t valuesNew{ static_cast<uint32_t>(packValues<stringLiteral>()) };
				uint32_t l;
				std::memcpy(&l, str, 4);
				return !(l ^ valuesNew) ? str + newCount : nullptr;
			} else if constexpr (newCount == 3) {
				static constexpr uint16_t valuesNew{ static_cast<uint16_t>(packValues<stringLiteral>()) };
				uint16_t l;
				std::memcpy(&l, str, 2);
				return (!(l ^ valuesNew) && (str[2] == stringLiteral[2])) ? str + newCount : nullptr;
			} else if constexpr (newCount == 2) {
				static constexpr uint16_t valuesNew{ static_cast<uint16_t>(packValues<stringLiteral>()) };
				uint16_t l;
				std::memcpy(&l, str, 2);
				return !(l ^ valuesNew) ? str + newCount : nullptr;
			} else if constexpr (newCount == 1) {
				return (*str == stringLiteral[0]) ? str + newCount : nullptr;
			} else {
				return str;
			}
		}
	};

	template<eq_16 sl_type, jsonifier::internal::remove_cvref_t<sl_type> stringNew> struct string_literal_comparitor<sl_type, stringNew, void> {
		JSONIFIER_INLINE static string_view_ptr impl(string_view_ptr str) noexcept {
			static constexpr auto newLiteral{ stringNew };
			JSONIFIER_ALIGN(16) static constexpr auto valuesNew{ packValues<newLiteral>() };
			JSONIFIER_ALIGN(16) char valuesToLoad[16];
			std::memcpy(valuesToLoad, str, 16);
			const jsonifier_simd_int_128 data1{ simd::gatherValues<jsonifier_simd_int_128>(valuesToLoad) };
			const jsonifier_simd_int_128 data2{ simd::gatherValues<jsonifier_simd_int_128>(valuesNew.data()) };
			return !simd::opTest(simd::opXor(data1, data2)) ? str + 16 : nullptr;
		}
	};

#if JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_AVX512) || JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_AVX2)

	template<eq_32 sl_type, jsonifier::internal::remove_cvref_t<sl_type> stringNew> struct string_literal_comparitor<sl_type, stringNew, void> {
		JSONIFIER_INLINE static string_view_ptr impl(string_view_ptr str) noexcept {
			static constexpr auto newLiteral{ stringNew };
			JSONIFIER_ALIGN(32) static constexpr auto valuesNew{ packValues<newLiteral>() };
			JSONIFIER_ALIGN(32) char valuesToLoad[32];
			std::memcpy(valuesToLoad, str, 32);
			const jsonifier_simd_int_256 data1{ simd::gatherValues<jsonifier_simd_int_256>(valuesToLoad) };
			const jsonifier_simd_int_256 data2{ simd::gatherValues<jsonifier_simd_int_256>(valuesNew.data()) };
			return !simd::opTest(simd::opXor(data1, data2)) ? str + 32 : nullptr;
		}
	};

#endif

#if JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_AVX512)
	template<eq_64 sl_type, jsonifier::internal::remove_cvref_t<sl_type> stringNew> struct string_literal_comparitor<sl_type, stringNew, void> {
		JSONIFIER_INLINE static string_view_ptr impl(string_view_ptr str) noexcept {
			static constexpr auto newLiteral{ stringNew };
			JSONIFIER_ALIGN(64) static constexpr auto valuesNew{ packValues<newLiteral>() };
			JSONIFIER_ALIGN(64) char valuesToLoad[64];
			std::memcpy(valuesToLoad, str, 64);
			const jsonifier_simd_int_512 data1{ simd::gatherValues<jsonifier_simd_int_512>(valuesToLoad) };
			const jsonifier_simd_int_512 data2{ simd::gatherValues<jsonifier_simd_int_512>(valuesNew.data()) };
			return !simd::opTest(simd::opXor(data1, data2)) ? str + 64 : nullptr;
		}
	};
#endif

	template<gt_16 sl_type, jsonifier::internal::remove_cvref_t<sl_type> stringNew> struct string_literal_comparitor<sl_type, stringNew, void> {
		static constexpr uint64_t split_mod{ getOffsetIntoLiteralSize(stringNew.size()) };
		static constexpr auto string_count{ get_split_string_count<split_mod>(stringNew) };

		JSONIFIER_INLINE static string_view_ptr impl(string_view_ptr str) noexcept {
			return string_literal_splitter<make_integer_sequence<string_count>, stringNew, split_mod>::impl(str);
		}
	};

	template<auto string> struct string_literal_comparator {
		JSONIFIER_INLINE static bool impl(string_view_ptr str, uint64_t string_length) noexcept {
			using sl_type = decltype(string);
			return string_length >= string.size() ? string_literal_comparitor<sl_type, string>::impl(str) != nullptr : false;
		}
	};

};
