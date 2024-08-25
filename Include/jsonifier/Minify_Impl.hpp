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

	template<typename derived_type> struct minify_impl : public writer<> {
		template<jsonifier::concepts::string_t string_type, typename minifier_type, typename iterator>
		JSONIFIER_ALWAYS_INLINE static void impl(iterator& iter, string_type&& out, uint64_t& index, minifier_type& minifier) noexcept {
			auto previousPtr = *iter;
			int64_t currentDistance{};

			++iter;

			while (*iter) {
				for (uint64_t x = 0; x < sixtyFourBitsPerStep; ++x) {
					jsonifierPrefetchImpl(*iter + bitsPerStep + (64 * x));
				}

				switch (asciiClassesMap[static_cast<uint8_t>(*previousPtr)]) {
					[[likely]] case json_structural_type::String: {
						currentDistance = *iter - previousPtr;
						while (whitespaceTable[static_cast<uint8_t>(previousPtr[--currentDistance])]) {
						}
						auto newSize = static_cast<uint64_t>(currentDistance) + 1;
						if (currentDistance > 0) [[likely]] {
							writeCharacters<false>(out, previousPtr, newSize, index);
						} else {
							static constexpr auto sourceLocation{ std::source_location::current() };
							minifier.getErrors().emplace_back(error::constructError<sourceLocation, error_classes::Minifying, minify_errors::Invalid_String_Length>(
								static_cast<int64_t>(getUnderlyingPtr(iter) - minifier.rootIter), static_cast<int64_t>(minifier.endIter - minifier.rootIter), minifier.rootIter));
							return;
						}
						break;
					}
					[[unlikely]] case json_structural_type::Comma:
						writeCharacter<',', false>(out, index);
						break;
					[[likely]] case json_structural_type::Number: {
						currentDistance = 0;
						while (!whitespaceTable[static_cast<uint8_t>(previousPtr[++currentDistance])] && ((previousPtr + currentDistance) < (*iter))) {
						}
						if (currentDistance > 0) [[likely]] {
							writeCharacters<false>(out, previousPtr, static_cast<uint64_t>(currentDistance), index);
						} else {
							static constexpr auto sourceLocation{ std::source_location::current() };
							minifier.getErrors().emplace_back(error::constructError<sourceLocation, error_classes::Minifying, minify_errors::Invalid_Number_Value>(
								static_cast<int64_t>(getUnderlyingPtr(iter) - minifier.rootIter), static_cast<int64_t>(minifier.endIter - minifier.rootIter), minifier.rootIter));
							return;
						}
						break;
					}
					[[unlikely]] case json_structural_type::Colon:
						writeCharacter<0x3A, false>(out, index);
						break;
					[[unlikely]] case json_structural_type::Array_Start:
						writeCharacter<'[', false>(out, index);
						break;
					[[unlikely]] case json_structural_type::Array_End:
						writeCharacter<']', false>(out, index);
						break;
					[[unlikely]] case json_structural_type::Null: {
						writeCharacters<"null", false>(out, index);
						break;
					}
					[[unlikely]] case json_structural_type::Bool: {
						if (*previousPtr == 't') {
							writeCharacters<"true", false>(out, index);
							break;
						} else {
							writeCharacters<"false", false>(out, index);
							break;
						}
					}
					[[unlikely]] case json_structural_type::Object_Start:
						writeCharacter<'{', false>(out, index);
						break;
					[[unlikely]] case json_structural_type::Object_End:
						writeCharacter<'}', false>(out, index);
						break;
					[[unlikely]] case json_structural_type::Unset:
					[[unlikely]] case json_structural_type::Error:
					[[unlikely]] case json_structural_type::Type_Count:
						[[fallthrough]];
					[[unlikely]] default: {
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
				writeCharacter<false>(*previousPtr, out, index);
			}
		}
	};


}// namespace jsonifier_internal