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

#include <jsonifier/Minifier.hpp>
#include <jsonifier/Simd.hpp>

namespace jsonifier_internal {

	template<typename derived_type> struct minify_impl {
		template<jsonifier::concepts::string_t string_type, typename minifier_type, typename iterator>
		JSONIFIER_ALWAYS_INLINE static void impl(iterator& iter, string_type&& out, uint64_t& index, minifier_type& minifier) noexcept {
			auto previousPtr = *iter;
			int64_t currentDistance{};

			++iter;

			while (*iter) {
				switch (static_cast<uint8_t>(*previousPtr)) {
					case '"': {
						currentDistance = *iter - previousPtr;
						while (whitespaceTable[static_cast<uint8_t>(previousPtr[--currentDistance])]) {
						}
						++currentDistance;
						if JSONIFIER_LIKELY ((currentDistance > 0)) {
							std::memcpy(&out[index], previousPtr, static_cast<uint64_t>(currentDistance));
							index += static_cast<uint64_t>(currentDistance);
						} else {
							static constexpr auto sourceLocation{ std::source_location::current() };
							minifier.getErrors().emplace_back(error::constructError<sourceLocation, error_classes::Minifying, minify_errors::Invalid_String_Length>(
								static_cast<int64_t>(getUnderlyingPtr(iter) - minifier.rootIter), static_cast<int64_t>(minifier.endIter - minifier.rootIter), minifier.rootIter));
							return;
						}
						break;
					}
					case ',':
						std::memcpy(&out[index], ",", 1);
						++index;
						break;
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
						currentDistance = 0;
						while (!whitespaceTable[static_cast<uint8_t>(previousPtr[++currentDistance])] && ((previousPtr + currentDistance) < (*iter))) {
						}
						if JSONIFIER_LIKELY ((currentDistance > 0)) {
							std::memcpy(&out[index], previousPtr, static_cast<uint64_t>(currentDistance));
							index += static_cast<uint64_t>(currentDistance);
						} else {
							static constexpr auto sourceLocation{ std::source_location::current() };
							minifier.getErrors().emplace_back(error::constructError<sourceLocation, error_classes::Minifying, minify_errors::Invalid_Number_Value>(
								static_cast<int64_t>(getUnderlyingPtr(iter) - minifier.rootIter), static_cast<int64_t>(minifier.endIter - minifier.rootIter), minifier.rootIter));
							return;
						}
						break;
					}
					case ':':
						std::memcpy(&out[index], ":", 1);
						++index;
						break;
					case '[':
						std::memcpy(&out[index], "[", 1);
						++index;
						break;
					case ']':
						std::memcpy(&out[index], "]", 1);
						++index;
						break;
					case 'n': {
						std::memcpy(&out[index], "null", 4);
						index += 4;
						break;
					}
					case 'f': {
						std::memcpy(&out[index], "false", 5);
						index += 5;
						break;
					}
					case 't': {
						std::memcpy(&out[index], "true", 4);
						index += 4;
						break;
					}
					case '{':
						std::memcpy(&out[index], "{", 1);
						++index;
						break;
					case '}':
						std::memcpy(&out[index], "}", 1);
						++index;
						break;
						[[fallthrough]];
					case '\0':
						return;
					default: {
						static constexpr auto sourceLocation{ std::source_location::current() };
						minifier.getErrors().emplace_back(error::constructError<sourceLocation, error_classes::Minifying, minify_errors::Incorrect_Structural_Index>(
							static_cast<int64_t>(getUnderlyingPtr(iter) - minifier.rootIter), static_cast<int64_t>(minifier.endIter - minifier.rootIter), minifier.rootIter));
						return;
					}
				}
				previousPtr = (*iter);
				++iter;
			}
			if (previousPtr) {
				out[index] = *previousPtr;
				++index;
			}
		}
	};


}// namespace jsonifier_internal