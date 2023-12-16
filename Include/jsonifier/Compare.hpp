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

#include <jsonifier/ISADetection.hpp>

namespace jsonifier_internal {

	using integer_list = jsonifier::concepts::type_list<uint64_t, uint32_t, uint16_t, uint8_t>;

	template<uint64_t index = 0, typename char_type01, typename char_type02> JSONIFIER_INLINE bool compareShort(char_type01* string1, char_type02* string2, uint64_t lengthNew) {
#if defined(_WIN32)
		using integer_type = typename jsonifier::concepts::get_type_at_index<integer_list, index>::type;
		static constexpr uint64_t size{ sizeof(integer_type) };
		integer_type value01[2]{};
		while (lengthNew >= size) {
			std::memcpy(value01, string1, sizeof(integer_type));
			std::memcpy(value01 + 1, string2, sizeof(integer_type));
			lengthNew -= size;
			string1 += size;
			string2 += size;
			if (value01[0] != value01[1]) {
				return false;
			}
		}
		if constexpr (index < integer_list::size - 1) {
			return compareShort<index + 1>(string1, string2, lengthNew);
		} else {
			return true;
		}
#else
		return std::memcmp(string1, string2, lengthNew) == 0;
#endif
	}

#if JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_ANY_AVX)

	template<uint64_t index = 0, typename char_type01, typename char_type02> JSONIFIER_INLINE bool compare(char_type01* string1, char_type02* string2, uint64_t lengthNew) {
	#if defined(_WIN32)
		using integer_type					 = typename jsonifier::concepts::get_type_at_index<avx_list, index>::type::integer_type;
		using simd_type						 = typename jsonifier::concepts::get_type_at_index<avx_list, index>::type::type;
		static constexpr uint64_t vectorSize = sizeof(simd_type);
		static constexpr integer_type maskValue{ jsonifier::concepts::get_type_at_index<avx_list, index>::type::mask };
		while (lengthNew >= vectorSize) {
			if (simd_base::opCmpEq(gatherValuesU<simd_type>(string1), gatherValuesU<simd_type>(string2)) != maskValue) {
				return false;
			}
			lengthNew -= vectorSize;
			string1 += vectorSize;
			string2 += vectorSize;
		}
		if constexpr (index < avx_list::size - 1) {
			if (lengthNew > 0) {
				return compare<index + 1>(string1, string2, lengthNew);
			}
		} else if (lengthNew > 0) {
			return compareShort(string1, string2, lengthNew);
		}
		return true;
	#else
		return std::memcmp(string1, string2, lengthNew) == 0;
	#endif
	}

#else

	JSONIFIER_INLINE bool compare(const void* string1, const void* string2, uint64_t lengthNew) {
		std::string_view string01{ static_cast<const char*>(string1), lengthNew };
		std::string_view string02{ static_cast<const char*>(string2), lengthNew };
		return string01 == string02;
	}

#endif

	class jsonifier_core_internal {
	  public:
		template<typename value_type01, typename value_type02> JSONIFIER_INLINE static bool compare(const value_type01* string1, const value_type02* string2, uint64_t lengthNew) {
			return jsonifier_internal::compare(string1, string2, lengthNew);
		}
	};

}// namespace jsonifier_internal
