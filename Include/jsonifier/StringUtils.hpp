/*
	MIT License

	Copyright (c) 2024 RealTimeChris

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
	while (whitespaceTable[static_cast<uint8_t>(*context.iter)]) { \
		++context.iter; \
	}

#define CHECK_FOR_END(iter) \
	if (iter >= context.endIter) { \
		return; \
	}

	template<typename iterator01, typename iterator02> JSONIFIER_ALWAYS_INLINE void skipMatchingWs(iterator01 wsStart, iterator02& context, uint64_t length) noexcept {
		if (length > 7) {
			uint64_t v[2];
			while (length > 8) {
				std::memcpy(v, wsStart, 8);
				std::memcpy(v + 1, context, 8);
				if JSONIFIER_LIKELY ((v[0] == v[1])) {
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
			constexpr uint64_t n{ sizeof(uint32_t) };
			if (length >= n) {
				uint32_t v[2];
				std::memcpy(v, wsStart, n);
				std::memcpy(v + 1, context, n);
				if JSONIFIER_LIKELY ((v[0] == v[1])) {
					wsStart += n;
					length -= n;
					context += n;
				} else {
					return;
				}
			}
		}
		{
			constexpr uint64_t n{ sizeof(uint16_t) };
			if (length >= n) {
				uint16_t v[2];
				std::memcpy(v, wsStart, n);
				std::memcpy(v + 1, context, n);
				if JSONIFIER_LIKELY ((v[0] == v[1])) {
					context += n;
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

	JSONIFIER_ALWAYS_INLINE const char* getUnderlyingPtr(const char** ptr) noexcept {
		return *ptr;
	}

	JSONIFIER_ALWAYS_INLINE const char* getUnderlyingPtr(const char* ptr) noexcept {
		return ptr;
	}

	constexpr uint32_t digitToVal32[]{ 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu,
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
		0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu };

	// Taken from simdjson: https://github.com/simdjson/simdjson
	template<typename char_type> JSONIFIER_ALWAYS_INLINE uint32_t hexToU32NoCheck(char_type* string1) noexcept {
		return digitToVal32[630ull + string1[0]] | digitToVal32[420ull + string1[1]] | digitToVal32[210ull + string1[2]] | digitToVal32[0ull + string1[3]];
	}

	// Taken from simdjson: https://github.com/simdjson/simdjson
	template<typename char_type> JSONIFIER_ALWAYS_INLINE size_t codePointToUtf8(uint32_t cp, char_type* c) noexcept {
		if (cp <= 0x7F) {
			c[0] = static_cast<char_type>(cp);
			return 1;
		}
		if (cp <= 0x7FF) {
			c[0] = static_cast<char_type>((cp >> 6) + 192);
			c[1] = static_cast<char_type>((cp & 63) + 128);
			return 2;
		} else if (cp <= 0xFFFF) {
			c[0] = static_cast<char_type>((cp >> 12) + 224);
			c[1] = static_cast<char_type>(((cp >> 6) & 63) + 128);
			c[2] = static_cast<char_type>((cp & 63) + 128);
			return 3;
		} else if (cp <= 0x10FFFF) {
			c[0] = static_cast<char_type>((cp >> 18) + 240);
			c[1] = static_cast<char_type>(((cp >> 12) & 63) + 128);
			c[2] = static_cast<char_type>(((cp >> 6) & 63) + 128);
			c[3] = static_cast<char_type>((cp & 63) + 128);
			return 4;
		}
		return 0;
	}

	// Taken from simdjson: https://github.com/simdjson/simdjson
	template<typename iterator_type01, typename iterator_type02> JSONIFIER_ALWAYS_INLINE bool handleUnicodeCodePoint(iterator_type01& srcPtr, iterator_type02& dstPtr) noexcept {
		static constexpr uint32_t substitutionCodePoint = 0xfffd;
		uint32_t codePoint								= hexToU32NoCheck(srcPtr + 2);
		srcPtr += 6;

		if (codePoint >= 0xd800 && codePoint < 0xdc00) {
			const auto* srcData = srcPtr;
			if (((srcData[0] << 8) | srcData[1]) != ((static_cast<uint8_t>('\\') << 8) | static_cast<uint8_t>('u'))) {
				codePoint = substitutionCodePoint;
			} else {
				uint32_t codePoint2 = hexToU32NoCheck(srcData + 2);
				uint32_t lowBit		= codePoint2 - 0xdc00;
				if (lowBit >> 10) {
					codePoint = substitutionCodePoint;
				} else {
					codePoint = (((codePoint - 0xd800) << 10) | lowBit) + 0x10000;
					srcPtr += 6;
				}
			}
		} else if (codePoint >= 0xdc00 && codePoint <= 0xdfff) {
			codePoint = substitutionCodePoint;
		}
		size_t offset = codePointToUtf8(codePoint, dstPtr);
		dstPtr += offset;
		return offset > 0;
	}

	template<char threshold, typename simd_type> JSONIFIER_ALWAYS_INLINE bool hasByteLessThanValue(simd_type values) noexcept {
		return simd_internal::opCmpLt(values, simd_internal::gatherValue<simd_type>(threshold)) != 0;
	}

	template<const auto n, typename value_type = uint64_t> JSONIFIER_ALWAYS_INLINE bool hasByteLessThanValue(const char* values) noexcept {
		value_type x;
		std::memcpy(&x, values, sizeof(value_type));
		static constexpr value_type factor	= ~value_type(0) / value_type(255);
		static constexpr value_type msbMask = value_type(128);
		return ((( x )-factor * n) & ~( x )&factor * msbMask) != 0;
	}

	template<typename simd_type, typename integer_type> JSONIFIER_ALWAYS_INLINE integer_type copyAndFindParse(const char* string1, char* string2, simd_type& simdValue,
		const simd_type& quotes, const simd_type& backslashes) noexcept {
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

	template<typename iterator_type01> JSONIFIER_ALWAYS_INLINE static void skipStringImpl(iterator_type01& string1, size_t& lengthNew) noexcept {
		auto endIter = string1 + lengthNew;
		while (string1 < endIter) {
			auto* newIter = char_comparison<'"'>::memchar(string1, lengthNew);
			if (newIter) {
				string1 = newIter;

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
			if JSONIFIER_LIKELY ((*string1 == '"' || *string1 == '\\')) {
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
						string2[0] = static_cast<char_type02>(escapeChar);
						lengthNew -= 2;
						string2 += 1;
						string1 += 2;
					}
				}
			} else if JSONIFIER_UNLIKELY ((static_cast<uint8_t>(*string2) < 32)) {
				return nullptr;
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
		std::remove_const_t<char_type01> escapeChar;
#if defined(JSONIFIER_AVX512)
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
				simdValue			 = simd_internal::gatherValuesU<simd_type>(string1);
				nextBackslashOrQuote = copyAndFindParse<simd_type, integer_type>(string1, string2, simdValue, quotes, backslashes);
				if JSONIFIER_LIKELY ((nextBackslashOrQuote < mask)) {
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
							string2[nextBackslashOrQuote] = static_cast<char_type01>(escapeChar);
							lengthNew -= nextBackslashOrQuote + 2ull;
							string2 += nextBackslashOrQuote + 1ull;
							string1 += nextBackslashOrQuote + 2ull;
						} else {
							lengthNew -= bytesProcessed;
							string2 += bytesProcessed;
							string1 += bytesProcessed;
						}
					}
				} else if JSONIFIER_UNLIKELY ((hasByteLessThanValue<31>(simdValue))) {
					return static_cast<iterator_type02>(nullptr);
				} else {
					lengthNew -= bytesProcessed;
					string2 += bytesProcessed;
					string1 += bytesProcessed;
				}
			}
		}
#endif
#if defined(JSONIFIER_AVX2)
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
				simdValue			 = simd_internal::gatherValuesU<simd_type>(string1);
				nextBackslashOrQuote = copyAndFindParse<simd_type, integer_type>(string1, string2, simdValue, quotes, backslashes);
				if JSONIFIER_LIKELY ((nextBackslashOrQuote < mask)) {
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
							string2[nextBackslashOrQuote] = static_cast<char_type01>(escapeChar);
							lengthNew -= nextBackslashOrQuote + 2ull;
							string2 += nextBackslashOrQuote + 1ull;
							string1 += nextBackslashOrQuote + 2ull;
						} else {
							lengthNew -= bytesProcessed;
							string2 += bytesProcessed;
							string1 += bytesProcessed;
						}
					}
				} else if JSONIFIER_UNLIKELY ((hasByteLessThanValue<31>(simdValue))) {
					return static_cast<iterator_type02>(nullptr);
				} else {
					lengthNew -= bytesProcessed;
					string2 += bytesProcessed;
					string1 += bytesProcessed;
				}
			}
		}
#endif
#if defined(JSONIFIER_AVX)
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
				simdValue			 = simd_internal::gatherValuesU<simd_type>(string1);
				nextBackslashOrQuote = copyAndFindParse<simd_type, integer_type>(string1, string2, simdValue, quotes, backslashes);
				if JSONIFIER_LIKELY ((nextBackslashOrQuote < mask)) {
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
							string2[nextBackslashOrQuote] = static_cast<char_type01>(escapeChar);
							lengthNew -= nextBackslashOrQuote + 2ull;
							string2 += nextBackslashOrQuote + 1ull;
							string1 += nextBackslashOrQuote + 2ull;
						} else {
							lengthNew -= bytesProcessed;
							string2 += bytesProcessed;
							string1 += bytesProcessed;
						}
					}
				} else if JSONIFIER_UNLIKELY ((hasByteLessThanValue<31>(simdValue))) {
					return static_cast<iterator_type02>(nullptr);
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
				if JSONIFIER_LIKELY ((nextBackslashOrQuote < mask)) {
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
							string2[nextBackslashOrQuote] = static_cast<char_type01>(escapeChar);
							lengthNew -= nextBackslashOrQuote + 2ull;
							string2 += nextBackslashOrQuote + 1ull;
							string1 += nextBackslashOrQuote + 2ull;
						} else {
							lengthNew -= bytesProcessed;
							string2 += bytesProcessed;
							string1 += bytesProcessed;
						}
					}
				} else if JSONIFIER_UNLIKELY (((hasByteLessThanValue<31, simd_type>(string1)))) {
					return static_cast<iterator_type02>(nullptr);
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
	JSONIFIER_MAYBE_ALWAYS_INLINE static void serializeShortStringImpl(iterator_type01 string1, iterator_type02& string2, size_t lengthNew) noexcept {
		auto* endIter = string1 + lengthNew;
		for (; string1 < endIter; ++string1) {
			auto escapeChar = escapeTable[static_cast<uint8_t>(*string1)];
			if JSONIFIER_LIKELY (((escapeChar))) {
				std::memcpy(string2, &escapeChar, 2);
				string2 += 2;
			} else {
				*string2 = *string1;
				++string2;
			}
		}
	}

	template<typename iterator_type01, typename iterator_type02>
	JSONIFIER_MAYBE_ALWAYS_INLINE static void serializeStringImpl(iterator_type01 string1, iterator_type02& string2, size_t lengthNew) noexcept {
		uint16_t escapeChar;
#if defined(JSONIFIER_AVX512)
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
				if JSONIFIER_LIKELY ((nextEscapeable < mask)) {
					escapeChar = escapeTable[static_cast<uint8_t>(string1[nextEscapeable])];
					if JSONIFIER_LIKELY ((escapeChar != 0u)) {
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
#if defined(JSONIFIER_AVX2)
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
				if JSONIFIER_LIKELY ((nextEscapeable < mask)) {
					escapeChar = escapeTable[static_cast<uint8_t>(string1[nextEscapeable])];
					if JSONIFIER_LIKELY ((escapeChar != 0u)) {
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
#if defined(JSONIFIER_AVX)
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
				if JSONIFIER_LIKELY ((nextEscapeable < mask)) {
					escapeChar = escapeTable[static_cast<uint8_t>(string1[nextEscapeable])];
					if JSONIFIER_LIKELY ((escapeChar != 0u)) {
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
				if JSONIFIER_LIKELY ((nextEscapeable < mask)) {
					escapeChar = escapeTable[static_cast<uint8_t>(string1[nextEscapeable])];
					if JSONIFIER_LIKELY ((escapeChar != 0u)) {
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
		static_assert(length <= 8, "Sorry, but that string is too int64_t!");
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

	template<string_literal string, typename char_type> JSONIFIER_ALWAYS_INLINE bool compareStringAsInt(const char_type* context) noexcept {
		static constexpr jsonifier_internal::string_literal stringNew{ string };
		static constexpr auto newString{ getStringAsInt<char_type, stringNew>() };
		if constexpr (stringNew.size() % 2 == 0) {
			convert_length_to_int_t<stringNew.size()> newerString;
			std::memcpy(&newerString, context, stringNew.size());
			return newString == newerString;
		} else {
			convert_length_to_int_t<stringNew.size()> newerString{};
			std::memcpy(&newerString, context, stringNew.size());
			return newString == newerString;
		}
	}

	template<typename context_type, jsonifier::concepts::bool_t bool_type> JSONIFIER_ALWAYS_INLINE bool parseBool(bool_type& value, context_type& context) noexcept {
		if (compareStringAsInt<"true">(context)) {
			value = true;
			context += 4;
			return true;
		} else {
			if JSONIFIER_LIKELY ((compareStringAsInt<"false">(context))) {
				value = false;
				context += 5;
				return true;
			} else {
				return false;
			}
		}
	}

	template<typename context_type> JSONIFIER_ALWAYS_INLINE bool parseNull(context_type& context) noexcept {
		if JSONIFIER_LIKELY ((compareStringAsInt<"null">(context))) {
			context += 4;
			return true;
		} else {
			return false;
		}
	}

	template<const auto options, typename context_type> struct derailleur {
		template<typename value_type> JSONIFIER_ALWAYS_INLINE static bool parseString(value_type&& value, context_type& context) noexcept {
			if JSONIFIER_LIKELY ((*context.iter == '"')) {
				++context.iter;
				auto newPtr = parseStringImpl(context.iter, stringBuffer.data(), context.endIter - context.iter);
				if JSONIFIER_LIKELY ((newPtr)) {
					auto newSize = static_cast<size_t>(newPtr - stringBuffer.data());
					if (value.size() != newSize) {
						value.resize(newSize);
					}
					std::memcpy(value.data(), stringBuffer.data(), newSize);
					value[newSize] = '\0';
					++context.iter;
				}
				JSONIFIER_UNLIKELY(else) {
					static constexpr auto sourceLocation{ std::source_location::current() };
					context.parserPtr->template reportError<sourceLocation, parse_errors::Invalid_String_Characters>(context);
					return false;
				}
				return true;
			}
			JSONIFIER_UNLIKELY(else) {
				static constexpr auto sourceLocation{ std::source_location::current() };
				context.parserPtr->template reportError<sourceLocation, parse_errors::Missing_String_Start>(context);
				return false;
			}
		}

		JSONIFIER_ALWAYS_INLINE static void skipString(context_type& context) noexcept {
			++context.iter;
			auto newLength = static_cast<size_t>(context.endIter - context.iter);
			skipStringImpl(context.iter, newLength);
			++context.iter;
		}

		template<typename iterator>
			requires(!std::same_as<context_type, iterator>)
		JSONIFIER_ALWAYS_INLINE static void skipString(iterator&& iter, iterator&& endIter) noexcept {
			++iter;
			auto newLength = static_cast<size_t>(endIter - iter);
			auto newIter   = static_cast<const char*&>(iter);
			skipStringImpl(newIter, newLength);
			iter = newIter;
			++iter;
		}

		template<typename iterator01> JSONIFIER_ALWAYS_INLINE static void skipWs(iterator01& iter) noexcept {
			while (whitespaceTable[static_cast<uint8_t>(*iter)]) {
				++iter;
			}
		}

		JSONIFIER_ALWAYS_INLINE static void skipKey(context_type& context) noexcept {
			auto newLength = static_cast<size_t>(context.endIter - context.iter);
			context.iter   = char_comparison<'"'>::memchar(context.iter, newLength);
		}

		JSONIFIER_INLINE static bool skipObject(context_type& context) noexcept {
			++context.iter;
			if constexpr (!options.minified) {
				if (context.iter < context.endIter) {
					JSONIFIER_SKIP_WS();
				} else {
					return true;
				}
			}
			if (*context.iter == '}') {
				--context.currentObjectDepth;
				++context.iter;
				return true;
			}
			while (true) {
				if (*context.iter != '"') {
					return false;
				}
				skipString(context);
				if constexpr (!options.minified) {
					if (context.iter < context.endIter) {
						JSONIFIER_SKIP_WS();
					} else {
						return false;
					}
				}
				if (*context.iter != ':') {
					return false;
				} else {
					++context.iter;
				}
				if constexpr (!options.minified) {
					if (context.iter < context.endIter) {
						JSONIFIER_SKIP_WS();
					} else {
						return false;
					}
				}
				if (!skipToNextValue(context)) {
					return false;
				}
				if constexpr (!options.minified) {
					if (context.iter < context.endIter) {
						JSONIFIER_SKIP_WS();
					} else {
						return false;
					}
				}
				if (*context.iter != ',') {
					break;
				}
				++context.iter;
				if constexpr (!options.minified) {
					if (context.iter < context.endIter) {
						JSONIFIER_SKIP_WS();
					} else {
						break;
					}
				}
			}
			if (*context.iter == '}') {
				--context.currentObjectDepth;
				++context.iter;
				return true;
			} else {
				return false;
			}
		}

		JSONIFIER_INLINE static bool skipArray(context_type& context) noexcept {
			++context.iter;
			if constexpr (!options.minified) {
				if (context.iter < context.endIter) {
					JSONIFIER_SKIP_WS();
				} else {
					return true;
				}
			}
			if (*context.iter == ']') {
				--context.currentArrayDepth;
				++context.iter;
				return true;
			}
			while (true) {
				if (!skipToNextValue(context)) {
					return false;
				}
				if constexpr (!options.minified) {
					if (context.iter < context.endIter) {
						JSONIFIER_SKIP_WS();
					} else {
						return false;
					}
				}
				if (context.iter < context.endIter) {
					if (*context.iter != ',') {
						break;
					}
				} else {
					return false;
				}
				++context.iter;
				if constexpr (!options.minified) {
					if (context.iter < context.endIter) {
						JSONIFIER_SKIP_WS();
					} else {
						break;
					}
				}
			}
			if (*context.iter == ']') {
				--context.currentArrayDepth;
				++context.iter;
				return true;
			} else {
				return false;
			}
		}

		JSONIFIER_INLINE static bool skipToNextValue(context_type& context) noexcept {
			if constexpr (!options.minified) {
				if (context.iter < context.endIter) {
					JSONIFIER_SKIP_WS();
				} else {
					return false;
				}
			}
			switch (*context.iter) {
				case '{': {
					++context.currentObjectDepth;
					if (!skipObject(context)) {
						return false;
					}
					break;
				}
				case '}': {
					++context.iter;
					return true;
				}
				case '[': {
					++context.currentArrayDepth;
					if (!skipArray(context)) {
						return false;
					}
					break;
				}
				case ']': {
					++context.iter;
					return true;
				}
				case '"': {
					skipString(context);
					if constexpr (!options.minified) {
						if (context.iter < context.endIter) {
							JSONIFIER_SKIP_WS();
						} else {
							return false;
						}
					}
					if (context.iter < context.endIter) {
						if (*context.iter == ':') {
							++context.iter;
							if constexpr (!options.minified) {
								if (context.iter < context.endIter) {
									JSONIFIER_SKIP_WS();
								} else {
									return false;
								}
							}
							if (!skipToNextValue(context)) {
								return false;
							}
						}
					} else {
						return false;
					}
					break;
				}
				case ':': {
					++context.iter;
					if constexpr (!options.minified) {
						if (context.iter < context.endIter) {
							JSONIFIER_SKIP_WS();
						} else {
							return false;
						}
					}
					if (!skipToNextValue(context)) {
						return false;
					}
					break;
				}
				case 'n': {
					++context.iter;
					if JSONIFIER_LIKELY ((compareStringAsInt<"ull">(context.iter))) {
						context.iter += 3;
					} else {
						return false;
					}
					break;
				}
				case 'f': {
					++context.iter;
					if JSONIFIER_LIKELY ((compareStringAsInt<"alse">(context.iter))) {
						context.iter += 4;
					} else {
						return false;
					}
					break;
				}
				case 't': {
					++context.iter;
					if JSONIFIER_LIKELY ((compareStringAsInt<"rue">(context.iter))) {
						context.iter += 3;
					} else {
						return false;
					}
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
					skipNumber(context);
					break;
				}
				default: {
					return false;
				}
			}
			if constexpr (!options.minified) {
				if (context.iter < context.endIter) {
					JSONIFIER_SKIP_WS();
				} else {
					return false;
				}
			}
			return true;
		}

		template<typename iterator>
			requires(!std::same_as<context_type, iterator>)
		JSONIFIER_INLINE static bool skipObject(iterator&& iter, iterator&& endIter) noexcept {
			++iter;
			if constexpr (!options.minified) {
				skipWs(iter);
			}
			if (*iter == '}') {
				++iter;
				return true;
			}
			while (true) {
				if (*iter != '"') {
					return false;
				}
				skipString(iter, endIter);
				if constexpr (!options.minified) {
					skipWs(iter);
				}
				if (*iter != ':') {
					return false;
				} else {
					++iter;
				}
				if constexpr (!options.minified) {
					skipWs(iter);
				}
				if (!skipToNextValue(iter, endIter)) {
					return false;
				}
				if constexpr (!options.minified) {
					skipWs(iter);
				}
				if (*iter != ',') {
					break;
				}
				++iter;
				if constexpr (!options.minified) {
					skipWs(iter);
				}
			}
			if (*iter == '}') {
				++iter;
				return true;
			} else {
				return false;
			}
		}

		template<typename iterator>
			requires(!std::same_as<context_type, iterator>)
		JSONIFIER_INLINE static bool skipArray(iterator&& iter, iterator&& endIter) noexcept {
			++iter;
			if constexpr (!options.minified) {
				skipWs(iter);
			}
			if (*iter == ']') {
				++iter;
				return true;
			}
			while (true) {
				if (!skipToNextValue(iter, endIter)) {
					return false;
				}
				if constexpr (!options.minified) {
					skipWs(iter);
				}
				if (*iter != ',') {
					break;
				}
				++iter;
				if constexpr (!options.minified) {
					skipWs(iter);
				}
			}
			if (*iter == ']') {
				++iter;
				return true;
			} else {
				return false;
			}
		}

		template<typename iterator>
			requires(!std::same_as<context_type, iterator>)
		JSONIFIER_INLINE static bool skipToNextValue(iterator&& iter, iterator&& endIter) noexcept {
			if constexpr (!options.minified) {
				skipWs(iter);
			}
			switch (*iter) {
				case '{': {
					if (!skipObject(iter, endIter)) {
						return false;
					}
					if constexpr (!options.minified) {
						skipWs(iter);
					}
					break;
				}
				case '[': {
					if (!skipArray(iter, endIter)) {
						return false;
					}
					if constexpr (!options.minified) {
						skipWs(iter);
					}
					break;
				}
				case '"': {
					skipString(iter, endIter);
					if constexpr (!options.minified) {
						skipWs(iter);
					}
					if (*iter == ':') {
						++iter;
						if constexpr (!options.minified) {
							skipWs(iter);
						}
						if (!skipToNextValue(iter, endIter)) {
							return false;
						}
					}
					break;
				}
				case ':': {
					++iter;
					if constexpr (!options.minified) {
						skipWs(iter);
					}
					if (!skipToNextValue(iter, endIter)) {
						return false;
					}
					break;
				}
				case 'n': {
					iter += 4;
					if constexpr (!options.minified) {
						skipWs(iter);
					}
					break;
				}
				case 'f': {
					iter += 5;
					if constexpr (!options.minified) {
						skipWs(iter);
					}
					break;
				}
				case 't': {
					iter += 4;
					if constexpr (!options.minified) {
						skipWs(iter);
					}
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
					skipNumber(iter, endIter);
					break;
				}
				default: {
					break;
				}
			}
			return true;
		}

		template<char startChar, char endChar, typename iterator>
			requires(!std::same_as<context_type, iterator>)
		JSONIFIER_ALWAYS_INLINE static size_t countValueElements(iterator iter, iterator endIter) noexcept {
			auto newValue = *iter;
			if JSONIFIER_UNLIKELY ((newValue == ']' || newValue == '}')) {
				return 0;
			}
			size_t currentCount{ 1 };
			while (iter != endIter) {
				switch (*iter) {
					[[unlikely]] case ',': {
						++currentCount;
						++iter;
						break;
					}
					[[unlikely]] case '{':
					[[unlikely]] case '[': {
						skipToNextValue(iter, endIter);
						break;
					}
					[[unlikely]] case endChar: { return currentCount; }
					[[likely]] case '"': {
						skipString(iter, endIter);
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
						skipNumber(iter, endIter);
						break;
					}
						[[likely]] default : {
							++iter;
							break;
						}
				}
			}
			return currentCount;
		}

		JSONIFIER_ALWAYS_INLINE static context_type skipWs(context_type context) noexcept {
			while (whitespaceTable[static_cast<uint8_t>(*context)]) {
				++context;
			}
			return context;
		}

		template<typename iterator> JSONIFIER_ALWAYS_INLINE static void skipNumber(iterator&& iter, iterator&& endIter) noexcept {
			iter += *iter == '-';
			auto sig_start_it  = iter;
			auto frac_start_it = endIter;
			auto fracStart	   = [&]() -> bool {
				frac_start_it = iter;
				while (iter < endIter && isNumberType(*iter)) {
					++iter;
				}
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
				while (iter < endIter && isNumberType(*iter)) {
					++iter;
				}
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
			while (iter < endIter && isNumberType(*iter)) {
				++iter;
			}
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

		JSONIFIER_ALWAYS_INLINE static void skipNumber(context_type& context) noexcept {
			context.iter += *context.iter == '-';
			auto sig_start_it  = context.iter;
			auto frac_start_it = context.endIter;
			auto fracStart	   = [&]() -> bool {
				frac_start_it = context.iter;
				while (context.iter < context.endIter && isNumberType(*context.iter)) {
					++context.iter;
				}
				if (context.iter == frac_start_it) {
					return true;
				}
				if ((*context.iter | ('E' ^ 'e')) != 'e') {
					return true;
				}
				++context.iter;
				return false;
			};

			auto expStart = [&]() -> bool {
				context.iter += *context.iter == '+' || *context.iter == '-';
				auto exp_start_it = context.iter;
				while (context.iter < context.endIter && isNumberType(*context.iter)) {
					++context.iter;
				}
				if (context.iter == exp_start_it) {
					return true;
				}
				return false;
			};
			if (*context.iter == '0') {
				++context.iter;
				if (*context.iter != '.') {
					return;
				}
				++context.iter;
				if (fracStart()) {
					return;
				}
			}
			while (context.iter < context.endIter && isNumberType(*context.iter)) {
				++context.iter;
			}
			if (context.iter == sig_start_it) {
				return;
			}
			if ((*context.iter | ('E' ^ 'e')) == 'e') {
				++context.iter;
				if (expStart()) {
					return;
				}
			}
			if (*context.iter != '.')
				return;
			++context.iter;
		}
	};

	template<const auto options, typename context_type> JSONIFIER_ALWAYS_INLINE size_t getKeyLength(context_type context) noexcept {
		if JSONIFIER_LIKELY ((*context.iter == '"')) {
			++context.iter;
			auto start	 = context.iter;
			context.iter = char_comparison<'"'>::memchar(context.iter, static_cast<size_t>(context.endIter - context.iter));
			return static_cast<size_t>(context.iter - start);
		} else {
			static constexpr auto sourceLocation{ std::source_location::current() };
			context.parserPtr->template reportError<sourceLocation, parse_errors::Missing_String_Start>(context);
			return {};
		}
	}

}// namespace jsonifier_internal