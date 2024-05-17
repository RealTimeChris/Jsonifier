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

	template<typename derived_type> struct validate_impl<json_structural_type::Object_Start, derived_type> {
		template<jsonifier::concepts::is_fwd_iterator iterator_type01> static bool impl(iterator_type01&& iter, uint64_t& depth) {
			if (!iter || *iter != 0x7Bu) {
				iter.template createError<error_classes::Validating>(validate_errors::Missing_Object_Start);
				return false;
			}
			++depth;
			++iter;
			if (*iter == 0x7Du) {
				++iter;
				--depth;
				return true;
			}

			while (iter) {
				if (!validate_impl<json_structural_type::String, derived_type>::impl(iter)) {
					iter.template createError<error_classes::Validating>(validate_errors::Invalid_String_Characters);
					return false;
				}

				if (*iter != 0x3Au) {
					iter.template createError<error_classes::Validating>(validate_errors::Missing_Colon);
					return false;
				}

				++iter;
				if (!validator<derived_type>::impl(iter, depth)) {
					return false;
				}

				if (*iter == 0x2Cu) {
					++iter;
				} else if (*iter == 0x7Du) {
					++iter;
					if (iter && *iter != 0x2Cu && *iter != 0x5Du && *iter != 0x7Du) {
						iter.template createError<error_classes::Validating>(validate_errors::Missing_Comma_Or_Closing_Brace);
						return false;
					}
					--depth;
					return true;
				} else {
					iter.template createError<error_classes::Validating>(validate_errors::Missing_Comma_Or_Closing_Brace);
					return false;
				}
			}
			if (*iter != 0x2Cu && *iter != 0x7Du) {
				iter.template createError<error_classes::Validating>(validate_errors::Missing_Comma_Or_Closing_Brace);
				return false;
			}
			--depth;
			return true;
		}
	};

	template<typename derived_type> struct validate_impl<json_structural_type::Array_Start, derived_type> {
		template<jsonifier::concepts::is_fwd_iterator iterator_type01> static bool impl(iterator_type01&& iter, uint64_t& depth) {
			if (!iter || *iter != 0x5Bu) {
				iter.template createError<error_classes::Validating>(validate_errors::Missing_Array_Start);
				return false;
			}
			++depth;
			++iter;

			if (*iter == 0x5Du) {
				++iter;
				--depth;
				return true;
			}

			while (iter) {
				if (!validator<derived_type>::impl(iter, depth)) {
					return false;
				}
				if (*iter == 0x2Cu) {
					++iter;
				} else if (*iter == 0x5Du) {
					++iter;
					if (iter && *iter != 0x2Cu && *iter != 0x5Du && *iter != 0x7Du) {
						iter.template createError<error_classes::Validating>(validate_errors::Missing_Comma_Or_Closing_Brace);
						return false;
					}
					--depth;
					return true;
				} else {
					iter.template createError<error_classes::Validating>(validate_errors::Missing_Comma_Or_Closing_Brace);
					return false;
				}
			}
			if (*iter != 0x2Cu && *iter != 0x5Du) {
				iter.template createError<error_classes::Validating>(validate_errors::Missing_Comma_Or_Closing_Brace);
				return false;
			}
			--depth;
			return true;
		}
	};

	constexpr std::array<bool, 256> hexDigits{ false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
		false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
		false, false, false, false, false, true, true, true, true, true, true, true, true, true, true, false, false, false, false, false, false, false, true, true, true, true,
		true, true, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
		false, false, false, true, true, true, true, true, true, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
		false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
		false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
		false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
		false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
		false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
		false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false };

	template<typename derived_type> struct validate_impl<json_structural_type::String, derived_type> {
		template<jsonifier::concepts::is_fwd_iterator iterator_type> static bool impl(iterator_type&& iter) {
			auto newPtr = iter.operator->();
			++iter;
			auto endPtr = iter.operator->();
			skipWs(newPtr);
			if (newPtr == endPtr || *newPtr != 0x22u) {
				iter.template createError<error_classes::Validating>(validate_errors::Invalid_String_Characters);
				return false;
			}
			++newPtr;
			while (newPtr != endPtr && *newPtr != 0x22u) {
				if (*newPtr == 0x5Cu) {
					++newPtr;

					if (*newPtr == 0x22u || *newPtr == 0x5Cu || *newPtr == 0x2Fu || *newPtr == 0x62u || *newPtr == 0x66u || *newPtr == 0x6Eu || *newPtr == 0x72u ||
						*newPtr == 0x74u) {
						++newPtr;
					} else if (*newPtr == 0x75u) {
						++newPtr;
						for (int32_t i = 0; i < 4; ++i) {
							if (!hexDigits[*newPtr]) {
								iter.template createError<error_classes::Validating>(validate_errors::Invalid_Escape_Characters);
								return false;
							}
							++newPtr;
						}
					} else {
						iter.template createError<error_classes::Validating>(validate_errors::Invalid_Escape_Characters);
						return false;
					}
				} else if (*newPtr < 0x20) {
					iter.template createError<error_classes::Validating>(validate_errors::Invalid_String_Characters);
					return false;
				} else {
					++newPtr;
				}
			}

			if (*newPtr != 0x22u) {
				iter.template createError<error_classes::Validating>(validate_errors::Invalid_String_Characters);
				return false;
			}

			return iter.operator bool();
		}
	};

	template<typename derived_type> struct validate_impl<json_structural_type::Number, derived_type> {
		template<jsonifier::concepts::is_fwd_iterator iterator_type01> static bool impl(iterator_type01&& iter) {
			auto newPtr = iter.operator->();
			++iter;
			auto end = iter.operator->();
			skipWs(newPtr);
			auto newSize = end - newPtr;
			if (newSize > 1 && *newPtr == 0x30u && numberTable[*(newPtr + 1)]) {
				iter.template createError<error_classes::Validating>(validate_errors::Invalid_Number_Value);
				return false;
			}

			skipWs(newPtr);

			auto consumeChar = [&](char expected) {
				if (*newPtr == expected) {
					++newPtr;
					return true;
				}
				return false;
			};

			auto consumeDigits = [&](uint64_t min_count = 1) {
				uint64_t count = 0;
				while (digitTableBool[static_cast<uint64_t>(*newPtr)]) {
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

			consumeSign();

			consumeDigits(1);

			if (consumeChar(0x2Eu)) {
				if (!consumeDigits(1)) {
					iter.template createError<error_classes::Validating>(validate_errors::Invalid_Number_Value);
					return false;
				}
			}
			if (consumeChar(0x65u) || consumeChar(0x45u)) {
				bool didWeFail{ false };
				consumeSign();
				didWeFail = !consumeDigits(1);
				if (didWeFail) {
					iter.template createError<error_classes::Validating>(validate_errors::Invalid_Number_Value);
					return false;
				}
			}
			skipWs(newPtr);
			if (newPtr != end) {
				iter.template createError<error_classes::Validating>(validate_errors::Invalid_Number_Value);
				return false;
			}

			return true;
		}
	};

	template<typename derived_type> struct validate_impl<json_structural_type::Bool, derived_type> {
		template<jsonifier::concepts::is_fwd_iterator iterator_type01> static bool impl(iterator_type01&& iter) {
			auto newPtr = iter.operator->();
			++iter;
			static constexpr char falseStr[]{ "false" };
			static constexpr char trueStr[]{ "true" };
			skipWs(newPtr);
			if (std::memcmp(newPtr, trueStr, std::strlen(trueStr)) == 0) {
				newPtr += std::size(trueStr) - 1;
			} else if (std::memcmp(newPtr, falseStr, std::strlen(falseStr)) == 0) {
				newPtr += std::size(falseStr) - 1;
			} else {
				iter.template createError<error_classes::Validating>(validate_errors::Invalid_Bool_Value);
				return false;
			}

			return iter.operator bool();
		}
	};

	template<typename derived_type> struct validate_impl<json_structural_type::Null, derived_type> {
		template<jsonifier::concepts::is_fwd_iterator iterator_type01> static bool impl(iterator_type01&& iter) {
			auto newPtr = iter.operator->();
			++iter;
			skipWs(newPtr);
			static constexpr char nullStr[]{ "null" };

			if (std::memcmp(newPtr, nullStr, std::strlen(nullStr)) == 0) {
				newPtr += std::size(nullStr) - 1;
			} else {
				iter.template createError<error_classes::Validating>(validate_errors::Invalid_Null_Value);
				return false;
			}

			return iter.operator bool();
		}
	};

}