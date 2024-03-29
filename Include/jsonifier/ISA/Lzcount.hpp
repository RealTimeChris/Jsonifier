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

#if JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_LZCNT)

	template<typename value_type> constexpr value_type lzcntInternal(value_type value) {
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

	template<jsonifier::concepts::uint32_type value_type> constexpr value_type lzcnt(value_type value) {
		if (std::is_constant_evaluated()) {
			return lzcntInternal(value);
		} else {
			return _lzcnt_u32(value);
		}
	}

	template<jsonifier::concepts::uint64_type value_type> constexpr value_type lzcnt(value_type value) {
		if (std::is_constant_evaluated()) {
			return lzcntInternal(value);
		} else {
			return _lzcnt_u64(value);
		}
	}

#else

	template<jsonifier::concepts::unsigned_t value_type> constexpr value_type lzcnt(value_type value) {
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