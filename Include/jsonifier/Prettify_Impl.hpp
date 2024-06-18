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
		template<const prettify_options_internal& options, jsonifier::concepts::string_t string_type, typename prettifier_type, typename iterator_type>
		JSONIFIER_INLINE static void impl(iterator_type& iter, string_type&& out, uint64_t& index, prettifier_type& prettifier) noexcept {
			jsonifier::vector<json_structural_type> state{};
			state.resize(64);

			while (iter) {
				switch (asciiClassesMap[static_cast<uint8_t>(*iter)]) {
					[[likely]] case json_structural_type::String: {
						auto valueNew = iter.operator->();
						++iter;
						auto newSize = static_cast<uint64_t>(iter.operator->() - valueNew);
						writeCharactersUnchecked(out, valueNew, newSize, index);
						break;
					}
					[[unlikely]] case json_structural_type::Comma: {
						writeCharacterUnchecked<','>(out, index);
						++iter;
						if constexpr (options.optionsReal.newLinesInArray) {
							writeNewLineUnchecked<options>(out, index);
						} else {
							if (state[options.indent] == json_structural_type::Object_Start) {
								writeNewLineUnchecked<options>(out, index);
							} else {
								writeCharacterUnchecked<options.optionsReal.indentChar>(out, index);
							}
						}
						break;
					}
					[[unlikely]] case json_structural_type::Number: {
						auto valueNew = iter.operator->();
						++iter;
						auto newSize = static_cast<uint64_t>(iter.operator->() - valueNew);
						writeCharactersUnchecked(out, valueNew, newSize, index);
						break;
					}
					[[unlikely]] case json_structural_type::Colon: {
						writeCharacterUnchecked<':'>(out, index);
						writeCharacterUnchecked<options.optionsReal.indentChar>(out, index);
						++iter;
						break;
					}
					[[unlikely]] case json_structural_type::Array_Start: {
						writeCharacterUnchecked<'['>(out, index);
						++iter;
						++options.indent;
						state[static_cast<uint64_t>(options.indent)] = json_structural_type::Array_Start;
						if (static_cast<uint64_t>(options.indent) >= state.size()) [[unlikely]] {
							state.resize(state.size() * 2);
						}
						if constexpr (options.optionsReal.newLinesInArray) {
							if (*iter != ']') {
								writeNewLineUnchecked<options>(out, index);
							}
						}
						break;
					}
					[[unlikely]] case json_structural_type::Array_End: {
						--options.indent;
						if (options.indent < 0) {
							static constexpr auto sourceLocation{ std::source_location::current() };
							prettifier.getErrors().emplace_back(error::constructError<sourceLocation, error_classes::Prettifying, prettify_errors::Incorrect_Structural_Index>(
								iter - prettifier.rootIter, iter.getEndPtr() - prettifier.rootIter, prettifier.rootIter));
							return;
						}
						if constexpr (options.optionsReal.newLinesInArray) {
							if (*(iter.sub(1)) != '[') {
								writeNewLineUnchecked<options>(out, index);
							}
						}
						writeCharacterUnchecked<']'>(out, index);
						++iter;
						break;
					}
					[[unlikely]] case json_structural_type::Null: {
						writeCharactersUnchecked<"null">(out, index);
						++iter;
						break;
					}
					[[unlikely]] case json_structural_type::Bool: {
						if (*iter == 't') {
							writeCharactersUnchecked<"true">(out, index);
							++iter;
							break;
						} else {
							writeCharactersUnchecked<"false">(out, index);
							++iter;
							break;
						}
					}
					[[unlikely]] case json_structural_type::Object_Start: {
						writeCharacterUnchecked<'{'>(out, index);
						++iter;
						++options.indent;
						state[static_cast<uint64_t>(options.indent)] = json_structural_type::Object_Start;
						if (static_cast<uint64_t>(options.indent) >= state.size()) [[unlikely]] {
							state.resize(state.size() * 2);
						}
						if (*iter != '}') {
							writeNewLineUnchecked<options>(out, index);
						}
						break;
					}
					[[unlikely]] case json_structural_type::Object_End: {
						--options.indent;
						if (options.indent < 0) {
							static constexpr auto sourceLocation{ std::source_location::current() };
							prettifier.getErrors().emplace_back(error::constructError<sourceLocation, error_classes::Prettifying, prettify_errors::Incorrect_Structural_Index>(
								iter - prettifier.rootIter, iter.getEndPtr() - prettifier.rootIter, prettifier.rootIter));
							return;
						}
						if (*(iter.sub(1)) != '{') {
							writeNewLineUnchecked<options>(out, index);
						}
						writeCharacterUnchecked<'}'>(out, index);
						++iter;
						break;
					} 
					case json_structural_type::Unset:
						[[fallthrough]];
					case json_structural_type::Error:
						[[fallthrough]];
					case json_structural_type::Type_Count:
						[[fallthrough]];
					[[unlikely]] default: {
						static constexpr auto sourceLocation{ std::source_location::current() };
						prettifier.getErrors().emplace_back(error::constructError<sourceLocation, error_classes::Prettifying, prettify_errors::Incorrect_Structural_Index>(
							iter - prettifier.rootIter, iter.getEndPtr() - prettifier.rootIter, prettifier.rootIter));
						return;
					}
				}
			}
			return;
		}
	};

}// namespace jsonifier_internal