/*
	MIT License

	Copyright (c) 2024 RealTimeChris

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

	template<jsonifier::prettify_options options, typename derived_type> struct prettify_impl : public writer<options> {
		template<jsonifier::concepts::string_t string_type, typename prettifier_type, typename iterator, typename prettify_pair_t>
		JSONIFIER_ALWAYS_INLINE static void impl(iterator& iter, string_type&& out, prettify_pair_t& prettifyPair, prettifier_type& prettifier) noexcept {
			const char* newPtr{};
			uint64_t newSize{};
			while (*iter) {
				switch (asciiClassesMap[static_cast<uint8_t>(**iter)]) {
					case json_structural_type::String: {
						newPtr = *iter;
						++iter;
						newSize = static_cast<uint64_t>((*iter) - newPtr);
						writer<options>::writeCharacters(out, newPtr, newSize, prettifyPair.index);
						break;
					}
					case json_structural_type::Comma: {
						writer<options>::template writeCharacter<','>(out, prettifyPair.index);
						++iter;
						if constexpr (options.newLinesInArray) {
							writer<options>::writeNewLine(out, prettifyPair);
						} else {
							if (prettifyPair.state[static_cast<uint64_t>(prettifyPair.indent)] == json_structural_type::Object_Start) {
								writer<options>::writeNewLine(out, prettifyPair);
							} else {
								writer<options>::template writeCharacter<options.indentChar>(out, prettifyPair.index);
							}
						}
						break;
					}
					case json_structural_type::Number: {
						newPtr = (*iter);
						++iter;
						newSize = static_cast<uint64_t>((*iter) - newPtr);
						writer<options>::writeCharacters(out, newPtr, newSize, prettifyPair.index);
						break;
					}
					case json_structural_type::Colon: {
						writer<options>::template writeCharacter<':'>(out, prettifyPair.index);
						writer<options>::template writeCharacter<options.indentChar>(out, prettifyPair.index);
						++iter;
						break;
					}
					case json_structural_type::Array_Start: {
						writer<options>::template writeCharacter<'['>(out, prettifyPair.index);
						++iter;
						++prettifyPair.indent;
						if JSONIFIER_UNLIKELY ((static_cast<size_t>(prettifyPair.indent) >= prettifyPair.state.size())) {
							prettifyPair.state.resize(prettifyPair.state.size() * 2);
						}
						prettifyPair.state[static_cast<uint64_t>(prettifyPair.indent)] = json_structural_type::Array_Start;
						if constexpr (options.newLinesInArray) {
							if JSONIFIER_UNLIKELY ((**iter != ']')) {
								writer<options>::writeNewLine(out, prettifyPair);
							}
						}
						break;
					}
					case json_structural_type::Array_End: {
						--prettifyPair.indent;
						if (prettifyPair.indent < 0) {
							static constexpr auto sourceLocation{ std::source_location::current() };
							prettifier.getErrors().emplace_back(error::constructError<sourceLocation, error_classes::Prettifying, prettify_errors::Incorrect_Structural_Index>(
								getUnderlyingPtr(iter) - prettifyPair.rootIter, prettifyPair.endIter - prettifyPair.rootIter, prettifyPair.rootIter));
							return;
						}
						if constexpr (options.newLinesInArray) {
							if (*iter[-1] != '[') {
								writer<options>::writeNewLine(out, prettifyPair);
							}
						}
						writer<options>::template writeCharacter<']'>(out, prettifyPair.index);
						++iter;
						break;
					}
					case json_structural_type::Null: {
						writer<options>::template writeCharacters<"null">(out, prettifyPair.index);
						++iter;
						break;
					}
					case json_structural_type::Bool: {
						if (**iter == 't') {
							writer<options>::template writeCharacters<"true">(out, prettifyPair.index);
							++iter;
							break;
						} else {
							writer<options>::template writeCharacters<"false">(out, prettifyPair.index);
							++iter;
							break;
						}
					}
					case json_structural_type::Object_Start: {
						writer<options>::template writeCharacter<'{'>(out, prettifyPair.index);
						++iter;
						++prettifyPair.indent;
						if JSONIFIER_UNLIKELY ((static_cast<size_t>(prettifyPair.indent) >= prettifyPair.state.size())) {
							prettifyPair.state.resize(prettifyPair.state.size() * 2);
						}
						prettifyPair.state[static_cast<uint64_t>(prettifyPair.indent)] = json_structural_type::Object_Start;
						if (**iter != '}') {
							writer<options>::writeNewLine(out, prettifyPair);
						}
						break;
					}
					case json_structural_type::Object_End: {
						--prettifyPair.indent;
						if (prettifyPair.indent < 0) {
							static constexpr auto sourceLocation{ std::source_location::current() };
							prettifier.getErrors().emplace_back(error::constructError<sourceLocation, error_classes::Prettifying, prettify_errors::Incorrect_Structural_Index>(
								getUnderlyingPtr(iter) - prettifyPair.rootIter, prettifyPair.endIter - prettifyPair.rootIter, prettifyPair.rootIter));
							return;
						}
						if (*iter[-1] != '{') {
							writer<options>::writeNewLine(out, prettifyPair);
						}
						writer<options>::template writeCharacter<'}'>(out, prettifyPair.index);
						++iter;
						break;
					}
					case json_structural_type::Unset:
						[[fallthrough]];
					case json_structural_type::Error:
						[[fallthrough]];
					case json_structural_type::Type_Count:
						[[fallthrough]];
					default: {
						static constexpr auto sourceLocation{ std::source_location::current() };
						prettifier.getErrors().emplace_back(error::constructError<sourceLocation, error_classes::Prettifying, prettify_errors::Incorrect_Structural_Index>(
							getUnderlyingPtr(iter) - prettifyPair.rootIter, prettifyPair.endIter - prettifyPair.rootIter, prettifyPair.rootIter));
						return;
					}
				}
			}
			return;
		}
	};

}// namespace jsonifier_internal