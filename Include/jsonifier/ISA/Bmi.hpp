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

#include <jsonifier/ISA/ISADetectionBase.hpp>

namespace jsonifier_internal {

#if JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_BMI)

	#include <immintrin.h>

	template<jsonifier::concepts::uint32_type value_type> JSONIFIER_INLINE value_type blsr(value_type value) {
		return _blsr_u32(value);
	}

	template<jsonifier::concepts::uint64_type value_type> JSONIFIER_INLINE value_type blsr(value_type value) {
		return _blsr_u64(value);
	}

	template<jsonifier::concepts::uint8_type value_type> JSONIFIER_INLINE value_type tzcnt(value_type value) {
	#if defined(__linux__)
		return static_cast<uint8_t>(__tzcnt_u16(static_cast<uint8_t>(value)));
	#else
		return static_cast<uint8_t>(_tzcnt_u16(static_cast<uint8_t>(value)));
	#endif
	}

	template<jsonifier::concepts::uint16_type value_type> JSONIFIER_INLINE value_type tzcnt(value_type value) {
	#if defined(__linux__)
		return __tzcnt_u16(value);
	#else
		return _tzcnt_u16(static_cast<uint16_t>(value));
	#endif
	}

	template<jsonifier::concepts::uint32_type value_type> JSONIFIER_INLINE value_type tzcnt(value_type value) {
		return _tzcnt_u32(value);
	}

	template<jsonifier::concepts::uint64_type value_type> JSONIFIER_INLINE value_type tzcnt(value_type value) {
		return _tzcnt_u64(value);
	}

#else

	template<jsonifier::concepts::unsigned_t value_type> JSONIFIER_INLINE value_type blsr(value_type value) {
		if (value == 0) {
			return 0;
		}

		return value & (value - 1);
	}

	template<jsonifier::concepts::unsigned_t value_type> JSONIFIER_INLINE value_type tzcnt(value_type value) {
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
