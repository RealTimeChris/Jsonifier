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

#include <jsonifier/TypeEntities.hpp>

namespace simd_internal {

#if JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_BMI2) || JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_ANY_AVX)

	template<jsonifier::concepts::uint32_type value_type> JSONIFIER_ALWAYS_INLINE value_type pdep(value_type value01, value_type value02) noexcept {
		return _pdep_u32(value01, value02);
	}

	template<jsonifier::concepts::uint64_type value_type> JSONIFIER_ALWAYS_INLINE value_type pdep(value_type value01, value_type value02) noexcept {
		return _pdep_u64(value01, value02);
	}

#else

	template<jsonifier::concepts::unsigned_type value_type> JSONIFIER_ALWAYS_INLINE value_type pdep(value_type src, value_type mask) noexcept {
		value_type result  = 0;
		value_type src_bit = 1;

		for (int32_t x = 0; x < 64; x++) {
			if (mask & 1) {
				result |= (src & src_bit);
				src_bit <<= 1;
			}
			mask >>= 1;
		}

		return result;
	}

#endif

}
