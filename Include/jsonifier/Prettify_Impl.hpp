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
		appendCharacter<0x0Au>(outPtr);
		if constexpr (tabs) {
			std::memset(outPtr, 0x09, indent);
			outPtr += indent;
		} else {
			std::memset(outPtr, 0x20u, indent * indentSize);
			outPtr += indent * indentSize;
		}
	};

	template<typename iterator_type01, typename iterator_type02> void appendValues(int64_t currentDistance, iterator_type01& outPtr, iterator_type02 currentVal) {
		if (currentDistance > 0) [[likely]] {
			std::memcpy(outPtr, currentVal, static_cast<uint64_t>(currentDistance));
			outPtr += currentDistance;
		}
	}

	template<typename derived_type> struct prettify_impl {
		template<bool newLinesInArray, bool tabs, uint64_t indentSize, uint64_t maxDepth, jsonifier::concepts::string_t string_type,
			jsonifier::concepts::is_fwd_iterator iterator_type>
		JSONIFIER_INLINE static uint64_t impl(iterator_type&& iter, string_type& out) noexcept {
			json_structural_type state[maxDepth]{};
			int64_t indent{};
			auto outPtr = out.data();

			while (iter) {
				switch (asciiClassesMap[*iter]) {
					case json_structural_type::String: {
						auto valueNew = iter.operator->();
						++iter;
						appendValues(iter.operator->() - valueNew, outPtr, valueNew);
						break;
					}
					case json_structural_type::Comma: {
						appendCharacter<0x2Cu>(outPtr);
						++iter;
						if constexpr (newLinesInArray) {
							appendNewLine<tabs, indentSize>(outPtr, indent);
						} else {
							if (state[indent] == json_structural_type::Object_Start) {
								appendNewLine<tabs, indentSize>(outPtr, indent);
							} else {
								appendCharacter<0x20u>(outPtr);
							}
						}
						break;
					}
					case json_structural_type::Number: {
						auto valueNew = iter.operator->();
						++iter;
						appendValues(iter.operator->() - valueNew, outPtr, valueNew);
						break;
					}
					case json_structural_type::Colon: {
						appendCharacter<0x3A>(outPtr);
						appendCharacter<0x20>(outPtr);
						++iter;
						break;
					}
					case json_structural_type::Array_Start: {
						appendCharacter<0x5Bu>(outPtr);
						++iter;
						++indent;
						state[indent] = json_structural_type::Array_Start;
						if (size_t(indent) >= std::size(state)) [[unlikely]] {
							iter.getErrors().emplace_back(createError<error_code::Prettify_Error>(iter));
							return std::numeric_limits<uint32_t>::max();
						}
						if constexpr (newLinesInArray) {
							if (*iter != 0x5Du) {
								appendNewLine<tabs, indentSize>(outPtr, indent);
							}
						} else {
							appendCharacter<0x20>(outPtr);
						}
						break;
					}
					case json_structural_type::Array_End: {
						--indent;
						if (indent < 0) {
							iter.getErrors().emplace_back(createError<error_code::Prettify_Error>(iter));
							return std::numeric_limits<uint32_t>::max();
						}
						if (*(iter.operator->() - 1) != 0x5Bu) {
							if constexpr (newLinesInArray) {
								appendNewLine<tabs, indentSize>(outPtr, indent);
							}
							appendCharacter<0x20>(outPtr);
						}
						appendCharacter<0x5Du>(outPtr);
						++iter;
						break;
					}
					case json_structural_type::Null: {
						appendValues(nullString.size(), outPtr, nullString.data());
						++iter;
						break;
					}
					case json_structural_type::Bool: {
						if (*iter == 0x74u) {
							appendValues(trueString.size(), outPtr, trueString.data());
						} else {
							appendValues(falseString.size(), outPtr, falseString.data());
						}
						++iter;
						break;
					}
					case json_structural_type::Object_Start: {
						appendCharacter<0x7Bu>(outPtr);
						++iter;
						++indent;
						state[indent] = json_structural_type::Object_Start;
						if (size_t(indent) >= std::size(state)) [[unlikely]] {
							iter.getErrors().emplace_back(createError<error_code::Prettify_Error>(iter));
							return std::numeric_limits<uint32_t>::max();
						}
						if (*iter != 0x7Du) {
							appendNewLine<tabs, indentSize>(outPtr, indent);
						}
						break;
					}
					case json_structural_type::Object_End: {
						--indent;
						if (indent < 0) {
							iter.getErrors().emplace_back(createError<error_code::Prettify_Error>(iter));
							return outPtr - out.data();
						}
						if (*(iter.operator->() - 1) != 0x7Bu) {
							appendNewLine<tabs, indentSize>(outPtr, indent);
						}
						appendCharacter<0x7Du>(outPtr);
						++iter;
						break;
					}
					case json_structural_type::Unset:
						[[fallthrough]];
					[[unlikely]] default: {
						iter.getErrors().emplace_back(createError<error_code::Prettify_Error>(iter));
						return std::numeric_limits<uint32_t>::max();
					}
				}
			}
			return outPtr - out.data();
		}
	};

}// namespace jsonifier_internal
