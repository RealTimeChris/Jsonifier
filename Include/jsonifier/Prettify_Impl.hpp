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
		template<jsonifier::concepts::string_t string_type, typename prettifier_type, typename iterator>
		JSONIFIER_ALWAYS_INLINE static uint64_t impl(iterator& iter, string_type&& out, prettifier_type& prettifier) noexcept {
			const char* newPtr{};
			uint64_t newSize{};
			uint64_t index{};
			while (*iter) {
				switch (static_cast<uint8_t>(**iter)) {
					case '"': {
						newPtr = *iter;
						++iter;
						newSize = static_cast<uint64_t>((*iter) - newPtr);
						std::memcpy(&out[index], newPtr, newSize);
						index += newSize;
						break;
					}
					case ',': {
						std::memcpy(&out[index], ",", 1);
						++index;
						++iter;
						if constexpr (options.newLinesInArray) {
							auto indentTotal	  = prettifier.indent * options.indentSize;
							out[index] = '\n';
							++index;
							std::memset(out.data() + index, options.indentChar, indentTotal);
							index += indentTotal;
						} else {
							if (prettifier.state[static_cast<uint64_t>(prettifier.indent)] == json_structural_type::Object_Start) {
								auto indentTotal	  = prettifier.indent * options.indentSize;
								out[index] = '\n';
								++index;
								std::memset(out.data() + index, options.indentChar, indentTotal);
								index += indentTotal;
							} else {
								out[index] = options.indentChar;
								++index;
							}
						}
						break;
					}
					case '0':
						[[fallthrou]];
					case '1':
						[[fallthrough]];
					case '2':
						[[fallthrough]];
					case '3':
						[[fallthrough]];
					case '4':
						[[fallthrough]];
					case '5':
						[[fallthrough]];
					case '6':
						[[fallthrough]];
					case '7':
						[[fallthrough]];
					case '8':
						[[fallthrough]];
					case '9':
						[[fallthrough]];
					case '-': {
						newPtr = (*iter);
						++iter;
						newSize = static_cast<uint64_t>((*iter) - newPtr);
						std::memcpy(&out[index], newPtr, newSize);
						index += newSize;
						break;
					}
					case ':': {
						std::memcpy(&out[index], ":", 1);
						++index;
						out[index] = options.indentChar;
						++index;
						++iter;
						break;
					}
					case '[': {
						std::memcpy(&out[index], "[", 1);
						++index;
						++iter;
						++prettifier.indent;
						if JSONIFIER_UNLIKELY ((static_cast<size_t>(prettifier.indent) >= prettifier.state.size())) {
							prettifier.state.resize(prettifier.state.size() * 2);
						}
						prettifier.state[static_cast<uint64_t>(prettifier.indent)] = json_structural_type::Array_Start;
						if constexpr (options.newLinesInArray) {
							if JSONIFIER_UNLIKELY ((**iter != ']')) {
								auto indentTotal	  = prettifier.indent * options.indentSize;
								out[index] = '\n';
								++index;
								std::memset(out.data() + index, options.indentChar, indentTotal);
								index += indentTotal;
							}
						}
						break;
					}
					case ']': {
						--prettifier.indent;
						if (prettifier.indent < 0) {
							static constexpr auto sourceLocation{ std::source_location::current() };
							prettifier.getErrors().emplace_back(error::constructError<sourceLocation, error_classes::Prettifying, prettify_errors::Incorrect_Structural_Index>(
								getUnderlyingPtr(iter) - prettifier.rootIter, prettifier.endIter - prettifier.rootIter, prettifier.rootIter));
							return std::numeric_limits<uint64_t>::max();
						}
						if constexpr (options.newLinesInArray) {
							if (*iter[-1] != '[') {
								auto indentTotal	  = prettifier.indent * options.indentSize;
								out[index] = '\n';
								++index;
								std::memset(out.data() + index, options.indentChar, indentTotal);
								index += indentTotal;
							}
						}
						std::memcpy(&out[index], "]", 1);
						++index;
						++iter;
						break;
					}
					case 'n': {
						std::memcpy(&out[index], "null", 4);
						index += 4;
						++iter;
						break;
					}
					case 't': {
						std::memcpy(&out[index], "true", 4);
						index += 4;
						++iter;
						break;
					}
					case 'f': {
						std::memcpy(&out[index], "false", 5);
						index += 5;
						++iter;
						break;
					}
					case '{': {
						std::memcpy(&out[index], "{", 1);
						++index;
						++iter;
						++prettifier.indent;
						if JSONIFIER_UNLIKELY ((static_cast<size_t>(prettifier.indent) >= prettifier.state.size())) {
							prettifier.state.resize(prettifier.state.size() * 2);
						}
						prettifier.state[static_cast<uint64_t>(prettifier.indent)] = json_structural_type::Object_Start;
						if (**iter != '}') {
							auto indentTotal	  = prettifier.indent * options.indentSize;
							out[index] = '\n';
							++index;
							std::memset(out.data() + index, options.indentChar, indentTotal);
							index += indentTotal;
						}
						break;
					}
					case '}': {
						--prettifier.indent;
						if (prettifier.indent < 0) {
							static constexpr auto sourceLocation{ std::source_location::current() };
							prettifier.getErrors().emplace_back(error::constructError<sourceLocation, error_classes::Prettifying, prettify_errors::Incorrect_Structural_Index>(
								getUnderlyingPtr(iter) - prettifier.rootIter, prettifier.endIter - prettifier.rootIter, prettifier.rootIter));
							return std::numeric_limits<uint64_t>::max();
						}
						if (*iter[-1] != '{') {
							auto indentTotal	  = prettifier.indent * options.indentSize;
							out[index] = '\n';
							++index;
							std::memset(out.data() + index, options.indentChar, indentTotal);
							index += indentTotal;
						}
						std::memcpy(&out[index], "}", 1);
						++index;
						++iter;
						break;
					}
					case '\0': {
						return index;
					}
					default: {
						static constexpr auto sourceLocation{ std::source_location::current() };
						prettifier.getErrors().emplace_back(error::constructError<sourceLocation, error_classes::Prettifying, prettify_errors::Incorrect_Structural_Index>(
							getUnderlyingPtr(iter) - prettifier.rootIter, prettifier.endIter - prettifier.rootIter, prettifier.rootIter));
						return std::numeric_limits<uint64_t>::max();
					}
				}
			}
			return index;
		}
	};

}// namespace jsonifier_internal