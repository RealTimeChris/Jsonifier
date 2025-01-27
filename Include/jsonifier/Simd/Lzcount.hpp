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

namespace jsonifier::simd {

#if JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_LZCNT) || JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_ANY_AVX)

	JSONIFIER_INLINE static uint32_t lzcnt(const uint32_t value) noexcept {
		return _lzcnt_u32(value);
	}

	JSONIFIER_INLINE static uint64_t lzcnt(const uint64_t value) noexcept {
		return _lzcnt_u64(value);
	}

#elif JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_NEON)

	template<concepts::uns32_t value_type> JSONIFIER_INLINE static value_type lzcnt(const value_type value) noexcept {
	#if defined(JSONIFIER_REGULAR_VISUAL_STUDIO)
		uint64_t leading_zero = 0;
		if (_BitScanReverse32(&leading_zero, value)) {
			return 32 - leading_zero;
		} else {
			return 32;
		}
	#else
		return __builtin_clz(value);
	#endif
	}

	template<concepts::uns64_t value_type> JSONIFIER_INLINE static value_type lzcnt(const value_type value) noexcept {
	#if defined(JSONIFIER_REGULAR_VISUAL_STUDIO)
		uint64_t leading_zero = 0;
		if (_BitScanReverse64(&leading_zero, value)) {
			return 63 - leading_zero;
		} else {
			return 64;
		}
	#else
		return __builtin_clzll(value);
	#endif
	}

#else

	template<concepts::unsigned_t value_type> JSONIFIER_INLINE static constexpr value_type lzcnt(const value_type value) noexcept {
		if (value == 0) {
			return sizeof(value_type) * 8;
		}

		value_type count{};
		value_type mask{ static_cast<value_type>(1) << (std::numeric_limits<value_type>::digits - 1) };

		while ((value & mask) == 0) {
			++count;
			mask >>= 1;
		}

		return count;
	}

#endif

}