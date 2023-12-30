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

	template<bool tabs, uint64_t indentSize, typename iterator_type> void appendNewLine(iterator_type& outPtr, int64_t indent) {
		*outPtr = 0x0Au;
		++outPtr;
		std::fill(outPtr, outPtr + (tabs ? indent : (indent * indentSize)),
			static_cast<std::remove_pointer_t<jsonifier::concepts::unwrap_t<decltype(outPtr)>>>(tabs ? 0x09u : 0x20u));
		outPtr += tabs ? indent : (indent * indentSize);
	};

	template<typename iterator_type01, typename iterator_type02>
	void updatePointers(iterator_type01& iter, iterator_type02& previousVal, iterator_type02& nextVal, iterator_type02& currentVal) {
		previousVal = currentVal;
		currentVal	= nextVal;
		++iter;
		nextVal = iter.operator->();
	};

	template<typename iterator_type01, typename iterator_type02> void appendValues(int64_t currentDistance, iterator_type01& outPtr, iterator_type02& currentVal) {
		if (currentDistance > 0) [[likely]] {
			std::memcpy(outPtr, currentVal, static_cast<uint64_t>(currentDistance));
			outPtr += currentDistance;
		}
	};

	template<typename derived_type> struct prettify_impl {
		template<bool newLinesInArray, bool tabs, uint64_t indentSize, uint64_t maxDepth, jsonifier::concepts::string_t string_type,
			jsonifier::concepts::is_fwd_iterator iterator_type>
		JSONIFIER_INLINE static uint64_t impl(iterator_type&& iter, string_type& out) noexcept {
			json_structural_type previousStructural[maxDepth]{ json_structural_type::Object_Start };
			int64_t currentDistance{};
			auto outPtr = out.data();
			int64_t indent{};

			appendCharacter(*iter, outPtr);
			previousStructural[indent] = asciiClassesMap[*iter];
			if (previousStructural[indent] != json_structural_type::Array_Start && previousStructural[indent] != json_structural_type::Object_Start) {
				iter.getErrors().emplace_back(createError<error_code::Prettify_Error>(iter));
				return std::numeric_limits<uint64_t>::max();
			}
			++indent;
			previousStructural[indent] = asciiClassesMap[*iter];
			if constexpr (newLinesInArray) {
				if (*iter == json_structural_type::Object_Start) {
					appendNewLine<tabs, indentSize>(outPtr, indent);
				}
			} else {
				appendCharacter(' ', outPtr);
			}
			auto previousVal = iter.operator->();
			++iter;
			auto currentVal = iter.operator->();
			++iter;
			auto nextVal = iter.operator->();

			while (iter && currentVal && previousVal && nextVal) {
				currentDistance = nextVal - currentVal;
				switch (asciiClassesMap[*currentVal]) {
					[[likely]] case json_structural_type::String : {
						appendValues(currentDistance, outPtr, currentVal);
						break;
					}
					[[unlikely]] case json_structural_type::Comma : {
						appendCharacter(*currentVal, outPtr);
						if constexpr (!newLinesInArray) {
							if (previousStructural[indent] != json_structural_type::Array_Start) {
								appendNewLine<tabs, indentSize>(outPtr, indent);
							} else {
								appendCharacter(' ', outPtr);
							}
						} else {
							appendNewLine<tabs, indentSize>(outPtr, indent);
						}
						break;
					}
					[[unlikely]] case json_structural_type::Number : {
						appendValues(currentDistance, outPtr, currentVal);
						break;
					}
					[[unlikely]] case json_structural_type::Colon : {
						appendCharacter(*currentVal, outPtr);
						appendCharacter(0x20u, outPtr);
						break;
					}
					[[unlikely]] case json_structural_type::Array_Start : {
						appendCharacter(*currentVal, outPtr);
						++indent;
						previousStructural[indent] = json_structural_type::Array_Start;
						if (indent >= maxDepth) {
							iter.getErrors().emplace_back(createError<error_code::Prettify_Error>(iter));
							return std::numeric_limits<uint64_t>::max();
						}
						if (*nextVal != 0x5Du && newLinesInArray) {
							appendNewLine<tabs, indentSize>(outPtr, indent);
						} else {
							appendCharacter(' ', outPtr);
						}
						break;
					}
					[[unlikely]] case json_structural_type::Array_End : {
						--indent;
						if (indent < 0) {
							iter.getErrors().emplace_back(createError<error_code::Prettify_Error>(iter));
							return std::numeric_limits<uint64_t>::max();
						}
						if (*previousVal != 0x5Bu && newLinesInArray) {
							appendNewLine<tabs, indentSize>(outPtr, indent);
						} else {
							appendCharacter(' ', outPtr);
						}
						appendCharacter(*currentVal, outPtr);
						break;
					}
					[[unlikely]] case json_structural_type::Null : {
						std::memcpy(outPtr, nullString.data(), nullString.size());
						outPtr += nullString.size();
						break;
					}
					[[unlikely]] case json_structural_type::Bool : {
						if (*currentVal == 't') {
							std::memcpy(outPtr, trueString.data(), trueString.size());
							outPtr += trueString.size();
							break;
						} else {
							std::memcpy(outPtr, falseString.data(), falseString.size());
							outPtr += falseString.size();
							break;
						}
					}
					[[unlikely]] case json_structural_type::Object_Start : {
						appendCharacter(*currentVal, outPtr);
						++indent;
						previousStructural[indent] = json_structural_type::Object_Start;
						if (indent >= maxDepth) {
							iter.getErrors().emplace_back(createError<error_code::Prettify_Error>(iter));
							return std::numeric_limits<uint64_t>::max();
						}
						if (*nextVal != 0x7Du) {
							appendNewLine<tabs, indentSize>(outPtr, indent);
						}
						break;
					}
					[[unlikely]] case json_structural_type::Object_End : {
						--indent;
						if (indent < 0) {
							iter.getErrors().emplace_back(createError<error_code::Prettify_Error>(iter));
							return std::numeric_limits<uint64_t>::max();
						}
						if (*previousVal != 0x7Bu) {
							appendNewLine<tabs, indentSize>(outPtr, indent);
						}
						appendCharacter(*currentVal, outPtr);
						break;
					}
					[[unlikely]] case json_structural_type::Unset:
						[[fallthrough]];
						[[unlikely]] default : {
							iter.getErrors().emplace_back(createError<error_code::Prettify_Error>(iter));
							return std::numeric_limits<uint64_t>::max();
						}
				}
				updatePointers(iter, previousVal, nextVal, currentVal);
			}
			if (currentVal && previousVal) [[likely]] {
				--indent;
				if constexpr (newLinesInArray) {
					if (*iter == json_structural_type::Object_End) {
						appendNewLine<tabs, indentSize>(outPtr, indent);
					}
				} else {
					appendCharacter(' ', outPtr);
				}
				appendCharacter(*currentVal, outPtr);
			}
			return static_cast<uint64_t>(outPtr - out.data());
		}
	};

}// namespace jsonifier_internal
