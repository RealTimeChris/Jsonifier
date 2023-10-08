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

#include <jsonifier/ISADetection/ISADetectionBase.hpp>

namespace jsonifier_internal {

#if JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_BMI)

	template<typename value_type> constexpr value_type blsr(value_type value) {
		static_assert(std::is_integral_v<value_type>, "Input must be an integer type");
		if constexpr (sizeof(value_type) == 4) {
			return _blsr_u32(value);
		} else if constexpr (sizeof(value_type) == 8) {
			return _blsr_u64(value);
		} else {
			static_assert(sizeof(value_type) == 4 || sizeof(value_type) == 8, "Unsupported size for blsr()");
			return value;
		}
	}

	template<typename value_type> constexpr value_type tzCount(value_type value) {
		static_assert(std::is_integral_v<value_type>, "Input must be an integer type");
		if constexpr (sizeof(value_type) == 2) {
			return _tzcnt_u16(value);
		} else if constexpr (sizeof(value_type) == 4) {
			return _tzcnt_u32(value);
		} else if constexpr (sizeof(value_type) == 8) {
			return _tzcnt_u64(value);
		} else {
			static_assert(sizeof(value_type) == 2 || sizeof(value_type) == 4 || sizeof(value_type) == 8, "Unsupported size for tzCount()");
			return value;
		}
	}


#else

	inline uint64_t blsr(uint64_t value) {
		if (value == 0) {
			return 0;
		}

		return value & (value - 1);
	}

	template<integer_t value_type> inline value_type tzCount(value_type value) {
		if (value == 0) {
			return sizeof(value_type) * 8;
		}

		value_type count{};
		while ((value & 1) == 0) {
			value >>= 1;
			++count;
		}

		return count;
	}

#endif

}
