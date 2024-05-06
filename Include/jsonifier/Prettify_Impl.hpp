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

	template<typename derived_type> struct prettify_impl {
		template<jsonifier::prettify_options options, jsonifier::concepts::string_t string_type,
			jsonifier::concepts::is_fwd_iterator iterator_type>
		JSONIFIER_INLINE static uint64_t impl(iterator_type&& iter, string_type&& out) noexcept {
			json_structural_type state[options.maxDepth]{};
			int64_t indent{};
			auto outPtr = out.data();

			while (iter) {
				switch (asciiClassesMap[static_cast<uint64_t>(*iter)]) {
					case json_structural_type::String: {
						auto valueNew = iter.operator->();
						++iter;
						writeCharacters(iter.operator->() - valueNew, outPtr, valueNew);
						break;
					}
					case json_structural_type::Comma: {
						writeCharacter<0x2Cu>(outPtr);
						++iter;
						if constexpr (options.newLinesInArray) {
							writeNewLine<options.tabs, options.indentSize>(outPtr, indent);
						} else {
							if (state[indent] == json_structural_type::Object_Start) {
								writeNewLine<options.tabs, options.indentSize>(outPtr, indent);
							} else {
								writeCharacter<0x20u>(outPtr);
							}
						}
						break;
					}
					case json_structural_type::Number: {
						auto valueNew = iter.operator->();
						++iter;
						writeCharacters(iter.operator->() - valueNew, outPtr, valueNew);
						break;
					}
					case json_structural_type::Colon: {
						writeCharacter<0x3A>(outPtr);
						writeCharacter<0x20u>(outPtr);
						++iter;
						break;
					}
					case json_structural_type::Array_Start: {
						writeCharacter<0x5Bu>(outPtr);
						++iter;
						++indent;
						state[indent] = json_structural_type::Array_Start;
						if (indent >= static_cast<int64_t>(options.maxDepth)) [[unlikely]] {
							iter.template createError<error_classes::Prettifying>(prettify_errors::Incorrect_Structural_Index);
							return std::numeric_limits<uint32_t>::max();
						}
						if constexpr (options.newLinesInArray) {
							if (*iter != 0x5Du) {
								writeNewLine<options.tabs, options.indentSize>(outPtr, indent);
							}
						} else {
							writeCharacter<0x20u>(outPtr);
						}
						break;
					}
					case json_structural_type::Array_End: {
						--indent;
						if (indent < 0) {
							iter.template createError<error_classes::Prettifying>(prettify_errors::Incorrect_Structural_Index);
							return std::numeric_limits<uint32_t>::max();
						}
						if (*(iter.operator->() - 1) != 0x5Bu) {
							if constexpr (options.newLinesInArray) {
								writeNewLine<options.tabs, options.indentSize>(outPtr, indent);
							} else {
								writeCharacter<0x20u>(outPtr);
							}
						}
						writeCharacter<0x5Du>(outPtr);
						++iter;
						break;
					}
					case json_structural_type::Null: {
						writeCharacters(static_cast<int64_t>(nullString.size()), outPtr, nullString.data());
						++iter;
						break;
					}
					case json_structural_type::Bool: {
						if (*iter == 0x74u) {
							writeCharacters(static_cast<int64_t>(trueString.size()), outPtr, trueString.data());
						} else {
							writeCharacters(static_cast<int64_t>(falseString.size()), outPtr, falseString.data());
						}
						++iter;
						break;
					}
					case json_structural_type::Object_Start: {
						writeCharacter<0x7Bu>(outPtr);
						++iter;
						++indent;
						state[indent] = json_structural_type::Object_Start;
						if (indent >= static_cast<int64_t>(options.maxDepth)) [[unlikely]] {
							iter.template createError<error_classes::Prettifying>(prettify_errors::Incorrect_Structural_Index);
							return std::numeric_limits<uint32_t>::max();
						}
						if (*iter != 0x7Du) {
							writeNewLine<options.tabs, options.indentSize>(outPtr, indent);
						}
						break;
					}
					case json_structural_type::Object_End: {
						--indent;
						if (indent < 0) {
							iter.template createError<error_classes::Prettifying>(prettify_errors::Incorrect_Structural_Index);
							return static_cast<uint64_t>(outPtr - out.data());
						}
						if (*(iter.operator->() - 1) != 0x7Bu) {
							writeNewLine<options.tabs, options.indentSize>(outPtr, indent);
						}
						writeCharacter<0x7Du>(outPtr);
						++iter;
						break;
					}
					case json_structural_type::Unset:
					case json_structural_type::Error:
					case json_structural_type::Type_Count:
						[[fallthrough]];
					[[unlikely]] default: {
						iter.template createError<error_classes::Prettifying>(prettify_errors::Incorrect_Structural_Index);
						return std::numeric_limits<uint32_t>::max();
					}
				}
			}
			return static_cast<uint64_t>(outPtr - out.data());
		}
	};

}// namespace jsonifier_internal