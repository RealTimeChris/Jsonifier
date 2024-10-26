/*
	MIT License

	Copyright (c) 2024 RealTimeChris

	Permission is hereby granted, free of charge, to any person obtaining a copy of this
	software and associated documentation files (the "Software"), to deal in the Software
	withbuffer restriction, including withbuffer limitation the rights to use, copy, modify, merge,
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

	template<jsonifier::prettify_options options, typename derived_type> struct prettify_impl {
		template<jsonifier::concepts::string_t string_type, typename prettifier_type, typename iterator, typename prettify_pair_t>
		JSONIFIER_ALWAYS_INLINE static void impl(iterator& iter, string_type&& buffer, prettify_pair_t& prettifyPair, prettifier_type& prettifier) noexcept {
			const char* newPtr{};
			uint64_t newSize{};
			while (*iter) {
				switch (static_cast<uint8_t>(**iter)) {
					case '"': {
						newPtr = *iter;
						++iter;
						newSize = static_cast<uint64_t>((*iter) - newPtr);
						std::memcpy(&buffer[prettifyPair.index], newPtr, newSize);
						prettifyPair.index += newSize;
						break;
					}
					case ',': {
						buffer[prettifyPair.index] = ',';
						++prettifyPair.index;
						++iter;
						if constexpr (options.newLinesInArray) {
							auto indentTotal		= prettifyPair.indent * options.indentSize;
							buffer[prettifyPair.index] = '\n';
							++prettifyPair.index;
							std::memset(buffer.data() + prettifyPair.index, options.indentChar, indentTotal);
							prettifyPair.index += indentTotal;
						} else {
							if (prettifyPair.state[static_cast<uint64_t>(prettifyPair.indent)] == json_structural_type::Object_Start) {
								auto indentTotal		= prettifyPair.indent * options.indentSize;
								buffer[prettifyPair.index] = '\n';
								++prettifyPair.index;
								std::memset(buffer.data() + prettifyPair.index, options.indentChar, indentTotal);
								prettifyPair.index += indentTotal;
							} else {
								buffer[prettifyPair.index] = options.indentChar;
								++prettifyPair.index;
							}
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
						newPtr = (*iter);
						++iter;
						newSize = static_cast<uint64_t>((*iter) - newPtr);
						std::memcpy(&buffer[prettifyPair.index], newPtr, newSize);
						prettifyPair.index += newSize;
						break;
					}
					case ':': {
						buffer[prettifyPair.index] = ':';
						++prettifyPair.index;
						buffer[prettifyPair.index] = options.indentChar;
						++prettifyPair.index;
						++iter;
						break;
					}
					case '[': {
						buffer[prettifyPair.index] = '[';
						++prettifyPair.index;
						++iter;
						++prettifyPair.indent;
						if JSONIFIER_UNLIKELY ((static_cast<size_t>(prettifyPair.indent) >= prettifyPair.state.size())) {
							prettifyPair.state.resize(prettifyPair.state.size() * 2);
						}
						prettifyPair.state[static_cast<uint64_t>(prettifyPair.indent)] = json_structural_type::Array_Start;
						if constexpr (options.newLinesInArray) {
							if JSONIFIER_UNLIKELY ((**iter != ']')) {
								auto indentTotal		= prettifyPair.indent * options.indentSize;
								buffer[prettifyPair.index] = '\n';
								++prettifyPair.index;
								std::memset(buffer.data() + prettifyPair.index, options.indentChar, indentTotal);
								prettifyPair.index += indentTotal;
							}
						}
						break;
					}
					case ']': {
						--prettifyPair.indent;
						if (prettifyPair.indent < 0) {
							static constexpr auto sourceLocation{ std::source_location::current() };
							prettifier.getErrors().emplace_back(error::constructError<sourceLocation, error_classes::Prettifying, prettify_errors::Incorrect_Structural_Index>(
								getUnderlyingPtr(iter) - prettifyPair.rootIter, prettifyPair.endIter - prettifyPair.rootIter, prettifyPair.rootIter));
							return;
						}
						if constexpr (options.newLinesInArray) {
							if (*iter[-1] != '[') {
								auto indentTotal		= prettifyPair.indent * options.indentSize;
								buffer[prettifyPair.index] = '\n';
								++prettifyPair.index;
								std::memset(buffer.data() + prettifyPair.index, options.indentChar, indentTotal);
								prettifyPair.index += indentTotal;
							}
						}
						buffer[prettifyPair.index] = ']';
						++prettifyPair.index;
						++iter;
						break;
					}
					case 'n': {
						std::memcpy(&buffer[prettifyPair.index], "null", 4);
						prettifyPair.index += 4;
						++iter;
						break;
					}
					case 't': {
						std::memcpy(&buffer[prettifyPair.index], "true", 4);
						prettifyPair.index += 4;
						++iter;
						break;
					}
					case 'f': {
						std::memcpy(&buffer[prettifyPair.index], "false", 5);
						prettifyPair.index += 5;
						++iter;
						break;
					}
					case '{': {
						buffer[prettifyPair.index] = '{';
						++prettifyPair.index;
						++iter;
						++prettifyPair.indent;
						if JSONIFIER_UNLIKELY ((static_cast<size_t>(prettifyPair.indent) >= prettifyPair.state.size())) {
							prettifyPair.state.resize(prettifyPair.state.size() * 2);
						}
						prettifyPair.state[static_cast<uint64_t>(prettifyPair.indent)] = json_structural_type::Object_Start;
						if (**iter != '}') {
							auto indentTotal		= prettifyPair.indent * options.indentSize;
							buffer[prettifyPair.index] = '\n';
							++prettifyPair.index;
							std::memset(buffer.data() + prettifyPair.index, options.indentChar, indentTotal);
							prettifyPair.index += indentTotal;
						}
						break;
					}
					case '}': {
						--prettifyPair.indent;
						if (prettifyPair.indent < 0) {
							static constexpr auto sourceLocation{ std::source_location::current() };
							prettifier.getErrors().emplace_back(error::constructError<sourceLocation, error_classes::Prettifying, prettify_errors::Incorrect_Structural_Index>(
								getUnderlyingPtr(iter) - prettifyPair.rootIter, prettifyPair.endIter - prettifyPair.rootIter, prettifyPair.rootIter));
							return;
						}
						if (*iter[-1] != '{') {
							auto indentTotal		= prettifyPair.indent * options.indentSize;
							buffer[prettifyPair.index] = '\n';
							++prettifyPair.index;
							std::memset(buffer.data() + prettifyPair.index, options.indentChar, indentTotal);
							prettifyPair.index += indentTotal;
						}
						buffer[prettifyPair.index] = '}';
						++prettifyPair.index;
						++iter;
						break;
					}
					case '\0':
						return;
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