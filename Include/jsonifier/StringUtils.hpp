/*
	MIT License

	Copyright (c) 2024 RealTimeChris

	Permission is hereby granted, free of charge, to any person obtaining a copy of this
	software and associated documentation files (the "Software"), to deal in the Software
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

#include <jsonifier/Reflection.hpp>
#include <jsonifier/Allocator.hpp>
#include <jsonifier/HashMap.hpp>
#include <jsonifier/StrToD.hpp>
#include <jsonifier/Error.hpp>
#include <jsonifier/Simd.hpp>

namespace jsonifier {

	struct serialize_options {
		size_t indentSize{ 3 };
		char indentChar{ ' ' };
		bool prettify{ false };
		size_t indent{};
	};

	struct parse_options {
		bool validateJson{ false };
		bool partialRead{ false };
		bool knownOrder{ false };
		bool minified{ false };
	};

}

namespace jsonifier_internal {

#define JSONIFIER_SKIP_WS() \
	while ((context.iter < context.endIter) && whitespaceTable[static_cast<uint8_t>(*context.iter)]) { \
		++context.iter; \
	}

	template<typename iterator01, typename iterator02> JSONIFIER_ALWAYS_INLINE void skipMatchingWs(iterator01 wsStart, iterator02& context, uint64_t length) noexcept {
		if (length > 7) {
			uint64_t v[2];
			while (length > 8) {
				std::memcpy(v, wsStart, 8);
				std::memcpy(v + 1, context, 8);
				if JSONIFIER_LIKELY (v[0] == v[1]) {
					length -= 8;
					wsStart += 8;
					context += 8;
				} else {
					return;
				}
			}

			const auto shift = 8 - length;
			context -= shift;
			return;
		}
		{
			static constexpr uint64_t nBytes{ sizeof(uint32_t) };
			if (length >= nBytes) {
				uint32_t v[2];
				std::memcpy(v, wsStart, nBytes);
				std::memcpy(v + 1, context, nBytes);
				if JSONIFIER_LIKELY (v[0] == v[1]) {
					wsStart += nBytes;
					length -= nBytes;
					context += nBytes;
				} else {
					return;
				}
			}
		}
		{
			static constexpr uint64_t nBytes{ sizeof(uint16_t) };
			if (length >= nBytes) {
				uint16_t v[2];
				std::memcpy(v, wsStart, nBytes);
				std::memcpy(v + 1, context, nBytes);
				if JSONIFIER_LIKELY (v[0] == v[1]) {
					context += nBytes;
				} else {
					return;
				}
			}
		}
		if (length > 0) {
			++context;
		}
	}

#define JSONIFIER_SKIP_MATCHING_WS() \
	if constexpr (newLines) { \
		context.iter += wsSize; \
	} else { \
		if (wsSize) { \
			skipMatchingWs(wsStart, context.iter, wsSize); \
		} \
	} \
	JSONIFIER_SKIP_WS()

	JSONIFIER_ALWAYS_INLINE string_view_ptr getUnderlyingPtr(string_view_ptr* ptr) noexcept {
		return *ptr;
	}

	JSONIFIER_ALWAYS_INLINE string_view_ptr getUnderlyingPtr(string_view_ptr ptr) noexcept {
		return ptr;
	}

	constexpr array<uint32_t, 886> digitToVal32{ { 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu,
		0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu,
		0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu,
		0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0x0u, 0x1u,
		0x2u, 0x3u, 0x4u, 0x5u, 0x6u, 0x7u, 0x8u, 0x9u, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xAu, 0xBu, 0xCu, 0xDu, 0xEu,
		0xFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu,
		0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu,
		0xFFFFFFFFu, 0xAu, 0xBu, 0xCu, 0xDu, 0xEu, 0xFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu,
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
		0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0x0u, 0x10u, 0x20u, 0x30u, 0x40u, 0x50u, 0x60u, 0x70u, 0x80u, 0x90u, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu,
		0xFFFFFFFFu, 0xFFFFFFFFu, 0xA0u, 0xB0u, 0xC0u, 0xD0u, 0xE0u, 0xF0u, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu,
		0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu,
		0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xA0u, 0xB0u, 0xC0u, 0xD0u, 0xE0u, 0xF0u, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu,
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
		0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0x0u, 0x100u, 0x200u, 0x300u, 0x400u, 0x500u, 0x600u, 0x700u, 0x800u, 0x900u,
		0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xA00u, 0xB00u, 0xC00u, 0xD00u, 0xE00u, 0xF00u, 0xFFFFFFFFu, 0xFFFFFFFFu,
		0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu,
		0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xA00u, 0xB00u, 0xC00u,
		0xD00u, 0xE00u, 0xF00u, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu,
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
		0xFFFFFFFFu, 0x0u, 0x1000u, 0x2000u, 0x3000u, 0x4000u, 0x5000u, 0x6000u, 0x7000u, 0x8000u, 0x9000u, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu,
		0xFFFFFFFFu, 0xFFFFFFFFu, 0xA000u, 0xB000u, 0xC000u, 0xD000u, 0xE000u, 0xF000u, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu,
		0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu,
		0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xA000u, 0xB000u, 0xC000u, 0xD000u, 0xE000u, 0xF000u, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu,
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
		0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu } };

	// Taken from simdjson: https://github.com/simdjson/simdjson
	JSONIFIER_ALWAYS_INLINE uint32_t hexToU32NoCheck(string_view_ptr string1) noexcept {
		return digitToVal32[630ull + string1[0]] | digitToVal32[420ull + string1[1]] | digitToVal32[210ull + string1[2]] | digitToVal32[0ull + string1[3]];
	}

	// Taken from simdjson: https://github.com/simdjson/simdjson
	JSONIFIER_ALWAYS_INLINE size_t codePointToUtf8(uint32_t cp, string_buffer_ptr c) noexcept {
		if (cp <= 0x7F) {
			c[0] = static_cast<char>(cp);
			return 1;
		}
		if (cp <= 0x7FF) {
			c[0] = static_cast<char>((cp >> 6) + 192);
			c[1] = static_cast<char>((cp & 63) + 128);
			return 2;
		} else if (cp <= 0xFFFF) {
			c[0] = static_cast<char>((cp >> 12) + 224);
			c[1] = static_cast<char>(((cp >> 6) & 63) + 128);
			c[2] = static_cast<char>((cp & 63) + 128);
			return 3;
		} else if (cp <= 0x10FFFF) {
			c[0] = static_cast<char>((cp >> 18) + 240);
			c[1] = static_cast<char>(((cp >> 12) & 63) + 128);
			c[2] = static_cast<char>(((cp >> 6) & 63) + 128);
			c[3] = static_cast<char>((cp & 63) + 128);
			return 4;
		}
		return 0;
	}

	// Taken from simdjson: https://github.com/simdjson/simdjson
	template<typename basic_iterator01, typename basic_iterator02>
	JSONIFIER_ALWAYS_INLINE bool handleUnicodeCodePoint(basic_iterator01& srcPtr, basic_iterator02& dstPtr) noexcept {
		static constexpr uint32_t subCodePoint = 0xFffd;
		uint32_t codePoint					   = hexToU32NoCheck(srcPtr + 2);
		static constexpr uint8_t bs{ '\\' };
		static constexpr uint8_t u{ 'u' };
		srcPtr += 6;
		if (codePoint >= 0xD800 && codePoint < 0xDc00) {
			if (((srcPtr[0] << 8) | srcPtr[1]) != ((bs << 8) | u)) {
				codePoint = subCodePoint;
			} else {
				const uint32_t codePoint2 = hexToU32NoCheck(srcPtr + 2);

				const uint32_t low_bit = codePoint2 - 0xDc00;
				if (low_bit >> 10) {
					codePoint = subCodePoint;
				} else {
					codePoint = (((codePoint - 0xD800) << 10) | low_bit) + 0x10000;
					srcPtr += 6;
				}
			}
		} else if (codePoint >= 0xDc00 && codePoint <= 0xDfff) {
			codePoint = subCodePoint;
		}
		const size_t offset = codePointToUtf8(codePoint, dstPtr);
		dstPtr += offset;
		return offset > 0;
	}

	template<char threshold, typename simd_type> JSONIFIER_ALWAYS_INLINE bool hasByteLessThanValue(const simd_type values) noexcept {
		return simd_internal::opCmpLt(values, simd_internal::gatherValue<simd_type>(threshold)) != 0;
	}

	template<const auto n> JSONIFIER_ALWAYS_INLINE bool hasByteLessThanValue(string_view_ptr values) noexcept {
		uint64_t x;
		std::memcpy(&x, values, sizeof(uint64_t));
		static constexpr uint64_t factor  = ~uint64_t(0) / uint64_t(255);
		static constexpr uint64_t msbMask = uint64_t(128);
		return ((( x )-factor * n) & ~( x )&factor * msbMask) != 0;
	}

	template<typename simd_type, typename integer_type>
	JSONIFIER_ALWAYS_INLINE integer_type findParse(const simd_type& simdValue, const simd_type& simdValues01, const simd_type& simdValues02) noexcept {
		return simd_internal::postCmpTzcnt(static_cast<integer_type>(
			simd_internal::opBitMaskRaw(simd_internal::opOr(simd_internal::opCmpEqRaw(simdValues01, simdValue), simd_internal::opCmpEqRaw(simdValues02, simdValue)))));
	}

	template<jsonifier::concepts::unsigned_t simd_type, jsonifier::concepts::unsigned_t integer_type>
	JSONIFIER_ALWAYS_INLINE integer_type findParse(simd_type& simdValue) noexcept {
		static constexpr integer_type mask{ repeatByte<0b01111111, integer_type>() };
		static constexpr integer_type hiBits{ repeatByte<0b10000000, integer_type>() };
		static constexpr integer_type quoteBits{ repeatByte<'"', integer_type>() };
		static constexpr integer_type bsBits{ repeatByte<'\\', integer_type>() };
		const integer_type lo7	= simdValue & mask;
		const integer_type next = ~((((lo7 ^ quoteBits) + mask) & ((lo7 ^ bsBits) + mask)) | simdValue) & hiBits;
		return static_cast<integer_type>(simd_internal::tzcnt(next) >> 3u);
	}

	template<typename simd_type, typename integer_type> JSONIFIER_ALWAYS_INLINE integer_type findSerialize(const simd_type& simdValue, const simd_type& simdValues01,
		const simd_type& simdValues02, const simd_type& simdValues03) noexcept {
		return simd_internal::postCmpTzcnt(static_cast<integer_type>(simd_internal::opBitMaskRaw(
			simd_internal::opOr(simd_internal::opOr(simd_internal::opCmpEqRaw(simdValues01, simdValue), simd_internal::opCmpEqRaw(simdValues02, simdValue)),
				simd_internal::opCmpLtRaw(simdValue, simdValues03)))));
	}

	template<jsonifier::concepts::unsigned_t simd_type, jsonifier::concepts::unsigned_t integer_type>
	JSONIFIER_ALWAYS_INLINE integer_type findSerialize(simd_type& simdValue) noexcept {
		static constexpr integer_type mask{ repeatByte<0b01111111, integer_type>() };
		static constexpr integer_type less32Bits{ repeatByte<0b01100000, integer_type>() };
		static constexpr integer_type hiBits{ repeatByte<0b10000000, integer_type>() };
		static constexpr integer_type quoteBits{ repeatByte<'"', integer_type>() };
		static constexpr integer_type bsBits{ repeatByte<'\\', integer_type>() };
		const integer_type lo7	= simdValue & mask;
		const integer_type next = ~((((lo7 ^ quoteBits) + mask) & ((lo7 ^ bsBits) + mask) & ((simdValue & less32Bits) + mask)) | simdValue) & hiBits;
		return static_cast<integer_type>(simd_internal::tzcnt(next) >> 3u);
	}

	template<typename basic_iterator01> JSONIFIER_ALWAYS_INLINE static void skipStringImpl(basic_iterator01& string1, size_t lengthNew) noexcept {
		if (static_cast<int64_t>(lengthNew) > 0) {
			const auto endIter = string1 + lengthNew;
			while (string1 < endIter) {
				auto* newIter = char_comparison<'"', std::remove_cvref_t<decltype(*string1)>>::memchar(string1, lengthNew);
				if (newIter) {
					string1	  = newIter;
					lengthNew = static_cast<size_t>(endIter - string1);

					auto* prev = string1 - 1;
					while (*prev == '\\') {
						--prev;
					}
					if (static_cast<size_t>(string1 - prev) % 2) {
						break;
					}
					++string1;
				} else {
					break;
				}
			}
		}
	}

	constexpr array<char, 256> escapeMap{ [] {
		array<char, 256> returnValues{};
		returnValues['"']  = '\"';
		returnValues['\\'] = '\\';
		returnValues['b']  = '\b';
		returnValues['f']  = '\f';
		returnValues['n']  = '\n';
		returnValues['r']  = '\r';
		returnValues['t']  = '\t';
		return returnValues;
	}() };

	template<jsonifier::parse_options options, typename basic_iterator01, typename basic_iterator02> struct string_parser {
		JSONIFIER_ALWAYS_INLINE static basic_iterator02 shortImpl(basic_iterator01& string1, basic_iterator02 string2, size_t lengthNew) noexcept {
			using char_t01 = typename std::conditional_t<std::is_pointer_v<basic_iterator01>, std::remove_pointer_t<basic_iterator01>,
				typename std::iterator_traits<basic_iterator01>::value_type>;
			using char_t02 = typename std::conditional_t<std::is_pointer_v<basic_iterator02>, std::remove_pointer_t<basic_iterator02>,
				typename std::iterator_traits<basic_iterator02>::value_type>;
			std::remove_const_t<char_t01> escapeChar;
			while (lengthNew > 0) {
				*string2 = static_cast<char_t02>(*string1);
				if JSONIFIER_LIKELY (*string1 == '"' || *string1 == '\\') {
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
								return nullptr;
							}
							string2[0] = static_cast<char_t02>(escapeChar);
							lengthNew -= 2;
							string2 += 1;
							string1 += 2;
						}
					}
				} else if JSONIFIER_UNLIKELY (static_cast<uint8_t>(*string2) < 32) {
					return nullptr;
				} else {
					--lengthNew;
					++string2;
					++string1;
				}
			}
			return string2;
		}

		JSONIFIER_ALWAYS_INLINE static basic_iterator02 impl(basic_iterator01& string1, basic_iterator02 string2, size_t lengthNew) noexcept {
			using char_t01 = typename std::conditional_t<std::is_pointer_v<basic_iterator01>, std::remove_pointer_t<basic_iterator01>,
				typename std::iterator_traits<basic_iterator01>::value_type>;
			std::remove_const_t<char_t01> escapeChar;
#if JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_AVX512)
			{
				using integer_type					   = typename get_type_at_index<simd_internal::avx_integer_list, 3>::type::integer_type;
				using simd_type						   = typename get_type_at_index<simd_internal::avx_integer_list, 3>::type::type;
				static constexpr size_t bytesProcessed = get_type_at_index<simd_internal::avx_integer_list, 3>::type::bytesProcessed;
				static constexpr integer_type mask	   = get_type_at_index<simd_internal::avx_integer_list, 3>::type::mask;
				const simd_type simdValues00		   = simd_internal::gatherValue<simd_type>('\\');
				const simd_type simdValues01		   = simd_internal::gatherValue<simd_type>('"');
				simd_type simdValue;
				integer_type nextBackslashOrQuote;
				while (static_cast<int64_t>(lengthNew) >= static_cast<int64_t>(bytesProcessed)) {
					simdValue = simd_internal::gatherValuesU<simd_type>(string1);
					std::memcpy(string2, string1, bytesProcessed);
					nextBackslashOrQuote = findParse<simd_type, integer_type>(simdValue, simdValues00, simdValues01);
					if JSONIFIER_LIKELY (nextBackslashOrQuote < mask)) {
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
											return static_cast<basic_iterator02>(nullptr);
										}
										continue;
									}
									escapeChar = escapeMap[static_cast<uint8_t>(escapeChar)];
									if (escapeChar == 0u) {
										return static_cast<basic_iterator02>(nullptr);
									}
									string2[nextBackslashOrQuote] = static_cast<char_t01>(escapeChar);
									lengthNew -= nextBackslashOrQuote + 2ull;
									string2 += nextBackslashOrQuote + 1ull;
									string1 += nextBackslashOrQuote + 2ull;
								} else {
									lengthNew -= bytesProcessed;
									string2 += bytesProcessed;
									string1 += bytesProcessed;
								}
							}
						}
					else if JSONIFIER_UNLIKELY (hasByteLessThanValue<31>(simdValue))) {
							return static_cast<basic_iterator02>(nullptr);
						}
					else {
						lengthNew -= bytesProcessed;
						string2 += bytesProcessed;
						string1 += bytesProcessed;
					}
				}
			}
#endif
#if JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_AVX512) || JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_AVX2)
			{
				using integer_type					   = typename get_type_at_index<simd_internal::avx_integer_list, 2>::type::integer_type;
				using simd_type						   = typename get_type_at_index<simd_internal::avx_integer_list, 2>::type::type;
				static constexpr size_t bytesProcessed = get_type_at_index<simd_internal::avx_integer_list, 2>::type::bytesProcessed;
				static constexpr integer_type mask	   = get_type_at_index<simd_internal::avx_integer_list, 2>::type::mask;
				const simd_type simdValues00		   = simd_internal::gatherValue<simd_type>('\\');
				const simd_type simdValues01		   = simd_internal::gatherValue<simd_type>('"');
				simd_type simdValue;
				integer_type nextBackslashOrQuote;
				while (static_cast<int64_t>(lengthNew) >= static_cast<int64_t>(bytesProcessed)) {
					simdValue = simd_internal::gatherValuesU<simd_type>(string1);
					std::memcpy(string2, string1, bytesProcessed);
					nextBackslashOrQuote = findParse<simd_type, integer_type>(simdValue, simdValues00, simdValues01);
					if JSONIFIER_LIKELY (nextBackslashOrQuote < mask) {
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
										return static_cast<basic_iterator02>(nullptr);
									}
									continue;
								}
								escapeChar = escapeMap[static_cast<uint8_t>(escapeChar)];
								if (escapeChar == 0u) {
									return static_cast<basic_iterator02>(nullptr);
								}
								string2[nextBackslashOrQuote] = static_cast<char_t01>(escapeChar);
								lengthNew -= nextBackslashOrQuote + 2ull;
								string2 += nextBackslashOrQuote + 1ull;
								string1 += nextBackslashOrQuote + 2ull;
							} else {
								lengthNew -= bytesProcessed;
								string2 += bytesProcessed;
								string1 += bytesProcessed;
							}
						}
					} else if JSONIFIER_UNLIKELY (hasByteLessThanValue<31>(simdValue)) {
						return static_cast<basic_iterator02>(nullptr);
					} else {
						lengthNew -= bytesProcessed;
						string2 += bytesProcessed;
						string1 += bytesProcessed;
					}
				}
			}
#endif
#if JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_AVX512) || JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_AVX2) || JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_AVX) || \
	JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_NEON)
			{
				using integer_type					   = typename get_type_at_index<simd_internal::avx_integer_list, 1>::type::integer_type;
				using simd_type						   = typename get_type_at_index<simd_internal::avx_integer_list, 1>::type::type;
				static constexpr size_t bytesProcessed = get_type_at_index<simd_internal::avx_integer_list, 1>::type::bytesProcessed;
				static constexpr integer_type mask	   = get_type_at_index<simd_internal::avx_integer_list, 1>::type::mask;
				const simd_type simdValues00		   = simd_internal::gatherValue<simd_type>('\\');
				const simd_type simdValues01		   = simd_internal::gatherValue<simd_type>('"');
				simd_type simdValue;
				integer_type nextBackslashOrQuote;
				while (static_cast<int64_t>(lengthNew) >= static_cast<int64_t>(bytesProcessed)) {
					simdValue = simd_internal::gatherValuesU<simd_type>(string1);
					std::memcpy(string2, string1, bytesProcessed);
					nextBackslashOrQuote = findParse<simd_type, integer_type>(simdValue, simdValues00, simdValues01);
					if JSONIFIER_LIKELY (nextBackslashOrQuote < mask) {
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
										return static_cast<basic_iterator02>(nullptr);
									}
									continue;
								}
								escapeChar = escapeMap[static_cast<uint8_t>(escapeChar)];
								if (escapeChar == 0u) {
									return static_cast<basic_iterator02>(nullptr);
								}
								string2[nextBackslashOrQuote] = static_cast<char_t01>(escapeChar);
								lengthNew -= nextBackslashOrQuote + 2ull;
								string2 += nextBackslashOrQuote + 1ull;
								string1 += nextBackslashOrQuote + 2ull;
							} else {
								lengthNew -= bytesProcessed;
								string2 += bytesProcessed;
								string1 += bytesProcessed;
							}
						}
					} else if JSONIFIER_UNLIKELY (hasByteLessThanValue<31>(simdValue)) {
						return static_cast<basic_iterator02>(nullptr);
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
					std::memcpy(&simdValue, string1, bytesProcessed);
					std::memcpy(string2, string1, bytesProcessed);
					nextBackslashOrQuote = findParse<simd_type, integer_type>(simdValue);
					if JSONIFIER_LIKELY (nextBackslashOrQuote < mask) {
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
										return static_cast<basic_iterator02>(nullptr);
									}
									continue;
								}
								escapeChar = escapeMap[static_cast<uint8_t>(escapeChar)];
								if (escapeChar == 0u) {
									return static_cast<basic_iterator02>(nullptr);
								}
								string2[nextBackslashOrQuote] = static_cast<char_t01>(escapeChar);
								lengthNew -= nextBackslashOrQuote + 2ull;
								string2 += nextBackslashOrQuote + 1ull;
								string1 += nextBackslashOrQuote + 2ull;
							} else {
								lengthNew -= bytesProcessed;
								string2 += bytesProcessed;
								string1 += bytesProcessed;
							}
						}
					} else if JSONIFIER_UNLIKELY (hasByteLessThanValue<31>(string1)) {
						return static_cast<basic_iterator02>(nullptr);
					} else {
						lengthNew -= bytesProcessed;
						string2 += bytesProcessed;
						string1 += bytesProcessed;
					}
				}
			}
			return shortImpl(string1, string2, lengthNew);
		}
	};

	constexpr array<uint16_t, 256> escapeTable{ [] {
		array<uint16_t, 256> returnValues{};
		returnValues['\"'] = 0x225Cu;
		returnValues['\\'] = 0x5C5Cu;
		returnValues['\b'] = 0x625Cu;
		returnValues['\f'] = 0x665Cu;
		returnValues['\n'] = 0x6E5Cu;
		returnValues['\r'] = 0x725Cu;
		returnValues['\t'] = 0x745Cu;
		return returnValues;
	}() };

	template<jsonifier::serialize_options options, typename basic_iterator01, typename basic_iterator02> struct string_serializer {
		JSONIFIER_ALWAYS_INLINE static auto shortImpl(basic_iterator01 string1, basic_iterator02& string2, size_t lengthNew) noexcept {
			const auto* endIter = string1 + lengthNew;
			for (; string1 < endIter; ++string1) {
				auto escapeChar = escapeTable[static_cast<uint8_t>(*string1)];
				if JSONIFIER_LIKELY (escapeChar) {
					std::memcpy(string2, &escapeChar, 2);
					string2 += 2;
				} else {
					*string2 = *string1;
					++string2;
				}
			}
			return string2;
		}

		JSONIFIER_ALWAYS_INLINE static auto impl(basic_iterator01 string1, basic_iterator02 string2, size_t lengthNew) noexcept {
			uint16_t escapeChar;
#if JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_AVX512)
			{
				using integer_type					   = typename get_type_at_index<simd_internal::avx_integer_list, 3>::type::integer_type;
				using simd_type						   = typename get_type_at_index<simd_internal::avx_integer_list, 3>::type::type;
				static constexpr size_t bytesProcessed = get_type_at_index<simd_internal::avx_integer_list, 3>::type::bytesProcessed;
				static constexpr integer_type mask	   = get_type_at_index<simd_internal::avx_integer_list, 3>::type::mask;
				const simd_type simdValues01		   = simd_internal::gatherValue<simd_type>('"');
				const simd_type simdValues02		   = simd_internal::gatherValue<simd_type>('\\');
				const simd_type simdValues03		   = simd_internal::gatherValue<simd_type>(static_cast<char>(31));
				simd_type simdValue;
				integer_type nextEscapeable;
				while (static_cast<int64_t>(lengthNew) >= static_cast<int64_t>(bytesProcessed)) {
					simdValue = simd_internal::gatherValuesU<simd_type>(string1);
					std::memcpy(string2, string1, bytesProcessed);
					nextEscapeable = findSerialize<simd_type, integer_type>(simdValue, simdValues01, simdValues02, simdValues03);
					if JSONIFIER_LIKELY (nextEscapeable < mask) {
						escapeChar = escapeTable[static_cast<uint8_t>(string1[nextEscapeable])];
						if JSONIFIER_LIKELY (escapeChar != 0u) {
							lengthNew -= nextEscapeable;
							string2 += nextEscapeable;
							string1 += nextEscapeable;
							std::memcpy(string2, &escapeChar, 2);
							string2 += 2ull;
							--lengthNew;
							++string1;
						} else {
							string2 += nextEscapeable;
							return string2;
						}
					} else {
						lengthNew -= bytesProcessed;
						string2 += bytesProcessed;
						string1 += bytesProcessed;
					}
				}
			}
#endif
#if JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_AVX512) || JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_AVX2)
			{
				using integer_type					   = typename get_type_at_index<simd_internal::avx_integer_list, 2>::type::integer_type;
				using simd_type						   = typename get_type_at_index<simd_internal::avx_integer_list, 2>::type::type;
				static constexpr size_t bytesProcessed = get_type_at_index<simd_internal::avx_integer_list, 2>::type::bytesProcessed;
				static constexpr integer_type mask	   = get_type_at_index<simd_internal::avx_integer_list, 2>::type::mask;
				const simd_type simdValues01		   = simd_internal::gatherValue<simd_type>('"');
				const simd_type simdValues02		   = simd_internal::gatherValue<simd_type>('\\');
				const simd_type simdValues03		   = simd_internal::gatherValue<simd_type>(static_cast<char>(31));
				simd_type simdValue;
				integer_type nextEscapeable;
				while (static_cast<int64_t>(lengthNew) >= static_cast<int64_t>(bytesProcessed)) {
					simdValue = simd_internal::gatherValuesU<simd_type>(string1);
					std::memcpy(string2, string1, bytesProcessed);
					nextEscapeable = findSerialize<simd_type, integer_type>(simdValue, simdValues01, simdValues02, simdValues03);
					if JSONIFIER_LIKELY (nextEscapeable < mask) {
						escapeChar = escapeTable[static_cast<uint8_t>(string1[nextEscapeable])];
						if JSONIFIER_LIKELY (escapeChar != 0u) {
							lengthNew -= nextEscapeable;
							string2 += nextEscapeable;
							string1 += nextEscapeable;
							std::memcpy(string2, &escapeChar, 2);
							string2 += 2ull;
							--lengthNew;
							++string1;
						} else {
							string2 += nextEscapeable;
							return string2;
						}
					} else {
						lengthNew -= bytesProcessed;
						string2 += bytesProcessed;
						string1 += bytesProcessed;
					}
				}
			}
#endif
#if JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_AVX512) || JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_AVX2) || JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_AVX) || \
	JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_NEON)
			{
				using integer_type					   = typename get_type_at_index<simd_internal::avx_integer_list, 1>::type::integer_type;
				using simd_type						   = typename get_type_at_index<simd_internal::avx_integer_list, 1>::type::type;
				static constexpr size_t bytesProcessed = get_type_at_index<simd_internal::avx_integer_list, 1>::type::bytesProcessed;
				static constexpr integer_type mask	   = get_type_at_index<simd_internal::avx_integer_list, 1>::type::mask;
				const simd_type simdValues01		   = simd_internal::gatherValue<simd_type>('"');
				const simd_type simdValues02		   = simd_internal::gatherValue<simd_type>('\\');
				const simd_type simdValues03		   = simd_internal::gatherValue<simd_type>(static_cast<char>(31));
				simd_type simdValue;
				integer_type nextEscapeable;
				while (static_cast<int64_t>(lengthNew) >= static_cast<int64_t>(bytesProcessed)) {
					simdValue = simd_internal::gatherValuesU<simd_type>(string1);
					std::memcpy(string2, string1, bytesProcessed);
					nextEscapeable = findSerialize<simd_type, integer_type>(simdValue, simdValues01, simdValues02, simdValues03);
					if JSONIFIER_LIKELY (nextEscapeable < mask) {
						escapeChar = escapeTable[static_cast<uint8_t>(string1[nextEscapeable])];
						if JSONIFIER_LIKELY (escapeChar != 0u) {
							lengthNew -= nextEscapeable;
							string2 += nextEscapeable;
							string1 += nextEscapeable;
							std::memcpy(string2, &escapeChar, 2);
							string2 += 2ull;
							--lengthNew;
							++string1;
						} else {
							string2 += nextEscapeable;
							return string2;
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
					std::memcpy(string2, string1, bytesProcessed);
					std::memcpy(&simdValue, string1, bytesProcessed);
					nextEscapeable = findSerialize<simd_type, integer_type>(simdValue);
					if JSONIFIER_LIKELY (nextEscapeable < mask) {
						escapeChar = escapeTable[static_cast<uint8_t>(string1[nextEscapeable])];
						if JSONIFIER_LIKELY (escapeChar != 0u) {
							lengthNew -= nextEscapeable;
							string2 += nextEscapeable;
							string1 += nextEscapeable;
							std::memcpy(string2, &escapeChar, 2);
							string2 += 2ull;
							--lengthNew;
							++string1;
						} else {
							string2 += nextEscapeable;
							return string2;
						}
					} else {
						lengthNew -= bytesProcessed;
						string2 += bytesProcessed;
						string1 += bytesProcessed;
					}
				}
			}
			return shortImpl(string1, string2, lengthNew);
		}
	};

	template<size_t length> struct convert_length_to_int {
		static_assert(length <= 8, "Sorry, but that string is too int64_t!");
		using type = std::conditional_t<length == 1, uint8_t,
			std::conditional_t<length <= 2, uint16_t, std::conditional_t<length <= 4, uint32_t, std::conditional_t<length <= 8, size_t, void>>>>;
	};

	template<size_t length> using convert_length_to_int_t = typename convert_length_to_int<length>::type;

	template<string_literal string> constexpr convert_length_to_int_t<string.size()> getStringAsInt() noexcept {
		string_view_ptr stringNew = string.data();
		convert_length_to_int_t<string.size()> returnValue{};
		for (size_t x = 0; x < string.size(); ++x) {
			returnValue |= static_cast<convert_length_to_int_t<string.size()>>(stringNew[x]) << x * 8;
		}
		return returnValue;
	}

	template<string_literal stringNew> JSONIFIER_ALWAYS_INLINE bool compareStringAsInt(const char* src) {
		static constexpr auto string{ stringNew };
		static constexpr auto stringInt{ getStringAsInt<string>() };
		if constexpr (string.size() == 4) {
			uint32_t sourceVal;
			std::memcpy(&sourceVal, src, string.size());
			return sourceVal ^ stringInt;
		} else {
			uint64_t sourceVal{};
			std::memcpy(&sourceVal, src, string.size());
			return sourceVal ^ stringInt;
		}
	}

	template<typename parse_context_type, jsonifier::concepts::bool_t bool_type> JSONIFIER_ALWAYS_INLINE bool parseBool(bool_type& value, parse_context_type& context) noexcept {
		const auto notTrue	= !compareStringAsInt<"true">(context);
		const auto notFalse = (!compareStringAsInt<"fals">(context)) && (context[4] == 'e');
		if JSONIFIER_LIKELY ((notTrue || notFalse)) {
			value = notTrue;
			context += 4 + notFalse;
			return true;
		} else {
			return false;
		}
	}

	template<typename parse_context_type> JSONIFIER_ALWAYS_INLINE bool parseNull(parse_context_type& context) noexcept {
		if JSONIFIER_LIKELY (!compareStringAsInt<"null">(context)) {
			context += 4;
			return true;
		} else {
			return false;
		}
	}

	static constexpr array<bool, 256> numericTable = [] {
		array<bool, 256> returnValues{};
		returnValues['0'] = true;
		returnValues['1'] = true;
		returnValues['2'] = true;
		returnValues['3'] = true;
		returnValues['4'] = true;
		returnValues['5'] = true;
		returnValues['6'] = true;
		returnValues['7'] = true;
		returnValues['8'] = true;
		returnValues['9'] = true;
		returnValues['.'] = true;
		returnValues['+'] = true;
		returnValues['-'] = true;
		returnValues['e'] = true;
		returnValues['E'] = true;
		return returnValues;
	}();

	template<const auto options, typename parse_context_type> struct derailleur {
		template<typename value_type> JSONIFIER_ALWAYS_INLINE static bool parseString(value_type& value, parse_context_type& context) noexcept {
			if constexpr (options.partialRead) {
				if JSONIFIER_LIKELY ((context.iter < context.endIter) && **context.iter == '"') {
					auto newerPtr	  = (*context.iter) + 1;
					const auto newPtr = string_parser<options, decltype(newerPtr), decltype(stringBuffer.data())>::impl(newerPtr, stringBuffer.data(),
						static_cast<size_t>(*context.endIter - *context.iter));
					if JSONIFIER_LIKELY (newPtr) {
						const auto newSize = static_cast<size_t>(newPtr - stringBuffer.data());
						if constexpr (jsonifier::concepts::has_resize<value_type>) {
							if JSONIFIER_UNLIKELY (value.size() != newSize) {
								value.resize(newSize);
							}
						}
						std::memcpy(value.data(), stringBuffer.data(), newSize);
						++context.iter;
					}
					JSONIFIER_ELSE_UNLIKELY(else) {
						context.parserPtr->template reportError<parse_errors::Invalid_String_Characters>(context);
						return false;
					}
					return true;
				}
				JSONIFIER_ELSE_UNLIKELY(else) {
					context.parserPtr->template reportError<parse_errors::Missing_String_Start>(context);
					return false;
				}
			} else {
				if JSONIFIER_LIKELY ((context.iter < context.endIter) && *context.iter == '"') {
					++context.iter;
					const auto newPtr = string_parser<options, decltype(context.iter), decltype(stringBuffer.data())>::impl(context.iter, stringBuffer.data(),
						static_cast<size_t>(context.endIter - context.iter));
					if JSONIFIER_LIKELY (newPtr) {
						const auto newSize = static_cast<size_t>(newPtr - stringBuffer.data());
						if constexpr (jsonifier::concepts::has_resize<value_type>) {
							if JSONIFIER_UNLIKELY (value.size() != newSize) {
								value.resize(newSize);
							}
						}
						std::memcpy(value.data(), stringBuffer.data(), newSize);
						++context.iter;
					}
					JSONIFIER_ELSE_UNLIKELY(else) {
						context.parserPtr->template reportError<parse_errors::Invalid_String_Characters>(context);
						return false;
					}
					return true;
				}
				JSONIFIER_ELSE_UNLIKELY(else) {
					context.parserPtr->template reportError<parse_errors::Missing_String_Start>(context);
					return false;
				}
			}
		}

		JSONIFIER_ALWAYS_INLINE static void skipString(parse_context_type& context) noexcept {
			if constexpr (options.partialRead) {
				++context.iter;
			} else {
				++context.iter;
				const auto newLength = static_cast<size_t>(context.endIter - context.iter);
				skipStringImpl(context.iter, newLength);
			}
		}

		template<typename value_type> JSONIFIER_ALWAYS_INLINE static void skipKey(parse_context_type& context) noexcept {
			if constexpr (options.partialRead) {
				++context.iter;
			} else {
				static constexpr auto keyLength{ keyStatsVal<value_type>.minLength - 1 };
				context.iter += keyLength;
				skipString(context);
			}
		}

		template<typename value_type> JSONIFIER_ALWAYS_INLINE static void skipKeyStarted(parse_context_type& context) noexcept {
			if constexpr (options.partialRead) {
				++context.iter;
			} else {
				const auto newLength = static_cast<size_t>(context.endIter - context.iter);
				skipStringImpl(context.iter, newLength);
			}
		}

		template<typename value_type> JSONIFIER_INLINE static void skipObject(parse_context_type& context) noexcept {
			if constexpr (options.partialRead) {
				++context.iter;
				size_t currentDepth{ 1 };
				while (context.iter != context.endIter && currentDepth > 0) {
					switch (**context.iter) {
						[[unlikely]] case '{': {
							++currentDepth;
							++context.iter;
							break;
						}
						[[unlikely]] case '}': {
							--currentDepth;
							if (currentDepth == 0) {
								++context.iter;
								return;
							}
							++context.iter;
							break;
						}
						[[likely]] default: {
							++context.iter;
							break;
						}
					}
				}
			} else {
				++context.iter;
				if constexpr (!options.minified) {
					JSONIFIER_SKIP_WS()
				}
				if JSONIFIER_LIKELY ((context.iter < context.endIter) && *context.iter == '}') {
					--context.currentObjectDepth;
					++context.iter;
					if constexpr (!options.minified) {
						JSONIFIER_SKIP_WS()
					}
					return;
				}
				while (true) {
					if JSONIFIER_LIKELY ((context.iter < context.endIter) && *context.iter == '"') {
						skipString(context);
						++context.iter;
					} else {
						return;
					}
					if constexpr (!options.minified) {
						JSONIFIER_SKIP_WS()
					}
					if JSONIFIER_LIKELY ((context.iter < context.endIter) && *context.iter == ':') {
						++context.iter;
					} else {
						return;
					}
					if constexpr (!options.minified) {
						JSONIFIER_SKIP_WS()
					}
					skipToNextValue<value_type>(context);
					if JSONIFIER_LIKELY ((context.iter < context.endIter) && *context.iter == ',') {
						++context.iter;
					} else {
						break;
					}
					if constexpr (!options.minified) {
						JSONIFIER_SKIP_WS()
					}
				}
				if JSONIFIER_LIKELY ((context.iter < context.endIter) && *context.iter == '}') {
					--context.currentObjectDepth;
					++context.iter;
					if constexpr (!options.minified) {
						JSONIFIER_SKIP_WS()
					}
					return;
				}
			}
		}

		template<char start, char end> JSONIFIER_ALWAYS_INLINE static const char* getNextOpenOrClose(parse_context_type& context, size_t length) {
			const char* nextOpen  = char_comparison<start, char>::memchar(context.iter, length);
			const char* nextClose = char_comparison<end, char>::memchar(context.iter, length);
			return (nextClose && (nextClose < nextOpen || !nextOpen)) ? nextClose : nextOpen;
		}

		template<char valueStart, char valueEnd> JSONIFIER_ALWAYS_INLINE static void skipToEndOfValue(parse_context_type& context) {
			if constexpr (options.partialRead) {
				size_t depth{ 1 };
				while (depth > 0 && context.iter < context.endIter) {
					switch (**context.iter) {
						case '{': {
							++depth;
							break;
						}
						case '}': {
							--depth;
							break;
						}
						default: {
							break;
						}
					}
					++context.iter;
				}
			} else {
				size_t depth		   = 1;
				size_t remainingLength = static_cast<size_t>(context.endIter - context.iter);

				const char* nextQuote		= char_comparison<'"', char>::memchar(context.iter, remainingLength);
				const char* nextOpenOrClose = getNextOpenOrClose<valueStart, valueEnd>(context, remainingLength);

				while (nextOpenOrClose && depth > 0 && context.iter + bytesPerStep < context.endIter) {
					if (nextQuote && (nextQuote < nextOpenOrClose)) {
						skipString(context);
						++context.iter;
						remainingLength = static_cast<size_t>(context.endIter - context.iter);
						nextQuote		= static_cast<const char*>(std::memchr(context.iter, '"', remainingLength));
					} else {
						if (*nextOpenOrClose == valueEnd) {
							--depth;
						} else if (*nextOpenOrClose == valueStart) {
							++depth;
						}
						context.iter = nextOpenOrClose;
						if (depth == 0) {
							return;
						}
						++context.iter;
						remainingLength = static_cast<size_t>(context.endIter - context.iter);
						nextOpenOrClose = getNextOpenOrClose<valueStart, valueEnd>(context, remainingLength);
					}
				}
			}
		}

		template<typename value_type> JSONIFIER_INLINE static void skipArray(parse_context_type& context) noexcept {
			if constexpr (options.partialRead) {
				++context.iter;
				size_t currentDepth{ 1 };
				while (context.iter != context.endIter && currentDepth > 0) {
					switch (**context.iter) {
						[[unlikely]] case '[': {
							++currentDepth;
							break;
						}
						[[unlikely]] case ']': {
							--currentDepth;
							break;
						}
						[[likely]] default: { break; }
					}
					++context.iter;
				}
			} else {
				++context.iter;
				if constexpr (!options.minified) {
					JSONIFIER_SKIP_WS()
				}
				if JSONIFIER_LIKELY ((context.iter < context.endIter) && *context.iter == ']') {
					--context.currentArrayDepth;
					++context.iter;
					if constexpr (!options.minified) {
						JSONIFIER_SKIP_WS()
					}
					return;
				}
				while (true) {
					skipToNextValue<value_type>(context);
					if JSONIFIER_LIKELY ((context.iter < context.endIter) && *context.iter == ',') {
						++context.iter;
					} else {
						break;
					}
					if constexpr (!options.minified) {
						JSONIFIER_SKIP_WS()
					}
				}
				if JSONIFIER_LIKELY ((context.iter < context.endIter) && *context.iter == ']') {
					--context.currentArrayDepth;
					++context.iter;
					if constexpr (!options.minified) {
						JSONIFIER_SKIP_WS()
					}
					return;
				}
			}
		}

		template<typename value_type> JSONIFIER_INLINE static void skipToNextValue(parse_context_type& context) noexcept {
			if constexpr (options.partialRead) {
				switch (**context.iter) {
					case '{': {
						skipObject<value_type>(context);
						break;
					}
					case '[': {
						skipArray<value_type>(context);
						break;
					}
					case '\0': {
						break;
					}
					default: {
						++context.iter;
					}
				}
			} else {
				if constexpr (!options.minified) {
					JSONIFIER_SKIP_WS()
				}
				if JSONIFIER_LIKELY ((context.iter + 1) < context.endIter) {
					switch (*context.iter) {
						case '{': {
							++context.currentObjectDepth;
							skipObject<value_type>(context);
							break;
						}
						case '[': {
							++context.currentArrayDepth;
							skipArray<value_type>(context);
							break;
						}
						case ',': {
							++context.iter;
							if constexpr (!options.minified) {
								JSONIFIER_SKIP_WS()
							}
							skipToNextValue<value_type>(context);
							break;
						}
						case '"': {
							skipString(context);
							if ((context.iter < context.endIter) && *context.iter == '"') {
								++context.iter;
							} else {
								context.currentObjectDepth++;
								return;
							}
							if constexpr (!options.minified) {
								JSONIFIER_SKIP_WS()
							}
							break;
						}
						case 'n': {
							context.iter += 4;
							if constexpr (!options.minified) {
								JSONIFIER_SKIP_WS()
							}
							break;
						}
						case 'f': {
							context.iter += 5;
							if constexpr (!options.minified) {
								JSONIFIER_SKIP_WS()
							}
							break;
						}
						case 't': {
							context.iter += 4;
							if constexpr (!options.minified) {
								JSONIFIER_SKIP_WS()
							}
							break;
						}
						case '0':
							[[fallthrough]];
						case '1':
							[[fallthrough]];
						case '2':
							[[fallthrough]];
						case '3':
							[[fallthrough]];
						case '4':
							[[fallthrough]];
						case '5':
							[[fallthrough]];
						case '6':
							[[fallthrough]];
						case '7':
							[[fallthrough]];
						case '8':
							[[fallthrough]];
						case '9':
							[[fallthrough]];
						case '-': {
							skipNumber(context);
							if constexpr (!options.minified) {
								JSONIFIER_SKIP_WS()
							}
							break;
						}
						default: {
							break;
						}
					}
				}
			}
		}

		template<typename iterator> JSONIFIER_ALWAYS_INLINE static void skipWs(iterator& context) noexcept {
			while (whitespaceTable[uint8_t(*context)]) {
				++context;
			}
		}

		JSONIFIER_ALWAYS_INLINE static void skipNumber(parse_context_type& context) noexcept {
			while (numericTable[uint8_t(*context.iter)]) {
				++context.iter;
			}
		}
	};

	template<const auto options, typename parse_context_type> JSONIFIER_ALWAYS_INLINE size_t getKeyLength(parse_context_type context) noexcept {
		if JSONIFIER_LIKELY ((context.iter < context.endIter) && *context.iter == '"') {
			++context.iter;
			const auto start = context.iter;
			context.iter	 = char_comparison<'"', std::remove_cvref_t<decltype(*context.iter)>>::memchar(context.iter, static_cast<size_t>(context.endIter - context.iter));
			return static_cast<size_t>(context.iter - start);
		} else {
			context.parserPtr->template reportError<parse_errors::Missing_String_Start>(context);
			return {};
		}
	}

}// namespace jsonifier_internal