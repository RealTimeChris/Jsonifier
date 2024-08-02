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

	template<jsonifier::concepts::is_double_ptr iterator> JSONIFIER_ALWAYS_INLINE void skipNumber(iterator& iter, iterator&) noexcept {
		++iter;
	}

	template<typename iterator> JSONIFIER_ALWAYS_INLINE iterator skipWs(iterator iter) noexcept {
		while (whitespaceTable[static_cast<uint8_t>(*iter)]) {
			++iter;
		}
		return iter;
	}

	template<jsonifier::concepts::is_double_ptr iterator> JSONIFIER_ALWAYS_INLINE void skipToEndOfValue(iterator& iter, iterator& end) noexcept {
		uint64_t currentDepth{ 1 };
		auto skipToEnd = [&]() {
			while (iter != end && currentDepth > 0) {
				switch (*iter) {
					[[unlikely]] case '[':
					[[unlikely]] case '{': {
						++currentDepth;
						++iter;
						break;
					}
					[[unlikely]] case ']':
					[[unlikely]] case '}': {
						--currentDepth;
						++iter;
						break;
					}
					default: {
						++iter;
						break;
					}
				}
			}
		};
		switch (**iter) {
			[[unlikely]] case '[':
			[[unlikely]] case '{': {
				++iter;
				skipToEnd();
				break;
			}
			case '"': {
				++iter;
				break;
			}
			case ':': {
				++iter;
				skipToEndOfValue(iter, end);
				break;
			}
			case 't': {
				++iter;
				break;
			}
			case 'f': {
				++iter;
				break;
			}
			case 'n': {
				++iter;
				break;
			}
			case '-':
			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9': {
				skipNumber(iter, end);
				break;
			}
			[[likely]] default: {
				++iter;
				break;
			}
		}
	}

	template<jsonifier::concepts::is_double_ptr iterator> JSONIFIER_ALWAYS_INLINE void skipToNextValue(iterator& iter, iterator& end) noexcept {
		uint64_t currentDepth{ 1 };
		while (iter != end && currentDepth > 0) {
			switch (**iter) {
				[[unlikely]] case '{':
				[[unlikely]] case '[': {
					++currentDepth;
					++iter;
					break;
				}
				[[unlikely]] case '}':
				[[unlikely]] case ']': {
					--currentDepth;
					++iter;
					break;
				}
				[[unlikely]] case ':': {
					++iter;
					break;
				}
				[[unlikely]] case ',': {
					if (currentDepth <= 1) [[unlikely]] {
						return;
					}
					++iter;
					++iter;
					break;
				}
				[[unlikely]] case '\\': {
					++iter;
					break;
				}
				[[unlikely]] case 't': {
					++iter;
					break;
				}
				[[unlikely]] case 'f': {
					++iter;
					break;
				}
				[[unlikely]] case 'n': {
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
					++iter;
					break;
				}
				[[likely]] default: {
					++iter;
					break;
				}
			}
		}
	}

	template<typename iterator> JSONIFIER_ALWAYS_INLINE void skipNumber(iterator& iter, iterator& end) noexcept {
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

	template<typename iterator> JSONIFIER_INLINE void skipToNextValue(iterator& iter, iterator& end) noexcept;
	template<jsonifier::concepts::is_double_ptr iterator> JSONIFIER_ALWAYS_INLINE void skipToNextValue(iterator& iter, iterator& end) noexcept;

	const char* getUnderlyingPtr(const char** ptr) {
		return *ptr;
	}

	const char* getUnderlyingPtr(const char* ptr) {
		return ptr;
	}

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

	template<typename iterator> JSONIFIER_ALWAYS_INLINE uint32_t hexToU32NoCheck(iterator string1) noexcept {
		return digitToVal32[630ull + string1[0]] | digitToVal32[420ull + string1[1]] | digitToVal32[210ull + string1[2]] | digitToVal32[0ull + string1[3]];
	}

	template<typename iterator> JSONIFIER_ALWAYS_INLINE uint32_t codePointToUtf8(uint32_t codePoint, iterator c) noexcept {
		static constexpr uint8_t utf8Table[4][4] = { { 0x00 }, { 0xC0, 0x80 }, { 0xE0, 0x80, 0x80 }, { 0xF0, 0x80, 0x80, 0x80 } };

		if (codePoint <= 0x7F) {
			c[0] = static_cast<char>(codePoint);
			return 1;
		}
		uint32_t leadingZeros = simd_internal::lzcnt(codePoint);
		uint32_t numBytes	  = (31u - leadingZeros) / 5u + 1u;
		uint32_t highBitsMask = (1u << (6u * numBytes)) - 1u;
		uint32_t utf8HighBits = simd_internal::pdep(codePoint, highBitsMask);
		std::memcpy(c, utf8Table[numBytes - 1], numBytes);
		for (uint32_t i = 0; i < numBytes; ++i) {
			c[i] |= uint8_t(utf8HighBits & 0xFF);
			utf8HighBits >>= 8;
		}
		return numBytes;
	}

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

	template<typename return_type> JSONIFIER_ALWAYS_INLINE constexpr return_type isLess32(return_type value) {
		constexpr return_type newBytes{ repeatByte<0b11100000u, return_type>() };
		return hasZero(value & newBytes);
	}

	template<typename simd_type, typename integer_type> JSONIFIER_ALWAYS_INLINE integer_type copyAndFindParse(const char* string1, char* string2, simd_type& simdValue) noexcept {
		simdValue = simd_internal::gatherValuesU<simd_type>(string1);
		std::memcpy(string2, string1, sizeof(simd_type));
		return tzcnt(static_cast<integer_type>(
			simd_internal::opCmpEq(simdValue, simd_internal::simdValue<'\\', simd_type>) | simd_internal::opCmpEq(simdValue, simd_internal::simdValue<'"', simd_type>)));
	}

	template<jsonifier::concepts::unsigned_type simd_type, jsonifier::concepts::unsigned_type integer_type>
	JSONIFIER_ALWAYS_INLINE integer_type copyAndFindParse(const char* string1, char* string2, simd_type& simdValue) noexcept {
		std::memcpy(&simdValue, string1, sizeof(simd_type));
		std::memcpy(string2, string1, sizeof(simd_type));
		static constexpr uint64_t mask{ repeatByte<0b01111111, integer_type>() };
		static constexpr uint64_t lowBitsMask{ repeatByte<0b10000000, integer_type>() };
		static constexpr auto quoteBits{ repeatByte<'"', integer_type>() };
		static constexpr auto bsBits{ repeatByte<'\\', integer_type>() };
		const uint64_t lo7		 = simdValue & mask;
		const uint64_t quote	 = (lo7 ^ quoteBits) + mask;
		const uint64_t backslash = (lo7 ^ bsBits) + mask;
		const uint64_t t0		 = ~((quote & backslash) | simdValue);
		uint64_t next			 = t0 & lowBitsMask;
		return static_cast<integer_type>(tzcnt(next) >> 3u);
	}

	template<typename simd_type, typename integer_type> JSONIFIER_ALWAYS_INLINE integer_type findParse(const char* string1, simd_type& simdValue) noexcept {
		simdValue = simd_internal::gatherValuesU<simd_type>(string1);
		return tzcnt(static_cast<integer_type>(
			simd_internal::opCmpEq(simdValue, simd_internal::simdValue<'\\', simd_type>) | simd_internal::opCmpEq(simdValue, simd_internal::simdValue<'"', simd_type>)));
	}

	template<jsonifier::concepts::unsigned_type simd_type, jsonifier::concepts::unsigned_type integer_type>
	JSONIFIER_ALWAYS_INLINE integer_type findParse(const char* string1, simd_type& simdValue) noexcept {
		std::memcpy(&simdValue, string1, sizeof(simd_type));
		static constexpr uint64_t mask{ repeatByte<0b01111111, integer_type>() };
		static constexpr uint64_t lowBitsMask{ repeatByte<0b10000000, integer_type>() };
		static constexpr auto quoteBits{ repeatByte<'"', integer_type>() };
		static constexpr auto bsBits{ repeatByte<'\\', integer_type>() };
		const uint64_t lo7		 = simdValue & mask;
		const uint64_t quote	 = (lo7 ^ quoteBits) + mask;
		const uint64_t backslash = (lo7 ^ bsBits) + mask;
		const uint64_t t0		 = ~((quote & backslash) | simdValue);
		uint64_t next			 = t0 & lowBitsMask;
		return static_cast<integer_type>(tzcnt(next) >> 3u);
	}

	template<typename simd_type, typename integer_type>
	JSONIFIER_ALWAYS_INLINE integer_type copyAndFindSerialize(const char* string1, char* string2, simd_type& simdValue) noexcept {
		simdValue = simd_internal::gatherValuesU<simd_type>(string1);
		std::memcpy(string2, string1, sizeof(simd_type));
		return tzcnt(static_cast<integer_type>(
			simd_internal::opCmpEq(simd_internal::opShuffle(simd_internal::simdValues<simd_internal::escapeableArray01<bytesPerStep>, simd_type>, simdValue), simdValue) |
			simd_internal::opCmpEq(simd_internal::opShuffle(simd_internal::simdValues<simd_internal::escapeableArray00<bytesPerStep>, simd_type>, simdValue), simdValue)));
	}

	template<jsonifier::concepts::unsigned_type simd_type, jsonifier::concepts::unsigned_type integer_type>
	JSONIFIER_ALWAYS_INLINE integer_type copyAndFindSerialize(const char* string1, char* string2, simd_type& simdValue) noexcept {
		std::memcpy(&simdValue, string1, sizeof(simd_type));
		std::memcpy(string2, string1, sizeof(simd_type));
		static constexpr uint64_t mask{ repeatByte<0b01111111, integer_type>() };
		static constexpr uint64_t lowBitsMask{ repeatByte<0b10000000, integer_type>() };
		static constexpr uint64_t midBitsMask{ repeatByte<0b01100000, integer_type>() };
		static constexpr auto quoteBits{ repeatByte<'"', integer_type>() };
		static constexpr auto bsBits{ repeatByte<'\\', integer_type>() };
		const uint64_t lo7		 = simdValue & mask;
		const uint64_t quote	 = (lo7 ^ quoteBits) + mask;
		const uint64_t backslash = (lo7 ^ bsBits) + mask;
		const uint64_t less_32	 = (simdValue & midBitsMask) + mask;
		const uint64_t t0		 = ~((quote & backslash & less_32) | simdValue);
		uint64_t next			 = t0 & lowBitsMask;
		return static_cast<integer_type>(tzcnt(next) >> 3u);
	}

	template<typename iterator_type01> JSONIFIER_ALWAYS_INLINE void skipShortStringImpl(iterator_type01& string1, uint64_t& lengthNew) noexcept {
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

	template<typename iterator_type01> JSONIFIER_ALWAYS_INLINE void skipStringImpl(iterator_type01& string1, uint64_t& lengthNew) noexcept {
		using char_type01 =
			typename std::conditional_t<std::is_pointer_v<iterator_type01>, std::remove_pointer_t<iterator_type01>, typename std::iterator_traits<iterator_type01>::value_type>;
		std::remove_const_t<char_type01> escapeChar;
		constexpr char quotesValue{ static_cast<char>('"') };
#if JSONIFIER_CHECK_FOR_AVX(JSONIFIER_AVX512)
		{
			using integer_type						 = typename get_type_at_index<simd_internal::avx_integer_list, 3>::type::integer_type;
			using simd_type							 = typename get_type_at_index<simd_internal::avx_integer_list, 3>::type::type;
			static constexpr uint64_t bytesProcessed = get_type_at_index<simd_internal::avx_integer_list, 3>::type::bytesProcessed;
			static constexpr integer_type mask		 = get_type_at_index<simd_internal::avx_integer_list, 3>::type::mask;
			simd_type collectionValue;
			integer_type nextBackslashOrQuote;
			while (static_cast<int64_t>(lengthNew) >= static_cast<int64_t>(bytesProcessed)) {
				nextBackslashOrQuote = findParse<simd_type, integer_type>(string1, collectionValue);
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
			using integer_type						 = typename get_type_at_index<simd_internal::avx_integer_list, 2>::type::integer_type;
			using simd_type							 = typename get_type_at_index<simd_internal::avx_integer_list, 2>::type::type;
			static constexpr uint64_t bytesProcessed = get_type_at_index<simd_internal::avx_integer_list, 2>::type::bytesProcessed;
			static constexpr integer_type mask		 = get_type_at_index<simd_internal::avx_integer_list, 2>::type::mask;
			simd_type collectionValue;
			integer_type nextBackslashOrQuote;
			while (static_cast<int64_t>(lengthNew) >= static_cast<int64_t>(bytesProcessed)) {
				nextBackslashOrQuote = findParse<simd_type, integer_type>(string1, collectionValue);
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
			using integer_type						 = typename get_type_at_index<simd_internal::avx_integer_list, 1>::type::integer_type;
			using simd_type							 = typename get_type_at_index<simd_internal::avx_integer_list, 1>::type::type;
			static constexpr uint64_t bytesProcessed = get_type_at_index<simd_internal::avx_integer_list, 1>::type::bytesProcessed;
			static constexpr integer_type mask		 = get_type_at_index<simd_internal::avx_integer_list, 1>::type::mask;
			simd_type collectionValue;
			integer_type nextBackslashOrQuote;
			while (static_cast<int64_t>(lengthNew) >= static_cast<int64_t>(bytesProcessed)) {
				nextBackslashOrQuote = findParse<simd_type, integer_type>(string1, collectionValue);
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
			using integer_type						 = typename get_type_at_index<simd_internal::avx_integer_list, 0>::type::integer_type;
			using simd_type							 = typename get_type_at_index<simd_internal::avx_integer_list, 0>::type::type;
			static constexpr uint64_t bytesProcessed = get_type_at_index<simd_internal::avx_integer_list, 0>::type::bytesProcessed;
			static constexpr integer_type mask		 = get_type_at_index<simd_internal::avx_integer_list, 0>::type::mask;
			simd_type collectionValue;
			integer_type nextBackslashOrQuote;
			while (static_cast<int64_t>(lengthNew) >= static_cast<int64_t>(bytesProcessed)) {
				nextBackslashOrQuote = findParse<simd_type, integer_type>(string1, collectionValue);
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
	JSONIFIER_ALWAYS_INLINE iterator_type02 parseShortStringImpl(iterator_type01& string1, iterator_type02 string2, uint64_t lengthNew) noexcept {
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
						escapeChar = escapeMap<char>[static_cast<uint8_t>(escapeChar)];
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
	JSONIFIER_ALWAYS_INLINE iterator_type02 parseStringImpl(iterator_type01& string1, iterator_type02 string2, uint64_t lengthNew) noexcept {
		using char_type01 =
			typename std::conditional_t<std::is_pointer_v<iterator_type01>, std::remove_pointer_t<iterator_type01>, typename std::iterator_traits<iterator_type01>::value_type>;
		using char_type02 =
			typename std::conditional_t<std::is_pointer_v<iterator_type02>, std::remove_pointer_t<iterator_type02>, typename std::iterator_traits<iterator_type02>::value_type>;
		std::remove_const_t<char_type01> escapeChar;
#if JSONIFIER_CHECK_FOR_AVX(JSONIFIER_AVX512)
		{
			using integer_type						 = typename get_type_at_index<simd_internal::avx_integer_list, 3>::type::integer_type;
			using simd_type							 = typename get_type_at_index<simd_internal::avx_integer_list, 3>::type::type;
			static constexpr uint64_t bytesProcessed = get_type_at_index<simd_internal::avx_integer_list, 3>::type::bytesProcessed;
			static constexpr integer_type mask		 = get_type_at_index<simd_internal::avx_integer_list, 3>::type::mask;
			simd_type collectionValue;
			integer_type nextBackslashOrQuote;
			while (static_cast<int64_t>(lengthNew) >= static_cast<int64_t>(bytesProcessed)) {
				nextBackslashOrQuote = copyAndFindParse<simd_type, integer_type>(string1, string2, collectionValue);
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
			using integer_type						 = typename get_type_at_index<simd_internal::avx_integer_list, 2>::type::integer_type;
			using simd_type							 = typename get_type_at_index<simd_internal::avx_integer_list, 2>::type::type;
			static constexpr uint64_t bytesProcessed = get_type_at_index<simd_internal::avx_integer_list, 2>::type::bytesProcessed;
			static constexpr integer_type mask		 = get_type_at_index<simd_internal::avx_integer_list, 2>::type::mask;
			simd_type collectionValue;
			integer_type nextBackslashOrQuote;
			while (static_cast<int64_t>(lengthNew) >= static_cast<int64_t>(bytesProcessed)) {
				nextBackslashOrQuote = copyAndFindParse<simd_type, integer_type>(string1, string2, collectionValue);
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
			using integer_type						 = typename get_type_at_index<simd_internal::avx_integer_list, 1>::type::integer_type;
			using simd_type							 = typename get_type_at_index<simd_internal::avx_integer_list, 1>::type::type;
			static constexpr uint64_t bytesProcessed = get_type_at_index<simd_internal::avx_integer_list, 1>::type::bytesProcessed;
			static constexpr integer_type mask		 = get_type_at_index<simd_internal::avx_integer_list, 1>::type::mask;
			simd_type collectionValue;
			integer_type nextBackslashOrQuote;
			while (static_cast<int64_t>(lengthNew) >= static_cast<int64_t>(bytesProcessed)) {
				nextBackslashOrQuote = copyAndFindParse<simd_type, integer_type>(string1, string2, collectionValue);
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
			using integer_type						 = typename get_type_at_index<simd_internal::avx_integer_list, 0>::type::integer_type;
			using simd_type							 = typename get_type_at_index<simd_internal::avx_integer_list, 0>::type::type;
			static constexpr uint64_t bytesProcessed = get_type_at_index<simd_internal::avx_integer_list, 0>::type::bytesProcessed;
			static constexpr integer_type mask		 = get_type_at_index<simd_internal::avx_integer_list, 0>::type::mask;
			simd_type collectionValue;
			integer_type nextBackslashOrQuote;
			while (static_cast<int64_t>(lengthNew) >= static_cast<int64_t>(bytesProcessed)) {
				nextBackslashOrQuote = copyAndFindParse<simd_type, integer_type>(string1, string2, collectionValue);
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
	JSONIFIER_ALWAYS_INLINE void serializeShortStringImpl(iterator_type01 string1, iterator_type02& string2, uint64_t lengthNew) noexcept {
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
	JSONIFIER_ALWAYS_INLINE void serializeStringImpl(iterator_type01 string1, iterator_type02& string2, uint64_t lengthNew) noexcept {
		uint16_t escapeChar;
#if JSONIFIER_CHECK_FOR_AVX(JSONIFIER_AVX512)
		{
			using integer_type						 = typename get_type_at_index<simd_internal::avx_integer_list, 3>::type::integer_type;
			using simd_type							 = typename get_type_at_index<simd_internal::avx_integer_list, 3>::type::type;
			static constexpr uint64_t bytesProcessed = get_type_at_index<simd_internal::avx_integer_list, 3>::type::bytesProcessed;
			static constexpr integer_type mask		 = get_type_at_index<simd_internal::avx_integer_list, 3>::type::mask;
			simd_type collectionValue;
			integer_type nextEscapeable;
			while (static_cast<int64_t>(lengthNew) >= static_cast<int64_t>(bytesProcessed)) {
				nextEscapeable = copyAndFindSerialize<simd_type, integer_type>(string1, string2, collectionValue, simdValues00, simdValues01);
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
			using integer_type						 = typename get_type_at_index<simd_internal::avx_integer_list, 2>::type::integer_type;
			using simd_type							 = typename get_type_at_index<simd_internal::avx_integer_list, 2>::type::type;
			static constexpr uint64_t bytesProcessed = get_type_at_index<simd_internal::avx_integer_list, 2>::type::bytesProcessed;
			static constexpr integer_type mask		 = get_type_at_index<simd_internal::avx_integer_list, 2>::type::mask;
			simd_type collectionValue;
			integer_type nextEscapeable;
			while (static_cast<int64_t>(lengthNew) >= static_cast<int64_t>(bytesProcessed)) {
				nextEscapeable = copyAndFindSerialize<simd_type, integer_type>(string1, string2, collectionValue);
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
			using integer_type						 = typename get_type_at_index<simd_internal::avx_integer_list, 1>::type::integer_type;
			using simd_type							 = typename get_type_at_index<simd_internal::avx_integer_list, 1>::type::type;
			static constexpr uint64_t bytesProcessed = get_type_at_index<simd_internal::avx_integer_list, 1>::type::bytesProcessed;
			static constexpr integer_type mask		 = get_type_at_index<simd_internal::avx_integer_list, 1>::type::mask;
			simd_type collectionValue;
			integer_type nextEscapeable;
			while (static_cast<int64_t>(lengthNew) >= static_cast<int64_t>(bytesProcessed)) {
				nextEscapeable = copyAndFindSerialize<simd_type, integer_type>(string1, string2, collectionValue);
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
			using integer_type						 = typename get_type_at_index<simd_internal::avx_integer_list, 0>::type::integer_type;
			using simd_type							 = typename get_type_at_index<simd_internal::avx_integer_list, 0>::type::type;
			static constexpr uint64_t bytesProcessed = get_type_at_index<simd_internal::avx_integer_list, 0>::type::bytesProcessed;
			static constexpr integer_type mask		 = get_type_at_index<simd_internal::avx_integer_list, 0>::type::mask;
			simd_type collectionValue;
			integer_type nextEscapeable;
			while (static_cast<int64_t>(lengthNew) >= static_cast<int64_t>(bytesProcessed)) {
				nextEscapeable = copyAndFindSerialize<simd_type, integer_type>(string1, string2, collectionValue);
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

	template<uint64_t length> struct convert_length_to_int {
		static_assert(length <= 8, "Sorry, but that string is too long!");
		using type = std::conditional_t<length == 1, uint8_t,
			std::conditional_t<length <= 2, uint16_t, std::conditional_t<length <= 4, uint32_t, std::conditional_t<length <= 8, uint64_t, void>>>>;
	};

	template<uint64_t length> using convert_length_to_int_t = typename convert_length_to_int<length>::type;

	template<typename char_type, string_literal string> JSONIFIER_ALWAYS_INLINE constexpr convert_length_to_int_t<string.size()> getStringAsInt() noexcept {
		const char_type* stringNew = string.data();
		convert_length_to_int_t<string.size()> returnValue{};
		for (uint64_t x = 0; x < string.size(); ++x) {
			returnValue |= static_cast<convert_length_to_int_t<string.size()>>(stringNew[x]) << x * 8;
		}
		return returnValue;
	}

	template<string_literal string, typename char_type> JSONIFIER_ALWAYS_INLINE bool compareStringAsInt(const char_type* iter) {
		static constexpr auto newString{ getStringAsInt<char_type, string>() };
		convert_length_to_int_t<string.size()> newerString{};
		std::memcpy(&newerString, iter, string.size());
		return newString == newerString;
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

	template<jsonifier::concepts::is_double_ptr iterator, jsonifier::concepts::bool_t bool_type> JSONIFIER_ALWAYS_INLINE bool parseBool(bool_type& value, iterator& iter) {
		if (compareStringAsInt<"true">(*iter)) {
			value = true;
			++iter;
			return true;
		} else {
			if (compareStringAsInt<"false">(*iter)) [[likely]] {
				value = false;
				++iter;
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

	template<jsonifier::concepts::is_double_ptr iterator> JSONIFIER_ALWAYS_INLINE bool parseNull(iterator& iter) {
		if (compareStringAsInt<"null">(*iter)) [[likely]] {
			++iter;
			return true;
		} else {
			return false;
		}
	}

	template<const auto& options, typename value_type, jsonifier::concepts::is_double_ptr iterator>
	JSONIFIER_ALWAYS_INLINE void parseString(value_type&& value, iterator& iter, iterator& end, jsonifier::vector<error>& errors) noexcept {
		auto newPtr = *iter;
		if (*newPtr == 0x22u) [[likely]] {
			++iter;
		} else {
			static constexpr auto sourceLocation{ std::source_location::current() };
			errors.emplace_back(error::constructError<sourceLocation, error_classes::Parsing, parse_errors::Missing_String_Start>(getUnderlyingPtr(iter) - options.rootIter,
				getUnderlyingPtr(end) - options.rootIter, options.rootIter));
			skipToNextValue(iter, end);
			return;
		}
		auto newSize = static_cast<uint64_t>((*iter) - newPtr);
		if (static_cast<int64_t>(newSize) > 1) [[likely]] {
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
				std::copy(newString.data(), newString.data() + newSize, value.data());
			} else {
				static constexpr auto sourceLocation{ std::source_location::current() };
				errors.emplace_back(error::constructError<sourceLocation, error_classes::Parsing, parse_errors::Invalid_String_Characters>(
					getUnderlyingPtr(iter) - options.rootIter, getUnderlyingPtr(end) - options.rootIter, options.rootIter));
				skipToNextValue(iter, end);
				return;
			}
		}
	}

	template<const auto& options, typename value_type, typename iterator>
	JSONIFIER_ALWAYS_INLINE void parseString(value_type&& value, iterator& iter, iterator& end, jsonifier::vector<error>& errors) noexcept {
		if (*iter == '"') [[likely]] {
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
			std::copy(newString.data(), newString.data() + newSize, value.data());
		} else {
			static constexpr auto sourceLocation{ std::source_location::current() };
			errors.emplace_back(error::constructError<sourceLocation, error_classes::Parsing, parse_errors::Invalid_String_Characters>(iter - options.rootIter,
				end - options.rootIter, options.rootIter));
			return;
		}
	}

	template<typename iterator> JSONIFIER_ALWAYS_INLINE void skipString(iterator& iter, iterator& end) noexcept {
		++iter;
		auto newLength = static_cast<uint64_t>(end - iter);
		skipStringImpl(iter, newLength);
	}

	template<typename iterator> JSONIFIER_ALWAYS_INLINE void skipToEndOfValue(iterator& iter, iterator& end) noexcept {
		uint64_t currentDepth{ 1 };
		auto skipToEnd = [&]() {
			while (iter != end && currentDepth > 0) {
				switch (*iter) {
					[[unlikely]] case '[':
					[[unlikely]] case '{': {
						++currentDepth;
						++iter;
						break;
					}
					[[unlikely]] case ']':
					[[unlikely]] case '}': {
						--currentDepth;
						++iter;
						break;
					}
					case '"': {
						skipString(iter, end);
						break;
					}
					default: {
						++iter;
						break;
					}
				}
			}
		};
		switch (*iter) {
			[[unlikely]] case '[':
			[[unlikely]] case '{': {
				++iter;
				skipToEnd();
				break;
			}
			case '"': {
				skipString(iter, end);
				break;
			}
			case ':': {
				++iter;
				skipToEndOfValue(iter, end);
				break;
			}
			case 't': {
				iter += 4;
				break;
			}
			case 'f': {
				iter += 5;
				break;
			}
			case 'n': {
				iter += 4;
				break;
			}
			case '-':
			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9': {
				skipNumber(iter, end);
				break;
			}
			[[likely]] default: {
				++iter;
				break;
			}
		}
	}

	template<typename iterator> JSONIFIER_INLINE void skipObject(iterator& iter, iterator& end) noexcept {
		++iter;
		if (*iter == '}') {
			++iter;
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
		++iter;
	}

	template<typename iterator> JSONIFIER_INLINE void skipArray(iterator& iter, iterator& end) noexcept {
		++iter;
		if (*iter == ']') {
			++iter;
			return;
		}
		while (true) {
			skipToNextValue(iter, end);
			if (*iter != ',') {
				break;
			}
			++iter;
		}
		++iter;
	}

	template<typename iterator> JSONIFIER_INLINE void skipToNextValue(iterator& iter, iterator& end) noexcept {
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
			case '\0': {
				break;
			}
			default: {
				skipNumber(iter, end);
			}
		}
	}

	template<char startChar, char endChar, typename iterator> JSONIFIER_ALWAYS_INLINE uint64_t countValueElements(iterator iter, iterator end) noexcept {
		auto newValue = *iter;
		if (newValue == ']' || newValue == '}') [[unlikely]] {
			return 0;
		}
		uint64_t currentCount{ 1 };
		while (iter != end) {
			switch (*iter) {
				[[unlikely]] case ',': {
					++currentCount;
					++iter;
					break;
				}
				[[unlikely]] case '{':
				[[unlikely]] case '[': {
					skipToEndOfValue(iter, end);
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

	struct key_stats_t {
		uint64_t minLength{ (std::numeric_limits<uint64_t>::max)() };
		uint64_t lengthRange{};
		uint64_t maxLength{};
	};

	template<typename value_type, size_t I> JSONIFIER_ALWAYS_INLINE constexpr const jsonifier::string_view& getKey() noexcept {
		return std::get<I>(jsonifier::concepts::coreV<value_type>).view();
	}

	template<typename value_type, uint64_t maxIndex, uint64_t index> JSONIFIER_ALWAYS_INLINE constexpr auto keyStatsImpl(key_stats_t stats) noexcept {
		if constexpr (index < maxIndex) {
			constexpr const jsonifier::string_view& key{ getKey<value_type, index>() };
			constexpr auto n{ key.size() };
			if (n < stats.minLength) {
				stats.minLength = n;
			}
			if (n > stats.maxLength) {
				stats.maxLength = n;
			}
			return keyStatsImpl<value_type, maxIndex, index + 1>(stats);
		} else {
			if constexpr (maxIndex > 0) {
				stats.lengthRange = stats.maxLength - stats.minLength;
			}
			return stats;
		}
	}

	template<typename value_type> JSONIFIER_ALWAYS_INLINE constexpr auto keyStats() noexcept {
		constexpr auto N{ std::tuple_size_v<jsonifier::concepts::core_t<value_type>> };

		return keyStatsImpl<value_type, N, 0>(key_stats_t{});
	}

	template<const auto& options, typename value_type, typename iterator>
	JSONIFIER_ALWAYS_INLINE uint64_t getKeyLength(iterator iter, iterator& end, jsonifier::vector<error>& errors) noexcept {
		if (*iter == '"') [[likely]] {
			++iter;
		} else {
			static constexpr auto sourceLocation{ std::source_location::current() };
			errors.emplace_back(
				error::constructError<sourceLocation, error_classes::Parsing, parse_errors::Missing_String_Start>(iter - options.rootIter, end - iter, options.rootIter));
			return {};
		}
		static constexpr auto N{ std::tuple_size_v<jsonifier::concepts::core_t<value_type>> };
		static constexpr auto keyStatsVal{ keyStats<value_type>() };
		static constexpr auto minLength{ keyStatsVal.minLength };
		static constexpr auto maxLength{ keyStatsVal.maxLength };

		if constexpr (N == 1) {
			static constexpr jsonifier::string_view key{ std::get<0>(jsonifier::concepts::coreV<value_type>).view() };
			return key.size();
		} else {
			auto start = iter;
			iter += minLength;
			memchar<'"'>(iter, static_cast<uint64_t>((iter + maxLength) - iter));
			return size_t(iter - start);
		}
	}

	template<const auto& options, typename value_type, jsonifier::concepts::is_double_ptr iterator>
	JSONIFIER_ALWAYS_INLINE uint64_t getKeyLength(iterator iter, iterator& end, jsonifier::vector<error>& errors) noexcept {
		const auto start{ *iter };

		if (*iter == '"') [[likely]] {
			++iter;
		} else {
			static constexpr auto sourceLocation{ std::source_location::current() };
			errors.emplace_back(
				error::constructError<sourceLocation, error_classes::Parsing, parse_errors::Missing_String_Start>(iter - options.rootIter, end - iter, options.rootIter));
			return {};
		}

		return static_cast<uint64_t>((*iter) - (start + 2));
	}

}// namespace jsonifier_internal