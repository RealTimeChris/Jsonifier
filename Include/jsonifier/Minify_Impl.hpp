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

#include <jsonifier/JsonStructuralIterator.hpp>
#include <jsonifier/Minifier.hpp>
#include <jsonifier/Simd.hpp>

namespace jsonifier_internal {

	template<typename derived_type> struct minify_impl {
		template<const minify_options_internal<derived_type>& options, jsonifier::concepts::string_t string_type, typename iterator_type>
		JSONIFIER_INLINE static void impl(iterator_type&& iter, string_type& out, uint64_t& index) noexcept {
			auto previousPtr = iter.operator->();
			int64_t currentDistance{};

			++iter;

			while (iter) {
				switch (asciiClassesMap[static_cast<uint8_t>(*previousPtr)]) {
					[[likely]] case json_structural_type::String: {
						currentDistance = iter.operator->() - previousPtr;
						while (whitespaceTable[static_cast<uint8_t>(previousPtr[--currentDistance])]) {
						}
						auto newSize = static_cast<uint64_t>(currentDistance) + 1;
						if (currentDistance > 0) [[likely]] {
							writeCharactersUnchecked(out, previousPtr, newSize, index);
						} else {
							static constexpr auto sourceLocation{ std::source_location::current() };
							options.minifierPtr->getErrors().emplace_back(error::constructError<sourceLocation, error_classes::Minifying, minify_errors::Invalid_String_Length>(
								static_cast<int64_t>(iter - iter.getRootPtr()), static_cast<int64_t>(iter.getEndPtr() - iter.getRootPtr()), iter.getRootPtr()));
							return;
						}
						break;
					}
					[[unlikely]] case json_structural_type::Comma:
						writeCharacterUnchecked<','>(out, index);
						break;
					[[likely]] case json_structural_type::Number: {
						currentDistance = 0;
						while (!whitespaceTable[static_cast<uint8_t>(previousPtr[++currentDistance])] && ((previousPtr + currentDistance) < iter.operator->())) {
						}
						if (currentDistance > 0) [[likely]] {
							writeCharactersUnchecked(out, previousPtr, static_cast<uint64_t>(currentDistance), index);
						} else {
							static constexpr auto sourceLocation{ std::source_location::current() };
							options.minifierPtr->getErrors().emplace_back(error::constructError<sourceLocation, error_classes::Minifying, minify_errors::Invalid_Number_Value>(
								static_cast<int64_t>(iter - iter.getRootPtr()), static_cast<int64_t>(iter.getEndPtr() - iter.getRootPtr()), iter.getRootPtr()));
							return;
						}
						break;
					}
					[[unlikely]] case json_structural_type::Colon:
						writeCharacterUnchecked<0x3A>(out, index);
						break;
					[[unlikely]] case json_structural_type::Array_Start:
						writeCharacterUnchecked<'['>(out, index);
						break;
					[[unlikely]] case json_structural_type::Array_End:
						writeCharacterUnchecked<']'>(out, index);
						break;
					[[unlikely]] case json_structural_type::Null: {
						writeCharactersUnchecked(out, nullString.data(), nullString.size(), index);
						break;
					}
					[[unlikely]] case json_structural_type::Bool: {
						if (*previousPtr == 't') {
							writeCharactersUnchecked(out, trueString.data(), trueString.size(), index);
							break;
						} else {
							writeCharactersUnchecked(out, falseString.data(), falseString.size(), index);
							break;
						}
					}
					[[unlikely]] case json_structural_type::Object_Start:
						writeCharacterUnchecked<'{'>(out, index);
						break;
					[[unlikely]] case json_structural_type::Object_End:
						writeCharacterUnchecked<'}'>(out, index);
						break;
					[[unlikely]] case json_structural_type::Unset:
					[[unlikely]] case json_structural_type::Error:
					[[unlikely]] case json_structural_type::Type_Count:
						[[fallthrough]];
					[[unlikely]] default: {
						static constexpr auto sourceLocation{ std::source_location::current() };
						options.minifierPtr->getErrors().emplace_back(error::constructError<sourceLocation, error_classes::Minifying, minify_errors::Incorrect_Structural_Index>(
							static_cast<int64_t>(iter - iter.getRootPtr()), static_cast<int64_t>(iter.getEndPtr() - iter.getRootPtr()), iter.getRootPtr()));
						return;
					}
				}
				previousPtr = iter.operator->();
				++iter;
			}
			writeCharacterUnchecked(*previousPtr, out, index);
		}
	};


}// namespace jsonifier_internal