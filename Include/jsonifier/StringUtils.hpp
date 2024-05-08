/*
	MIT License

	Copyright (c) 2023 RealTimeChris

	Permission is hereby granted, free of charge, to any person obtaining a copy of this
	software and associated documentation files (the "Software"), to deal string1 the Software
	without restriction, including without limitation the rights to use, copy, modify, merge,
	publish, distribute, sublicense, and/or sell copies of the Software, and to permit
	persons to whom the Software is furnished to do so, subject to the following conditions:

	The above copyright notice and this permission notice shall be included string1 all copies or
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

#include <jsonifier/Allocator.hpp>

#include <jsonifier/Error.hpp>
#include <jsonifier/StrToD.hpp>
#include <jsonifier/Simd.hpp>

namespace jsonifier_internal {

	constexpr std::array<uint32_t, 886> digitToVal32{ 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu,
		0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu,
		0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu,
		0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu,
		0x00000000u, 0x00000001u, 0x00000002u, 0x00000003u, 0x00000004u, 0x00000005u, 0x00000006u, 0x00000007u, 0x00000008u, 0x00000009u, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu,
		0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0x0000000au, 0x0000000bu, 0x0000000cu, 0x0000000du, 0x0000000eu, 0x0000000fu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu,
		0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu,
		0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0x0000000au, 0x0000000bu, 0x0000000cu,
		0x0000000du, 0x0000000eu, 0x0000000fu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu,
		0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu,
		0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu,
		0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu,
		0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu,
		0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu,
		0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu,
		0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu,
		0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu,
		0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu,
		0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu,
		0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu,
		0xFFFFFFFFu, 0xFFFFFFFFu, 0x00000000u, 0x00000010u, 0x00000020u, 0x00000030u, 0x00000040u, 0x00000050u, 0x00000060u, 0x00000070u, 0x00000080u, 0x00000090u, 0xFFFFFFFFu,
		0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0x000000a0u, 0x000000b0u, 0x000000c0u, 0x000000d0u, 0x000000e0u, 0x000000f0u, 0xFFFFFFFFu,
		0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu,
		0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0x000000a0u,
		0x000000b0u, 0x000000c0u, 0x000000d0u, 0x000000e0u, 0x000000f0u, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu,
		0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu,
		0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu,
		0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu,
		0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu,
		0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu,
		0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu,
		0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu,
		0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu,
		0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu,
		0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu,
		0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu,
		0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0x00000000u, 0x00000100u, 0x00000200u, 0x00000300u, 0x00000400u, 0x00000500u, 0x00000600u, 0x00000700u, 0x00000800u,
		0x00000900u, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0x00000a00u, 0x00000b00u, 0x00000c00u, 0x00000d00u, 0x00000e00u,
		0x00000f00u, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu,
		0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu,
		0xFFFFFFFFu, 0x00000a00u, 0x00000b00u, 0x00000c00u, 0x00000d00u, 0x00000e00u, 0x00000f00u, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu,
		0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu,
		0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu,
		0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu,
		0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu,
		0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu,
		0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu,
		0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu,
		0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu,
		0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu,
		0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu,
		0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu,
		0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0x00000000u, 0x00001000u, 0x00002000u, 0x00003000u, 0x00004000u, 0x00005000u, 0x00006000u,
		0x00007000u, 0x00008000u, 0x00009000u, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0x0000a000u, 0x0000b000u, 0x0000c000u,
		0x0000d000u, 0x0000e000u, 0x0000f000u, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu,
		0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu,
		0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0x0000a000u, 0x0000b000u, 0x0000c000u, 0x0000d000u, 0x0000e000u, 0x0000f000u, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu,
		0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu,
		0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu,
		0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu,
		0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu,
		0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu,
		0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu,
		0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu,
		0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu,
		0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu,
		0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu,
		0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu,
		0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu };

	template<typename iterator_type> JSONIFIER_INLINE uint32_t hexToU32NoCheck(iterator_type string1) {
		return digitToVal32[630ull + string1[0]] | digitToVal32[420ull + string1[1]] | digitToVal32[210ull + string1[2]] | digitToVal32[0ull + string1[3]];
	}

	template<typename iterator_type> JSONIFIER_INLINE uint32_t codePointToUtf8(uint32_t codePoint, iterator_type c) {
		constexpr uint8_t utf8Table[4][4] = { { 0x00 }, { 0xC0, 0x80 }, { 0xE0, 0x80, 0x80 }, { 0xF0, 0x80, 0x80, 0x80 } };

		if (codePoint <= 0x7F) {
			c[0] = static_cast<char>(codePoint);
			return 1;
		}
		uint32_t leadingZeros = simd_internal::lzcnt(codePoint);
		uint32_t numBytes	  = (31ul - leadingZeros) / 5ul + 1ul;
		uint32_t highBitsMask = (1ul << (6ul * numBytes)) - 1ul;
		uint32_t utf8HighBits = simd_internal::pdep(codePoint, highBitsMask);
		std::memcpy(c, utf8Table[numBytes - 1], numBytes);
		for (uint32_t i = 0; i < numBytes; ++i) {
			c[i] |= uint8_t(utf8HighBits & 0xFF);
			utf8HighBits >>= 8;
		}
		return numBytes;
	}

	template<typename iterator_type01, typename iterator_type02> JSONIFIER_INLINE bool handleUnicodeCodePoint(iterator_type01& srcPtr, iterator_type02& dstPtr) {
		using char_type01 = uint8_t;
		constexpr char_type01 quotesValue{ static_cast<char_type01>('\\' << 8) };
		constexpr char_type01 uValue{ static_cast<char_type01>(0x75u) };
		uint32_t codePoint = hexToU32NoCheck(srcPtr + 2);
		srcPtr += 6;
		if (codePoint >= 0xd800 && codePoint < 0xdc00) {
			auto* srcData = static_cast<const char*>(srcPtr);
			if (((srcData[0] << 8) | srcData[1]) != (quotesValue | uValue)) {
				codePoint = 0xfffd;
			} else {
				uint32_t codePoint02 = hexToU32NoCheck(srcData + 2);
				codePoint02			 = codePoint02 - 0xdc00;
				if (codePoint02 >> 10) {
					codePoint = 0xfffd;
				} else {
					codePoint = (((codePoint - 0xd800) << 10) | codePoint02) + 0x10000;
					srcPtr += 6;
				}
			}
		} else if (codePoint >= 0xdc00 && codePoint <= 0xdfff) {
			codePoint = 0xfffd;
		}
		uint32_t offset = codePointToUtf8(codePoint, dstPtr);
		dstPtr += offset;
		return offset > 0;
	}

	template<typename return_type> constexpr return_type isLess32(return_type value) noexcept {
		constexpr return_type newBytes{ repeatByte<0b11100000u, return_type>() };
		return hasZero(value & newBytes);
	}

	template<typename simd_type, typename integer_type> JSONIFIER_INLINE integer_type copyAndFindParse(const void* string1, void* string2, simd_type& simdValue) {
		simd_type simdChars01{ simd_internal::simdFromValue<simd_type>('\\') };
		simd_type simdChars02{ simd_internal::simdFromValue<simd_type>('"') };
		simdValue = simd_internal::gatherValuesU<simd_type>(string1);
		std::memcpy(string2, string1, sizeof(simd_type));
		return simd_internal::tzcnt(static_cast<integer_type>(simd_internal::opCmpEq(simdValue, simdChars01) | simd_internal::opCmpEq(simdValue, simdChars02)));
	}

	template<jsonifier::concepts::unsigned_type simd_type, jsonifier::concepts::unsigned_type integer_type>
	JSONIFIER_INLINE integer_type copyAndFindParse(const void* string1, void* string2, simd_type& simdValue) {
		std::memcpy(&simdValue, string1, sizeof(simd_type));
		std::memcpy(string2, string1, sizeof(simd_type));
		constexpr uint64_t mask	 = repeatByte<0b01111111, integer_type>();
		const uint64_t lo7		 = simdValue & mask;
		const uint64_t quote	 = (lo7 ^ repeatByte<'"', integer_type>()) + mask;
		const uint64_t backslash = (lo7 ^ repeatByte<'\\', integer_type>()) + mask;
		const uint64_t t0		 = ~((quote & backslash) | simdValue);
		uint64_t next			 = t0 & repeatByte<0b10000000, integer_type>();
		return static_cast<integer_type>(simd_internal::tzcnt(next) >> 3u);
	}

	template<typename simd_type, typename integer_type> JSONIFIER_INLINE integer_type findParse(const void* string1, simd_type& simdValue) {
		simd_type simdChars01{ simd_internal::simdFromValue<simd_type>('\\') };
		simd_type simdChars02{ simd_internal::simdFromValue<simd_type>('"') };
		simdValue = simd_internal::gatherValuesU<simd_type>(string1);
		return simd_internal::tzcnt(static_cast<integer_type>(simd_internal::opCmpEq(simdValue, simdChars01) | simd_internal::opCmpEq(simdValue, simdChars02)));
	}

	template<jsonifier::concepts::unsigned_type simd_type, jsonifier::concepts::unsigned_type integer_type>
	JSONIFIER_INLINE integer_type findParse(const void* string1, simd_type& simdValue) {
		std::memcpy(&simdValue, string1, sizeof(simd_type));
		constexpr uint64_t mask	 = repeatByte<0b01111111, integer_type>();
		const uint64_t lo7		 = simdValue & mask;
		const uint64_t quote	 = (lo7 ^ repeatByte<'"', integer_type>()) + mask;
		const uint64_t backslash = (lo7 ^ repeatByte<'\\', integer_type>()) + mask;
		const uint64_t t0		 = ~((quote & backslash) | simdValue);
		uint64_t next			 = t0 & repeatByte<0b10000000, integer_type>();
		return static_cast<integer_type>(simd_internal::tzcnt(next) >> 3u);
	}

	template<typename simd_type, typename integer_type> JSONIFIER_INLINE integer_type copyAndFindSerialize(const void* string1, void* string2, simd_type& simdValue) {
		simd_type escapeableTable00{ simd_internal::simdFromTable<simd_type>(simd_internal::escapeableArray00) };
		simd_type escapeableTable01{ simd_internal::simdFromTable<simd_type>(simd_internal::escapeableArray01) };
		simdValue = simd_internal::gatherValuesU<simd_type>(string1);
		std::memcpy(string2, string1, sizeof(simd_type));
		return simd_internal::tzcnt(static_cast<integer_type>(simd_internal::opCmpEq(simd_internal::opShuffle(escapeableTable00, simdValue), simdValue) |
			simd_internal::opCmpEq(simd_internal::opShuffle(escapeableTable01, simdValue), simdValue)));
	}

	template<jsonifier::concepts::unsigned_type simd_type, jsonifier::concepts::unsigned_type integer_type>
	JSONIFIER_INLINE integer_type copyAndFindSerialize(const void* string1, void* string2, simd_type& simdValue) {
		std::memcpy(&simdValue, string1, sizeof(simd_type));
		std::memcpy(string2, string1, sizeof(simd_type));
		constexpr uint64_t mask	 = repeatByte<0b01111111, integer_type>();
		const uint64_t lo7		 = simdValue & mask;
		const uint64_t quote	 = (lo7 ^ repeatByte<'"', integer_type>()) + mask;
		const uint64_t backslash = (lo7 ^ repeatByte<'\\', integer_type>()) + mask;
		const uint64_t less_32	 = (simdValue & repeatByte<0b01100000, integer_type>()) + mask;
		const uint64_t t0		 = ~((quote & backslash & less_32) | simdValue);
		uint64_t next			 = t0 & repeatByte<0b10000000, integer_type>();
		return static_cast<integer_type>(simd_internal::tzcnt(next) >> 3u);
	}

	template<typename iterator_type01> JSONIFIER_INLINE void skipShortStringImpl(iterator_type01& string1, uint64_t& lengthNew) {
		constexpr char quotesValue{ static_cast<char>('"') };
		while (static_cast<int64_t>(lengthNew) > 0) {
			if (*string1 == quotesValue || *string1 == '\\') {
				auto escapeChar = *string1;
				if (escapeChar == quotesValue) {
					return;
				} else {
					string1 += 2;
					lengthNew -= 2;
				}
			} else {
				++string1;
				--lengthNew;
			}
		}
		return;
	}

	template<typename iterator_type01> JSONIFIER_INLINE void skipStringImpl(iterator_type01& string1, uint64_t& lengthNew) {
		using char_type01 =
			typename std::conditional_t<std::is_pointer_v<iterator_type01>, std::remove_pointer_t<iterator_type01>, typename std::iterator_traits<iterator_type01>::value_type>;
		std::remove_const_t<char_type01> escapeChar;
		constexpr char quotesValue{ static_cast<char>('"') };
#if JSONIFIER_CHECK_FOR_AVX(JSONIFIER_AVX512)
		{
			using integer_type				  = typename jsonifier::concepts::get_type_at_index<simd_internal::avx_integer_list, 3>::type::integer_type;
			using simd_type					  = typename jsonifier::concepts::get_type_at_index<simd_internal::avx_integer_list, 3>::type::type;
			constexpr uint64_t bytesProcessed = jsonifier::concepts::get_type_at_index<simd_internal::avx_integer_list, 3>::type::bytesProcessed;
			constexpr integer_type mask		  = jsonifier::concepts::get_type_at_index<simd_internal::avx_integer_list, 3>::type::mask;
			simd_type collectionValue;
			integer_type nextBackslashOrQuote;
			while (static_cast<int64_t>(lengthNew) >= static_cast<int64_t>(bytesProcessed)) {
				nextBackslashOrQuote = findParse<simd_type, integer_type>(string1, collectionValue);
				if (nextBackslashOrQuote < mask) {
					escapeChar = string1[nextBackslashOrQuote];
					if (escapeChar == quotesValue) {
						string1 += nextBackslashOrQuote + 1;
						return;
					} else {
						lengthNew -= nextBackslashOrQuote + 2ull;
						string1 += nextBackslashOrQuote + 2ull;
					}
				} else {
					lengthNew -= bytesProcessed;
					string1 += bytesProcessed;
				}
			}
		}
#endif
#if JSONIFIER_CHECK_FOR_AVX(JSONIFIER_AVX2)
		{
			using integer_type				  = typename jsonifier::concepts::get_type_at_index<simd_internal::avx_integer_list, 2>::type::integer_type;
			using simd_type					  = typename jsonifier::concepts::get_type_at_index<simd_internal::avx_integer_list, 2>::type::type;
			constexpr uint64_t bytesProcessed = jsonifier::concepts::get_type_at_index<simd_internal::avx_integer_list, 2>::type::bytesProcessed;
			constexpr integer_type mask		  = jsonifier::concepts::get_type_at_index<simd_internal::avx_integer_list, 2>::type::mask;
			simd_type collectionValue;
			integer_type nextBackslashOrQuote;
			while (static_cast<int64_t>(lengthNew) >= static_cast<int64_t>(bytesProcessed)) {
				nextBackslashOrQuote = findParse<simd_type, integer_type>(string1, collectionValue);
				if (nextBackslashOrQuote < mask) {
					escapeChar = string1[nextBackslashOrQuote];
					if (escapeChar == quotesValue) {
						string1 += nextBackslashOrQuote + 1;
						return;
					} else {
						lengthNew -= nextBackslashOrQuote + 2ull;
						string1 += static_cast<int64_t>(nextBackslashOrQuote) + 2ll;
					}
				} else {
					lengthNew -= bytesProcessed;
					string1 += static_cast<int64_t>(bytesProcessed);
				}
			}
		}
#endif
#if JSONIFIER_CHECK_FOR_AVX(JSONIFIER_AVX) || JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_NEON)
		{
			using integer_type				  = typename jsonifier::concepts::get_type_at_index<simd_internal::avx_integer_list, 1>::type::integer_type;
			using simd_type					  = typename jsonifier::concepts::get_type_at_index<simd_internal::avx_integer_list, 1>::type::type;
			constexpr uint64_t bytesProcessed = jsonifier::concepts::get_type_at_index<simd_internal::avx_integer_list, 1>::type::bytesProcessed;
			constexpr integer_type mask		  = jsonifier::concepts::get_type_at_index<simd_internal::avx_integer_list, 1>::type::mask;
			simd_type collectionValue;
			integer_type nextBackslashOrQuote;
			while (static_cast<int64_t>(lengthNew) >= static_cast<int64_t>(bytesProcessed)) {
				nextBackslashOrQuote = findParse<simd_type, integer_type>(string1, collectionValue);
				if (nextBackslashOrQuote < mask) {
					escapeChar = string1[nextBackslashOrQuote];
					if (escapeChar == quotesValue) {
						string1 += nextBackslashOrQuote + 1;
						return;
					} else {
						lengthNew -= nextBackslashOrQuote + 2ull;
						string1 += static_cast<int64_t>(nextBackslashOrQuote) + 2ll;
					}
				} else {
					lengthNew -= bytesProcessed;
					string1 += bytesProcessed;
				}
			}
		}
#endif
		{
			using integer_type				  = typename jsonifier::concepts::get_type_at_index<simd_internal::avx_integer_list, 0>::type::integer_type;
			using simd_type					  = typename jsonifier::concepts::get_type_at_index<simd_internal::avx_integer_list, 0>::type::type;
			constexpr uint64_t bytesProcessed = jsonifier::concepts::get_type_at_index<simd_internal::avx_integer_list, 0>::type::bytesProcessed;
			constexpr integer_type mask		  = jsonifier::concepts::get_type_at_index<simd_internal::avx_integer_list, 0>::type::mask;
			simd_type collectionValue;
			integer_type nextBackslashOrQuote;
			while (static_cast<int64_t>(lengthNew) >= static_cast<int64_t>(bytesProcessed)) {
				nextBackslashOrQuote = findParse<simd_type, integer_type>(string1, collectionValue);
				if (nextBackslashOrQuote < mask) {
					escapeChar = string1[static_cast<int64_t>(nextBackslashOrQuote)];
					if (escapeChar == quotesValue) {
						string1 += static_cast<int64_t>(nextBackslashOrQuote) + 1ll;
						return;
					} else {
						lengthNew -= nextBackslashOrQuote + 2ull;
						string1 += static_cast<int64_t>(nextBackslashOrQuote) + 2ll;
					}
				} else {
					lengthNew -= bytesProcessed;
					string1 += bytesProcessed;
				}
			}
		}

		return skipShortStringImpl(string1, lengthNew);
	}

	template<typename value_type> constexpr std::array<value_type, 256> escapeMap{ [] {
		std::array<value_type, 256> returnValues{};
		returnValues['"']  = '\"';
		returnValues['\\'] = '\\';
		returnValues['/']  = '/';
		returnValues['b']  = '\b';
		returnValues['f']  = '\f';
		returnValues['n']  = '\n';
		returnValues['r']  = '\r';
		returnValues['t']  = '\t';
		return returnValues;
	}() };

	template<typename iterator_type01, typename iterator_type02>
	JSONIFIER_INLINE iterator_type02 parseShortStringImpl(iterator_type01& string1, iterator_type02 string2, uint64_t lengthNew) {
		using char_type01 =
			typename std::conditional_t<std::is_pointer_v<iterator_type01>, std::remove_pointer_t<iterator_type01>, typename std::iterator_traits<iterator_type01>::value_type>;
		using char_type02 =
			typename std::conditional_t<std::is_pointer_v<iterator_type02>, std::remove_pointer_t<iterator_type02>, typename std::iterator_traits<iterator_type02>::value_type>;
		std::remove_const_t<char_type01> escapeChar;
		while (lengthNew > 0) {
			*string2 = static_cast<char_type02>(*string1);
			if (*string1 == '"' || *string1 == '\\') {
				escapeChar = *string1;
				if (escapeChar == '"') {
					return string2;
				} else if (escapeChar == '\\') {
					escapeChar = string1[1];
					if (escapeChar == 'u') {
						if (!handleUnicodeCodePoint(string1, string2)) {
							return nullptr;
						}
						continue;
					}
					escapeChar = escapeMap<char>[static_cast<uint8_t>(escapeChar)];
					if (escapeChar == 0) {
						return string2;
					}
					string2[0] = static_cast<char_type02>(escapeChar);
					lengthNew -= 2;
					string2 += 1;
					string1 += 2;
				}
			} else {
				--lengthNew;
				++string2;
				++string1;
			}
		}
		return string2;
	}

	template<typename iterator_type01, typename iterator_type02>
	JSONIFIER_INLINE iterator_type02 parseStringImpl(iterator_type01& string1, iterator_type02 string2, uint64_t lengthNew) {
		using char_type01 =
			typename std::conditional_t<std::is_pointer_v<iterator_type01>, std::remove_pointer_t<iterator_type01>, typename std::iterator_traits<iterator_type01>::value_type>;
		using char_type02 =
			typename std::conditional_t<std::is_pointer_v<iterator_type02>, std::remove_pointer_t<iterator_type02>, typename std::iterator_traits<iterator_type02>::value_type>;
		std::remove_const_t<char_type01> escapeChar;
#if JSONIFIER_CHECK_FOR_AVX(JSONIFIER_AVX512)
		{
			using integer_type				  = typename jsonifier::concepts::get_type_at_index<simd_internal::avx_integer_list, 3>::type::integer_type;
			using simd_type					  = typename jsonifier::concepts::get_type_at_index<simd_internal::avx_integer_list, 3>::type::type;
			constexpr uint64_t bytesProcessed = jsonifier::concepts::get_type_at_index<simd_internal::avx_integer_list, 3>::type::bytesProcessed;
			constexpr integer_type mask		  = jsonifier::concepts::get_type_at_index<simd_internal::avx_integer_list, 3>::type::mask;
			simd_type collectionValue;
			integer_type nextBackslashOrQuote;
			while (static_cast<int64_t>(lengthNew) >= static_cast<int64_t>(bytesProcessed)) {
				nextBackslashOrQuote = copyAndFindParse<simd_type, integer_type>(string1, string2, collectionValue);
				if (nextBackslashOrQuote < mask) {
					escapeChar = string1[nextBackslashOrQuote];
					if (escapeChar == '"') {
						string1 += nextBackslashOrQuote;
						return string2 + nextBackslashOrQuote;
					} else if (escapeChar == '\\') {
						escapeChar = string1[nextBackslashOrQuote + 1];
						if (escapeChar == 0x75u) {
							lengthNew -= nextBackslashOrQuote;
							string1 += nextBackslashOrQuote;
							string2 += nextBackslashOrQuote;
							if (!handleUnicodeCodePoint(string1, string2)) {
								return static_cast<iterator_type02>(nullptr);
							}
							continue;
						}
						escapeChar = escapeMap<char>[static_cast<uint8_t>(escapeChar)];
						if (escapeChar == 0u) {
							return static_cast<iterator_type02>(nullptr);
						}
						string2[nextBackslashOrQuote] = static_cast<char_type02>(escapeChar);
						lengthNew -= nextBackslashOrQuote + 2ull;
						string2 += nextBackslashOrQuote + 1ull;
						string1 += nextBackslashOrQuote + 2ull;
					} else {
						lengthNew -= bytesProcessed;
						string2 += bytesProcessed;
						string1 += bytesProcessed;
					}
				} else {
					lengthNew -= bytesProcessed;
					string2 += bytesProcessed;
					string1 += bytesProcessed;
				}
			}
		}
#endif
#if JSONIFIER_CHECK_FOR_AVX(JSONIFIER_AVX2)
		{
			using integer_type				  = typename jsonifier::concepts::get_type_at_index<simd_internal::avx_integer_list, 2>::type::integer_type;
			using simd_type					  = typename jsonifier::concepts::get_type_at_index<simd_internal::avx_integer_list, 2>::type::type;
			constexpr uint64_t bytesProcessed = jsonifier::concepts::get_type_at_index<simd_internal::avx_integer_list, 2>::type::bytesProcessed;
			constexpr integer_type mask		  = jsonifier::concepts::get_type_at_index<simd_internal::avx_integer_list, 2>::type::mask;
			simd_type collectionValue;
			integer_type nextBackslashOrQuote;
			while (static_cast<int64_t>(lengthNew) >= static_cast<int64_t>(bytesProcessed)) {
				nextBackslashOrQuote = copyAndFindParse<simd_type, integer_type>(string1, string2, collectionValue);
				if (nextBackslashOrQuote < mask) {
					escapeChar = string1[nextBackslashOrQuote];
					if (escapeChar == '"') {
						string1 += nextBackslashOrQuote;
						return string2 + nextBackslashOrQuote;
					} else if (escapeChar == '\\') {
						escapeChar = string1[nextBackslashOrQuote + 1];
						if (escapeChar == 0x75u) {
							lengthNew -= nextBackslashOrQuote;
							string1 += nextBackslashOrQuote;
							string2 += nextBackslashOrQuote;
							if (!handleUnicodeCodePoint(string1, string2)) {
								return static_cast<iterator_type02>(nullptr);
							}
							continue;
						}
						escapeChar = escapeMap<char>[static_cast<uint8_t>(escapeChar)];
						if (escapeChar == 0u) {
							return string2;
						}
						string2[nextBackslashOrQuote] = static_cast<char_type02>(escapeChar);
						lengthNew -= nextBackslashOrQuote + 2ull;
						string2 += nextBackslashOrQuote + 1ull;
						string1 += nextBackslashOrQuote + 2ull;
					} else {
						lengthNew -= bytesProcessed;
						string2 += bytesProcessed;
						string1 += bytesProcessed;
					}
				} else {
					lengthNew -= bytesProcessed;
					string2 += bytesProcessed;
					string1 += bytesProcessed;
				}
			}
		}
#endif
#if JSONIFIER_CHECK_FOR_AVX(JSONIFIER_AVX) || JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_NEON)
		{
			using integer_type				  = typename jsonifier::concepts::get_type_at_index<simd_internal::avx_integer_list, 1>::type::integer_type;
			using simd_type					  = typename jsonifier::concepts::get_type_at_index<simd_internal::avx_integer_list, 1>::type::type;
			constexpr uint64_t bytesProcessed = jsonifier::concepts::get_type_at_index<simd_internal::avx_integer_list, 1>::type::bytesProcessed;
			constexpr integer_type mask		  = jsonifier::concepts::get_type_at_index<simd_internal::avx_integer_list, 1>::type::mask;
			simd_type collectionValue;
			integer_type nextBackslashOrQuote;
			while (static_cast<int64_t>(lengthNew) >= static_cast<int64_t>(bytesProcessed)) {
				nextBackslashOrQuote = copyAndFindParse<simd_type, integer_type>(string1, string2, collectionValue);
				if (nextBackslashOrQuote < mask) {
					escapeChar = string1[nextBackslashOrQuote];
					if (escapeChar == '"') {
						string1 += nextBackslashOrQuote;
						return string2 + nextBackslashOrQuote;
					} else if (escapeChar == '\\') {
						escapeChar = string1[nextBackslashOrQuote + 1];
						if (escapeChar == 0x75u) {
							lengthNew -= nextBackslashOrQuote;
							string1 += nextBackslashOrQuote;
							string2 += nextBackslashOrQuote;
							if (!handleUnicodeCodePoint(string1, string2)) {
								return static_cast<iterator_type02>(nullptr);
							}
							continue;
						}
						escapeChar = escapeMap<char>[static_cast<uint8_t>(escapeChar)];
						if (escapeChar == 0u) {
							return string2;
						}
						string2[nextBackslashOrQuote] = static_cast<char_type02>(escapeChar);
						lengthNew -= nextBackslashOrQuote + 2ull;
						string2 += nextBackslashOrQuote + 1ull;
						string1 += nextBackslashOrQuote + 2ull;
					} else {
						lengthNew -= bytesProcessed;
						string2 += bytesProcessed;
						string1 += bytesProcessed;
					}
				} else {
					lengthNew -= bytesProcessed;
					string2 += bytesProcessed;
					string1 += bytesProcessed;
				}
			}
		}
#endif
		{
			using integer_type				  = typename jsonifier::concepts::get_type_at_index<simd_internal::avx_integer_list, 0>::type::integer_type;
			using simd_type					  = typename jsonifier::concepts::get_type_at_index<simd_internal::avx_integer_list, 0>::type::type;
			constexpr uint64_t bytesProcessed = jsonifier::concepts::get_type_at_index<simd_internal::avx_integer_list, 0>::type::bytesProcessed;
			constexpr integer_type mask		  = jsonifier::concepts::get_type_at_index<simd_internal::avx_integer_list, 0>::type::mask;
			simd_type collectionValue;
			integer_type nextBackslashOrQuote;
			while (static_cast<int64_t>(lengthNew) >= static_cast<int64_t>(bytesProcessed)) {
				nextBackslashOrQuote = copyAndFindParse<simd_type, integer_type>(string1, string2, collectionValue);
				if (nextBackslashOrQuote < mask) {
					escapeChar = string1[nextBackslashOrQuote];
					if (escapeChar == '"') {
						string1 += nextBackslashOrQuote;
						return string2 + nextBackslashOrQuote;
					} else if (escapeChar == '\\') {
						escapeChar = string1[nextBackslashOrQuote + 1];
						if (escapeChar == 0x75u) {
							lengthNew -= nextBackslashOrQuote;
							string1 += nextBackslashOrQuote;
							string2 += nextBackslashOrQuote;
							if (!handleUnicodeCodePoint(string1, string2)) {
								return static_cast<iterator_type02>(nullptr);
							}
							continue;
						}
						escapeChar = escapeMap<char>[static_cast<uint8_t>(escapeChar)];
						if (escapeChar == 0u) {
							return string2;
						}
						string2[nextBackslashOrQuote] = static_cast<char_type02>(escapeChar);
						lengthNew -= nextBackslashOrQuote + 2ull;
						string2 += nextBackslashOrQuote + 1ull;
						string1 += nextBackslashOrQuote + 2ull;
					} else {
						lengthNew -= bytesProcessed;
						string2 += bytesProcessed;
						string1 += bytesProcessed;
					}
				} else {
					lengthNew -= bytesProcessed;
					string2 += bytesProcessed;
					string1 += bytesProcessed;
				}
			}
		}
		return parseShortStringImpl(string1, string2, lengthNew);
	}

	constexpr std::array<uint16_t, 256> escapeTable{ [] {
		std::array<uint16_t, 256> returnValues{};
		returnValues['\"'] = 0x225Cu;
		returnValues['\\'] = 0x5C5Cu;
		returnValues['\/'] = 0x2F5Cu;
		returnValues['\b'] = 0x625Cu;
		returnValues['\f'] = 0x665Cu;
		returnValues['\n'] = 0x6E5Cu;
		returnValues['\r'] = 0x725Cu;
		returnValues['\t'] = 0x745Cu;
		return returnValues;
	}() };

	template<typename iterator_type01, typename iterator_type02>
	JSONIFIER_INLINE void serializeShortStringImpl(iterator_type01 string1, iterator_type02& string2, uint64_t lengthNew) {
		auto* end = string1 + lengthNew;
		for (; string1 < end; ++string1) {
			if (auto escapeChar = escapeTable[static_cast<uint8_t>(*string1)]; escapeChar) [[likely]] {
				std::memcpy(string2, &escapeChar, 2);
				string2 += 2;
			} else {
				*string2 = *string1;
				++string2;
			}
		}
	}

	template<typename iterator_type01, typename iterator_type02> JSONIFIER_INLINE void serializeStringImpl(iterator_type01 string1, iterator_type02& string2, uint64_t lengthNew) {
		uint16_t escapeChar;
#if JSONIFIER_CHECK_FOR_AVX(JSONIFIER_AVX512)
		{
			using integer_type				  = typename jsonifier::concepts::get_type_at_index<simd_internal::avx_integer_list, 3>::type::integer_type;
			using simd_type					  = typename jsonifier::concepts::get_type_at_index<simd_internal::avx_integer_list, 3>::type::type;
			constexpr uint64_t bytesProcessed = jsonifier::concepts::get_type_at_index<simd_internal::avx_integer_list, 3>::type::bytesProcessed;
			constexpr integer_type mask		  = jsonifier::concepts::get_type_at_index<simd_internal::avx_integer_list, 3>::type::mask;
			simd_type collectionValue;
			integer_type nextEscapeable;
			while (static_cast<int64_t>(lengthNew) >= static_cast<int64_t>(bytesProcessed)) {
				nextEscapeable = copyAndFindSerialize<simd_type, integer_type>(string1, string2, collectionValue);
				if (nextEscapeable < mask) {
					escapeChar = escapeTable[static_cast<uint8_t>(string1[nextEscapeable])];
					if (escapeChar != 0u) [[likely]] {
						lengthNew -= nextEscapeable;
						string2 += nextEscapeable;
						string1 += nextEscapeable;
						std::memcpy(string2, &escapeChar, 2);
						string2 += 2ull;
						--lengthNew;
						++string1;
					} else {
						string2 += nextEscapeable;
						return;
					}
				} else {
					lengthNew -= bytesProcessed;
					string2 += bytesProcessed;
					string1 += bytesProcessed;
				}
			}
		}
#endif
#if JSONIFIER_CHECK_FOR_AVX(JSONIFIER_AVX2)
		{
			using integer_type				  = typename jsonifier::concepts::get_type_at_index<simd_internal::avx_integer_list, 2>::type::integer_type;
			using simd_type					  = typename jsonifier::concepts::get_type_at_index<simd_internal::avx_integer_list, 2>::type::type;
			constexpr uint64_t bytesProcessed = jsonifier::concepts::get_type_at_index<simd_internal::avx_integer_list, 2>::type::bytesProcessed;
			constexpr integer_type mask		  = jsonifier::concepts::get_type_at_index<simd_internal::avx_integer_list, 2>::type::mask;
			simd_type collectionValue;
			integer_type nextEscapeable;
			while (static_cast<int64_t>(lengthNew) >= static_cast<int64_t>(bytesProcessed)) {
				nextEscapeable = copyAndFindSerialize<simd_type, integer_type>(string1, string2, collectionValue);
				if (nextEscapeable < mask) {
					escapeChar = escapeTable[static_cast<uint8_t>(string1[nextEscapeable])];
					if (escapeChar != 0u) [[likely]] {
						lengthNew -= nextEscapeable;
						string2 += nextEscapeable;
						string1 += nextEscapeable;
						std::memcpy(string2, &escapeChar, 2);
						string2 += 2ull;
						--lengthNew;
						++string1;
					} else {
						string2 += nextEscapeable;
						return;
					}
				} else {
					lengthNew -= bytesProcessed;
					string2 += bytesProcessed;
					string1 += bytesProcessed;
				}
			}
		}
#endif
#if JSONIFIER_CHECK_FOR_AVX(JSONIFIER_AVX) || JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_NEON)
		{
			using integer_type				  = typename jsonifier::concepts::get_type_at_index<simd_internal::avx_integer_list, 1>::type::integer_type;
			using simd_type					  = typename jsonifier::concepts::get_type_at_index<simd_internal::avx_integer_list, 1>::type::type;
			constexpr uint64_t bytesProcessed = jsonifier::concepts::get_type_at_index<simd_internal::avx_integer_list, 1>::type::bytesProcessed;
			constexpr integer_type mask		  = jsonifier::concepts::get_type_at_index<simd_internal::avx_integer_list, 1>::type::mask;
			simd_type collectionValue;
			integer_type nextEscapeable;
			while (static_cast<int64_t>(lengthNew) >= static_cast<int64_t>(bytesProcessed)) {
				nextEscapeable = copyAndFindSerialize<simd_type, integer_type>(string1, string2, collectionValue);
				if (nextEscapeable < mask) {
					escapeChar = escapeTable[static_cast<uint8_t>(string1[nextEscapeable])];
					if (escapeChar != 0u) [[likely]] {
						lengthNew -= nextEscapeable;
						string2 += nextEscapeable;
						string1 += nextEscapeable;
						std::memcpy(string2, &escapeChar, 2);
						string2 += 2ull;
						--lengthNew;
						++string1;
					} else {
						string2 += nextEscapeable;
						return;
					}
				} else {
					lengthNew -= bytesProcessed;
					string2 += bytesProcessed;
					string1 += bytesProcessed;
				}
			}
		}
#endif
		{
			using integer_type				  = typename jsonifier::concepts::get_type_at_index<simd_internal::avx_integer_list, 0>::type::integer_type;
			using simd_type					  = typename jsonifier::concepts::get_type_at_index<simd_internal::avx_integer_list, 0>::type::type;
			constexpr uint64_t bytesProcessed = jsonifier::concepts::get_type_at_index<simd_internal::avx_integer_list, 0>::type::bytesProcessed;
			constexpr integer_type mask		  = jsonifier::concepts::get_type_at_index<simd_internal::avx_integer_list, 0>::type::mask;
			simd_type collectionValue;
			integer_type nextEscapeable;
			while (static_cast<int64_t>(lengthNew) >= static_cast<int64_t>(bytesProcessed)) {
				nextEscapeable = copyAndFindSerialize<simd_type, integer_type>(string1, string2, collectionValue);
				if (nextEscapeable < mask) {
					escapeChar = escapeTable[static_cast<uint8_t>(string1[nextEscapeable])];
					if (escapeChar != 0u) [[likely]] {
						lengthNew -= nextEscapeable;
						string2 += nextEscapeable;
						string1 += nextEscapeable;
						std::memcpy(string2, &escapeChar, 2);
						string2 += 2ull;
						--lengthNew;
						++string1;
					} else {
						string2 += nextEscapeable;
						return;
					}
				} else {
					lengthNew -= bytesProcessed;
					string2 += bytesProcessed;
					string1 += bytesProcessed;
				}
			}
		}
		return serializeShortStringImpl(string1, string2, lengthNew);
	}

	template<uint64_t length> struct convert_length_to_int {
		static_assert(length <= 8, "Sorry, but that string is too long!");
		using type = std::conditional_t<length == 1, uint8_t,
			std::conditional_t<length <= 2, uint16_t, std::conditional_t<length <= 4, uint32_t, std::conditional_t<length <= 8, uint64_t, void>>>>;
	};

	template<uint64_t length> using convert_length_to_int_t = typename convert_length_to_int<length>::type;

	template<string_literal string> constexpr convert_length_to_int_t<string.size()> getStringAsInt() {
		const char* stringNew = string.data();
		convert_length_to_int_t<string.size()> returnValue{};
		for (uint64_t x = 0; x < string.size(); ++x) {
			returnValue |= static_cast<convert_length_to_int_t<string.size()>>(stringNew[x]) << x * 8;
		}
		return returnValue;
	}

	template<string_literal string> JSONIFIER_INLINE bool compareStringAsInt(const char* iter) {
		constexpr auto newString{ getStringAsInt<string>() };
		convert_length_to_int_t<string.size()> newString02{};
		std::memcpy(&newString02, iter, string.size());
		return newString == newString02;
	}

	template<typename iterator_type, jsonifier::concepts::bool_t bool_type> JSONIFIER_INLINE bool parseBool(bool_type& value, iterator_type&& iter) {
		if (compareStringAsInt<"true">(iter)) {
			value = true;
			iter += 4;
			return true;
		} else if (compareStringAsInt<"false">(iter)) {
			value = false;
			iter += 5;
			return true;
		} else {
			return false;
		}
	}

	template<simd_structural_iterator_t iterator_type, jsonifier::concepts::bool_t bool_type> JSONIFIER_INLINE bool parseBool(bool_type& value, iterator_type&& iter) {
		if (compareStringAsInt<"true">(iter)) {
			value = true;
			++iter;
			return true;
		} else if (compareStringAsInt<"false">(iter)) {
			value = false;
			++iter;
			return true;
		} else {
			return false;
		}
	}

	template<typename iterator_type> JSONIFIER_INLINE bool parseNull(iterator_type&& iter) {
		if (compareStringAsInt<"null">(iter)) {
			iter += 4;
			return true;
		} else {
			return false;
		}
	}

	template<simd_structural_iterator_t iterator_type> JSONIFIER_INLINE bool parseNull(iterator_type&& iter) {
		if (compareStringAsInt<"null">(iter)) {
			++iter;
			return true;
		} else {
			return false;
		}
	}

	template<const auto& options, typename value_type, simd_structural_iterator_t iterator_type>
	JSONIFIER_INLINE void parseString(value_type&& value, iterator_type&& iter, iterator_type&& end, jsonifier::vector<error>& errors) {
		auto newPtr = iter.operator->();
		if (*newPtr == 0x22u) [[likely]] {
			++iter;
		} else {
			static constexpr auto sourceLocation{ std::source_location::current() };
			errors.emplace_back(error::constructError<sourceLocation, error_classes::Parsing, parse_errors::Missing_String_Start>(iter - options.rootIter, end - options.rootIter,
				options.rootIter));
			skipToNextValue(iter, end);
			return;
		}
		auto newSize = static_cast<uint64_t>(iter.operator->() - newPtr);
		if (static_cast<int64_t>(newSize) > 1) {
			static thread_local jsonifier::string_base<char, 1024 * 1024> newString{};
			if (newSize > newString.size()) [[unlikely]] {
				newString.resize(newSize);
			}
			++newPtr;
			newPtr = parseStringImpl(newPtr, newString.data(), newSize);
			if (newPtr) [[likely]] {
				newSize = static_cast<uint64_t>(newPtr - newString.data());
				if (value.size() != newSize) {
					value.resize(newSize);
				}
				std::memcpy(value.data(), newString.data(), newSize);
			} else {
				static constexpr auto sourceLocation{ std::source_location::current() };
				errors.emplace_back(error::constructError<sourceLocation, error_classes::Parsing, parse_errors::Invalid_String_Characters>(iter - options.rootIter,
					end - options.rootIter, options.rootIter));
				skipToNextValue(iter, end);
				return;
			}
		}
	}

	template<const auto& options, typename value_type, typename iterator_type>
	JSONIFIER_INLINE void parseString(value_type&& value, iterator_type&& iter, iterator_type&& end, jsonifier::vector<error>& errors) {
		if (*iter == '"') [[unlikely]] {
			++iter;
		} else {
			static constexpr auto sourceLocation{ std::source_location::current() };
			errors.emplace_back(error::constructError<sourceLocation, error_classes::Parsing, parse_errors::Missing_String_Start>(iter - options.rootIter, end - options.rootIter,
				options.rootIter));
			return;
		}
		static thread_local jsonifier::string_base<char, 1024 * 1024> newString{};
		auto newSize = end - iter;
		if (static_cast<uint64_t>(newSize) > newString.size()) [[unlikely]] {
			newString.resize(static_cast<uint64_t>(newSize));
		}
		auto newerPtr = parseStringImpl(iter, newString.data(), static_cast<uint64_t>(newSize));
		if (newerPtr) [[likely]] {
			++iter;
			newSize = newerPtr - newString.data();
			if (value.size() != static_cast<uint64_t>(newSize)) {
				value.resize(static_cast<uint64_t>(newSize));
			}
			std::memcpy(value.data(), newString.data(), static_cast<uint64_t>(newSize));
		} else {
			static constexpr auto sourceLocation{ std::source_location::current() };
			errors.emplace_back(error::constructError<sourceLocation, error_classes::Parsing, parse_errors::Invalid_String_Characters>(iter - options.rootIter,
				end - options.rootIter, options.rootIter));
			return;
		}
	}

}// namespace jsonifier_internal