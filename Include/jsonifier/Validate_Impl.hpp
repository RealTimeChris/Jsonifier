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
/// Feb 20, 2023
#pragma once

#include <jsonifier/Validator.hpp>

namespace jsonifier_internal {

	template<jsonifier::concepts::is_fwd_iterator iterator_type01, typename errors_vector_type> bool validateNumber(iterator_type01&& start, errors_vector_type& errors) {
		auto newPtr = start.operator->();
		++start;
		auto end = start.operator->();
		derailleur::skipWs(newPtr);
		auto newSize = end - newPtr;
		if (newSize > 1 && *newPtr == 0x30u && numberTable[*(newPtr + 1)]) {
			errors.emplace_back(createError<error_code::Invalid_Number_Value>(start));
			return false;
		}

		derailleur::skipWs(newPtr);

		auto consumeChar = [&](char expected) {
			if (*newPtr == expected) {
				++newPtr;
				return true;
			}
			return false;
		};

		auto consumeDigits = [&](uint64_t min_count = 1) {
			uint64_t count = 0;
			while (digitTable[static_cast<uint64_t>(*newPtr)]) {
				++newPtr;
				++count;
			}
			return count >= min_count;
		};

		auto consumeSign = [&] {
			if (*newPtr == 0x2Du || *newPtr == 0x2Bu) {
				++newPtr;
				return true;
			}
			return false;
		};

		if (consumeSign()) {
		}

		if (consumeDigits(1)) {
		}

		if (consumeChar(0x2Eu)) {
			if (!consumeDigits(1)) {
				errors.emplace_back(createError<error_code::Invalid_Number_Value>(start));
				return false;
			}
		}
		if (consumeChar(0x65u) || consumeChar(0x45u)) {
			bool didWeFail{ false };
			consumeSign();
			didWeFail = !consumeDigits(1);
			if (didWeFail) {
				errors.emplace_back(createError<error_code::Invalid_Number_Value>(start));
				return false;
			}
		}
		derailleur::skipWs(newPtr);
		if (newPtr != end) {
			errors.emplace_back(createError<error_code::Invalid_Number_Value>(start));
			return false;
		}

		return true;
	}

	template<jsonifier::concepts::is_fwd_iterator iterator_type01, typename errors_vector_type> bool validateBool(iterator_type01&& start, errors_vector_type& errors) {
		auto newPtr = start.operator->();
		++start;
		static constexpr char falseStr[]{ "false" };
		static constexpr char trueStr[]{ "true" };
		derailleur::skipWs(newPtr);
		if (std::memcmp(newPtr, trueStr, std::strlen(trueStr)) == 0) {
			newPtr += std::size(trueStr) - 1;
		} else if (std::memcmp(newPtr, falseStr, std::strlen(falseStr)) == 0) {
			newPtr += std::size(falseStr) - 1;
		} else {
			errors.emplace_back(createError<error_code::Invalid_Bool_Value>(start));
			return false;
		}

		return start.operator bool();
	}

	template<jsonifier::concepts::is_fwd_iterator iterator_type01, typename errors_vector_type> bool validateNull(iterator_type01&& start, errors_vector_type& errors) {
		auto newPtr = start.operator->();
		++start;
		derailleur::skipWs(newPtr);
		static constexpr char nullStr[]{ "null" };

		if (std::memcmp(newPtr, nullStr, std::strlen(nullStr)) == 0) {
			newPtr += std::size(nullStr) - 1;
		} else {
			errors.emplace_back(createError<error_code::Invalid_Null_Value>(start));
			return false;
		}

		return start.operator bool();
	}

	template<jsonifier::concepts::is_fwd_iterator iterator_type, typename errors_vector_type> bool validateString(iterator_type&& start, errors_vector_type& errors) {
		auto newPtr = start.operator->();
		++start;
		auto endPtr = start.operator->();
		derailleur::skipWs(newPtr);
		if (newPtr == endPtr || *newPtr != 0x22u) {
			errors.emplace_back(createError<error_code::Invalid_String_Characters>(start));
			return false;
		}
		++newPtr;

		while (newPtr != endPtr && *newPtr != 0x22u) {
			if (*newPtr == 0x5Cu) {
				++newPtr;

				if (*newPtr == 0x22u || *newPtr == 0x5Cu || *newPtr == 0x2Fu || *newPtr == 0x62u || *newPtr == 0x66u || *newPtr == 0x6Eu || *newPtr == 0x72u || *newPtr == 0x74u) {
					++newPtr;
				} else if (*newPtr == 0x75u) {
					++newPtr;
					for (int i = 0; i < 4; ++i) {
						if (!hexDigits[*newPtr]) {
							errors.emplace_back(createError<error_code::Invalid_Escape_Characters>(start));
							return false;
						}
						++newPtr;
					}
				} else {
					errors.emplace_back(createError<error_code::Invalid_Escape_Characters>(start));
					return false;
				}
			} else if (*newPtr < 0x20) {
				errors.emplace_back(createError<error_code::Invalid_String_Characters>(start));
				return false;
			} else {
				++newPtr;
			}
		}

		if (*newPtr != 0x22u) {
			errors.emplace_back(createError<error_code::Invalid_String_Characters>(start));
			return false;
		}

		return start.operator bool();
	}


	template<jsonifier::concepts::is_fwd_iterator iterator_type01, typename errors_vector_type>
	bool validateArray(iterator_type01&& start, errors_vector_type& errors, uint64_t& depth) {
		if (!start || *start != 0x5Bu) {
			errors.emplace_back(createError<error_code::Broken_Array_Start>(start));
			return false;
		}
		++depth;
		++start;

		if (*start == 0x5Du) {
			++start;
			--depth;
			return true;
		}

		while (start) {
			if (!validate::impl(start, depth)) {
				return false;
			}
			if (*start == 0x2Cu) {
				++start;
			} else if (*start == 0x5Du) {
				++start;
				if (start && *start != 0x2Cu && *start != 0x5Du && *start != 0x7Du) {
					errors.emplace_back(createError<error_code::Missing_Comma_Or_Closing_Brace>(start));
					return false;
				}
				--depth;
				return true;
			} else {
				errors.emplace_back(createError<error_code::Missing_Comma_Or_Closing_Brace>(start));
				return false;
			}
		}
		if (*start != 0x2Cu && *start != 0x5Du) {
			errors.emplace_back(createError<error_code::Missing_Comma_Or_Closing_Brace>(start));
			return false;
		}
		--depth;
		return true;
	}

	template<jsonifier::concepts::is_fwd_iterator iterator_type01, typename errors_vector_type>
	bool validateObject(iterator_type01&& start, errors_vector_type& errors, uint64_t& depth) {
		if (!start || *start != 0x7Bu) {
			errors.emplace_back(createError<error_code::Broken_Object_Start>(start));
			return false;
		}
		++depth;
		++start;
		if (*start == 0x7Du) {
			++start;
			--depth;
			return true;
		}

		while (start) {
			if (!validateString(start, errors)) {
				errors.emplace_back(createError<error_code::Invalid_String_Characters>(start));
				return false;
			}

			if (*start != 0x3Au) {
				errors.emplace_back(createError<error_code::Missing_Colon>(start));
				return false;
			}

			++start;
			if (!validate::impl(start, depth)) {
				return false;
			}

			if (*start == 0x2Cu) {
				++start;
			} else if (*start == 0x7Du) {
				++start;
				if (start && *start != 0x2Cu && *start != 0x5Du && *start != 0x7Du) {
					errors.emplace_back(createError<error_code::Missing_Comma_Or_Closing_Brace>(start));
					return false;
				}
				--depth;
				return true;
			} else {
				errors.emplace_back(createError<error_code::Missing_Comma_Or_Closing_Brace>(start));
				return false;
			}
		}
		if (*start != 0x2Cu && *start != 0x7Du) {
			errors.emplace_back(createError<error_code::Missing_Comma_Or_Closing_Brace>(start));
			return false;
		}
		--depth;
		return true;
	}

	template<jsonifier::concepts::is_fwd_iterator iterator_type> JSONIFIER_INLINE bool validate::impl(iterator_type&& iter, uint64_t& depth) {
		if (*iter == 0x7Bu) {
			return validateObject(iter, iter.getErrors(), depth);
		} else if (*iter == 0x5Bu) {
			return validateArray(iter, iter.getErrors(), depth);
		} else if (*iter == 0x22u) {
			return validateString(iter, iter.getErrors());
		} else if (digitTable[*iter]) {
			return validateNumber(iter, iter.getErrors());
		} else if (*iter == 0x74u || *iter == 0x66u) {
			return validateBool(iter, iter.getErrors());
		} else if (*iter == 0x6Eu) {
			return validateNull(iter, iter.getErrors());
		} else {
			return false;
		}
	}

}