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

	template<const prettify_options_internal& options, typename derived_type> struct prettify_impl : public writer<options> {
		template<jsonifier::concepts::string_t string_type, typename prettifier_type, typename iterator, typename prettify_pair_t>
		JSONIFIER_ALWAYS_INLINE static void impl(iterator& iter, string_type&& out, prettify_pair_t& prettifyPair, prettifier_type& prettifier) noexcept {
			while (*iter) {
				for (uint64_t x = 0; x < sixtyFourBitsPerStep; ++x) {
					jsonifierPrefetchImpl(*iter + bitsPerStep + (64 * x));
				}

				switch (asciiClassesMap[uint8_t(**iter)]) {
					[[likely]] case json_structural_type::String: {
						const auto newPtr = *iter;
						++iter;
						const auto newSize = static_cast<uint64_t>((*iter) - newPtr);
						writer<options>::template writeCharacters<false>(out, newPtr, newSize, prettifyPair.index);
						break;
					}
					[[unlikely]] case json_structural_type::Comma: {
						writer<options>::template writeCharacter<',', false>(out, prettifyPair.index);
						++iter;
						if constexpr (options.optionsReal.newLinesInArray) {
							writer<options>::template writeNewLine<false>(out, prettifyPair);
						} else {
							if (prettifyPair.state[static_cast<uint64_t>(prettifyPair.indent)] == json_structural_type::Object_Start) {
								writer<options>::template writeNewLine<false>(out, prettifyPair);
							} else {
								writer<options>::template writeCharacter<options.optionsReal.indentChar, false>(out, prettifyPair.index);
							}
						}
						break;
					}
					[[likely]] case json_structural_type::Number: {
						const auto newPtr = (*iter);
						++iter;
						const auto newSize = static_cast<uint64_t>((*iter) - newPtr);
						writer<options>::template writeCharacters<false>(out, newPtr, newSize, prettifyPair.index);
						break;
					}
					[[unlikely]] case json_structural_type::Colon: {
						writer<options>::template writeCharacter<':', false>(out, prettifyPair.index);
						writer<options>::template writeCharacter<options.optionsReal.indentChar, false>(out, prettifyPair.index);
						++iter;
						break;
					}
					[[unlikely]] case json_structural_type::Array_Start: {
						writer<options>::template writeCharacter<'[', false>(out, prettifyPair.index);
						++iter;
						++prettifyPair.indent;
						if (size_t(prettifyPair.indent) >= prettifyPair.state.size()) [[unlikely]] {
							prettifyPair.state.resize(prettifyPair.state.size() * 2);
						}
						prettifyPair.state[static_cast<uint64_t>(prettifyPair.indent)] = json_structural_type::Array_Start;
						if constexpr (options.optionsReal.newLinesInArray) {
							if (**iter != ']') [[unlikely]] {
								writer<options>::template writeNewLine<false>(out, prettifyPair);
							}
						}
						break;
					}
					[[unlikely]] case json_structural_type::Array_End: {
						--prettifyPair.indent;
						if (prettifyPair.indent < 0) {
							static constexpr auto sourceLocation{ std::source_location::current() };
							prettifier.getErrors().emplace_back(error::constructError<sourceLocation, error_classes::Prettifying, prettify_errors::Incorrect_Structural_Index>(
								getUnderlyingPtr(iter) - prettifier.rootIter, prettifier.endIter - prettifier.rootIter, prettifier.rootIter));
							return;
						}
						if constexpr (options.optionsReal.newLinesInArray) {
							if (*iter[-1] != '[') {
								writer<options>::template writeNewLine<false>(out, prettifyPair);
							}
						}
						writer<options>::template writeCharacter<']', false>(out, prettifyPair.index);
						++iter;
						break;
					}
					[[unlikely]] case json_structural_type::Null: {
						writer<options>::template writeCharacters<"null", false>(out, prettifyPair.index);
						++iter;
						break;
					}
					[[unlikely]] case json_structural_type::Bool: {
						if (**iter == 't') {
							writer<options>::template writeCharacters<"true", false>(out, prettifyPair.index);
							++iter;
							break;
						} else {
							writer<options>::template writeCharacters<"false", false>(out, prettifyPair.index);
							++iter;
							break;
						}
					}
					[[unlikely]] case json_structural_type::Object_Start: {
						writer<options>::template writeCharacter<'{', false>(out, prettifyPair.index);
						++iter;
						++prettifyPair.indent;
						if (size_t(prettifyPair.indent) >= prettifyPair.state.size()) [[unlikely]] {
							prettifyPair.state.resize(prettifyPair.state.size() * 2);
						}
						prettifyPair.state[static_cast<uint64_t>(prettifyPair.indent)] = json_structural_type::Object_Start;
						if (**iter != '}') {
							writer<options>::template writeNewLine<false>(out, prettifyPair);
						}
						break;
					}
					[[unlikely]] case json_structural_type::Object_End: {
						--prettifyPair.indent;
						if (prettifyPair.indent < 0) {
							static constexpr auto sourceLocation{ std::source_location::current() };
							prettifier.getErrors().emplace_back(error::constructError<sourceLocation, error_classes::Prettifying, prettify_errors::Incorrect_Structural_Index>(
								getUnderlyingPtr(iter) - prettifier.rootIter, prettifier.endIter - prettifier.rootIter, prettifier.rootIter));
							return;
						}
						if (*iter[-1] != '{') {
							writer<options>::template writeNewLine<false>(out, prettifyPair);
						}
						writer<options>::template writeCharacter<'}', false>(out, prettifyPair.index);
						++iter;
						break;
					}
					[[unlikely]] case json_structural_type::Unset:
						[[fallthrough]];
					[[unlikely]] case json_structural_type::Error:
						[[fallthrough]];
					[[unlikely]] case json_structural_type::Type_Count:
						[[fallthrough]];
					[[unlikely]] default: {
						static constexpr auto sourceLocation{ std::source_location::current() };
						prettifier.getErrors().emplace_back(error::constructError<sourceLocation, error_classes::Prettifying, prettify_errors::Incorrect_Structural_Index>(
							getUnderlyingPtr(iter) - prettifier.rootIter, prettifier.endIter - prettifier.rootIter, prettifier.rootIter));
						return;
					}
				}
			}
			return;
		}
	};

}// namespace jsonifier_internal