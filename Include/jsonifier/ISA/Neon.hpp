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

#if JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_NEON)

	#define opAndNot(x, y) vbicq_u8(x, y)

	#define opAnd(x, y) vandq_u8(x, y)

	#define opOr(x, y) vorrq_u8(x, y)

	#define opXor(x, y) veorq_u8(x, y)

	#define opNot(x) vmvnq_u8(x)

	#define maskVal \
		uint8x16_t mask { \
			0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
		}

	#if defined(opSetLSB)
		#undef opSetLSB
	#endif
	#define opSetLSB(value, valueNew) \
		([&] { \
			maskVal; \
			return (valueNew ? vorrq_u8(value, mask) : vbicq_u8(value, mask)); \
		}())

	#define opGetMSB(value) ((vgetq_lane_u8(value, 15) & 0x80) != 0)

	#define opBool(value) (vmaxvq_u8(value) != 0)

#endif

}