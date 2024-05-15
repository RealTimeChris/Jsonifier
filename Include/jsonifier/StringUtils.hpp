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
/// Most of the code string1 this header was sampled from simdjson - https://github.com/simdjson
#pragma once

#include <jsonifier/Allocator.hpp>
#include <jsonifier/ISADetection.hpp>
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

	template<typename value_type> JSONIFIER_INLINE uint32_t hexToU32NoCheck(value_type* string1) {
		return digitToVal32[630ull + string1[0]] | digitToVal32[420ull + string1[1]] | digitToVal32[210ull + string1[2]] | digitToVal32[0ull + string1[3]];
	}

	template<typename value_type> JSONIFIER_INLINE uint32_t codePointToUtf8(uint32_t codePoint, value_type* c) {
		static constexpr uint8_t utf8Table[4][4] = { { 0x00 }, { 0xC0, 0x80 }, { 0xE0, 0x80, 0x80 }, { 0xF0, 0x80, 0x80, 0x80 } };

		if (codePoint <= 0x7F) {
			c[0] = uint8_t(codePoint);
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

	template<typename value_type01, typename value_type02> JSONIFIER_INLINE bool handleUnicodeCodePoint(value_type01*& srcPtr, value_type02*& dstPtr) {
		using char_type = std::remove_pointer_t<value_type01>;
		static constexpr uint32_t subCodePoint{ 0xfffd };
		static constexpr uint32_t codePointLessThan01{ 0xd800 };
		static constexpr uint32_t codePointLessThan02{ 0xdc00 };
		static constexpr char_type quotesValue{ static_cast<char_type>(0x5Cu << 8) };
		static constexpr char_type uValue{ static_cast<char_type>(0x75u) };
		uint32_t codePoint = hexToU32NoCheck(srcPtr + 2);
		srcPtr += 6;
		if (codePoint >= codePointLessThan01 && codePoint < codePointLessThan02) {
			value_type01* srcData = srcPtr;
			if (((srcData[0] << 8) | srcData[1]) != (quotesValue | uValue)) {
				codePoint = subCodePoint;
			} else {
				uint32_t codePoint02 = hexToU32NoCheck(srcData + 2);
				codePoint02			 = codePoint02 - codePointLessThan02;
				if (codePoint02 >> 10) {
					codePoint = subCodePoint;
				} else {
					codePoint = (((codePoint - codePointLessThan01) << 10) | codePoint02) + 0x10000;
					srcPtr += 6;
				}
			}
		} else if (codePoint >= codePointLessThan02 && codePoint <= 0xdfff) {
			codePoint = subCodePoint;
		}
		uint32_t offset = codePointToUtf8(codePoint, dstPtr);
		dstPtr += offset;
		return offset > 0;
	}

	template<typename return_type> constexpr return_type isLess32(return_type value) noexcept {
		constexpr return_type newBytes{ repeatByte<0b11100000u, return_type>() };
		return hasZero(value & newBytes);
	}

	template<typename simd_type, typename integer_type, typename char_type01, typename char_type02>
	JSONIFIER_INLINE integer_type copyAndFindParse(char_type01* string1, char_type02* string2, simd_type& simdValue) {
		simdValue = simd_internal::gatherValuesU<simd_type>(string1);
		std::memcpy(string2, string1, sizeof(simd_type));
		return simd_internal::tzcnt(static_cast<integer_type>(
			simd_internal::opCmpEq(simdValue, simd_internal::backslashes<simd_type>) | simd_internal::opCmpEq(simdValue, simd_internal::quotes<simd_type>)));
	}

	template<jsonifier::concepts::unsigned_type simd_type, jsonifier::concepts::unsigned_type integer_type, typename char_type01, typename char_type02>
	JSONIFIER_INLINE integer_type copyAndFindParse(char_type01* string1, char_type02* string2, simd_type& simdValue) {
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

	template<typename simd_type, typename integer_type, typename char_type01> JSONIFIER_INLINE integer_type findParse(char_type01* string1, simd_type& simdValue) {
		simdValue = simd_internal::gatherValuesU<simd_type>(string1);
		return simd_internal::tzcnt(static_cast<integer_type>(
			simd_internal::opCmpEq(simdValue, simd_internal::backslashes<simd_type>) | simd_internal::opCmpEq(simdValue, simd_internal::quotes<simd_type>)));
	}

	template<jsonifier::concepts::unsigned_type simd_type, jsonifier::concepts::unsigned_type integer_type, typename char_type01>
	JSONIFIER_INLINE integer_type findParse(char_type01* string1, simd_type& simdValue) {
		std::memcpy(&simdValue, string1, sizeof(simd_type));
		constexpr uint64_t mask	 = repeatByte<0b01111111, integer_type>();
		const uint64_t lo7		 = simdValue & mask;
		const uint64_t quote	 = (lo7 ^ repeatByte<'"', integer_type>()) + mask;
		const uint64_t backslash = (lo7 ^ repeatByte<'\\', integer_type>()) + mask;
		const uint64_t t0		 = ~((quote & backslash) | simdValue);
		uint64_t next			 = t0 & repeatByte<0b10000000, integer_type>();
		return static_cast<integer_type>(simd_internal::tzcnt(next) >> 3u);
	}

	template<typename simd_type, typename integer_type, typename char_type01, typename char_type02>
	JSONIFIER_INLINE integer_type copyAndFindSerialize(char_type01* string1, char_type02* string2, simd_type& simdValue) {
		simdValue = simd_internal::gatherValuesU<simd_type>(string1);
		std::memcpy(string2, string1, sizeof(simd_type));
		return simd_internal::tzcnt(static_cast<integer_type>(simd_internal::opCmpEq(simd_internal::opShuffle(simd_internal::escapeableTable00<simd_type>, simdValue), simdValue) |
			simd_internal::opCmpEq(simd_internal::opShuffle(simd_internal::escapeableTable01<simd_type>, simdValue), simdValue)));
	}

	template<jsonifier::concepts::unsigned_type simd_type, jsonifier::concepts::unsigned_type integer_type, typename char_type01, typename char_type02>
	JSONIFIER_INLINE integer_type copyAndFindSerialize(char_type01* string1, char_type02* string2, simd_type& simdValue) {
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

	template<typename char_type01> JSONIFIER_INLINE char_type01* skipShortStringImpl(char_type01* string1, uint64_t& lengthNew) {
		static constexpr char_type01 quotesValue{ static_cast<char_type01>(0x22u) };
		while (static_cast<int64_t>(lengthNew) > 0) {
			if (*string1 == quotesValue || *string1 == 0x5Cu) {
				auto escapeChar = *string1;
				if (escapeChar == quotesValue) {
					return string1;
				} else {
					string1 += 2;
					lengthNew -= 2;
				}
			} else {
				++string1;
				--lengthNew;
			}
		}
		return string1;
	}

	template<typename char_type01> JSONIFIER_INLINE char_type01* skipStringImpl(char_type01* string1, uint64_t& lengthNew) {
		static constexpr char_type01 quotesValue{ static_cast<char_type01>(0x22u) };
#if JSONIFIER_CHECK_FOR_AVX(JSONIFIER_AVX512)
		{
			using integer_type						 = typename jsonifier::concepts::get_type_at_index<simd_internal::avx_integer_list, 3>::type::integer_type;
			using simd_type							 = typename jsonifier::concepts::get_type_at_index<simd_internal::avx_integer_list, 3>::type::type;
			static constexpr uint64_t bytesProcessed = jsonifier::concepts::get_type_at_index<simd_internal::avx_integer_list, 3>::type::bytesProcessed;
			static constexpr integer_type mask		 = jsonifier::concepts::get_type_at_index<simd_internal::avx_integer_list, 3>::type::mask;
			simd_type collectionValue;
			while (static_cast<int64_t>(lengthNew) >= bytesProcessed) {
				integer_type nextBackslashOrQuote = findParse<simd_type, integer_type>(string1, collectionValue);
				if (nextBackslashOrQuote < mask) {
					auto escapeChar = string1[nextBackslashOrQuote];
					if (escapeChar == quotesValue) {
						string1 += nextBackslashOrQuote + 1;
						return string1;
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
			using integer_type						 = typename jsonifier::concepts::get_type_at_index<simd_internal::avx_integer_list, 2>::type::integer_type;
			using simd_type							 = typename jsonifier::concepts::get_type_at_index<simd_internal::avx_integer_list, 2>::type::type;
			static constexpr uint64_t bytesProcessed = jsonifier::concepts::get_type_at_index<simd_internal::avx_integer_list, 2>::type::bytesProcessed;
			static constexpr integer_type mask		 = jsonifier::concepts::get_type_at_index<simd_internal::avx_integer_list, 2>::type::mask;
			simd_type collectionValue;
			while (static_cast<int64_t>(lengthNew) >= bytesProcessed) {
				integer_type nextBackslashOrQuote = findParse<simd_type, integer_type>(string1, collectionValue);
				if (nextBackslashOrQuote < mask) {
					auto escapeChar = string1[nextBackslashOrQuote];
					if (escapeChar == quotesValue) {
						string1 += nextBackslashOrQuote + 1;
						return string1;
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
#if JSONIFIER_CHECK_FOR_AVX(JSONIFIER_AVX) || JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_NEON)
		{
			using integer_type						 = typename jsonifier::concepts::get_type_at_index<simd_internal::avx_integer_list, 1>::type::integer_type;
			using simd_type							 = typename jsonifier::concepts::get_type_at_index<simd_internal::avx_integer_list, 1>::type::type;
			static constexpr uint64_t bytesProcessed = jsonifier::concepts::get_type_at_index<simd_internal::avx_integer_list, 1>::type::bytesProcessed;
			static constexpr integer_type mask		 = jsonifier::concepts::get_type_at_index<simd_internal::avx_integer_list, 1>::type::mask;
			simd_type collectionValue;
			while (static_cast<int64_t>(lengthNew) >= bytesProcessed) {
				integer_type nextBackslashOrQuote = findParse<simd_type, integer_type>(string1, collectionValue);
				if (nextBackslashOrQuote < mask) {
					auto escapeChar = string1[nextBackslashOrQuote];
					if (escapeChar == quotesValue) {
						string1 += nextBackslashOrQuote + 1;
						return string1;
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
		{
			using integer_type						 = typename jsonifier::concepts::get_type_at_index<simd_internal::avx_integer_list, 0>::type::integer_type;
			using simd_type							 = typename jsonifier::concepts::get_type_at_index<simd_internal::avx_integer_list, 0>::type::type;
			static constexpr uint64_t bytesProcessed = jsonifier::concepts::get_type_at_index<simd_internal::avx_integer_list, 0>::type::bytesProcessed;
			static constexpr integer_type mask		 = jsonifier::concepts::get_type_at_index<simd_internal::avx_integer_list, 0>::type::mask;
			simd_type collectionValue;
			while (static_cast<int64_t>(lengthNew) >= bytesProcessed) {
				integer_type nextBackslashOrQuote = findParse<simd_type, integer_type>(string1, collectionValue);
				if (nextBackslashOrQuote < mask) {
					auto escapeChar = string1[nextBackslashOrQuote];
					if (escapeChar == quotesValue) {
						string1 += nextBackslashOrQuote + 1;
						return string1;
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

		return skipShortStringImpl(string1, lengthNew);
	}

	template<typename value_type> constexpr std::array<std::remove_const_t<value_type>, 256> escapeMap{ [] {
		std::array<std::remove_const_t<value_type>, 256> returnValues{};
		returnValues['b']  = 0x08u;
		returnValues['t']  = 0x09u;
		returnValues['n']  = 0x0Au;
		returnValues['v']  = 0x0Bu;
		returnValues['f']  = 0x0Cu;
		returnValues['r']  = 0x0Du;
		returnValues['"']  = 0x22u;
		returnValues['/']  = 0x2Fu;
		returnValues['\\'] = 0x5Cu;
		return returnValues;
	}() };

	template<typename char_type01, typename char_type02> JSONIFIER_INLINE char_type02* parseShortStringImpl(char_type01* string1, char_type02* string2, uint64_t lengthNew) {
		while (lengthNew > 0) {
			*string2 = static_cast<char_type02>(*string1);
			if (*string1 == '"' || *string1 == '\\') {
				auto escapeChar = *string1;
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
					escapeChar = escapeMap<char_type01>[escapeChar];
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

	template<typename char_type01, typename char_type02> JSONIFIER_INLINE char_type02* parseStringImpl(char_type01* string1, char_type02* string2, uint64_t lengthNew) {
#if JSONIFIER_CHECK_FOR_AVX(JSONIFIER_AVX512)
		{
			using integer_type						 = typename jsonifier::concepts::get_type_at_index<simd_internal::avx_integer_list, 3>::type::integer_type;
			using simd_type							 = typename jsonifier::concepts::get_type_at_index<simd_internal::avx_integer_list, 3>::type::type;
			static constexpr uint64_t bytesProcessed = jsonifier::concepts::get_type_at_index<simd_internal::avx_integer_list, 3>::type::bytesProcessed;
			static constexpr integer_type mask		 = jsonifier::concepts::get_type_at_index<simd_internal::avx_integer_list, 3>::type::mask;
			simd_type collectionValue;
			while (static_cast<int64_t>(lengthNew) >= bytesProcessed) {
				integer_type nextBackslashOrQuote = copyAndFindParse<simd_type, integer_type>(string1, string2, collectionValue);
				if (nextBackslashOrQuote < mask) {
					auto escapeChar = string1[nextBackslashOrQuote];
					if (escapeChar == 0x22u) {
						string1 += nextBackslashOrQuote;
						return string2 + nextBackslashOrQuote;
					} else if (escapeChar == 0x5Cu) {
						escapeChar = string1[nextBackslashOrQuote + 1];
						if (escapeChar == 0x75u) {
							lengthNew -= nextBackslashOrQuote;
							string1 += nextBackslashOrQuote;
							string2 += nextBackslashOrQuote;
							if (!handleUnicodeCodePoint(string1, string2)) {
								return static_cast<char_type02*>(nullptr);
							}
							continue;
						}
						escapeChar = escapeMap<char_type01>[escapeChar];
						if (escapeChar == 0u) {
							return static_cast<char_type02*>(nullptr);
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
			using integer_type						 = typename jsonifier::concepts::get_type_at_index<simd_internal::avx_integer_list, 2>::type::integer_type;
			using simd_type							 = typename jsonifier::concepts::get_type_at_index<simd_internal::avx_integer_list, 2>::type::type;
			static constexpr uint64_t bytesProcessed = jsonifier::concepts::get_type_at_index<simd_internal::avx_integer_list, 2>::type::bytesProcessed;
			static constexpr integer_type mask		 = jsonifier::concepts::get_type_at_index<simd_internal::avx_integer_list, 2>::type::mask;
			simd_type collectionValue;
			while (static_cast<int64_t>(lengthNew) >= bytesProcessed) {
				integer_type nextBackslashOrQuote = copyAndFindParse<simd_type, integer_type>(string1, string2, collectionValue);
				if (nextBackslashOrQuote < mask) {
					auto escapeChar = string1[nextBackslashOrQuote];
					if (escapeChar == 0x22u) {
						string1 += nextBackslashOrQuote;
						return string2 + nextBackslashOrQuote;
					} else if (escapeChar == 0x5Cu) {
						escapeChar = string1[nextBackslashOrQuote + 1];
						if (escapeChar == 0x75u) {
							lengthNew -= nextBackslashOrQuote;
							string1 += nextBackslashOrQuote;
							string2 += nextBackslashOrQuote;
							if (!handleUnicodeCodePoint(string1, string2)) {
								return static_cast<char_type02*>(nullptr);
							}
							continue;
						}
						escapeChar = escapeMap<char_type01>[escapeChar];
						if (escapeChar == 0u) {
							return static_cast<char_type02*>(nullptr);
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
			using integer_type						 = typename jsonifier::concepts::get_type_at_index<simd_internal::avx_integer_list, 1>::type::integer_type;
			using simd_type							 = typename jsonifier::concepts::get_type_at_index<simd_internal::avx_integer_list, 1>::type::type;
			static constexpr uint64_t bytesProcessed = jsonifier::concepts::get_type_at_index<simd_internal::avx_integer_list, 1>::type::bytesProcessed;
			static constexpr integer_type mask		 = jsonifier::concepts::get_type_at_index<simd_internal::avx_integer_list, 1>::type::mask;
			simd_type collectionValue;
			while (static_cast<int64_t>(lengthNew) >= bytesProcessed) {
				integer_type nextBackslashOrQuote = copyAndFindParse<simd_type, integer_type>(string1, string2, collectionValue);
				if (nextBackslashOrQuote < mask) {
					auto escapeChar = string1[nextBackslashOrQuote];
					if (escapeChar == 0x22u) {
						string1 += nextBackslashOrQuote;
						return string2 + nextBackslashOrQuote;
					} else if (escapeChar == 0x5Cu) {
						escapeChar = string1[nextBackslashOrQuote + 1];
						if (escapeChar == 0x75u) {
							lengthNew -= nextBackslashOrQuote;
							string1 += nextBackslashOrQuote;
							string2 += nextBackslashOrQuote;
							if (!handleUnicodeCodePoint(string1, string2)) {
								return static_cast<char_type02*>(nullptr);
							}
							continue;
						}
						escapeChar = escapeMap<char_type01>[escapeChar];
						if (escapeChar == 0u) {
							return static_cast<char_type02*>(nullptr);
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
			using integer_type						 = typename jsonifier::concepts::get_type_at_index<simd_internal::avx_integer_list, 0>::type::integer_type;
			using simd_type							 = typename jsonifier::concepts::get_type_at_index<simd_internal::avx_integer_list, 0>::type::type;
			static constexpr uint64_t bytesProcessed = jsonifier::concepts::get_type_at_index<simd_internal::avx_integer_list, 0>::type::bytesProcessed;
			static constexpr integer_type mask		 = jsonifier::concepts::get_type_at_index<simd_internal::avx_integer_list, 0>::type::mask;
			simd_type collectionValue;
			while (static_cast<int64_t>(lengthNew) >= bytesProcessed) {
				integer_type nextBackslashOrQuote = copyAndFindParse<simd_type, integer_type>(string1, string2, collectionValue);
				if (nextBackslashOrQuote < mask) {
					auto escapeChar = string1[nextBackslashOrQuote];
					if (escapeChar == 0x22u) {
						string1 += nextBackslashOrQuote;
						return string2 + nextBackslashOrQuote;
					} else if (escapeChar == 0x5Cu) {
						escapeChar = string1[nextBackslashOrQuote + 1];
						if (escapeChar == 0x75u) {
							lengthNew -= nextBackslashOrQuote;
							string1 += nextBackslashOrQuote;
							string2 += nextBackslashOrQuote;
							if (!handleUnicodeCodePoint(string1, string2)) {
								return static_cast<char_type02*>(nullptr);
							}
							continue;
						}
						escapeChar = escapeMap<char_type01>[escapeChar];
						if (escapeChar == 0u) {
							return static_cast<char_type02*>(nullptr);
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
		returnValues['\b'] = 0x625Cu;
		returnValues['\t'] = 0x745Cu;
		returnValues['\n'] = 0x6E5Cu;
		returnValues['\f'] = 0x665Cu;
		returnValues['\r'] = 0x725Cu;
		returnValues['\"'] = 0x225Cu;
		returnValues['\\'] = 0x5C5Cu;
		return returnValues;
	}() };

	template<typename char_type01, typename char_type02> JSONIFIER_INLINE void serializeShortStringImpl(char_type01* string1, char_type02*& string2, uint64_t lengthNew) {
		char_type01* end = string1 + lengthNew;
		for (; string1 < end; ++string1) {
			if (const auto escapeChar = escapeTable[uint8_t(*string1)]; escapeChar) [[likely]] {
				std::memcpy(string2, &escapeChar, 2);
				string2 += 2;
			} else {
				*string2 = *string1;
				++string2;
			}
		}
	}

	template<typename char_type01, typename char_type02> JSONIFIER_INLINE void serializeStringImpl(char_type01* string1, char_type02*& string2, uint64_t lengthNew) {
#if JSONIFIER_CHECK_FOR_AVX(JSONIFIER_AVX512)
		{
			using integer_type						 = typename jsonifier::concepts::get_type_at_index<simd_internal::avx_integer_list, 3>::type::integer_type;
			using simd_type							 = typename jsonifier::concepts::get_type_at_index<simd_internal::avx_integer_list, 3>::type::type;
			static constexpr uint64_t bytesProcessed = jsonifier::concepts::get_type_at_index<simd_internal::avx_integer_list, 3>::type::bytesProcessed;
			static constexpr integer_type mask		 = jsonifier::concepts::get_type_at_index<simd_internal::avx_integer_list, 3>::type::mask;
			simd_type collectionValue;
			while (static_cast<int64_t>(lengthNew) >= bytesProcessed) {
				integer_type nextEscapeable = copyAndFindSerialize<simd_type, integer_type>(string1, string2, collectionValue);
				if (nextEscapeable < mask) {
					auto escapeResult = escapeTable[static_cast<uint64_t>(string1[nextEscapeable])];
					if (escapeResult != 0u) [[likely]] {
						lengthNew -= nextEscapeable;
						string2 += nextEscapeable;
						string1 += nextEscapeable;
						std::memcpy(string2, &escapeResult, 2);
						string2 += 2ULL;
						--lengthNew;
						++string1;
					} else {
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
			using integer_type						 = typename jsonifier::concepts::get_type_at_index<simd_internal::avx_integer_list, 2>::type::integer_type;
			using simd_type							 = typename jsonifier::concepts::get_type_at_index<simd_internal::avx_integer_list, 2>::type::type;
			static constexpr uint64_t bytesProcessed = jsonifier::concepts::get_type_at_index<simd_internal::avx_integer_list, 2>::type::bytesProcessed;
			static constexpr integer_type mask		 = jsonifier::concepts::get_type_at_index<simd_internal::avx_integer_list, 2>::type::mask;
			simd_type collectionValue;
			while (static_cast<int64_t>(lengthNew) >= bytesProcessed) {
				integer_type nextEscapeable = copyAndFindSerialize<simd_type, integer_type>(string1, string2, collectionValue);
				if (nextEscapeable < mask) {
					auto escapeResult = escapeTable[static_cast<uint64_t>(string1[nextEscapeable])];
					if (escapeResult != 0u) [[likely]] {
						lengthNew -= nextEscapeable;
						string2 += nextEscapeable;
						string1 += nextEscapeable;
						std::memcpy(string2, &escapeResult, 2);
						string2 += 2ULL;
						--lengthNew;
						++string1;
					} else {
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
			using integer_type						 = typename jsonifier::concepts::get_type_at_index<simd_internal::avx_integer_list, 1>::type::integer_type;
			using simd_type							 = typename jsonifier::concepts::get_type_at_index<simd_internal::avx_integer_list, 1>::type::type;
			static constexpr uint64_t bytesProcessed = jsonifier::concepts::get_type_at_index<simd_internal::avx_integer_list, 1>::type::bytesProcessed;
			static constexpr integer_type mask		 = jsonifier::concepts::get_type_at_index<simd_internal::avx_integer_list, 1>::type::mask;
			simd_type collectionValue;
			while (static_cast<int64_t>(lengthNew) >= bytesProcessed) {
				integer_type nextEscapeable = copyAndFindSerialize<simd_type, integer_type>(string1, string2, collectionValue);
				if (nextEscapeable < mask) {
					auto escapeResult = escapeTable[static_cast<uint64_t>(string1[nextEscapeable])];
					if (escapeResult != 0u) [[likely]] {
						lengthNew -= nextEscapeable;
						string2 += nextEscapeable;
						string1 += nextEscapeable;
						std::memcpy(string2, &escapeResult, 2);
						string2 += 2ULL;
						--lengthNew;
						++string1;
					} else {
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
			using integer_type						 = typename jsonifier::concepts::get_type_at_index<simd_internal::avx_integer_list, 0>::type::integer_type;
			using simd_type							 = typename jsonifier::concepts::get_type_at_index<simd_internal::avx_integer_list, 0>::type::type;
			static constexpr uint64_t bytesProcessed = jsonifier::concepts::get_type_at_index<simd_internal::avx_integer_list, 0>::type::bytesProcessed;
			static constexpr integer_type mask		 = jsonifier::concepts::get_type_at_index<simd_internal::avx_integer_list, 0>::type::mask;
			simd_type collectionValue;
			while (static_cast<int64_t>(lengthNew) >= bytesProcessed) {
				integer_type nextEscapeable = copyAndFindSerialize<simd_type, integer_type>(string1, string2, collectionValue);
				if (nextEscapeable < mask) {
					auto escapeResult = escapeTable[static_cast<uint64_t>(string1[nextEscapeable])];
					if (escapeResult != 0u) [[likely]] {
						lengthNew -= nextEscapeable;
						string2 += nextEscapeable;
						string1 += nextEscapeable;
						std::memcpy(string2, &escapeResult, 2);
						string2 += 2ULL;
						--lengthNew;
						++string1;
					} else {
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

	template<jsonifier::concepts::bool_t bool_type, typename char_type> JSONIFIER_INLINE bool parseBool(bool_type&& value, char_type* json) {
		static constexpr uint8_t valueNew00[5]{ "true" };
		static constexpr uint8_t valueNew01[6]{ "false" };
		if (compare<4>(valueNew00, json)) {
			value = true;
			return true;
		} else if (compare<5>(valueNew01, json)) {
			value = false;
			return true;
		} else {
			return false;
		}
	}

	template<typename char_type> JSONIFIER_INLINE bool parseNull(char_type* json) {
		static constexpr uint8_t valueNew00[5]{ "null" };
		if (compare<4>(valueNew00, json)) {
			return true;
		} else {
			return false;
		}
	}
}// namespace jsonifier_internal