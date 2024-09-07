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
#include <jsonifier/Reflection.hpp>
#include <jsonifier/Error.hpp>
#include <jsonifier/StrToD.hpp>
#include <jsonifier/Simd.hpp>

namespace jsonifier_internal {

#define JSONIFIER_SKIP_WS() \
	if constexpr (!options.optionsReal.minified) { \
		while (whitespaceTable[static_cast<uint8_t>(*iter)]) { \
			++iter; \
		} \
	}

#define JSONIFIER_SKIP_WS_SIZED(wsSize) \
	if constexpr (!options.optionsReal.minified) { \
		if (iter + wsSize >= end) [[unlikely]] { \
			static constexpr auto sourceLocation{ std::source_location::current() }; \
			options.parserPtr->getErrors().emplace_back(error::constructError<sourceLocation, error_classes::Parsing, parse_errors::Unexpected_String_End>( \
				iter - options.rootIter, end - options.rootIter, options.rootIter)); \
			derailleur<options>::skipToNextValue(iter, end); \
			return; \
		} \
		iter += wsSize; \
		JSONIFIER_SKIP_WS(); \
	}

	JSONIFIER_ALWAYS_INLINE const char* getUnderlyingPtr(const char** ptr) noexcept {
		return *ptr;
	}

	JSONIFIER_ALWAYS_INLINE const char* getUnderlyingPtr(const char* ptr) noexcept {
		return ptr;
	}

	constexpr std::array<uint32_t, 886> digitToVal32{ { 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu,
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
		0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu } };

	// Taken from simdjson: https://github.com/simdjson/simdjson
	template<typename iterator> JSONIFIER_ALWAYS_INLINE uint32_t hexToU32NoCheck(iterator string1) noexcept {
		return digitToVal32[630ull + string1[0]] | digitToVal32[420ull + string1[1]] | digitToVal32[210ull + string1[2]] | digitToVal32[0ull + string1[3]];
	}

	// Taken from simdjson: https://github.com/simdjson/simdjson
	template<typename iterator> JSONIFIER_ALWAYS_INLINE size_t codePointToUtf8(uint32_t cp, iterator c) noexcept {
		if (cp <= 0x7F) {
			c[0] = uint8_t(cp);
			return 1;
		}
		if (cp <= 0x7FF) {
			c[0] = uint8_t((cp >> 6) + 192);
			c[1] = uint8_t((cp & 63) + 128);
			return 2;
		} else if (cp <= 0xFFFF) {
			c[0] = uint8_t((cp >> 12) + 224);
			c[1] = uint8_t(((cp >> 6) & 63) + 128);
			c[2] = uint8_t((cp & 63) + 128);
			return 3;
		} else if (cp <= 0x10FFFF) {
			c[0] = uint8_t((cp >> 18) + 240);
			c[1] = uint8_t(((cp >> 12) & 63) + 128);
			c[2] = uint8_t(((cp >> 6) & 63) + 128);
			c[3] = uint8_t((cp & 63) + 128);
			return 4;
		}
		return 0;
	}

	// Taken from simdjson: https://github.com/simdjson/simdjson
	template<typename iterator_type01, typename iterator_type02> JSONIFIER_ALWAYS_INLINE bool handleUnicodeCodePoint(iterator_type01& srcPtr, iterator_type02& dstPtr) noexcept {
		using char_type01 = uint8_t;
		static constexpr char_type01 quotesValue{ static_cast<char_type01>('\\' << 8) };
		static constexpr char_type01 uValue{ static_cast<char_type01>(0x75u) };
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
		} else {
			if (codePoint >= 0xdc00 && codePoint <= 0xdfff) {
				codePoint = 0xfffd;
			}
		}
		uint32_t offset = codePointToUtf8(codePoint, dstPtr);
		dstPtr += offset;
		return offset > 0;
	}

	template<typename simd_type, typename integer_type> JSONIFIER_ALWAYS_INLINE integer_type copyAndFindParse(const char* string1, char* string2, simd_type& simdValue,
		const simd_type& quotes, const simd_type& backslashes) noexcept {
		simdValue = simd_internal::gatherValuesU<simd_type>(string1);
		std::memcpy(string2, string1, sizeof(simd_type));
		return simd_internal::tzcnt(static_cast<integer_type>(simd_internal::opCmpEq(simdValue, backslashes) | simd_internal::opCmpEq(simdValue, quotes)));
	}

	template<jsonifier::concepts::unsigned_type simd_type, jsonifier::concepts::unsigned_type integer_type>
	JSONIFIER_ALWAYS_INLINE integer_type copyAndFindParse(const char* string1, char* string2, simd_type& simdValue) noexcept {
		static constexpr integer_type mask{ repeatByte<0b01111111, integer_type>() };
		static constexpr integer_type lowBitsMask{ repeatByte<0b10000000, integer_type>() };
		static constexpr integer_type quoteBits{ repeatByte<'"', integer_type>() };
		static constexpr integer_type bsBits{ repeatByte<'\\', integer_type>() };
		std::memcpy(string2, string1, sizeof(simd_type));
		std::memcpy(&simdValue, string1, sizeof(simd_type));
		const size_t lo7  = simdValue & mask;
		const size_t next = (~((((lo7 ^ quoteBits) + mask) & ((lo7 ^ bsBits) + mask)) | simdValue)) & lowBitsMask;
		return static_cast<integer_type>(simd_internal::tzcnt(next) >> 3u);
	}

	template<typename simd_type, typename integer_type>
	JSONIFIER_ALWAYS_INLINE integer_type findParse(const char* string1, simd_type& simdValue, const simd_type& quotes, const simd_type& backslashes) noexcept {
		simdValue = simd_internal::gatherValuesU<simd_type>(string1);
		return simd_internal::tzcnt(static_cast<integer_type>(simd_internal::opCmpEq(simdValue, quotes) | simd_internal::opCmpEq(simdValue, backslashes)));
	}

	template<jsonifier::concepts::unsigned_type simd_type, jsonifier::concepts::unsigned_type integer_type>
	JSONIFIER_ALWAYS_INLINE integer_type findParse(const char* string1, simd_type& simdValue) noexcept {
		static constexpr integer_type mask{ repeatByte<0b01111111, integer_type>() };
		static constexpr integer_type lowBitsMask{ repeatByte<0b10000000, integer_type>() };
		static constexpr integer_type quoteBits{ repeatByte<'"', integer_type>() };
		static constexpr integer_type bsBits{ repeatByte<'\\', integer_type>() };
		std::memcpy(&simdValue, string1, sizeof(simd_type));
		const size_t lo7  = simdValue & mask;
		const size_t next = (~((((lo7 ^ quoteBits) + mask) & ((lo7 ^ bsBits) + mask)) | simdValue)) & lowBitsMask;
		return static_cast<integer_type>(simd_internal::tzcnt(next) >> 3u);
	}

	template<typename simd_type, typename integer_type> JSONIFIER_ALWAYS_INLINE integer_type copyAndFindSerialize(const char* string1, char* string2, simd_type& simdValue,
		const simd_type& simdValues01, const simd_type& simdValues02) noexcept {
		simdValue = simd_internal::gatherValuesU<simd_type>(string1);
		std::memcpy(string2, string1, sizeof(simd_type));
		return simd_internal::tzcnt(static_cast<integer_type>(simd_internal::opCmpEq(simd_internal::opShuffle(simdValues01, simdValue), simdValue) |
			simd_internal::opCmpEq(simd_internal::opShuffle(simdValues02, simdValue), simdValue)));
	}

	template<jsonifier::concepts::unsigned_type simd_type, jsonifier::concepts::unsigned_type integer_type>
	JSONIFIER_ALWAYS_INLINE integer_type copyAndFindSerialize(const char* string1, char* string2, simd_type& simdValue) noexcept {
		static constexpr integer_type mask{ repeatByte<0b01111111, integer_type>() };
		static constexpr integer_type lowBitsMask{ repeatByte<0b10000000, integer_type>() };
		static constexpr integer_type midBitsMask{ repeatByte<0b01100000, integer_type>() };
		static constexpr integer_type quoteBits{ repeatByte<'"', integer_type>() };
		static constexpr integer_type bsBits{ repeatByte<'\\', integer_type>() };
		std::memcpy(string2, string1, sizeof(simd_type));
		std::memcpy(&simdValue, string1, sizeof(simd_type));
		const size_t lo7  = simdValue & mask;
		const size_t next = ~((((lo7 ^ quoteBits) + mask) & ((lo7 ^ bsBits) + mask) & ((simdValue & midBitsMask) + mask)) | simdValue) & lowBitsMask;
		return static_cast<integer_type>(simd_internal::tzcnt(next) >> 3u);
	}

	template<typename iterator_type01> JSONIFIER_ALWAYS_INLINE static void skipShortStringImpl(iterator_type01& string1, size_t& lengthNew) noexcept {
		static constexpr char quotesValue{ static_cast<char>('"') };
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

	template<typename iterator_type01> JSONIFIER_ALWAYS_INLINE static void skipStringImpl(iterator_type01& string1, size_t& lengthNew) noexcept {
		using char_type01 =
			typename std::conditional_t<std::is_pointer_v<iterator_type01>, std::remove_pointer_t<iterator_type01>, typename std::iterator_traits<iterator_type01>::value_type>;
		std::remove_const_t<char_type01> escapeChar;
		constexpr char quotesValue{ static_cast<char>('"') };
#if JSONIFIER_CHECK_FOR_AVX(JSONIFIER_AVX512)
		{
			using integer_type					   = typename get_type_at_index<simd_internal::avx_integer_list, 3>::type::integer_type;
			using simd_type						   = typename get_type_at_index<simd_internal::avx_integer_list, 3>::type::type;
			static constexpr size_t bytesProcessed = get_type_at_index<simd_internal::avx_integer_list, 3>::type::bytesProcessed;
			static constexpr integer_type mask	   = get_type_at_index<simd_internal::avx_integer_list, 3>::type::mask;
			const simd_type quotes				   = simd_internal::gatherValue<simd_type>('\\');
			const simd_type backslashes			   = simd_internal::gatherValue<simd_type>('"');
			simd_type simdValue;
			integer_type nextBackslashOrQuote;
			while (static_cast<int64_t>(lengthNew) >= static_cast<int64_t>(bytesProcessed)) {
				nextBackslashOrQuote = findParse<simd_type, integer_type>(string1, simdValue, quotes, backslashes);
				if (nextBackslashOrQuote < mask) [[likely]] {
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
			using integer_type					   = typename get_type_at_index<simd_internal::avx_integer_list, 2>::type::integer_type;
			using simd_type						   = typename get_type_at_index<simd_internal::avx_integer_list, 2>::type::type;
			static constexpr size_t bytesProcessed = get_type_at_index<simd_internal::avx_integer_list, 2>::type::bytesProcessed;
			static constexpr integer_type mask	   = get_type_at_index<simd_internal::avx_integer_list, 2>::type::mask;
			const simd_type quotes				   = simd_internal::gatherValue<simd_type>('\\');
			const simd_type backslashes			   = simd_internal::gatherValue<simd_type>('"');
			simd_type simdValue;
			integer_type nextBackslashOrQuote;
			while (static_cast<int64_t>(lengthNew) >= static_cast<int64_t>(bytesProcessed)) {
				nextBackslashOrQuote = findParse<simd_type, integer_type>(string1, simdValue, quotes, backslashes);
				if (nextBackslashOrQuote < mask) [[likely]] {
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
			using integer_type					   = typename get_type_at_index<simd_internal::avx_integer_list, 1>::type::integer_type;
			using simd_type						   = typename get_type_at_index<simd_internal::avx_integer_list, 1>::type::type;
			static constexpr size_t bytesProcessed = get_type_at_index<simd_internal::avx_integer_list, 1>::type::bytesProcessed;
			static constexpr integer_type mask	   = get_type_at_index<simd_internal::avx_integer_list, 1>::type::mask;
			const simd_type quotes				   = simd_internal::gatherValue<simd_type>('\\');
			const simd_type backslashes			   = simd_internal::gatherValue<simd_type>('"');
			simd_type simdValue;
			integer_type nextBackslashOrQuote;
			while (static_cast<int64_t>(lengthNew) >= static_cast<int64_t>(bytesProcessed)) {
				nextBackslashOrQuote = findParse<simd_type, integer_type>(string1, simdValue, quotes, backslashes);
				if (nextBackslashOrQuote < mask) [[likely]] {
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
			using integer_type					   = typename get_type_at_index<simd_internal::avx_integer_list, 0>::type::integer_type;
			using simd_type						   = typename get_type_at_index<simd_internal::avx_integer_list, 0>::type::type;
			static constexpr size_t bytesProcessed = get_type_at_index<simd_internal::avx_integer_list, 0>::type::bytesProcessed;
			static constexpr integer_type mask	   = get_type_at_index<simd_internal::avx_integer_list, 0>::type::mask;
			simd_type simdValue;
			integer_type nextBackslashOrQuote;
			while (static_cast<int64_t>(lengthNew) >= static_cast<int64_t>(bytesProcessed)) {
				nextBackslashOrQuote = findParse<simd_type, integer_type>(string1, simdValue);
				if (nextBackslashOrQuote < mask) [[likely]] {
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

	constexpr std::array<char, 256> escapeMap{ [] {
		std::array<char, 256> returnValues{};
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
	JSONIFIER_ALWAYS_INLINE iterator_type02 parseShortStringImpl(iterator_type01& string1, iterator_type02 string2, size_t lengthNew) noexcept {
		using char_type01 =
			typename std::conditional_t<std::is_pointer_v<iterator_type01>, std::remove_pointer_t<iterator_type01>, typename std::iterator_traits<iterator_type01>::value_type>;
		using char_type02 =
			typename std::conditional_t<std::is_pointer_v<iterator_type02>, std::remove_pointer_t<iterator_type02>, typename std::iterator_traits<iterator_type02>::value_type>;
		std::remove_const_t<char_type01> escapeChar;
		while (lengthNew > 0) {
			*string2 = static_cast<char_type02>(*string1);
			if (*string1 == '"' || *string1 == '\\') [[likely]] {
				escapeChar = *string1;
				if (escapeChar == '"') {
					return string2;
				} else {
					if (escapeChar == '\\') {
						escapeChar = string1[1];
						if (escapeChar == 'u') {
							if (!handleUnicodeCodePoint(string1, string2)) {
								return nullptr;
							}
							continue;
						}
						escapeChar = escapeMap[static_cast<uint8_t>(escapeChar)];
						if (escapeChar == 0) {
							return string2;
						}
						string2[0] = static_cast<char_type02>(escapeChar);
						lengthNew -= 2;
						string2 += 1;
						string1 += 2;
					}
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
	JSONIFIER_ALWAYS_INLINE iterator_type02 parseStringImpl(iterator_type01& string1, iterator_type02 string2, size_t lengthNew) noexcept {
		using char_type01 =
			typename std::conditional_t<std::is_pointer_v<iterator_type01>, std::remove_pointer_t<iterator_type01>, typename std::iterator_traits<iterator_type01>::value_type>;
		using char_type02 =
			typename std::conditional_t<std::is_pointer_v<iterator_type02>, std::remove_pointer_t<iterator_type02>, typename std::iterator_traits<iterator_type02>::value_type>;
		std::remove_const_t<char_type01> escapeChar;
#if JSONIFIER_CHECK_FOR_AVX(JSONIFIER_AVX512)
		{
			using integer_type					   = typename get_type_at_index<simd_internal::avx_integer_list, 3>::type::integer_type;
			using simd_type						   = typename get_type_at_index<simd_internal::avx_integer_list, 3>::type::type;
			static constexpr size_t bytesProcessed = get_type_at_index<simd_internal::avx_integer_list, 3>::type::bytesProcessed;
			static constexpr integer_type mask	   = get_type_at_index<simd_internal::avx_integer_list, 3>::type::mask;
			const simd_type quotes				   = simd_internal::gatherValue<simd_type>('\\');
			const simd_type backslashes			   = simd_internal::gatherValue<simd_type>('"');
			simd_type simdValue;
			integer_type nextBackslashOrQuote;
			while (static_cast<int64_t>(lengthNew) >= static_cast<int64_t>(bytesProcessed)) {
				nextBackslashOrQuote = copyAndFindParse<simd_type, integer_type>(string1, string2, simdValue, quotes, backslashes);
				if (nextBackslashOrQuote < mask) [[likely]] {
					escapeChar = string1[nextBackslashOrQuote];
					if (escapeChar == '"') {
						string1 += nextBackslashOrQuote;
						return string2 + nextBackslashOrQuote;
					} else {
						if (escapeChar == '\\') {
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
							escapeChar = escapeMap[static_cast<uint8_t>(escapeChar)];
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
			using integer_type					   = typename get_type_at_index<simd_internal::avx_integer_list, 2>::type::integer_type;
			using simd_type						   = typename get_type_at_index<simd_internal::avx_integer_list, 2>::type::type;
			static constexpr size_t bytesProcessed = get_type_at_index<simd_internal::avx_integer_list, 2>::type::bytesProcessed;
			static constexpr integer_type mask	   = get_type_at_index<simd_internal::avx_integer_list, 2>::type::mask;
			const simd_type quotes				   = simd_internal::gatherValue<simd_type>('\\');
			const simd_type backslashes			   = simd_internal::gatherValue<simd_type>('"');
			simd_type simdValue;
			integer_type nextBackslashOrQuote;
			while (static_cast<int64_t>(lengthNew) >= static_cast<int64_t>(bytesProcessed)) {
				nextBackslashOrQuote = copyAndFindParse<simd_type, integer_type>(string1, string2, simdValue, quotes, backslashes);
				if (nextBackslashOrQuote < mask) [[likely]] {
					escapeChar = string1[nextBackslashOrQuote];
					if (escapeChar == '"') {
						string1 += nextBackslashOrQuote;
						return string2 + nextBackslashOrQuote;
					} else {
						if (escapeChar == '\\') {
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
							escapeChar = escapeMap[static_cast<uint8_t>(escapeChar)];
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
			using integer_type					   = typename get_type_at_index<simd_internal::avx_integer_list, 1>::type::integer_type;
			using simd_type						   = typename get_type_at_index<simd_internal::avx_integer_list, 1>::type::type;
			static constexpr size_t bytesProcessed = get_type_at_index<simd_internal::avx_integer_list, 1>::type::bytesProcessed;
			static constexpr integer_type mask	   = get_type_at_index<simd_internal::avx_integer_list, 1>::type::mask;
			const simd_type quotes				   = simd_internal::gatherValue<simd_type>('\\');
			const simd_type backslashes			   = simd_internal::gatherValue<simd_type>('"');
			simd_type simdValue;
			integer_type nextBackslashOrQuote;
			while (static_cast<int64_t>(lengthNew) >= static_cast<int64_t>(bytesProcessed)) {
				nextBackslashOrQuote = copyAndFindParse<simd_type, integer_type>(string1, string2, simdValue, quotes, backslashes);
				if (nextBackslashOrQuote < mask) [[likely]] {
					escapeChar = string1[nextBackslashOrQuote];
					if (escapeChar == '"') {
						string1 += nextBackslashOrQuote;
						return string2 + nextBackslashOrQuote;
					} else {
						if (escapeChar == '\\') {
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
							escapeChar = escapeMap[static_cast<uint8_t>(escapeChar)];
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
			using integer_type					   = typename get_type_at_index<simd_internal::avx_integer_list, 0>::type::integer_type;
			using simd_type						   = typename get_type_at_index<simd_internal::avx_integer_list, 0>::type::type;
			static constexpr size_t bytesProcessed = get_type_at_index<simd_internal::avx_integer_list, 0>::type::bytesProcessed;
			static constexpr integer_type mask	   = get_type_at_index<simd_internal::avx_integer_list, 0>::type::mask;
			simd_type simdValue;
			integer_type nextBackslashOrQuote;
			while (static_cast<int64_t>(lengthNew) >= static_cast<int64_t>(bytesProcessed)) {
				nextBackslashOrQuote = copyAndFindParse<simd_type, integer_type>(string1, string2, simdValue);
				if (nextBackslashOrQuote < mask) [[likely]] {
					escapeChar = string1[nextBackslashOrQuote];
					if (escapeChar == '"') {
						string1 += nextBackslashOrQuote;
						return string2 + nextBackslashOrQuote;
					} else {
						if (escapeChar == '\\') {
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
							escapeChar = escapeMap[static_cast<uint8_t>(escapeChar)];
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
		returnValues['/']  = 0x2F5Cu;
		returnValues['\b'] = 0x625Cu;
		returnValues['\f'] = 0x665Cu;
		returnValues['\n'] = 0x6E5Cu;
		returnValues['\r'] = 0x725Cu;
		returnValues['\t'] = 0x745Cu;
		return returnValues;
	}() };

	template<typename iterator_type01, typename iterator_type02>
	JSONIFIER_ALWAYS_INLINE static void serializeShortStringImpl(iterator_type01 string1, iterator_type02& string2, size_t lengthNew) noexcept {
		auto* end = string1 + lengthNew;
		for (; string1 < end; ++string1) {
			auto escapeChar = escapeTable[static_cast<uint8_t>(*string1)];
			if ((escapeChar)) [[likely]] {
				std::memcpy(string2, &escapeChar, 2);
				string2 += 2;
			} else {
				*string2 = *string1;
				++string2;
			}
		}
	}

	template<typename iterator_type01, typename iterator_type02>
	JSONIFIER_ALWAYS_INLINE static void serializeStringImpl(iterator_type01 string1, iterator_type02& string2, size_t lengthNew) noexcept {
		uint16_t escapeChar;
#if JSONIFIER_CHECK_FOR_AVX(JSONIFIER_AVX512)
		{
			using integer_type					   = typename get_type_at_index<simd_internal::avx_integer_list, 3>::type::integer_type;
			using simd_type						   = typename get_type_at_index<simd_internal::avx_integer_list, 3>::type::type;
			static constexpr size_t bytesProcessed = get_type_at_index<simd_internal::avx_integer_list, 3>::type::bytesProcessed;
			static constexpr integer_type mask	   = get_type_at_index<simd_internal::avx_integer_list, 3>::type::mask;
			const simd_type simdValues00		   = simd_internal::gatherValues<simd_type>(simd_internal::escapeableArray00<bytesPerStep>.data());
			const simd_type simdValues01		   = simd_internal::gatherValues<simd_type>(simd_internal::escapeableArray01<bytesPerStep>.data());
			simd_type simdValue;
			integer_type nextEscapeable;
			while (static_cast<int64_t>(lengthNew) >= static_cast<int64_t>(bytesProcessed)) {
				nextEscapeable = copyAndFindSerialize<simd_type, integer_type>(string1, string2, simdValue, simdValues00, simdValues01);
				if (nextEscapeable < mask) [[likely]] {
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
			using integer_type					   = typename get_type_at_index<simd_internal::avx_integer_list, 2>::type::integer_type;
			using simd_type						   = typename get_type_at_index<simd_internal::avx_integer_list, 2>::type::type;
			static constexpr size_t bytesProcessed = get_type_at_index<simd_internal::avx_integer_list, 2>::type::bytesProcessed;
			static constexpr integer_type mask	   = get_type_at_index<simd_internal::avx_integer_list, 2>::type::mask;
			const simd_type simdValues00		   = simd_internal::gatherValues<simd_type>(simd_internal::escapeableArray00<bytesPerStep>.data());
			const simd_type simdValues01		   = simd_internal::gatherValues<simd_type>(simd_internal::escapeableArray01<bytesPerStep>.data());
			simd_type simdValue;
			integer_type nextEscapeable;
			while (static_cast<int64_t>(lengthNew) >= static_cast<int64_t>(bytesProcessed)) {
				nextEscapeable = copyAndFindSerialize<simd_type, integer_type>(string1, string2, simdValue, simdValues00, simdValues01);
				if (nextEscapeable < mask) [[likely]] {
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
			using integer_type					   = typename get_type_at_index<simd_internal::avx_integer_list, 1>::type::integer_type;
			using simd_type						   = typename get_type_at_index<simd_internal::avx_integer_list, 1>::type::type;
			static constexpr size_t bytesProcessed = get_type_at_index<simd_internal::avx_integer_list, 1>::type::bytesProcessed;
			static constexpr integer_type mask	   = get_type_at_index<simd_internal::avx_integer_list, 1>::type::mask;
			const simd_type simdValues00		   = simd_internal::gatherValues<simd_type>(simd_internal::escapeableArray00<bytesPerStep>.data());
			const simd_type simdValues01		   = simd_internal::gatherValues<simd_type>(simd_internal::escapeableArray01<bytesPerStep>.data());
			simd_type simdValue;
			integer_type nextEscapeable;
			while (static_cast<int64_t>(lengthNew) >= static_cast<int64_t>(bytesProcessed)) {
				nextEscapeable = copyAndFindSerialize<simd_type, integer_type>(string1, string2, simdValue, simdValues00, simdValues01);
				if (nextEscapeable < mask) [[likely]] {
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
			using integer_type					   = typename get_type_at_index<simd_internal::avx_integer_list, 0>::type::integer_type;
			using simd_type						   = typename get_type_at_index<simd_internal::avx_integer_list, 0>::type::type;
			static constexpr size_t bytesProcessed = get_type_at_index<simd_internal::avx_integer_list, 0>::type::bytesProcessed;
			static constexpr integer_type mask	   = get_type_at_index<simd_internal::avx_integer_list, 0>::type::mask;
			simd_type simdValue;
			integer_type nextEscapeable;
			while (static_cast<int64_t>(lengthNew) >= static_cast<int64_t>(bytesProcessed)) {
				nextEscapeable = copyAndFindSerialize<simd_type, integer_type>(string1, string2, simdValue);
				if (nextEscapeable < mask) [[likely]] {
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

	template<size_t length> struct convert_length_to_int {
		static_assert(length <= 8, "Sorry, but that string is too long!");
		using type = std::conditional_t<length == 1, uint8_t,
			std::conditional_t<length <= 2, uint16_t, std::conditional_t<length <= 4, uint32_t, std::conditional_t<length <= 8, size_t, void>>>>;
	};

	template<size_t length> using convert_length_to_int_t = typename convert_length_to_int<length>::type;

	template<typename char_type, string_literal string> JSONIFIER_ALWAYS_INLINE constexpr convert_length_to_int_t<string.size()> getStringAsInt() noexcept {
		const char_type* stringNew = string.data();
		convert_length_to_int_t<string.size()> returnValue{};
		for (size_t x = 0; x < string.size(); ++x) {
			returnValue |= static_cast<convert_length_to_int_t<string.size()>>(stringNew[x]) << x * 8;
		}
		return returnValue;
	}

	template<string_literal string, typename char_type> JSONIFIER_ALWAYS_INLINE bool compareStringAsInt(const char_type* iter) {
		static constexpr auto newString{ getStringAsInt<char_type, string>() };
		if constexpr (string.size() % 2 == 0) {
			convert_length_to_int_t<string.size()> newerString;
			std::memcpy(&newerString, iter, string.size());
			return newString == newerString;
		} else {
			convert_length_to_int_t<string.size()> newerString{};
			std::memcpy(&newerString, iter, string.size());
			return newString == newerString;
		}
	}

	template<typename iterator, jsonifier::concepts::bool_t bool_type> JSONIFIER_ALWAYS_INLINE bool parseBool(bool_type& value, iterator& iter) {
		if (compareStringAsInt<"true">(iter)) {
			value = true;
			iter += 4;
			return true;
		} else {
			if (compareStringAsInt<"false">(iter)) [[likely]] {
				value = false;
				iter += 5;
				return true;
			} else {
				return false;
			}
		}
	}

	template<typename iterator> JSONIFIER_ALWAYS_INLINE bool parseNull(iterator& iter) {
		if (compareStringAsInt<"null">(iter)) [[likely]] {
			iter += 4;
			return true;
		} else {
			return false;
		}
	}

	template<const auto& options> struct derailleur {
		template<typename value_type, typename iterator> JSONIFIER_ALWAYS_INLINE static bool parseString(value_type&& value, iterator& iter, iterator& end) noexcept {
			if (*iter == '"') [[likely]] {
				++iter;
				auto newSize = end - iter;
				if (static_cast<size_t>(newSize) > options.parserPtr->getStringBuffer().size()) [[unlikely]] {
					options.parserPtr->getStringBuffer().resize(static_cast<size_t>(newSize));
				}
				auto newerPtr = parseStringImpl(iter, options.parserPtr->getStringBuffer().data(), static_cast<size_t>(newSize));
				if (newerPtr) [[likely]] {
					if (*iter == '"') [[likely]] {
						++iter;
						newSize = newerPtr - options.parserPtr->getStringBuffer().data();
						if (value.size() != static_cast<size_t>(newSize)) {
							value.resize(static_cast<size_t>(newSize));
						}
						std::copy(options.parserPtr->getStringBuffer().data(), options.parserPtr->getStringBuffer().data() + newSize, value.data());
					} else {
						static constexpr auto sourceLocation{ std::source_location::current() };
						options.parserPtr->getErrors().emplace_back(error::constructError<sourceLocation, error_classes::Parsing, parse_errors::Unexpected_String_End>(
							iter - options.rootIter, end - options.rootIter, options.rootIter));
						return false;
					}
				} else {
					static constexpr auto sourceLocation{ std::source_location::current() };
					options.parserPtr->getErrors().emplace_back(error::constructError<sourceLocation, error_classes::Parsing, parse_errors::Invalid_String_Characters>(
						iter - options.rootIter, end - options.rootIter, options.rootIter));
					return false;
				}
			} else {
				static constexpr auto sourceLocation{ std::source_location::current() };
				options.parserPtr->getErrors().emplace_back(error::constructError<sourceLocation, error_classes::Parsing, parse_errors::Missing_String_Start>(
					iter - options.rootIter, end - options.rootIter, options.rootIter));
				return false;
			}
			return true;
		}

		template<typename iterator> JSONIFIER_ALWAYS_INLINE static void skipString(iterator& iter, iterator& end) noexcept {
			++iter;
			auto newLength = static_cast<size_t>(end - iter);
			skipStringImpl(iter, newLength);
		}

		template<typename iterator> JSONIFIER_ALWAYS_INLINE static void skipKey(iterator& iter, iterator& end) noexcept {
			auto newLength = static_cast<size_t>(end - iter);
			iter		   = char_comparison<'"', unwrap_t<decltype(*iter)>>::memchar(iter, newLength);
		}

		template<typename iterator> JSONIFIER_INLINE static void skipObject(iterator& iter, iterator& end) noexcept {
			++iter;
			if (*iter == '}') {
				return;
			}
			while (true) {
				if (*iter != '"') {
					return;
				}
				skipString(iter, end);
				skipToNextValue(iter, end);
				if (*iter != ',') {
					break;
				}
				++iter;
			}
		}

		template<typename iterator> JSONIFIER_INLINE static void skipArray(iterator& iter, iterator& end) noexcept {
			++iter;
			if (*iter == ']') {
				return;
			}
			while (true) {
				skipToNextValue(iter, end);
				if (*iter != ',') {
					break;
				}
				++iter;
			}
		}

		template<typename iterator> JSONIFIER_INLINE static void skipToNextValue(iterator& iter, iterator& end) noexcept {
			JSONIFIER_SKIP_WS();
			switch (*iter) {
				case '{': {
					skipObject(iter, end);
					break;
				}
				case '[': {
					skipArray(iter, end);
					break;
				}
				case '"': {
					skipString(iter, end);
					if (*iter == ':') {
						++iter;
						skipToNextValue(iter, end);
					}
					break;
				}
				case ':': {
					++iter;
					skipToNextValue(iter, end);
					break;
				}
				case 'n': {
					iter += 4;
					break;
				}
				case 'f': {
					iter += 5;
					break;
				}
				case 't': {
					iter += 4;
					break;
				}
				case '0':
				case '1':
				case '2':
				case '3':
				case '4':
				case '5':
				case '6':
				case '7':
				case '8':
				case '9':
				case '-': {
					skipNumber(iter, end);
				}
				default: {
					break;
				}
			}
		}

		template<char startChar, char endChar, typename iterator> JSONIFIER_ALWAYS_INLINE static size_t countValueElements(iterator iter, iterator end) noexcept {
			auto newValue = *iter;
			if (newValue == ']' || newValue == '}') [[unlikely]] {
				return 0;
			}
			size_t currentCount{ 1 };
			while (iter != end) {
				switch (*iter) {
					[[unlikely]] case ',': {
						++currentCount;
						++iter;
						break;
					}
					[[unlikely]] case '{':
					[[unlikely]] case '[': {
						skipToNextValue(iter, end);
						break;
					}
					[[unlikely]] case endChar: { return currentCount; }
					[[likely]] case '"': {
						skipString(iter, end);
						break;
					}
					[[unlikely]] case '\\': {
						++iter;
						++iter;
						break;
					}
					[[unlikely]] case 't': {
						iter += 4;
						break;
					}
					[[unlikely]] case 'f': {
						iter += 5;
						break;
					}
					[[unlikely]] case 'n': {
						iter += 4;
						break;
					}
					[[unlikely]] case ':': {
						++iter;
						break;
					}
					[[unlikely]] case '0':
					[[unlikely]] case '1':
					[[unlikely]] case '2':
					[[unlikely]] case '3':
					[[unlikely]] case '4':
					[[unlikely]] case '5':
					[[unlikely]] case '6':
					[[unlikely]] case '7':
					[[unlikely]] case '8':
					[[unlikely]] case '9':
					[[unlikely]] case '-': {
						skipNumber(iter, end);
						break;
					}
					[[likely]] default: {
						++iter;
						break;
					}
				}
			}
			return currentCount;
		}

		template<typename iterator> JSONIFIER_ALWAYS_INLINE static iterator skipWs(iterator iter) noexcept {
			while (whitespaceTable[static_cast<uint8_t>(*iter)]) {
				++iter;
			}
			return iter;
		}

		template<typename iterator> JSONIFIER_ALWAYS_INLINE static void skipNumber(iterator& iter, iterator& end) noexcept {
			iter += *iter == '-';
			auto sig_start_it  = iter;
			auto frac_start_it = end;
			auto fracStart	   = [&]() -> bool {
				frac_start_it = iter;
				iter		  = std::find_if_not(iter, end, isNumberType);
				if (iter == frac_start_it) {
					return true;
				}
				if ((*iter | ('E' ^ 'e')) != 'e') {
					return true;
				}
				++iter;
				return false;
			};

			auto expStart = [&]() -> bool {
				iter += *iter == '+' || *iter == '-';
				auto exp_start_it = iter;
				iter			  = std::find_if_not(iter, end, isNumberType);
				if (iter == exp_start_it) {
					return true;
				}
				return false;
			};
			if (*iter == '0') {
				++iter;
				if (*iter != '.') {
					return;
				}
				++iter;
				if (fracStart()) {
					return;
				}
			}
			iter = std::find_if_not(iter, end, isNumberType);
			if (iter == sig_start_it) {
				return;
			}
			if ((*iter | ('E' ^ 'e')) == 'e') {
				++iter;
				if (expStart()) {
					return;
				}
			}
			if (*iter != '.')
				return;
			++iter;
		}
	};

	template<const auto& options, typename iterator> JSONIFIER_ALWAYS_INLINE size_t getKeyLength(iterator iter, iterator& end) noexcept {
		if (*iter == '"') [[likely]] {
			++iter;
		} else {
			static constexpr auto sourceLocation{ std::source_location::current() };
			options.parserPtr->getErrors().emplace_back(
				error::constructError<sourceLocation, error_classes::Parsing, parse_errors::Missing_String_Start>(iter - options.rootIter, end - iter, options.rootIter));
			return {};
		}
		auto start = iter;
		iter	   = char_comparison<'"', decltype(*iter)>::memchar(iter, static_cast<size_t>(end - iter));
		return size_t(iter - start);
	}

}// namespace jsonifier_internal