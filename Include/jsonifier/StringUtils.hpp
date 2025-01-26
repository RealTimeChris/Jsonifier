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

#include <jsonifier/Allocator.hpp>
#include <jsonifier/HashMap.hpp>
#include <jsonifier/StrToD.hpp>
#include <jsonifier/Error.hpp>
#include <jsonifier/Simd.hpp>

namespace jsonifier {

	struct serialize_options {
		uint64_t indentSize{ 3 };
		char indentChar{ ' ' };
		uint64_t indent{};
		bool prettify{};
	};

	struct parse_options {
		bool validateJson{};
		bool partialRead{};
		bool knownOrder{};
		bool minified{};
	};

}

namespace jsonifier::internal {

#define JSONIFIER_SKIP_WS() \
	while ((context.iter < context.endIter) && whitespaceTable[static_cast<uint8_t>(*context.iter)]) { \
		++context.iter; \
	}

	template<typename iterator01, typename iterator02> JSONIFIER_INLINE void skipMatchingWs(iterator01 wsStart, iterator02& context, uint64_t length) noexcept {
		if (length > 7) {
			uint64_t v[2];
			while (length >= 8) {
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

	JSONIFIER_INLINE string_view_ptr getUnderlyingPtr(string_view_ptr* ptr) noexcept {
		return *ptr;
	}

	JSONIFIER_INLINE string_view_ptr getUnderlyingPtr(string_view_ptr ptr) noexcept {
		return ptr;
	}

	template<typename = void> struct digit_tables {
		static constexpr uint32_t digitToVal32[]{ 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu,
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
			0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0x0u, 0x10u, 0x20u, 0x30u, 0x40u, 0x50u, 0x60u, 0x70u, 0x80u, 0x90u, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu,
			0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xA0u, 0xB0u, 0xC0u, 0xD0u, 0xE0u, 0xF0u, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu,
			0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu,
			0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xA0u, 0xB0u, 0xC0u, 0xD0u, 0xE0u, 0xF0u, 0xFFFFFFFFu, 0xFFFFFFFFu,
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
			0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0x0u, 0x100u, 0x200u, 0x300u, 0x400u,
			0x500u, 0x600u, 0x700u, 0x800u, 0x900u, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xA00u, 0xB00u, 0xC00u, 0xD00u,
			0xE00u, 0xF00u, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu,
			0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu,
			0xFFFFFFFFu, 0xFFFFFFFFu, 0xA00u, 0xB00u, 0xC00u, 0xD00u, 0xE00u, 0xF00u, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu,
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
			0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0x0u, 0x1000u, 0x2000u, 0x3000u, 0x4000u, 0x5000u, 0x6000u, 0x7000u, 0x8000u, 0x9000u, 0xFFFFFFFFu,
			0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xA000u, 0xB000u, 0xC000u, 0xD000u, 0xE000u, 0xF000u, 0xFFFFFFFFu, 0xFFFFFFFFu,
			0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu,
			0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xA000u, 0xB000u,
			0xC000u, 0xD000u, 0xE000u, 0xF000u, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu,
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
			0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu,
			0xFFFFFFFFu };
	};

	/// Sampled from Simdjson library: https://github.com/simdjson/simdjson
	JSONIFIER_INLINE uint32_t hexToU32NoCheck(string_view_ptr string1) noexcept {
		return digit_tables<>::digitToVal32[630ull + string1[0]] | digit_tables<>::digitToVal32[420ull + string1[1]] | digit_tables<>::digitToVal32[210ull + string1[2]] |
			digit_tables<>::digitToVal32[0ull + string1[3]];
	}

	/// Sampled from Simdjson library: https://github.com/simdjson/simdjson
	JSONIFIER_INLINE uint64_t codePointToUtf8(uint32_t cp, string_buffer_ptr c) noexcept {
		if (cp <= 0x7F) {
			c[0] = static_cast<char>(cp);
			return 1;
		}
		if (cp <= 0x7FF) {
			c[0] = static_cast<char>(0xC0 | ((cp >> 6) & 0x1F));
			c[1] = static_cast<char>(0x80 | (cp & 0x3F));
			return 2;
		}
		if (cp <= 0xFFFF) {
			c[0] = static_cast<char>(0xE0 | ((cp >> 12) & 0x0F));
			c[1] = static_cast<char>(0x80 | ((cp >> 6) & 0x3F));
			c[2] = static_cast<char>(0x80 | (cp & 0x3F));
			return 3;
		}
		if (cp <= 0x10FFFF) {
			c[0] = static_cast<char>(0xF0 | ((cp >> 18) & 0x07));
			c[1] = static_cast<char>(0x80 | ((cp >> 12) & 0x3F));
			c[2] = static_cast<char>(0x80 | ((cp >> 6) & 0x3F));
			c[3] = static_cast<char>(0x80 | (cp & 0x3F));
			return 4;
		}
		return 0;
	}

	/// Sampled from Simdjson library: https://github.com/simdjson/simdjson
	template<typename basic_iterator01, typename basic_iterator02> JSONIFIER_INLINE bool handleUnicodeCodePoint(basic_iterator01& srcPtr, basic_iterator02& dstPtr) noexcept {
		static constexpr uint8_t bs{ '\\' };
		static constexpr uint8_t u{ 'u' };
		static constexpr uint32_t subCodePoint = 0xFFFD;
		uint32_t codePoint					   = hexToU32NoCheck(srcPtr + 2);
		srcPtr += 6;
		if (codePoint >= 0xD800 && codePoint < 0xDc00) {
			if (((srcPtr[0] << 8) | srcPtr[1]) != ((bs << 8) | u)) {
				codePoint = subCodePoint;
			} else {
				const uint32_t codePoint2 = hexToU32NoCheck(srcPtr + 2);

				const uint32_t lowBit = codePoint2 - 0xDc00;
				if (lowBit >> 10) {
					codePoint = subCodePoint;
				} else {
					codePoint = (((codePoint - 0xD800) << 10) | lowBit) + 0x10000;
					srcPtr += 6;
				}
			}
		} else if (codePoint >= 0xDc00 && codePoint <= 0xDfff) {
			codePoint = subCodePoint;
		}
		const uint64_t offset = codePointToUtf8(codePoint, dstPtr);
		dstPtr += offset;
		return offset > 0;
	}

	template<char threshold, typename simd_type> JSONIFIER_INLINE bool hasByteLessThanValue(const simd_type values) noexcept {
		return simd::opCmpLt(values, simd::gatherValue<simd_type>(threshold)) != 0;
	}

	template<const auto lengthNew> JSONIFIER_INLINE bool hasByteLessThanValue(string_view_ptr values) noexcept {
		uint64_t x;
		std::memcpy(&x, values, sizeof(uint64_t));
		static constexpr uint64_t factor  = ~uint64_t(0) / uint64_t(255);
		static constexpr uint64_t msbMask = uint64_t(128);
		return ((x - factor * lengthNew) & ~x & factor * msbMask) != 0;
	}

	template<typename simd_type, typename integer_type>
	JSONIFIER_INLINE integer_type findSerialize(const simd_type& simdValue, const simd_type& simdValues01, const simd_type& simdValues02, const simd_type& simdValues03) noexcept {
		return simd::postCmpTzcnt(static_cast<integer_type>(simd::opBitMaskRaw(
			simd::opOr(simd::opOr(simd::opCmpEqRaw(simdValues01, simdValue), simd::opCmpEqRaw(simdValues02, simdValue)), simd::opCmpLtRaw(simdValue, simdValues03)))));
	}

	template<concepts::unsigned_t simd_type, concepts::unsigned_t integer_type> JSONIFIER_INLINE integer_type findSerialize(simd_type& simdValue) noexcept {
		static constexpr integer_type mask{ repeatByte<0b01111111, integer_type>() };
		static constexpr integer_type less32Bits{ repeatByte<0b01100000, integer_type>() };
		static constexpr integer_type hiBits{ repeatByte<0b10000000, integer_type>() };
		static constexpr integer_type quoteBits{ repeatByte<'"', integer_type>() };
		static constexpr integer_type bsBits{ repeatByte<'\\', integer_type>() };
		const integer_type lo7	= simdValue & mask;
		const integer_type next = ~((((lo7 ^ quoteBits) + mask) & ((lo7 ^ bsBits) + mask) & ((simdValue & less32Bits) + mask)) | simdValue) & hiBits;
		return static_cast<integer_type>(simd::tzcnt(next) >> 3u);
	}

	/// Sampled from Stephen Berry and his library, Glaze library: https://github.com/StephenBerry/Glaze
	template<typename basic_iterator01> JSONIFIER_INLINE static void skipStringImpl(basic_iterator01& string1, uint64_t lengthNew) noexcept {
		if (static_cast<int64_t>(lengthNew) > 0) {
			const auto endIter = string1 + lengthNew;
			while (string1 < endIter) {
				auto* newIter = char_comparison<'"', jsonifier::internal::remove_cvref_t<decltype(*string1)>>::memchar(string1, lengthNew);
				if (newIter) {
					string1	  = newIter;
					lengthNew = static_cast<uint64_t>(endIter - string1);

					auto* prev = string1 - 1;
					while (*prev == '\\') {
						--prev;
					}
					if (static_cast<uint64_t>(string1 - prev) % 2) {
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

	template<size_t index, parse_options options, typename basic_iterator01, typename basic_iterator02> struct string_parser_impl {
		JSONIFIER_INLINE static void impl(basic_iterator01& string1, basic_iterator01 iter, basic_iterator02 string2, uint64_t& lengthNew) noexcept {
			using integer_type						 = typename get_type_at_index<simd::avx_integer_list, index>::type::integer_type;
			using simd_type							 = typename get_type_at_index<simd::avx_integer_list, index>::type::type;
			static constexpr uint64_t bytesProcessed = get_type_at_index<simd::avx_integer_list, index>::type::bytesProcessed;
			static constexpr integer_type mask		 = get_type_at_index<simd::avx_integer_list, index>::type::mask;
			const simd_type simdValues00			 = simd::gatherValue<simd_type>('\\');
			const simd_type simdValues01			 = simd::gatherValue<simd_type>(static_cast<char>(32));
			simd_type simdValue;
			integer_type nextBackslashOrQuote;
			while (string1 + bytesProcessed < iter) {
				std::memcpy(string2, string1, bytesProcessed);
				simdValue			 = simd::gatherValuesU<simd_type>(string2);
				nextBackslashOrQuote = simd::opBitMaskRaw(simd::opOr(simd::opCmpEqRaw(simdValue, simdValues00), simd::opCmpLtRaw(simdValue, simdValues01)));

				if (nextBackslashOrQuote == 0) {
					string1 += bytesProcessed;
					string2 += bytesProcessed;
					continue;
				}

				nextBackslashOrQuote = simd::postCmpTzcnt(nextBackslashOrQuote);
				string1 += nextBackslashOrQuote;
				if (string1 >= iter) {
					break;
				}

				if ((*string1 & 0b11100000) == 0) [[unlikely]] {
					return;
				}
				++string1;
				if (*string1 == 'u') {
					++string1;
					string2 += nextBackslashOrQuote;
					const auto mark	  = string1;
					const auto offset = handleUnicodeCodePoint(string1, string2);
					if (offset == 0) [[unlikely]] {
						return;
					}
					lengthNew += offset;
					lengthNew -= 2 + uint32_t(string1 - mark);
				} else {
					string2 += nextBackslashOrQuote;
					*string2 = escapeMap[uint8_t(*string1)];
					if (*string2 == 0) [[unlikely]] {
						return;
					}
					++string2;
					++string1;
					--lengthNew;
				}
			}
			if constexpr (index > 0) {
				return string_parser_impl<index - 1, options, basic_iterator01, basic_iterator02>::impl(string1, iter, string2, lengthNew);
			}
		}
	};

	template<parse_options options, typename basic_iterator01, typename basic_iterator02> struct string_parser;

	template<parse_options options, typename basic_iterator01, typename basic_iterator02> struct string_parser_impl<0, options, basic_iterator01, basic_iterator02> {
		JSONIFIER_INLINE static void impl(basic_iterator01& string1, basic_iterator01 iter, basic_iterator02 string2, uint64_t& lengthNew) noexcept {
			using integer_type						 = typename get_type_at_index<simd::avx_integer_list, 0>::type::integer_type;
			using simd_type							 = typename get_type_at_index<simd::avx_integer_list, 0>::type::type;
			static constexpr uint64_t bytesProcessed = get_type_at_index<simd::avx_integer_list, 0>::type::bytesProcessed;
			static constexpr integer_type mask		 = get_type_at_index<simd::avx_integer_list, 0>::type::mask;
			simd_type simdValue;
			integer_type nextBackslashOrQuote;
			while (string1 + 8 < iter) {
				std::memcpy(string2, string1, 8);
				std::memcpy(&simdValue, string2, 8);

				constexpr uint64_t lo7_mask = repeatByte<0b01111111, uint64_t>();
				const uint64_t lo7			= simdValue & lo7_mask;
				const uint64_t backslash	= (lo7 ^ repeatByte<'\\', uint64_t>()) + lo7_mask;
				const uint64_t less_32		= (simdValue & repeatByte<0b01100000, uint64_t>()) + lo7_mask;
				nextBackslashOrQuote		= ~((backslash & less_32) | simdValue);

				nextBackslashOrQuote &= repeatByte<0b10000000, uint64_t>();
				if (nextBackslashOrQuote == 0) {
					string1 += 8;
					string2 += 8;
					continue;
				}

				nextBackslashOrQuote = simd::tzcnt(nextBackslashOrQuote) >> 3;
				string1 += nextBackslashOrQuote;
				if (string1 >= iter) {
					break;
				}

				if ((*string1 & 0b11100000) == 0) [[unlikely]] {
					return;
				}
				++string1;
				if (*string1 == 'u') {
					++string1;
					string2 += nextBackslashOrQuote;
					const auto mark	  = string1;
					const auto offset = handleUnicodeCodePoint(string1, string2);
					if (offset == 0) [[unlikely]] {
						return;
					}
					lengthNew += offset;
					lengthNew -= 2 + uint32_t(string1 - mark);
				} else {
					string2 += nextBackslashOrQuote;
					*string2 = escapeMap[uint8_t(*string1)];
					if (*string2 == 0) [[unlikely]] {
						return;
					}
					++string2;
					++string1;
					--lengthNew;
				}
			}
			return string_parser<options, basic_iterator01, basic_iterator02>::shortImpl(string1, iter, string2, lengthNew);
		}
	};

	template<parse_options options, typename basic_iterator01, typename basic_iterator02> struct string_parser {
		JSONIFIER_INLINE static void shortImpl(basic_iterator01& string1, basic_iterator01 iter, basic_iterator02 string2, uint64_t& lengthNew) noexcept {
			while (string1 < iter) {
				uint8_t currentByte = *string1;

				if (currentByte == '\\' || (currentByte < 0x20)) {
					if ((currentByte & 0b11100000) == 0) [[unlikely]] {
						return;
					}
					++string1;
					if (currentByte == 'u') {
						++string1;
						const auto mark	  = string1;
						const auto offset = handleUnicodeCodePoint(string1, string2);
						if (offset == 0) [[unlikely]] {
							return;
						}
						lengthNew += offset;
						lengthNew -= 2 + uint32_t(string1 - mark);
					} else {
						*string2 = escapeMap[currentByte];
						if (*string2 == 0) [[unlikely]] {
							return;
						}
						++string2;
						--lengthNew;
					}
				} else {
					*string2 = currentByte;
					++string2;
				}
				++string1;
			}
			return;
		}

		JSONIFIER_INLINE static void impl(basic_iterator01& string1, basic_iterator01 iter, basic_iterator02 string2, uint64_t& lengthNew) noexcept {
#if JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_AVX512)
			return string_parser_impl<3, options, basic_iterator01, basic_iterator02>::impl(string1, iter, string2, lengthNew);
#elif JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_AVX2)
			return string_parser_impl<2, options, basic_iterator01, basic_iterator02>::impl(string1, iter, string2, lengthNew);
#elif JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_AVX)
			return string_parser_impl<1, options, basic_iterator01, basic_iterator02>::impl(string1, iter, string2, lengthNew);
#else
			return string_parser_impl<0, options, basic_iterator01, basic_iterator02>::impl(string1, iter, string2, lengthNew);
#endif
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

	template<serialize_options options, typename basic_iterator01, typename basic_iterator02> struct string_serializer {
		JSONIFIER_INLINE static auto shortImpl(basic_iterator01 string1, basic_iterator02& string2, uint64_t lengthNew) noexcept {
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

		JSONIFIER_INLINE static auto impl(basic_iterator01 string1, basic_iterator02 string2, uint64_t lengthNew) noexcept {
			uint16_t escapeChar;
#if JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_AVX512)
			{
				using integer_type						 = typename get_type_at_index<simd::avx_integer_list, 3>::type::integer_type;
				using simd_type							 = typename get_type_at_index<simd::avx_integer_list, 3>::type::type;
				static constexpr uint64_t bytesProcessed = get_type_at_index<simd::avx_integer_list, 3>::type::bytesProcessed;
				static constexpr integer_type mask		 = get_type_at_index<simd::avx_integer_list, 3>::type::mask;
				const simd_type simdValues01			 = simd::gatherValue<simd_type>('"');
				const simd_type simdValues02			 = simd::gatherValue<simd_type>('\\');
				const simd_type simdValues03			 = simd::gatherValue<simd_type>(static_cast<char>(31));
				simd_type simdValue;
				integer_type nextEscapeable;
				while (static_cast<int64_t>(lengthNew) >= static_cast<int64_t>(bytesProcessed)) {
					simdValue = simd::gatherValuesU<simd_type>(string1);
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
				using integer_type						 = typename get_type_at_index<simd::avx_integer_list, 2>::type::integer_type;
				using simd_type							 = typename get_type_at_index<simd::avx_integer_list, 2>::type::type;
				static constexpr uint64_t bytesProcessed = get_type_at_index<simd::avx_integer_list, 2>::type::bytesProcessed;
				static constexpr integer_type mask		 = get_type_at_index<simd::avx_integer_list, 2>::type::mask;
				const simd_type simdValues01			 = simd::gatherValue<simd_type>('"');
				const simd_type simdValues02			 = simd::gatherValue<simd_type>('\\');
				const simd_type simdValues03			 = simd::gatherValue<simd_type>(static_cast<char>(31));
				simd_type simdValue;
				integer_type nextEscapeable;
				while (static_cast<int64_t>(lengthNew) >= static_cast<int64_t>(bytesProcessed)) {
					simdValue = simd::gatherValuesU<simd_type>(string1);
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
#if JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_AVX512) || JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_AVX2) || JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_AVX)
			{
				using integer_type						 = typename get_type_at_index<simd::avx_integer_list, 1>::type::integer_type;
				using simd_type							 = typename get_type_at_index<simd::avx_integer_list, 1>::type::type;
				static constexpr uint64_t bytesProcessed = get_type_at_index<simd::avx_integer_list, 1>::type::bytesProcessed;
				static constexpr integer_type mask		 = get_type_at_index<simd::avx_integer_list, 1>::type::mask;
				const simd_type simdValues01			 = simd::gatherValue<simd_type>('"');
				const simd_type simdValues02			 = simd::gatherValue<simd_type>('\\');
				const simd_type simdValues03			 = simd::gatherValue<simd_type>(static_cast<char>(31));
				simd_type simdValue;
				integer_type nextEscapeable;
				while (static_cast<int64_t>(lengthNew) >= static_cast<int64_t>(bytesProcessed)) {
					simdValue = simd::gatherValuesU<simd_type>(string1);
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
				using integer_type						 = typename get_type_at_index<simd::avx_integer_list, 0>::type::integer_type;
				using simd_type							 = typename get_type_at_index<simd::avx_integer_list, 0>::type::type;
				static constexpr uint64_t bytesProcessed = get_type_at_index<simd::avx_integer_list, 0>::type::bytesProcessed;
				static constexpr integer_type mask		 = get_type_at_index<simd::avx_integer_list, 0>::type::mask;
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

	template<uint64_t length> struct convert_length_to_int {
		static_assert(length <= 8, "Sorry, but that string is too int64_t!");
		using type = jsonifier::internal::conditional_t<length == 1, uint8_t,
			jsonifier::internal::conditional_t<length <= 2, uint16_t,
				jsonifier::internal::conditional_t<length <= 4, uint32_t, jsonifier::internal::conditional_t<length <= 8, uint64_t, void>>>>;
	};

	template<uint64_t length> using convert_length_to_int_t = typename convert_length_to_int<length>::type;

	template<string_literal string> constexpr convert_length_to_int_t<string.size()> getStringAsInt() noexcept {
		string_view_ptr stringNew = string.data();
		convert_length_to_int_t<string.size()> returnValue{};
		for (uint64_t x = 0; x < string.size(); ++x) {
			returnValue |= static_cast<convert_length_to_int_t<string.size()>>(stringNew[x]) << x * 8;
		}
		return returnValue;
	}

	template<string_literal stringNew> JSONIFIER_INLINE bool compareStringAsInt(string_view_ptr src) {
		static constexpr auto string{ stringNew };
		static constexpr auto stringInt{ getStringAsInt<string>() };
		uint32_t sourceVal;
		std::memcpy(&sourceVal, src, string.size());
		return !static_cast<bool>(sourceVal ^ stringInt);
	}

	template<jsonifier::concepts::bool_t bool_type> JSONIFIER_INLINE bool parseBool(bool_type& value, string_view_ptr& context) noexcept {
		if (compareStringAsInt<"true">(context)) {
			value = true;
			context += 4;
			return true;
		} else if (compareStringAsInt<"fals">(context) && context[4] == 'e') {
			value = false;
			context += 5;
			return true;
		}
		return false;
	}

	JSONIFIER_INLINE bool parseNull(string_view_ptr& context) noexcept {
		if JSONIFIER_LIKELY (compareStringAsInt<"null">(context)) {
			context += 4;
			return true;
		} else {
			return false;
		}
	}

	JSONIFIER_INLINE bool validateBool(string_view_ptr context) noexcept {
		if (compareStringAsInt<"true">(context)) {
			return true;
		} else if (compareStringAsInt<"fals">(context) && context[4] == 'e') {
			return true;
		}
		return false;
	}

	JSONIFIER_INLINE bool validateNull(string_view_ptr context) noexcept {
		if JSONIFIER_LIKELY (compareStringAsInt<"null">(context)) {
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

	template<const auto options, typename context_type> struct derailleur {
		template<typename value_type> JSONIFIER_INLINE static bool parseString(value_type& value, context_type& context) noexcept {
			if constexpr (options.partialRead) {
				if JSONIFIER_LIKELY ((context.iter < context.endIter) && **context.iter == '"') {
					auto string1 = *(context.iter) + 1;
					++context.iter;

					auto lengthNew = static_cast<uint64_t>(*context.iter - string1) - 1;
					value.resize(lengthNew + bytesPerStep);

					auto* string2 = value.data();

					string_parser<options, decltype(string1), decltype(stringBuffer.data())>::impl(string1, *context.iter, string2, lengthNew);
					if JSONIFIER_LIKELY (string1) {
						value.resize(lengthNew);
						return true;
					} else {
						context.parserPtr->template reportError<parse_errors::Invalid_String_Characters>(context);
						return false;
					}
				}
				JSONIFIER_ELSE_UNLIKELY(else) {
					context.parserPtr->template reportError<parse_errors::Missing_String_Start>(context);
					return false;
				}
			} else {
				if JSONIFIER_LIKELY ((context.iter < context.endIter) && *context.iter == '"') {
					++context.iter;
					auto string1 = context.iter;
					skipStringImpl(context.iter, static_cast<size_t>(context.endIter - context.iter));

					auto lengthNew = static_cast<uint64_t>(context.iter - string1);
					value.resize(lengthNew + bytesPerStep);

					auto* string2 = value.data();

					string_parser<options, decltype(string1), decltype(stringBuffer.data())>::impl(string1, context.iter, string2, lengthNew);
					if JSONIFIER_LIKELY (string1) {
						value.resize(lengthNew);
						++context.iter;
						return true;
					} else {
						context.parserPtr->template reportError<parse_errors::Invalid_String_Characters>(context);
						return false;
					}
				}
				JSONIFIER_ELSE_UNLIKELY(else) {
					context.parserPtr->template reportError<parse_errors::Missing_String_Start>(context);
					return false;
				}
			}
		}

		JSONIFIER_INLINE static void skipString(context_type& context) noexcept {
			if constexpr (options.partialRead) {
				++context.iter;
			} else {
				++context.iter;
				const auto newLength = static_cast<uint64_t>(context.endIter - context.iter);
				skipStringImpl(context.iter, newLength);
			}
		}

		template<typename value_type> JSONIFIER_INLINE static void skipKey(context_type& context) noexcept {
			if constexpr (options.partialRead) {
				++context.iter;
			} else {
				static constexpr auto keyLength{ keyStatsVal<value_type>.minLength - 1 };
				context.iter += keyLength;
				skipString(context);
			}
		}

		template<typename value_type> JSONIFIER_INLINE static void skipKeyStarted(context_type& context) noexcept {
			if constexpr (options.partialRead) {
				++context.iter;
			} else {
				const auto newLength = static_cast<uint64_t>(context.endIter - context.iter);
				skipStringImpl(context.iter, newLength);
			}
		}

		JSONIFIER_INLINE static void skipNumber(context_type& context) noexcept {
			while (numericTable[uint8_t(*context.iter)]) {
				++context.iter;
			}
		}

		template<char string1, char end> JSONIFIER_INLINE static string_view_ptr getNextOpenOrClose(context_type& context, uint64_t length) {
			string_view_ptr nextOpen  = char_comparison<string1, char>::memchar(context.iter, length);
			string_view_ptr nextClose = char_comparison<end, char>::memchar(context.iter, length);
			return (nextClose && (nextClose < nextOpen || !nextOpen)) ? nextClose : nextOpen;
		}

		template<char valueStart, char valueEnd> JSONIFIER_INLINE static void skipToEndOfValue(context_type& context) {
			if constexpr (options.partialRead) {
				uint64_t depth{ 1 };
				while (depth > 0 && context.iter < context.endIter) {
					switch (**context.iter) {
						case valueStart: {
							++depth;
							break;
						}
						case valueEnd: {
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
				uint64_t depth			 = 1;
				uint64_t remainingLength = static_cast<uint64_t>(context.endIter - context.iter);
				if (context.iter + bytesPerStep < context.endIter) {
					string_view_ptr nextQuote		= char_comparison<'"', char>::memchar(context.iter, remainingLength);
					string_view_ptr nextOpenOrClose = getNextOpenOrClose<valueStart, valueEnd>(context, remainingLength);

					while (nextOpenOrClose && depth > 0 && context.iter + bytesPerStep < context.endIter) {
						if (nextQuote && (nextQuote < nextOpenOrClose)) {
							skipString(context);
							++context.iter;
							remainingLength = static_cast<uint64_t>(context.endIter - context.iter);
							nextQuote		= static_cast<string_view_ptr>(std::memchr(context.iter, '"', remainingLength));
						} else {
							if (*nextOpenOrClose == valueEnd) {
								--depth;
							} else if (*nextOpenOrClose == valueStart) {
								++depth;
							}
							context.iter = nextOpenOrClose;
							if (depth == 0) {
								++context.iter;
								return;
							}
							++context.iter;
							remainingLength = static_cast<uint64_t>(context.endIter - context.iter);
							nextOpenOrClose = getNextOpenOrClose<valueStart, valueEnd>(context, remainingLength);
						}
					}
				}
			}
		}

		static void skipObject(context_type& context) noexcept {
			if constexpr (options.partialRead) {
				++context.iter;
				uint64_t currentDepth{ 1 };
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
						if constexpr (!options.minified) {
							JSONIFIER_SKIP_WS()
						}
						if JSONIFIER_LIKELY ((context.iter < context.endIter) && *context.iter == ':') {
							++context.iter;
							if constexpr (!options.minified) {
								JSONIFIER_SKIP_WS()
							}
							skipToNextValue(context);
							if JSONIFIER_LIKELY ((context.iter < context.endIter) && *context.iter == ',') {
								++context.iter;
								if constexpr (!options.minified) {
									JSONIFIER_SKIP_WS()
								}
							}
							JSONIFIER_ELSE_UNLIKELY(else) {
								break;
							}
						}
						JSONIFIER_ELSE_UNLIKELY(else) {
							return;
						}
					}
					JSONIFIER_ELSE_UNLIKELY(else) {
						return;
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

		static void skipArray(context_type& context) noexcept {
			if constexpr (options.partialRead) {
				++context.iter;
				uint64_t currentDepth{ 1 };
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
					skipToNextValue(context);
					if JSONIFIER_LIKELY ((context.iter < context.endIter) && *context.iter == ',') {
						++context.iter;
						if constexpr (!options.minified) {
							JSONIFIER_SKIP_WS()
						}
					} else {
						break;
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

		static void skipToNextValue(context_type& context) noexcept {
			if constexpr (options.partialRead) {
				switch (**context.iter) {
					case '{': {
						skipObject(context);
						break;
					}
					case '[': {
						skipArray(context);
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
				if JSONIFIER_LIKELY ((context.iter + 1) < context.endIter) {
					switch (*context.iter) {
						case '{': {
							++context.currentObjectDepth;
							skipObject(context);
							break;
						}
						case '[': {
							++context.currentArrayDepth;
							skipArray(context);
							break;
						}
						case ',': {
							++context.iter;
							if constexpr (!options.minified) {
								JSONIFIER_SKIP_WS()
							}
							skipToNextValue(context);
							break;
						}
						case '"': {
							skipString(context);
							if ((context.iter < context.endIter) && *context.iter == '"') {
								++context.iter;
								if constexpr (!options.minified) {
									JSONIFIER_SKIP_WS()
								}
							} else {
								context.currentObjectDepth++;
								return;
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

		template<typename iterator> JSONIFIER_INLINE static void skipWs(iterator& context) noexcept {
			while (whitespaceTable[uint8_t(*context)]) {
				++context;
			}
		}
	};

}// namespace internal