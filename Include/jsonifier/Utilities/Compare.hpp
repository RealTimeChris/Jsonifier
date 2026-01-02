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

#include <jsonifier/Utilities/TypeEntities.hpp>
#include <jsonifier/Simd/SimdCommon.hpp>
#include <jsonifier/Utilities/StringLiteral.hpp>

namespace jsonifier::internal {

	template<const uint8_t repeat, concepts::uns16_t return_type> static constexpr return_type repeatByte() noexcept {
		return 0x0101ull * repeat;
	}

	template<const uint8_t repeat, concepts::uns32_t return_type> static constexpr return_type repeatByte() noexcept {
		return 0x01010101ull * repeat;
	}

	template<const uint8_t repeat, concepts::uns64_t return_type> static constexpr return_type repeatByte() noexcept {
		return 0x0101010101010101ull * repeat;
	}

	template<char valueNewer, typename char_type> struct char_comparison {
		static constexpr char value{ valueNewer };
		JSONIFIER_INLINE static const char_type* memchar(const char_type* data, uint64_t lengthNew) noexcept {
#if JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_AVX512)
			if (lengthNew >= 64) {
				using simd_type						 = typename get_type_at_index<simd::avx_list, 2>::type::type::type;
				using integer_type					 = typename get_type_at_index<simd::avx_list, 2>::type::integer_type;
				static constexpr uint64_t vectorSize = get_type_at_index<simd::avx_list, 2>::type::bytesProcessed;
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
				using simd_type						 = typename get_type_at_index<simd::avx_list, 0>::type::type::type;
				using integer_type					 = typename get_type_at_index<simd::avx_list, 0>::type::integer_type;
				static constexpr uint64_t vectorSize = get_type_at_index<simd::avx_list, 0>::type::bytesProcessed;
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
				using simd_type						 = typename get_type_at_index<simd::avx_list, 2>::type::type::type;
				static constexpr uint64_t vectorSize = get_type_at_index<simd::avx_list, 2>::type::bytesProcessed;
				static constexpr uint64_t mask		 = get_type_at_index<simd::avx_list, 2>::type::mask;
				JSONIFIER_ALIGN(64) char valuesToLoad[64];
				simd_type value01, value02;
				while (lengthNew >= vectorSize) {
					std::memcpy(valuesToLoad, lhs, 64);
					value01 = simd::gatherValues<simd_type>(valuesToLoad);
					std::memcpy(valuesToLoad, rhs, 64);
					value02 = simd::gatherValues<simd_type>(valuesToLoad);
					if (simd::opCmpEq(value01, value02) != mask) {
						return false;
					};
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
					};
					lengthNew -= vectorSize;
					lhs += vectorSize;
					rhs += vectorSize;
				}
			}
#endif
			if (lengthNew >= 16) {
				using simd_type						 = typename get_type_at_index<simd::avx_list, 0>::type::type::type;
				static constexpr uint64_t vectorSize = get_type_at_index<simd::avx_list, 0>::type::bytesProcessed;
				static constexpr uint64_t mask		 = get_type_at_index<simd::avx_list, 0>::type::mask;
				JSONIFIER_ALIGN(16) char valuesToLoad[16];
				simd_type value01, value02;
				while (lengthNew >= vectorSize) {
					std::memcpy(valuesToLoad, lhs, 16);
					value01 = simd::gatherValues<simd_type>(valuesToLoad);
					std::memcpy(valuesToLoad, rhs, 16);
					value02 = simd::gatherValues<simd_type>(valuesToLoad);
					if (simd::opCmpEq(value01, value02) != mask) {
						return false;
					};
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
		for (size_t x = 0; x < string.length; ++x) {
			returnValues |= static_cast<convert_length_to_int_t<string.length>>(static_cast<uint64_t>(string[x]) << ((x % 8) * 8));
		}
		return returnValues;
	}

	template<size_t size> static constexpr size_t getPackingSize() {
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
		for (size_t x = 0; x < string.length; ++x) {
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

	template<size_t index, typename string_type> static constexpr auto stringLiteralFromView(string_type str) noexcept {
		string_literal<index + 1> sl{};
		std::copy_n(str.data(), str.size(), sl.values);
		sl[index] = '\0';
		return sl;
	}

	template<string_literal string, size_t offset> static constexpr auto offSetNewLiteral() noexcept {
		constexpr size_t originalSize = string.length;
		constexpr size_t newSize	  = (offset >= originalSize) ? 0 : originalSize - offset;
		string_literal<newSize + 1> sl{};
		if constexpr (newSize > 0) {
			std::copy_n(string.data() + offset, newSize, sl.values);
			sl.values[newSize] = '\0';
		}
		return sl;
	}

	template<string_literal string, size_t offset> static constexpr auto offSetIntoLiteral() noexcept {
		constexpr size_t originalSize = string.length;
		constexpr size_t newSize	  = (offset >= originalSize) ? originalSize : offset;
		string_literal<newSize + 1> sl{};
		if constexpr (newSize > 0) {
			std::copy_n(string.data(), newSize, sl.values);
			sl.values[newSize] = '\0';
		}
		return sl;
	}

	template<typename sl_type, jsonifier::internal::remove_cvref_t<sl_type> stringNew> struct string_literal_comparitor;

	template<equals_0 sl_type, jsonifier::internal::remove_cvref_t<sl_type> stringNew> struct string_literal_comparitor<sl_type, stringNew> {
		JSONIFIER_INLINE static bool impl(string_view_ptr) noexcept {
			return true;
		}
	};

	template<gt_0_lt_16 sl_type, jsonifier::internal::remove_cvref_t<sl_type> stringNew> struct string_literal_comparitor<sl_type, stringNew> {
		JSONIFIER_INLINE static bool impl(string_view_ptr str) noexcept {
			static constexpr auto stringLiteral{ stringNew };
			static constexpr auto newCount{ stringLiteral.size() };
			if constexpr (newCount > 8) {
				JSONIFIER_ALIGN(16) static constexpr auto valuesNew{ packValues<stringLiteral>() };
				jsonifier_simd_int_128 data1{};
				std::memcpy(&data1, str, newCount);
				const jsonifier_simd_int_128 data2{ simd::gatherValues<jsonifier_simd_int_128>(valuesNew.data()) };
				return !simd::opTest(simd::opXor(data1, data2));
			} else if constexpr (newCount == 8) {
				static constexpr auto valuesNew{ packValues<stringLiteral>() };
				uint64_t l;
				std::memcpy(&l, str, 8);
				return !(l ^ valuesNew);
			} else if constexpr (newCount == 7) {
				static constexpr auto valuesNew{ packValues<stringLiteral>() };
				uint64_t l{};
				std::memcpy(&l, str, 7);
				return !(l ^ valuesNew);
			} else if constexpr (newCount == 6) {
				static constexpr auto valuesNew{ packValues<stringLiteral>() };
				uint64_t l{};
				std::memcpy(&l, str, 6);
				return !(l ^ valuesNew);
			} else if constexpr (newCount == 5) {
				static constexpr uint32_t valuesNew{ static_cast<uint32_t>(packValues<stringLiteral>()) };
				uint32_t l;
				std::memcpy(&l, str, 4);
				return !(l ^ valuesNew) && (str[4] == stringLiteral[4]);
			} else if constexpr (newCount == 4) {
				static constexpr uint32_t valuesNew{ static_cast<uint32_t>(packValues<stringLiteral>()) };
				uint32_t l;
				std::memcpy(&l, str, 4);
				return !(l ^ valuesNew);
			} else if constexpr (newCount == 3) {
				static constexpr uint16_t valuesNew{ static_cast<uint16_t>(packValues<stringLiteral>()) };
				uint16_t l;
				std::memcpy(&l, str, 2);
				return !(l ^ valuesNew) && (str[2] == stringLiteral[2]);
			} else if constexpr (newCount == 2) {
				static constexpr uint16_t valuesNew{ static_cast<uint16_t>(packValues<stringLiteral>()) };
				uint16_t l;
				std::memcpy(&l, str, 2);
				return !(l ^ valuesNew);
			} else if constexpr (newCount == 1) {
				return *str == stringLiteral[0];
			} else {
				return true;
			}
		};
	};

	template<eq_16 sl_type, jsonifier::internal::remove_cvref_t<sl_type> stringNew> struct string_literal_comparitor<sl_type, stringNew> {
		JSONIFIER_INLINE static bool impl(string_view_ptr str) noexcept {
			static constexpr auto newLiteral{ stringNew };
			JSONIFIER_ALIGN(16) static constexpr auto valuesNew{ packValues<newLiteral>() };
			JSONIFIER_ALIGN(16) char valuesToLoad[16];
			std::memcpy(valuesToLoad, str, 16);
			const jsonifier_simd_int_128 data1{ simd::gatherValues<jsonifier_simd_int_128>(valuesToLoad) };
			const jsonifier_simd_int_128 data2{ simd::gatherValues<jsonifier_simd_int_128>(valuesNew.data()) };
			return !simd::opTest(simd::opXor(data1, data2));
		}
	};

#if JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_AVX512) || JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_AVX2)

	template<eq_32 sl_type, jsonifier::internal::remove_cvref_t<sl_type> stringNew> struct string_literal_comparitor<sl_type, stringNew> {
		JSONIFIER_INLINE static bool impl(string_view_ptr str) noexcept {
			static constexpr auto newLiteral{ stringNew };
			JSONIFIER_ALIGN(32) static constexpr auto valuesNew{ packValues<newLiteral>() };
			JSONIFIER_ALIGN(32) char valuesToLoad[32];
			std::memcpy(valuesToLoad, str, 32);
			const jsonifier_simd_int_256 data1{ simd::gatherValues<jsonifier_simd_int_256>(valuesToLoad) };
			const jsonifier_simd_int_256 data2{ simd::gatherValues<jsonifier_simd_int_256>(valuesNew.data()) };
			return !simd::opTest(simd::opXor(data1, data2));
		}
	};

#endif

#if JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_AVX512)
	template<eq_64 sl_type, sl_type stringNew> struct string_literal_comparitor<sl_type, stringNew> {
		JSONIFIER_INLINE static bool impl(string_view_ptr str) noexcept {
			static constexpr auto newLiteral{ stringNew };
			JSONIFIER_ALIGN(64) static constexpr auto valuesNew{ packValues<newLiteral>() };
			JSONIFIER_ALIGN(64) char valuesToLoad[64];
			std::memcpy(valuesToLoad, str, 64);
			const jsonifier_simd_int_512 data1{ simd::gatherValues<jsonifier_simd_int_512>(valuesToLoad) };
			const jsonifier_simd_int_512 data2{ simd::gatherValues<jsonifier_simd_int_512>(valuesNew.data()) };
			return !simd::opTest(simd::opXor(data1, data2));
		}
	};
#endif

	static constexpr auto getOffsetIntoLiteralSize(size_t inputSize) noexcept {
		if (inputSize >= 64 && bytesPerStep >= 64) {
			return 64;
		} else if (inputSize >= 32 && bytesPerStep >= 32) {
			return 32;
		} else {
			return 16;
		}
	}

	template<gt_16 sl_type, jsonifier::internal::remove_cvref_t<sl_type> stringNew> struct string_literal_comparitor<sl_type, stringNew> {
		JSONIFIER_INLINE static bool impl(string_view_ptr str) noexcept {
			static constexpr auto string{ offSetIntoLiteral<stringNew, getOffsetIntoLiteralSize(stringNew.size())>() };
			if (!string_literal_comparitor<decltype(string), string>::impl(str)) {
				return false;
			} else {
				static constexpr auto stringSize = string.size();
				str += stringSize;
				static constexpr auto stringNewer{ offSetNewLiteral<stringNew, stringSize>() };
				return string_literal_comparitor<decltype(stringNewer), stringNewer>::impl(str);
			}
		}
	};

};
