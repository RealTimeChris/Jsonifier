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

#if JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_AVX512)

	template<typename value_type01, typename value_type02> jsonifier_inline uint64_t find(const value_type01* str, uint64_t length, const value_type02* sub, uint64_t subLength = 1) {
		static constexpr uint64_t vecSize = sizeof(simd_int_512);
		uint64_t remainingBytes{ length };
		uint64_t index{};

		if (subLength == 0) {
			return 0;
		}
		auto strNew = str;

		while (remainingBytes >= vecSize) {
			simd_int_512 currentVec = gatherValuesU<simd_int_512>(strNew);

			const simd_int_512 subVec = _mm512_set1_epi8(static_cast<char>(sub[0]));
			uint64_t mask			  = static_cast<uint64_t>(_mm512_cmpeq_epi8_mask(subVec, currentVec));
			do {
				if (mask != 0) {
					uint64_t pos = tzcnt(mask);
					if (memcmp(strNew + pos, sub, subLength) == 0) {
						return pos + index;
					}
					mask = blsr(mask);
				}
			} while (mask != 0);

			strNew += vecSize;
			index += vecSize;
			remainingBytes -= vecSize;
		}

		if (remainingBytes >= subLength) {
			for (int64_t i = 0; i < static_cast<int64_t>(remainingBytes); ++i) {
				if (str[index + i] == sub[0]) {
					if (memcmp(str + index + i, sub, subLength) == 0) {
						return index + i;
					}
				}
			}
		}

		return std::numeric_limits<uint64_t>::max();
	}

	template<typename value_type01, typename value_type02> jsonifier_inline bool compareValues(const value_type01* string1, const value_type02* string2, uint64_t length);

	template<typename value_type, typename IndexType> constexpr uint64_t getIntervalCount(uint64_t originalLength) {
		return originalLength * sizeof(value_type) / sizeof(IndexType);
	}

	template<jsonifier::concepts::float_t value_type01, jsonifier::concepts::float_t value_type02>
	jsonifier_inline bool compareValues16(const value_type01* string1, const value_type02* string2, uint64_t length) {
		static constexpr uint64_t vectorSize = sizeof(simd_int_128) / sizeof(value_type01);
		const uint64_t intervalCount		 = getIntervalCount<value_type01, simd_int_128>(length);
		const uint64_t remainder			 = length % vectorSize;
		static constexpr uint16_t maskValue{ 0xffff };

		auto destvector	  = gatherValuesU<simd_float_128>(string1);
		auto sourcevector = gatherValuesU<simd_float_128>(string2);
		if (_mm_movemask_pd(_mm_cmpeq_pd(destvector, sourcevector)) != maskValue) {
			return false;
		}

		for (uint64_t x = 1; x < intervalCount; ++x) {
			destvector	 = gatherValuesU<simd_float_128>(string1 + x * vectorSize);
			sourcevector = gatherValuesU<simd_float_128>(string2 + x * vectorSize);
			if (_mm_movemask_pd(_mm_cmpeq_pd(destvector, sourcevector)) != maskValue) {
				return false;
			}
		}

		if (remainder > 0) {
			return compareValues(string1 + (vectorSize * intervalCount), string2 + (vectorSize * intervalCount), remainder);
		}

		return true;
	}

	template<typename value_type01, typename value_type02> jsonifier_inline bool compareValues16(const value_type01* string1, const value_type02* string2, uint64_t length) {
		static constexpr uint64_t vectorSize = sizeof(simd_int_128) / sizeof(value_type01);
		const uint64_t intervalCount		 = getIntervalCount<value_type01, simd_int_128>(length);
		const uint64_t remainder			 = length % vectorSize;
		static constexpr uint16_t maskValue{ 0xffff };
		simd_int_128 destvector	  = gatherValuesU<simd_int_128>(string1);
		simd_int_128 sourcevector = gatherValuesU<simd_int_128>(string2);
		if (_mm_movemask_epi8(_mm_cmpeq_epi8(destvector, sourcevector)) != maskValue) {
			return false;
		}
		for (uint64_t x = 1; x < intervalCount; ++x) {
			destvector	 = gatherValuesU<simd_int_128>(string1 + x * vectorSize);
			sourcevector = gatherValuesU<simd_int_128>(string2 + x * vectorSize);
			if (_mm_movemask_epi8(_mm_cmpeq_epi8(destvector, sourcevector)) != maskValue) {
				return false;
			}
		}
		if (remainder > 0) {
			return compareValues(string1 + (vectorSize * intervalCount), string2 + (vectorSize * intervalCount), remainder);
		}
		return true;
	}

	template<jsonifier::concepts::float_t value_type01, jsonifier::concepts::float_t value_type02>
	jsonifier_inline bool compareValues32(const value_type01* string1, const value_type02* string2, uint64_t length) {
		static constexpr uint64_t vectorSize = sizeof(simd_int_256) / sizeof(value_type01);
		const uint64_t intervalCount		 = getIntervalCount<value_type01, simd_int_256>(length);
		const uint64_t remainder			 = length % vectorSize;
		static constexpr uint32_t maskValue{ 0xffffffffu };
		simd_float_256 destvector	= gatherValuesU<simd_float_256>(string1);
		simd_float_256 sourcevector = gatherValuesU<simd_float_256>(string2);
		if (_mm256_movemask_pd(_mm256_cmp_pd(destvector, sourcevector, _CMP_EQ_OQ)) != maskValue) {
			return false;
		}
		for (uint64_t x = 1; x < intervalCount; ++x) {
			destvector	 = gatherValuesU<simd_float_256>(string1 + x * vectorSize);
			sourcevector = gatherValuesU<simd_float_256>(string2 + x * vectorSize);
			if (_mm256_movemask_pd(_mm256_cmp_pd(destvector, sourcevector, _CMP_EQ_OQ)) != maskValue) {
				return false;
			}
		}
		if (remainder > 0) {
			return compareValues(string1 + (vectorSize * intervalCount), string2 + (vectorSize * intervalCount), remainder);
		}
		return true;
	}

	template<typename value_type01, typename value_type02> jsonifier_inline bool compareValues32(const value_type01* string1, const value_type02* string2, uint64_t length) {
		static constexpr uint64_t vectorSize = sizeof(simd_int_256) / sizeof(value_type01);
		const uint64_t intervalCount		 = getIntervalCount<value_type01, simd_int_256>(length);
		const uint64_t remainder			 = length % vectorSize;
		static constexpr uint32_t maskValue{ 0xffffffffu };
		simd_int_256 destvector	  = gatherValuesU<simd_int_256>(string1);
		simd_int_256 sourcevector = gatherValuesU<simd_int_256>(string2);
		if (_mm256_movemask_epi8(_mm256_cmpeq_epi8(destvector, sourcevector)) != maskValue) {
			return false;
		}
		for (uint64_t x = 1; x < intervalCount; ++x) {
			destvector	 = gatherValuesU<simd_int_256>(string1 + x * vectorSize);
			sourcevector = gatherValuesU<simd_int_256>(string2 + x * vectorSize);
			if (_mm256_movemask_epi8(_mm256_cmpeq_epi8(destvector, sourcevector)) != maskValue) {
				return false;
			}
		}
		if (remainder > 0) {
			return compareValues(string1 + (vectorSize * intervalCount), string2 + (vectorSize * intervalCount), remainder);
		}
		return true;
	}

	template<jsonifier::concepts::float_t value_type01, jsonifier::concepts::float_t value_type02>
	jsonifier_inline bool compareValues64(const value_type01* string1, const value_type02* string2, uint64_t length) {
		static constexpr uint64_t vectorSize = sizeof(simd_int_512) / sizeof(value_type01);
		const uint64_t intervalCount		 = getIntervalCount<value_type01, simd_int_512>(length);
		const uint64_t remainder			 = length % vectorSize;
		static constexpr uint64_t maskValue{ 0xffffffffffffffff };
		auto destvector	  = gatherValuesU<simd_float_512>(string1);
		auto sourcevector = gatherValuesU<simd_float_512>(string2);
		if (_mm512_cmpeq_epi8_mask(destvector, sourcevector) != maskValue) {
			return false;
		}
		for (uint64_t x = 1; x < intervalCount; ++x) {
			destvector	 = gatherValuesU<simd_float_512>(string1 + x * vectorSize);
			sourcevector = gatherValuesU<simd_float_512>(string2 + x * vectorSize);
			if (_mm512_cmpeq_epi8_mask(destvector, sourcevector) != maskValue) {
				return false;
			}
		}
		if (remainder > 0) {
			return compareValues(string1 + (vectorSize * intervalCount), string2 + (vectorSize * intervalCount), remainder);
		}
		return true;
	}

	template<typename value_type01, typename value_type02> jsonifier_inline bool compareValues64(const value_type01* string1, const value_type02* string2, uint64_t length) {
		static constexpr uint64_t vectorSize = sizeof(simd_int_512) / sizeof(value_type01);
		const uint64_t intervalCount		 = getIntervalCount<value_type01, simd_int_512>(length);
		const uint64_t remainder			 = length % vectorSize;
		static constexpr uint64_t maskValue{ 0xffffffffffffffff };
		simd_int_512 destvector	  = gatherValuesU<simd_int_512>(string1);
		simd_int_512 sourcevector = gatherValuesU<simd_int_512>(string2);
		if (_mm512_cmpeq_epi8_mask(destvector, sourcevector) != maskValue) {
			return false;
		}
		for (uint64_t x = 1; x < intervalCount; ++x) {
			destvector	 = gatherValuesU<simd_int_512>(string1 + x * vectorSize);
			sourcevector = gatherValuesU<simd_int_512>(string2 + x * vectorSize);
			if (_mm512_cmpeq_epi8_mask(destvector, sourcevector) != maskValue) {
				return false;
			}
		}
		if (remainder > 0) {
			return compareValues(string1 + (vectorSize * intervalCount), string2 + (vectorSize * intervalCount), remainder);
		}
		return true;
	}

	template<typename value_type01, typename value_type02> jsonifier_inline bool compareValues(const value_type01* string1, const value_type02* string2, uint64_t length) {
		if (length >= 64) {
			return compareValues64(string1, string2, length);
		} else if (length >= 32) {
			return compareValues32(string1, string2, length);
		} else if (length >= 16) {
			return compareValues16(string1, string2, length);
		} else {
			return (std::memcmp(string1, string2, length * sizeof(value_type01)) == 0);
		}
	}

#elif JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_AVX2)

	template<typename value_type01, typename value_type02> jsonifier_inline uint64_t find(const value_type01* str, uint64_t length, const value_type02* sub, uint64_t subLength = 1) {
		static constexpr uint64_t vecSize = sizeof(simd_int_256);
		uint64_t remainingBytes{ length };
		uint64_t index{};

		if (subLength == 0) {
			return 0;
		}
		auto strNew = str;

		while (remainingBytes >= vecSize) {
			simd_int_256 currentVec = gatherValuesU<simd_int_256>(strNew);

			const simd_int_256 subVec = _mm256_set1_epi8(static_cast<char>(sub[0]));
			uint32_t mask			  = static_cast<uint32_t>(_mm256_movemask_epi8(_mm256_cmpeq_epi8(subVec, currentVec)));
			do {
				if (mask != 0) {
					uint64_t pos = static_cast<uint64_t>(tzcnt(mask));
					if (memcmp(strNew + pos, sub, subLength) == 0) {
						return pos + index;
					}
					mask = blsr(mask);
				}
			} while (mask != 0);

			strNew += vecSize;
			index += vecSize;
			remainingBytes -= vecSize;
		}

		if (remainingBytes >= subLength) {
			for (int64_t i = 0; i < static_cast<int64_t>(remainingBytes); ++i) {
				if (str[index + i] == sub[0]) {
					if (memcmp(str + index + i, sub, subLength) == 0) {
						return index + i;
					}
				}
			}
		}

		return std::numeric_limits<uint64_t>::max();
	}

	template<typename value_type01, typename value_type02> jsonifier_inline bool compareValues(const value_type01* string1, const value_type02* string2, uint64_t length);

	template<typename value_type, typename IndexType> constexpr uint64_t getIntervalCount(uint64_t originalLength) {
		return originalLength * sizeof(value_type) / sizeof(IndexType);
	}

	template<jsonifier::concepts::float_t value_type01, jsonifier::concepts::float_t value_type02>
	jsonifier_inline bool compareValues16(const value_type01* string1, const value_type02* string2, uint64_t length) {
		static constexpr uint64_t vectorSize = sizeof(simd_int_128) / sizeof(value_type01);
		const uint64_t intervalCount		 = getIntervalCount<value_type01, simd_int_128>(length);
		const uint64_t remainder			 = length % vectorSize;
		static constexpr uint16_t maskValue{ 0xffff };

		simd_float_128 destvector	= gatherValuesU<simd_int_128>(string1);
		simd_float_128 sourcevector = gatherValuesU<simd_int_128>(string2);
		if (_mm_movemask_pd(_mm_cmpeq_pd(destvector, sourcevector)) != maskValue) {
			return false;
		}

		for (uint64_t x = 1; x < intervalCount; ++x) {
			destvector	 = gatherValuesU<simd_int_128>(string1 + x * vectorSize);
			sourcevector = gatherValuesU<simd_int_128>(string2 + x * vectorSize);
			if (_mm_movemask_pd(_mm_cmpeq_pd(destvector, sourcevector)) != maskValue) {
				return false;
			}
		}

		if (remainder > 0) {
			return compareValues(string1 + (vectorSize * intervalCount), string2 + (vectorSize * intervalCount), remainder);
		}

		return true;
	}

	template<typename value_type01, typename value_type02> jsonifier_inline bool compareValues16(const value_type01* string1, const value_type02* string2, uint64_t length) {
		static constexpr uint64_t vectorSize = sizeof(simd_int_128) / sizeof(value_type01);
		const uint64_t intervalCount		 = getIntervalCount<value_type01, simd_int_128>(length);
		const uint64_t remainder			 = length % vectorSize;
		static constexpr uint16_t maskValue{ 0xffff };
		simd_int_128 destvector	  = gatherValuesU<simd_int_128>(string1);
		simd_int_128 sourcevector = gatherValuesU<simd_int_128>(string2);
		if (_mm_movemask_epi8(_mm_cmpeq_epi8(destvector, sourcevector)) != maskValue) {
			return false;
		}
		for (uint64_t x = 1; x < intervalCount; ++x) {
			destvector	 = gatherValuesU<simd_int_128>(string1 + x * vectorSize);
			sourcevector = gatherValuesU<simd_int_128>(string2 + x * vectorSize);
			if (_mm_movemask_epi8(_mm_cmpeq_epi8(destvector, sourcevector)) != maskValue) {
				return false;
			}
		}
		if (remainder > 0) {
			return compareValues(string1 + (vectorSize * intervalCount), string2 + (vectorSize * intervalCount), remainder);
		}
		return true;
	}

	template<jsonifier::concepts::float_t value_type01, jsonifier::concepts::float_t value_type02>
	jsonifier_inline bool compareValues32(const value_type01* string1, const value_type02* string2, uint64_t length) {
		static constexpr uint64_t vectorSize = sizeof(simd_int_256) / sizeof(value_type01);
		const uint64_t intervalCount		 = getIntervalCount<value_type01, simd_int_256>(length);
		const uint64_t remainder			 = length % vectorSize;
		static constexpr uint32_t maskValue{ 0xffffffffu };
		simd_float_256 destvector	= gatherValuesU<simd_float_256>(string1);
		simd_float_256 sourcevector = gatherValuesU<simd_float_256>(string2);
		if (_mm256_movemask_pd(_mm256_cmp_pd(destvector, sourcevector, _CMP_EQ_OQ)) != maskValue) {
			return false;
		}
		for (uint64_t x = 1; x < intervalCount; ++x) {
			destvector	 = gatherValuesU<simd_float_256>(string1 + x * vectorSize);
			sourcevector = gatherValuesU<simd_float_256>(string2 + x * vectorSize);
			if (_mm256_movemask_pd(_mm256_cmp_pd(destvector, sourcevector, _CMP_EQ_OQ)) != maskValue) {
				return false;
			}
		}
		if (remainder > 0) {
			return compareValues(string1 + (vectorSize * intervalCount), string2 + (vectorSize * intervalCount), remainder);
		}
		return true;
	}

	template<typename value_type01, typename value_type02> inline bool compareValues32(const value_type01* string1, const value_type02* string2, uint64_t length) {
		static constexpr uint64_t vectorSize = sizeof(simd_int_256) / sizeof(value_type01);
		const uint64_t intervalCount		 = getIntervalCount<value_type01, simd_int_256>(length);
		const uint64_t remainder			 = length % vectorSize;
		static constexpr uint32_t maskValue{ 0xffffffffu };
		simd_int_256 destvector	  = gatherValuesU<simd_int_256>(string1);
		simd_int_256 sourcevector = gatherValuesU<simd_int_256>(string2);
		if (_mm256_movemask_epi8(_mm256_cmpeq_epi8(destvector, sourcevector)) != static_cast<int32_t>(maskValue)) {
			return false;
		}
		for (uint64_t x = 1; x < intervalCount; ++x) {
			destvector	 = gatherValuesU<simd_int_256>(string1 + x * vectorSize);
			sourcevector = gatherValuesU<simd_int_256>(string2 + x * vectorSize);
			if (_mm256_movemask_epi8(_mm256_cmpeq_epi8(destvector, sourcevector)) != static_cast<int32_t>(maskValue)) {
				return false;
			}
		}
		if (remainder > 0) {
			return compareValues(string1 + (vectorSize * intervalCount), string2 + (vectorSize * intervalCount), remainder);
		}
		return true;
	}

	template<typename value_type01, typename value_type02> jsonifier_inline bool compareValues(const value_type01* string1, const value_type02* string2, uint64_t length) {
		if (length >= 32) {
			return compareValues32(string1, string2, length);
		} else if (length >= 16) {
			return compareValues16(string1, string2, length);
		} else {
			return (std::memcmp(string1, string2, length * sizeof(value_type01)) == 0);
		}
	}

#elif JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_AVX)

	template<typename value_type01, typename value_type02> jsonifier_inline uint64_t find(const value_type01* str, uint64_t length, const value_type02* sub, uint64_t subLength = 1) {
		static constexpr uint64_t vecSize = sizeof(simd_int_128);
		uint64_t remainingBytes{ length };
		uint64_t index{};

		if (subLength == 0) {
			return 0;
		}
		auto strNew = str;

		while (remainingBytes >= vecSize) {
			simd_int_128 currentVec = gatherValuesU<simd_int_128>(strNew);

			const simd_int_128 subVec = _mm_set1_epi8(static_cast<char>(sub[0]));
			uint16_t mask			  = static_cast<uint16_t>(_mm_movemask_epi8(_mm_cmpeq_epi8(subVec, currentVec)));
			do {
				if (mask != 0) {
					uint64_t pos = static_cast<uint64_t>(tzcnt(mask));
					if (memcmp(strNew + pos, sub, subLength) == 0) {
						return pos + index;
					}
					mask = blsr(mask);
				}
			} while (mask != 0);

			strNew += vecSize;
			index += vecSize;
			remainingBytes -= vecSize;
		}

		if (remainingBytes >= subLength) {
			for (int64_t i = 0; i < static_cast<int64_t>(remainingBytes); ++i) {
				if (str[index + i] == sub[0]) {
					if (memcmp(str + index + i, sub, subLength) == 0) {
						return index + i;
					}
				}
			}
		}

		return std::numeric_limits<uint64_t>::max();
	}

	template<typename value_type01, typename value_type02> jsonifier_inline bool compareValues(const value_type01* string1, const value_type02* string2, uint64_t length) {
		return (std::memcmp(string1, string2, length * sizeof(value_type01)) == 0);
	}

#else

	template<typename value_type01, typename value_type02>
	jsonifier_inline uint64_t find(const value_type01* str, uint64_t length, value_type02* target, uint64_t subLength = std::char_traits<value_type02>::length(target)) {
		std::basic_string_view<std::remove_pointer_t<value_type01>> newString{ target, subLength };
		return std::basic_string_view<value_type01>{ static_cast<const value_type01*>(str), length }.find(newString);
	}

	template<typename value_type01, typename value_type02> jsonifier_inline bool compareValues(const void* destvector, const void* sourcevector, uint64_t length) {
		return std::basic_string_view<std::remove_pointer_t<value_type01>>{ static_cast<const value_type01*>(destvector), length } ==
			std::basic_string_view<std::remove_pointer_t<value_type01>>{ static_cast<const value_type01*>(sourcevector), length };
	}

#endif

	class jsonifier_core_internal {
	  public:
		template<typename value_type01, typename value_type02> jsonifier_inline static bool compare(const value_type01* destvector, const value_type02* sourcevector, uint64_t length) {
			return compareValues<value_type01, value_type02>(destvector, sourcevector, length);
		}
	};

}// namespace jsonifier_internal
