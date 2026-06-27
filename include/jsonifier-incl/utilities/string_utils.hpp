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

#include <jsonifier-incl/containers/allocator.hpp>
#include <jsonifier-incl/utilities/hash_map.hpp>
#include <jsonifier-incl/utilities/str_to_d.hpp>
#include <jsonifier-incl/utilities/error.hpp>
#include <jsonifier-incl/utilities/simd.hpp>

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
	while (context.iter < context.endIter && whitespaceTable[static_cast<uint8_t>(*context.iter)]) { \
		++context.iter; \
	}

	template<typename iterator01, typename iterator02> JSONIFIER_INLINE void skipMatchingWs(const iterator01* ws, iterator02&& it, uint64_t length) noexcept {
		while (length >= 8) {
			uint64_t v0, v1;
			std::memcpy(&v0, ws, 8);
			std::memcpy(&v1, it, 8);
			uint64_t comp_result = (v0 ^ v1);
			if (comp_result) {
				it += std::countr_zero(comp_result) >> 3;
				return;
			}
			length -= 8;
			ws += 8;
			it += 8;
		}

		if (length >= 4) {
			uint32_t v0, v1;
			std::memcpy(&v0, ws, 4);
			std::memcpy(&v1, it, 4);
			if (v0 ^ v1) {
				return;
			}
			length -= 4;
			ws += 4;
			it += 4;
		}

		if (length >= 2) {
			uint16_t v0, v1;
			std::memcpy(&v0, ws, 2);
			std::memcpy(&v1, it, 2);
			if (v0 ^ v1) {
				return;
			}
			length -= 2;
			ws += 2;
			it += 2;
		}
	}

#define JSONIFIER_SKIP_MATCHING_WS() \
	if constexpr (newLines) { \
		uint64_t length = wsSize < static_cast<uint64_t>(context.endIter - context.iter) ? wsSize : static_cast<uint64_t>(context.endIter - context.iter); \
		skipMatchingWs(wsStart, context.iter, length); \
	} else { \
		if (wsSize) { \
			skipMatchingWs(wsStart, context.iter, wsSize); \
		} \
	} \
	JSONIFIER_SKIP_WS()

	JSONIFIER_INLINE static string_view_ptr getUnderlyingPtr(string_view_ptr ptr) noexcept {
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
	JSONIFIER_INLINE static uint32_t hexToU32NoCheck(string_view_ptr string1) noexcept {
		return digit_tables<>::digitToVal32[630ull + static_cast<uint64_t>(string1[0])] | digit_tables<>::digitToVal32[420ull + static_cast<uint64_t>(string1[1])] |
			digit_tables<>::digitToVal32[210ull + static_cast<uint64_t>(string1[2])] | digit_tables<>::digitToVal32[0ull + static_cast<uint64_t>(string1[3])];
	}

	/// Sampled from Simdjson library: https://github.com/simdjson/simdjson
	JSONIFIER_INLINE static uint32_t codePointToUtf8(uint32_t cp, char* c) noexcept {
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
	template<typename basic_iterator01, typename basic_iterator02>
	JSONIFIER_INLINE static bool handleUnicodeCodePoint(basic_iterator01& srcPtr, basic_iterator02& dstPtr) noexcept {
		static constexpr uint8_t bs{ '\\' };
		static constexpr uint8_t u{ 'u' };

		uint32_t codePoint = hexToU32NoCheck(srcPtr + 2);
		srcPtr += 6;

		if (codePoint >= 0xD800 && codePoint <= 0xDFFF) {
			if (codePoint >= 0xDC00) {
				return false;
			}
			if (((srcPtr[0] << 8) | srcPtr[1]) != ((bs << 8) | u)) {
				return false;
			}
			uint32_t lowSurrogate = hexToU32NoCheck(srcPtr + 2);
			uint32_t lowBit		  = lowSurrogate - 0xDC00;
			if (lowBit >> 10) {
				return false;
			}
			codePoint = (((codePoint - 0xD800) << 10) | lowBit) + 0x10000;
			srcPtr += 6;
		}

		const uint64_t offset = codePointToUtf8(codePoint, dstPtr);
		dstPtr += offset;
		return offset > 0;
	}

	template<char threshold, typename simd_type> [[maybe_unused]] JSONIFIER_INLINE static bool hasByteLessThanValue(const simd_type values) noexcept {
		return simd::opCmpLt(values, simd::gatherValue<simd_type>(threshold)) != 0;
	}

	template<char threshold, typename simd_type> [[maybe_unused]] JSONIFIER_INLINE static bool hasByteLessThanValue(string_view_ptr values) noexcept {
		JSONIFIER_ALIGN(16)
		char valuesNew[16]{ constEval(threshold + 1), constEval(threshold + 1), constEval(threshold + 1), constEval(threshold + 1), constEval(threshold + 1),
			constEval(threshold + 1), constEval(threshold + 1), constEval(threshold + 1), constEval(threshold + 1), constEval(threshold + 1), constEval(threshold + 1),
			constEval(threshold + 1), constEval(threshold + 1), constEval(threshold + 1), constEval(threshold + 1), constEval(threshold + 1) };
		std::memcpy(valuesNew, values, 8);
		return simd::opCmpLt(simd::gatherValues<simd_type>(valuesNew), simd::gatherValue<simd_type>(threshold)) != 0;
	}

	template<auto maskValue, typename simd_type, typename integer_type>
	[[maybe_unused]] JSONIFIER_INLINE static integer_type findParse(const simd_type& simdValue, const simd_type& simdValues01, const simd_type& simdValues02) noexcept {
		auto result01 = simd::opOr(simd::opCmpEqRaw(simdValue, simdValues02), simd::opCmpEqRaw(simdValue, simdValues01));
		return simd::postCmpTzcnt(static_cast<integer_type>(simd::opBitMaskRaw(result01)));
	}

	template<concepts::unsigned_t simd_type, concepts::unsigned_t integer_type> [[maybe_unused]] JSONIFIER_INLINE static integer_type findParse(simd_type& simdValue) noexcept {
		static constexpr integer_type mask{ repeatByte<0b01111111, integer_type>() };
		static constexpr integer_type hiBits{ repeatByte<0b10000000, integer_type>() };
		static constexpr integer_type quoteBits{ repeatByte<'"', integer_type>() };
		static constexpr integer_type bsBits{ repeatByte<'\\', integer_type>() };
		const integer_type lo7	= simdValue & mask;
		const integer_type next = ~((((lo7 ^ quoteBits) + mask) & ((lo7 ^ bsBits) + mask)) | simdValue) & hiBits;
		return static_cast<integer_type>(simd::tzcnt(next) >> 3u);
	}

	template<typename simd_type, typename integer_type> [[maybe_unused]] JSONIFIER_INLINE static integer_type findSerialize(const simd_type& simdValue,
		const simd_type& simdValues01, const simd_type& simdValues02, const simd_type& simdValues03) noexcept {
		auto result01 = simd::opOr(simd::opOr(simd::opCmpLtRaw(simdValue, simdValues03), simd::opCmpEqRaw(simdValue, simdValues02)), simd::opCmpEqRaw(simdValue, simdValues01));
		return simd::postCmpTzcnt(static_cast<integer_type>(simd::opBitMaskRaw(result01)));
	}

	template<concepts::unsigned_t simd_type, concepts::unsigned_t integer_type> [[maybe_unused]] JSONIFIER_INLINE static integer_type findSerialize(simd_type& simdValue) noexcept {
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
	template<typename basic_iterator01> [[maybe_unused]] JSONIFIER_INLINE static void skipStringImpl(basic_iterator01& string1, uint64_t lengthNew) noexcept {
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

	static constexpr array<char, 256> escapeMap{ []() constexpr {
		array<char, 256> returnValues{};
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

	template<typename executor_type, typename integer_sequence> struct string_parse_executor;

	template<typename executor_type, uint64_t... indices> struct string_parse_executor<executor_type, integer_sequence<indices...>> {
		template<typename... arg_types> JSONIFIER_INLINE static bool impl(arg_types&&... args) noexcept {
			return ((executor_type::template impl<indices>(std::forward<arg_types>(args)...) != nullptr) && ...);
		}
	};

#if JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_AVX512)
	static constexpr uint64_t start_index = 0;
#elif JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_AVX2)
	static constexpr uint64_t start_index = 1;
#elif JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_AVX)
	static constexpr uint64_t start_index = 2;
#else
	static constexpr uint64_t start_index = 2;
#endif

	static constexpr uint64_t list_size = 3;

	template<uint64_t start, uint64_t end, uint64_t... indices> struct make_ascending_range_impl : make_ascending_range_impl<start + 1, end, indices..., start> {};
	template<uint64_t end, uint64_t... indices> struct make_ascending_range_impl<end, end, indices...> {
		using type = integer_sequence<indices...>;
	};
	template<uint64_t start, uint64_t end> using make_ascending_range = typename make_ascending_range_impl<start, end>::type;

	template<parse_options options, typename basic_iterator01, typename basic_iterator02> struct string_parser {
		struct string_parse_step {
			template<uint64_t index> JSONIFIER_INLINE static basic_iterator01 impl(basic_iterator01& string1Start, const basic_iterator01 string1End, basic_iterator02& string2,
				basic_iterator02& result) noexcept {
				using simd_list_local					 = typename get_type_at_index<simd::avx_integer_list, index>::type;
				using integer_type						 = typename simd_list_local::integer_type;
				using simd_type							 = typename simd_list_local::type::type;
				static constexpr uint64_t bytesProcessed = simd_list_local::bytesProcessed;
				static constexpr integer_type mask		 = simd_list_local::mask;

				char escapeChar;
				simd_type simdValue;
				integer_type nextBackslashOrQuote;
				const auto stringEndNew = string1End - bytesProcessed;

				const simd_type simdValues00 = simd::gatherValue<simd_type>('\\');
				const simd_type simdValues01 = simd::gatherValue<simd_type>('"');
				while (string1Start < stringEndNew) {
					simdValue = simd::gatherValuesU<simd_type>(string1Start);
					simd::storeU(simdValue, string2);
					nextBackslashOrQuote = findParse<mask, simd_type, integer_type>(simdValue, simdValues00, simdValues01);
					if JSONIFIER_LIKELY (nextBackslashOrQuote != mask) {
						escapeChar = string1Start[nextBackslashOrQuote];
						if (escapeChar == '"') {
							string1Start += nextBackslashOrQuote;
							result = string2 + nextBackslashOrQuote;
							return static_cast<basic_iterator01>(nullptr);
						} else if (escapeChar == '\\') {
							escapeChar = string1Start[nextBackslashOrQuote + 1];
							if (escapeChar == 0x75u) {
								string1Start += nextBackslashOrQuote;
								string2 += nextBackslashOrQuote;
								if (!handleUnicodeCodePoint(string1Start, string2)) {
									result = static_cast<basic_iterator02>(nullptr);
									return static_cast<basic_iterator01>(nullptr);
								}
								continue;
							}
							escapeChar = escapeMap[static_cast<uint8_t>(escapeChar)];
							if (escapeChar == 0u) {
								result = static_cast<basic_iterator02>(nullptr);
								return static_cast<basic_iterator01>(nullptr);
							}
							string2[nextBackslashOrQuote] = escapeChar;
							string2 += nextBackslashOrQuote + 1ull;
							string1Start += nextBackslashOrQuote + 2ull;
						} else {
							string2 += bytesProcessed;
							string1Start += bytesProcessed;
						}
					} else if JSONIFIER_UNLIKELY (hasByteLessThanValue<32>(simdValue)) {
						result = static_cast<basic_iterator02>(nullptr);
						return static_cast<basic_iterator01>(nullptr);
					} else {
						string2 += bytesProcessed;
						string1Start += bytesProcessed;
					}
				}
				return string1Start;
			}
		};

		JSONIFIER_INLINE static basic_iterator02 shortImpl(basic_iterator01& string1Start JSONIFIER_LIFETIME_BOUND, const basic_iterator01 string1End JSONIFIER_LIFETIME_BOUND,
			basic_iterator02 string2 JSONIFIER_LIFETIME_BOUND) noexcept {
			char escapeChar;
			while (string1Start <= string1End) {
				*string2   = *string1Start;
				escapeChar = *string1Start;
				if (escapeChar == '"') {
					return string2;
				} else if (escapeChar == '\\') {
					escapeChar = string1Start[1];
					if (escapeChar == 'u') {
						if (!handleUnicodeCodePoint(string1Start, string2)) {
							return nullptr;
						}
						continue;
					}
					escapeChar = escapeMap[static_cast<uint8_t>(escapeChar)];
					if (escapeChar == 0) {
						return nullptr;
					}
					string2[0] = escapeChar;
					string2 += 1;
					string1Start += 2;
				} else if JSONIFIER_UNLIKELY (static_cast<uint8_t>(*string2) < 32) {
					return nullptr;
				} else {
					++string2;
					++string1Start;
				}
			}
			return nullptr;
		}

		JSONIFIER_INLINE static basic_iterator02 impl(basic_iterator01& string1Start JSONIFIER_LIFETIME_BOUND, basic_iterator02 string2 JSONIFIER_LIFETIME_BOUND,
			uint64_t lengthNew) noexcept {
			const basic_iterator01 string1End = static_cast<basic_iterator01>(string1Start + lengthNew);
			basic_iterator02 result{};
			
			bool result_01{ string_parse_executor<string_parse_step, make_ascending_range<start_index, list_size>>::impl(string1Start, string1End, string2, result) };

			if (!result_01) {
				return result;
			}
			return shortImpl(string1Start, string1End, string2);
		}
	};

	static constexpr array<jsonifier::string_view, 256> escapeTable{ { "", R"(\u0001)", R"(\u0002)", R"(\u0003)", R"(\u0004)", R"(\u0005)", R"(\u0006)", R"(\a)", R"(\b)", R"(\t)",
		R"(\n)", R"(\v)", R"(\f)", R"(\r)", R"(\u000E)", R"(\u000F)", R"(\u0010)", R"(\u0011)", R"(\u0012)", R"(\u0013)", R"(\u0014)", R"(\u0015)", R"(\u0016)", R"(\u0017)",
		R"(\u0018)", R"(\u0019)", R"(\u001A)", R"(\u001B)", R"(\u001C)", R"(\u001D)", R"(\u001E)", R"(\u001F)", "", "", R"(\")", "", "", "", "", "", "", "", "", "", "", "", "", "",
		"", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
		"", R"(\\)", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "" } };

	static constexpr array<const char*, 256> escapeTablePtrs{ []() constexpr {
		array<const char*, 256> returnValues{};
		for (uint64_t x = 0; x < 256; ++x) {
			returnValues[x] = escapeTable[x].data();
		}
		return returnValues;
	}() };

	static constexpr array<uint64_t, 256> escapeTableSizes{ []() constexpr {
		array<uint64_t, 256> returnValues{};
		for (uint64_t x = 0; x < 256; ++x) {
			returnValues[x] = escapeTable[x].size();
		}
		return returnValues;
	}() };

	template<serialize_options options, typename basic_iterator01, typename basic_iterator02> struct string_serializer {
		struct string_serialize_step {
			template<uint64_t index> JSONIFIER_INLINE static auto* impl(basic_iterator01& string1Start, const basic_iterator01 string1End, basic_iterator02& string2) noexcept {
				using simd_list_local					 = typename get_type_at_index<simd::avx_integer_list, index>::type;
				using integer_type						 = typename simd_list_local::integer_type;
				using simd_type							 = typename simd_list_local::type::type;
				static constexpr uint64_t bytesProcessed = simd_list_local::bytesProcessed;
				static constexpr integer_type mask		 = simd_list_local::mask;

				simd_type simdValue;
				uint64_t nextSize;
				uint8_t nextChar;
				integer_type nextEscapeable;
				const char* escapeChar;
				const auto stringEndNew = string1End - bytesProcessed;

				const simd_type simdValues01 = simd::gatherValue<simd_type>('"');
				const simd_type simdValues02 = simd::gatherValue<simd_type>('\\');
				const simd_type simdValues03 = simd::gatherValue<simd_type>(static_cast<char>(32));
				while (string1Start < stringEndNew) {
					simdValue = simd::gatherValuesU<simd_type>(string1Start);
					simd::storeU(simdValue, string2);
					nextEscapeable = findSerialize<simd_type, integer_type>(simdValue, simdValues01, simdValues02, simdValues03);
					if JSONIFIER_LIKELY (nextEscapeable != mask) {
						nextChar   = static_cast<uint8_t>(string1Start[nextEscapeable]);
						nextSize   = escapeTableSizes[nextChar];
						escapeChar = escapeTablePtrs[nextChar];
						string2 += nextEscapeable;
						string1Start += nextEscapeable;
						std::memcpy(string2, escapeChar, nextSize);
						string2 += nextSize;
						++string1Start;
					} else {
						string2 += bytesProcessed;
						string1Start += bytesProcessed;
					}
				}
				return string1Start;
			}
		};

		JSONIFIER_INLINE static basic_iterator02 shortImpl(basic_iterator01& string1Start, const basic_iterator01 string1End, basic_iterator02 string2) noexcept {
			const char* escapeChar;
			uint8_t nextChar;
			uint64_t escapeSize;
			for (; string1Start < string1End; ++string1Start) {
				nextChar   = static_cast<uint8_t>(*string1Start);
				escapeSize = escapeTableSizes[nextChar];
				if (escapeSize > 0) {
					escapeChar = escapeTablePtrs[nextChar];
					std::memcpy(string2, escapeChar, escapeSize);
					string2 += escapeSize;
				} else {
					*string2 = *string1Start;
					++string2;
				}
			}
			return string2;
		}

		JSONIFIER_INLINE static basic_iterator02 impl(basic_iterator01 string1Start, basic_iterator02 string2, uint64_t lengthNew) noexcept {
			const basic_iterator01 string1End = string1Start + lengthNew;
			string_parse_executor<string_serialize_step, make_ascending_range<start_index, list_size>>::impl(string1Start, string1End, string2);
			return shortImpl(string1Start, string1End, string2);
		}
	};

	template<string_literal string> static constexpr convert_length_to_int_t<string.size()> getStringAsInt() noexcept {
		string_view_ptr stringNew = string.data();
		convert_length_to_int_t<string.size()> returnValue{};
		for (uint64_t x = 0; x < string.size(); ++x) {
			returnValue |= static_cast<convert_length_to_int_t<string.size()>>(stringNew[x]) << x * 8;
		}
		return returnValue;
	}

	template<string_literal stringNew> JSONIFIER_INLINE static bool compareStringAsInt(string_view_ptr src) {
		static constexpr auto string{ stringNew };
		static_assert(stringNew.size() == 4, "Sorry, but please only use a string with a length of 4 in this function!");
		static constexpr auto stringInt{ getStringAsInt<string>() };
		uint32_t sourceVal;
		std::memcpy(&sourceVal, src, string.size());
		return !static_cast<bool>(sourceVal ^ stringInt);
	}

	template<jsonifier::concepts::bool_t bool_type> JSONIFIER_INLINE static bool parseBool(bool_type& value, string_view_ptr& context) noexcept {
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

	template<typename context_type, jsonifier::concepts::bool_t bool_type> JSONIFIER_INLINE static bool parseBool(bool_type& value, context_type& context) noexcept {
		if (compareStringAsInt<"true">(&context.stringRoot[*context.iter])) {
			value = true;
			++context.iter;
			return true;
		} else if (compareStringAsInt<"fals">(&context.stringRoot[*context.iter]) && context.stringRoot[(*context.iter) + 4] == 'e') {
			value = false;
			++context.iter;
			return true;
		}
		return false;
	}

	template<typename context_type> JSONIFIER_INLINE static bool parseNull(context_type& context) noexcept {
		if JSONIFIER_LIKELY (compareStringAsInt<"null">(&context.stringRoot[*context.iter])) {
			++context.iter;
			return true;
		} else {
			return false;
		}
	}

	JSONIFIER_INLINE static bool parseNull(string_view_ptr& context) noexcept {
		if JSONIFIER_LIKELY (compareStringAsInt<"null">(context)) {
			context += 4;
			return true;
		} else {
			return false;
		}
	}

	JSONIFIER_INLINE static bool validateBool(string_view_ptr context) noexcept {
		if (compareStringAsInt<"true">(context)) {
			return true;
		} else if (compareStringAsInt<"fals">(context) && context[4] == 'e') {
			return true;
		}
		return false;
	}

	JSONIFIER_INLINE static bool validateNull(string_view_ptr context) noexcept {
		if JSONIFIER_LIKELY (compareStringAsInt<"null">(context)) {
			return true;
		} else {
			return false;
		}
	}

	static constexpr array<bool, 256> numericTable = []() constexpr {
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
				if JSONIFIER_LIKELY ((context.iter < context.endIter) && context.stringRoot[*context.iter] == '"') {
					auto newerPtr	  = (&context.stringRoot[(*context.iter)]) + 1;
					const auto newPtr = string_parser<options, decltype(newerPtr), decltype(context.parserPtr->getStringBuffer().data())>::impl(newerPtr,
						context.parserPtr->getStringBuffer().data(), static_cast<uint64_t>(*context.endIter - *context.iter));
					if JSONIFIER_LIKELY (newPtr) {
						const auto newSize = static_cast<uint64_t>(newPtr - context.parserPtr->getStringBuffer().data());
						if constexpr (concepts::has_resize<value_type>) {
							if JSONIFIER_UNLIKELY (value.size() != newSize) {
								value.resize(newSize);
							}
						}
						std::memcpy(value.data(), context.parserPtr->getStringBuffer().data(), newSize);
						++context.iter;
					}
					JSONIFIER_ELSE_UNLIKELY(else) {
						context.parserPtr->template reportError<parse_status::Invalid_String_Characters>(context);
						return false;
					}
					return true;
				}
				JSONIFIER_ELSE_UNLIKELY(else) {
					context.parserPtr->template reportError<parse_status::Missing_String_Start>(context);
					return false;
				}
			} else {
				if JSONIFIER_LIKELY ((context.iter < context.endIter) && *context.iter == '"') {
					++context.iter;
					const auto newPtr = string_parser<options, decltype(context.iter), decltype(context.parserPtr->getStringBuffer().data())>::impl(context.iter,
						context.parserPtr->getStringBuffer().data(), static_cast<uint64_t>(context.endIter - context.iter));
					if JSONIFIER_LIKELY (newPtr) {
						const auto newSize = static_cast<uint64_t>(newPtr - context.parserPtr->getStringBuffer().data());
						if constexpr (concepts::has_resize<value_type>) {
							if JSONIFIER_UNLIKELY (value.size() != newSize) {
								value.resize(newSize);
							}
						}
						std::memcpy(value.data(), context.parserPtr->getStringBuffer().data(), newSize);
						++context.iter;
					}
					JSONIFIER_ELSE_UNLIKELY(else) {
						context.parserPtr->template reportError<parse_status::Invalid_String_Characters>(context);
						return false;
					}
					return true;
				}
				JSONIFIER_ELSE_UNLIKELY(else) {
					context.parserPtr->template reportError<parse_status::Missing_String_Start>(context);
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

		template<char start, char end> JSONIFIER_INLINE static string_view_ptr getNextOpenOrClose(context_type& context, uint64_t length) {
			string_view_ptr nextOpen  = char_comparison<start, char>::memchar(context.iter, length);
			string_view_ptr nextClose = char_comparison<end, char>::memchar(context.iter, length);
			return (nextClose && (nextClose < nextOpen || !nextOpen)) ? nextClose : nextOpen;
		}

		template<char valueStart, char valueEnd> JSONIFIER_INLINE static void skipToEndOfValue(context_type& context) {
			if constexpr (options.partialRead) {
				uint64_t depth{ 1 };
				while (depth > 0 && context.iter < context.endIter) {
					switch (context.stringRoot[*context.iter]) {
						case '{': {
							++depth;
							break;
						}
						case '}': {
							--depth;
							break;
						}
						case '[': {
							++depth;
							break;
						}
						case ']': {
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
					switch (context.stringRoot[*context.iter]) {
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
					switch (context.stringRoot[*context.iter]) {
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
				switch (context.stringRoot[*context.iter]) {
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
