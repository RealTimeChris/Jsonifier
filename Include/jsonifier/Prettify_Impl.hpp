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

#include <jsonifier/Prettifier.hpp>

namespace jsonifier_internal {

	template<bool newLinesInArray, bool tabs, uint64_t indentSize, uint64_t maxDepth, jsonifier::concepts::string_t string_type, jsonifier::concepts::is_fwd_iterator iterator_type>
	JSONIFIER_INLINE uint64_t prettify::impl(iterator_type&& iter, string_type& out) noexcept {
		ascii_classes previousStructural[maxDepth]{ ascii_classes::lcurb };
		int64_t currentDistance{};
		auto outPtr = out.data();
		int64_t indent{};

		auto appendNewLine = [&]() {
			*outPtr = 0x0Au;
			++outPtr;
			std::fill(outPtr, outPtr + (tabs ? indent : (indent * indentSize)), static_cast<typename string_type::value_type>(tabs ? 0x09u : 0x20u));
			outPtr += tabs ? indent : (indent * indentSize);
		};

		auto appendCharacter = [&](auto character) {
			*outPtr = static_cast<typename string_type::value_type>(character);
			++outPtr;
		};

		appendCharacter(*iter);

		++indent;
		previousStructural[indent] = asciiClassesMap[*iter];
		if (previousStructural[indent] != ascii_classes::lsqrb && previousStructural[indent] != ascii_classes::lcurb) {
			iter.getErrors().emplace_back(createError<error_code::Prettify_Error>(iter));
			return std::numeric_limits<uint64_t>::max();
		}
		appendNewLine();
		auto previousVal = iter.operator->();
		++iter;
		auto currentVal = iter.operator->();
		++iter;
		auto nextVal = iter.operator->();

		auto updatePointers = [&]() {
			previousVal = currentVal;
			currentVal	= nextVal;
			++iter;
			nextVal = iter.operator->();
		};

		auto appendValues = [&]() {
			if (currentDistance > 0) [[likely]] {
				std::memcpy(outPtr, currentVal, static_cast<uint64_t>(currentDistance));
				outPtr += currentDistance;
			}
		};

		while (iter && currentVal && previousVal && nextVal) {
			currentDistance = nextVal - currentVal;
			switch (asciiClassesMap[*currentVal]) {
				[[likely]] case ascii_classes::quote : {
					appendValues();
					break;
				}
				[[unlikely]] case ascii_classes::colon : {
					appendCharacter(*currentVal);
					appendCharacter(0x20u);
					break;
				}
				[[unlikely]] case ascii_classes::comma : {
					appendCharacter(*currentVal);
					if constexpr (!newLinesInArray) {
						if (previousStructural[indent] != ascii_classes::lsqrb) {
							appendNewLine();
						}
					} else {
						appendNewLine();
					}
					break;
				}
				[[unlikely]] case ascii_classes::lsqrb : {
					appendCharacter(*currentVal);
					++indent;
					if (indent >= maxDepth) {
						iter.getErrors().emplace_back(createError<error_code::Prettify_Error>(iter));
						return std::numeric_limits<uint64_t>::max();
					}
					if constexpr (!newLinesInArray) {
						previousStructural[indent] = ascii_classes::lsqrb;
					}
					if (*nextVal != 0x5Du) {
						appendNewLine();
					}
					break;
				}
				[[unlikely]] case ascii_classes::rsqrb : {
					--indent;
					if (indent < 0) {
						iter.getErrors().emplace_back(createError<error_code::Prettify_Error>(iter));
						return std::numeric_limits<uint64_t>::max();
					}
					if (*previousVal != 0x5Bu) {
						appendNewLine();
					}
					appendCharacter(*currentVal);
					break;
				}
				[[unlikely]] case ascii_classes::false_val : {
					std::memcpy(outPtr, falseString.data(), falseString.size());
					outPtr += falseString.size();
					break;
				}
				[[unlikely]] case ascii_classes::true_val : {
					std::memcpy(outPtr, trueString.data(), trueString.size());
					outPtr += trueString.size();
					break;
				}
				[[unlikely]] case ascii_classes::null_val : {
					std::memcpy(outPtr, nullString.data(), nullString.size());
					outPtr += nullString.size();
					break;
				}
				[[unlikely]] case ascii_classes::lcurb : {
					appendCharacter(*currentVal);
					++indent;
					if (indent >= maxDepth) {
						iter.getErrors().emplace_back(createError<error_code::Prettify_Error>(iter));
						return std::numeric_limits<uint64_t>::max();
					}
					if constexpr (!newLinesInArray) {
						previousStructural[indent] = ascii_classes::lcurb;
					}
					if (*nextVal != 0x7Du) {
						appendNewLine();
					}
					break;
				}
				[[unlikely]] case ascii_classes::rcurb : {
					--indent;
					if (indent < 0) {
						iter.getErrors().emplace_back(createError<error_code::Prettify_Error>(iter));
						return std::numeric_limits<uint64_t>::max();
					}
					if (*previousVal != 0x7Bu) {
						appendNewLine();
					}
					appendCharacter(*currentVal);
					break;
				}
				[[unlikely]] case ascii_classes::num_val : {
					appendValues();
					break;
				}
				[[unlikely]] case ascii_classes::class_count:
					[[fallthrough]];
				[[unlikely]] case ascii_classes::errorVal:
					[[fallthrough]];
					[[unlikely]] default : {
						iter.getErrors().emplace_back(createError<error_code::Prettify_Error>(iter));
						return std::numeric_limits<uint64_t>::max();
					}
			}
			updatePointers();
		}
		if (currentVal && previousVal) [[likely]] {
			--indent;
			appendNewLine();
			appendCharacter(*currentVal);
		}
		return static_cast<uint64_t>(outPtr - out.data());
	}

}// namespace jsonifier_internal
