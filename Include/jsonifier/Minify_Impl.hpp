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

#include <jsonifier/SimdStructuralIterator.hpp>
#include <jsonifier/Derailleur.hpp>
#include <jsonifier/Simd.hpp>

namespace jsonifier_internal {

	template<jsonifier::concepts::string_t string_type, jsonifier::concepts::is_fwd_iterator iterator_type>
	JSONIFIER_INLINE uint64_t minify::impl(iterator_type&& iter, string_type& out) noexcept {
		auto previousPtr = iter.operator->();
		int64_t currentDistance{};
		auto outPtr = out.data();

		if (!iter || !previousPtr || (asciiClassesMap[*previousPtr] != ascii_classes::lsqrb && asciiClassesMap[*previousPtr] != ascii_classes::lcurb)) {
			iter.getErrors().emplace_back(createError<error_code::Minify_Error>(iter));
			return std::numeric_limits<uint64_t>::max();
		}

		++iter;

		while (iter && previousPtr) {
			switch (asciiClassesMap[*previousPtr]) {
				[[likely]] case ascii_classes::quote : {
					currentDistance = iter.operator->() - previousPtr;
					while (whiteSpaceTable[previousPtr[--currentDistance]] && ((previousPtr + currentDistance) < iter.operator->())) {
					}
					if (currentDistance > 0) {
						std::memcpy(outPtr, previousPtr, static_cast<uint64_t>(currentDistance + 1));
						outPtr += currentDistance + 1;
					} else {
						iter.getErrors().emplace_back(createError<error_code::Minify_Error>(iter));
						return std::numeric_limits<uint64_t>::max();
					}
					break;
				}
				[[unlikely]] case ascii_classes::colon:
					[[fallthrough]];
				[[unlikely]] case ascii_classes::comma:
					[[fallthrough]];
				[[unlikely]] case ascii_classes::lsqrb:
					[[fallthrough]];
				[[unlikely]] case ascii_classes::lcurb:
					[[fallthrough]];
				[[unlikely]] case ascii_classes::rcurb:
					[[fallthrough]];
				[[unlikely]] case ascii_classes::rsqrb : {
					appendCharacter(*previousPtr, outPtr);
					break;
				}
				[[unlikely]] case ascii_classes::false_val : {
					std::memcpy(outPtr, falseString.data(), falseString.size());
					outPtr += falseString.size();
					break;
				}
				[[unlikely]] case ascii_classes::true_val : {
					std::memcpy(outPtr, trueString.data(), trueString.size());
					outPtr += trueString.size();
					break;
				}
				[[unlikely]] case ascii_classes::null_val : {
					std::memcpy(outPtr, nullString.data(), nullString.size());
					outPtr += nullString.size();
					break;
				}
				[[likely]] case ascii_classes::num_val : {
					currentDistance = 0;
					while (!whiteSpaceTable[previousPtr[++currentDistance]] && ((previousPtr + currentDistance) < iter.operator->())) {
					}
					if (currentDistance > 0) {
						std::memcpy(outPtr, previousPtr, static_cast<uint64_t>(currentDistance));
						outPtr += currentDistance;
					} else {
						iter.getErrors().emplace_back(createError<error_code::Minify_Error>(iter));
						return std::numeric_limits<uint64_t>::max();
					}
					break;
				}
				[[unlikely]] case ascii_classes::class_count:
					[[fallthrough]];
				[[unlikely]] case ascii_classes::errorVal:
					[[fallthrough]];
					[[unlikely]] default : {
						iter.getErrors().emplace_back(createError<error_code::Minify_Error>(iter));
						return std::numeric_limits<uint64_t>::max();
					}
			}
			previousPtr = iter.operator->();
			++iter;
		}
		if (previousPtr) [[likely]] {
			std::memcpy(outPtr, previousPtr, 1);
			++outPtr;
		}
		return static_cast<uint64_t>(outPtr - out.data());
	}


}// namespace jsonifier_internal
