/*
	MIT License

	Copyright (c) 2024 RealTimeChris

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

	static constexpr bool optionsVal{};

	template<typename derived_type> struct validate_impl<json_structural_type::Object_Start, derived_type> {
		template<typename validator_type, typename iterator> static bool impl(iterator&& iter, iterator&& end, uint64_t& depth, validator_type& validatorRef) noexcept {
			if (!*iter || **iter != lBrace) {
				validatorRef.getErrors().emplace_back(error::constructError<error_classes::Validating, validate_errors::Missing_Object_Start>(
					getUnderlyingPtr(*iter) - validatorRef.rootIter, validatorRef.endIter - validatorRef.rootIter, validatorRef.rootIter));
				return false;
			}
			++depth;
			++iter;
			if (!*iter) {
				validatorRef.getErrors().emplace_back(error::constructError<error_classes::Validating, validate_errors::Missing_Object_Start>(
					getUnderlyingPtr(*iter) - validatorRef.rootIter, validatorRef.endIter - validatorRef.rootIter, validatorRef.rootIter));
				return false;
			}
			if (*iter && **iter == rBrace) {
				++iter;
				--depth;
				return true;
			}

			while (*iter < *end) {
				if (!validate_impl<json_structural_type::String, derived_type>::impl(iter, validatorRef)) {
					validatorRef.getErrors().emplace_back(error::constructError<error_classes::Validating, validate_errors::Invalid_String_Characters>(
						getUnderlyingPtr(*iter) - validatorRef.rootIter, validatorRef.endIter - validatorRef.rootIter, validatorRef.rootIter));
					return false;
				}

				if (!*iter || **iter != colon) {
					validatorRef.getErrors().emplace_back(error::constructError<error_classes::Validating, validate_errors::Missing_Colon>(
						getUnderlyingPtr(*iter) - validatorRef.rootIter, validatorRef.endIter - validatorRef.rootIter, validatorRef.rootIter));
					return false;
				}

				++iter;
				if (!validator<derived_type>::impl(iter, end, depth, validatorRef)) {
					validatorRef.getErrors().emplace_back(error::constructError<error_classes::Validating, validate_errors::Missing_Object_Start>(
						getUnderlyingPtr(*iter) - validatorRef.rootIter, validatorRef.endIter - validatorRef.rootIter, validatorRef.rootIter));
					return false;
				}

				if (*iter >= *end || **iter == 0) {
					return true;
				}

				if (*iter && **iter == comma) {
					++iter;
				} else {
					if (!*iter || **iter == rBrace) {
						++iter;
						if (*iter >= *end || **iter == 0) {
							return true;
						}
						if (*iter && **iter != comma && **iter != rBracket && **iter != rBrace) {
							validatorRef.getErrors().emplace_back(error::constructError<error_classes::Validating, validate_errors::Missing_Comma_Or_Closing_Brace>(
								getUnderlyingPtr(*iter) - validatorRef.rootIter, validatorRef.endIter - validatorRef.rootIter, validatorRef.rootIter));
							return false;
						}
						--depth;
						return true;
					} else {
						validatorRef.getErrors().emplace_back(error::constructError<error_classes::Validating, validate_errors::Missing_Comma_Or_Closing_Brace>(
							getUnderlyingPtr(*iter) - validatorRef.rootIter, validatorRef.endIter - validatorRef.rootIter, validatorRef.rootIter));
						return false;
					}
				}
			}
			if (!*iter || (**iter != comma && **iter != rBrace)) {
				validatorRef.getErrors().emplace_back(error::constructError<error_classes::Validating, validate_errors::Missing_Comma_Or_Closing_Brace>(
					getUnderlyingPtr(*iter) - validatorRef.rootIter, validatorRef.endIter - validatorRef.rootIter, validatorRef.rootIter));
				return false;
			}
			--depth;
			return true;
		}
	};

	template<typename derived_type> struct validate_impl<json_structural_type::Array_Start, derived_type> {
		template<typename validator_type, typename iterator> static bool impl(iterator&& iter, iterator&& end, uint64_t& depth, validator_type& validatorRef) noexcept {
			if (!*iter || **iter != lBracket) {
				validatorRef.getErrors().emplace_back(error::constructError<error_classes::Validating, validate_errors::Missing_Array_Start>(
					getUnderlyingPtr(*iter) - validatorRef.rootIter, validatorRef.endIter - validatorRef.rootIter, validatorRef.rootIter));
				return false;
			}
			++depth;
			++iter;

			if (!*iter) {
				validatorRef.getErrors().emplace_back(error::constructError<error_classes::Validating, validate_errors::Missing_Array_Start>(
					getUnderlyingPtr(*iter) - validatorRef.rootIter, validatorRef.endIter - validatorRef.rootIter, validatorRef.rootIter));
				return false;
			}
			if (*iter && **iter == rBracket) {
				++iter;
				--depth;
				return true;
			}

			while (*iter) {
				if (!validator<derived_type>::impl(iter, end, depth, validatorRef)) {
					validatorRef.getErrors().emplace_back(error::constructError<error_classes::Validating, validate_errors::Missing_Object_Start>(
						getUnderlyingPtr(*iter) - validatorRef.rootIter, validatorRef.endIter - validatorRef.rootIter, validatorRef.rootIter));
					return false;
				}
				if (*iter && **iter == comma) {
					++iter;
				} else {
					if (*iter && **iter == rBracket) {
						++iter;
						if (*iter && **iter != comma && **iter != rBracket && **iter != rBrace) {
							validatorRef.getErrors().emplace_back(error::constructError<error_classes::Validating, validate_errors::Missing_Comma_Or_Closing_Brace>(
								getUnderlyingPtr(*iter) - validatorRef.rootIter, validatorRef.endIter - validatorRef.rootIter, validatorRef.rootIter));
							return false;
						}
						--depth;
						return true;
					} else {
						validatorRef.getErrors().emplace_back(error::constructError<error_classes::Validating, validate_errors::Missing_Comma_Or_Closing_Brace>(
							getUnderlyingPtr(*iter) - validatorRef.rootIter, validatorRef.endIter - validatorRef.rootIter, validatorRef.rootIter));
						return false;
					}
				}
			}
			if (!*iter || (**iter != comma && **iter != rBracket)) {
				validatorRef.getErrors().emplace_back(error::constructError<error_classes::Validating, validate_errors::Missing_Comma_Or_Closing_Brace>(
					getUnderlyingPtr(*iter) - validatorRef.rootIter, validatorRef.endIter - validatorRef.rootIter, validatorRef.rootIter));
				return false;
			}
			--depth;
			return true;
		}
	};

	constexpr bool hexDigits[]{ false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
		false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
		false, false, false, true, true, true, true, true, true, true, true, true, true, false, false, false, false, false, false, false, true, true, true, true, true, true, false,
		false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
		false, true, true, true, true, true, true, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
		false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
		false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
		false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
		false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
		false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
		false, false, false, false, false, false, false, false, false, false, false, false, false, false, false };

	template<typename derived_type> struct validate_impl<json_structural_type::String, derived_type> {
		template<typename validator_type, typename iterator> static bool impl(iterator&& iter, validator_type& validatorRef) noexcept {
			auto newPtr = static_cast<string_view_ptr>(*iter);
			++iter;
			auto endPtr = static_cast<string_view_ptr>(*iter);
			derailleur<optionsVal, bool, iterator>::skipWs(newPtr);
			if (newPtr == endPtr || *newPtr != quote) {
				validatorRef.getErrors().emplace_back(error::constructError<error_classes::Validating, validate_errors::Invalid_String_Characters>(
					getUnderlyingPtr(*iter) - validatorRef.rootIter, validatorRef.endIter - validatorRef.rootIter, validatorRef.rootIter));
				return false;
			}
			++newPtr;
			while (newPtr != endPtr && *newPtr != quote) {
				if (*newPtr == backslash) {
					++newPtr;

					if (*newPtr == quote || *newPtr == backslash || *newPtr == 0x2Fu || *newPtr == 0x62u || *newPtr == 0x66u || *newPtr == n || *newPtr == 0x72u ||
						*newPtr == 't') {
						++newPtr;
					} else {
						if (*newPtr == 0x75u) {
							++newPtr;
							for (uint64_t i = 0ull; i < 4ull; ++i) {
								if (!hexDigits[static_cast<uint64_t>(*newPtr)]) {
									validatorRef.getErrors().emplace_back(error::constructError<error_classes::Validating, validate_errors::Invalid_Escape_Characters>(
										getUnderlyingPtr(*iter) - validatorRef.rootIter, validatorRef.endIter - validatorRef.rootIter, validatorRef.rootIter));
									return false;
								}
								++newPtr;
							}
						} else {
							validatorRef.getErrors().emplace_back(error::constructError<error_classes::Validating, validate_errors::Invalid_Escape_Characters>(
								getUnderlyingPtr(*iter) - validatorRef.rootIter, validatorRef.endIter - validatorRef.rootIter, validatorRef.rootIter));
							return false;
						}
					}
				} else {
					if (static_cast<uint8_t>(*newPtr) < 0x20u) {
						validatorRef.getErrors().emplace_back(error::constructError<error_classes::Validating, validate_errors::Invalid_String_Characters>(
							getUnderlyingPtr(*iter) - validatorRef.rootIter, validatorRef.endIter - validatorRef.rootIter, validatorRef.rootIter));
						return false;
					} else {
						++newPtr;
					}
				}
			}

			if (*newPtr != quote) {
				validatorRef.getErrors().emplace_back(error::constructError<error_classes::Validating, validate_errors::Invalid_String_Characters>(
					getUnderlyingPtr(*iter) - validatorRef.rootIter, validatorRef.endIter - validatorRef.rootIter, validatorRef.rootIter));
				return false;
			}

			return true;
		}
	};

	template<typename derived_type> struct validate_impl<json_structural_type::Number, derived_type> {
		template<typename validator_type, typename iterator> static bool impl(iterator&& iter, validator_type& validatorRef) noexcept {
			auto newPtr = static_cast<string_view_ptr>(*iter);
			++iter;
			if (!*iter || (*newPtr == 0x30u && numberTable[static_cast<uint64_t>(*(newPtr + 1))])) {
				validatorRef.getErrors().emplace_back(error::constructError<error_classes::Validating, validate_errors::Invalid_Number_Value>(
					getUnderlyingPtr(*iter) - validatorRef.rootIter, validatorRef.endIter - validatorRef.rootIter, validatorRef.rootIter));
				return false;
			}
			auto endPtr = static_cast<string_view_ptr>(*iter);

			derailleur<optionsVal, bool, iterator>::skipWs(newPtr);

			auto consumeChar = [&](char expected) {
				if (*newPtr == expected) {
					++newPtr;
					return true;
				}
				return false;
			};

			auto consumeDigits = [&](uint64_t min_count = 1) {
				uint64_t count = 0;
				while (JSONIFIER_IS_DIGIT(*newPtr)) {
					++newPtr;
					++count;
				}
				return count >= min_count;
			};

			auto consumeSign = [&] {
				if (*newPtr == '-' || *newPtr == '+') {
					++newPtr;
				}
				return true;
			};

			if (!consumeSign()) {
				validatorRef.getErrors().emplace_back(error::constructError<error_classes::Validating, validate_errors::Invalid_Number_Value>(
					getUnderlyingPtr(*iter) - validatorRef.rootIter, validatorRef.endIter - validatorRef.rootIter, validatorRef.rootIter));
				return false;
			}

			consumeDigits();

			if (consumeChar(0x2Eu)) {
				if (!*iter || !consumeDigits(1)) {
					validatorRef.getErrors().emplace_back(error::constructError<error_classes::Validating, validate_errors::Invalid_Number_Value>(
						getUnderlyingPtr(*iter) - validatorRef.rootIter, validatorRef.endIter - validatorRef.rootIter, validatorRef.rootIter));
					return false;
				}
			}
			if (consumeChar(0x65u) || consumeChar(0x45u)) {
				bool didWeFail{};
				consumeSign();
				didWeFail = !consumeDigits(1);
				if (!*iter || didWeFail) {
					validatorRef.getErrors().emplace_back(error::constructError<error_classes::Validating, validate_errors::Invalid_Number_Value>(
						getUnderlyingPtr(*iter) - validatorRef.rootIter, validatorRef.endIter - validatorRef.rootIter, validatorRef.rootIter));
					return false;
				}
			}
			derailleur<optionsVal, bool, iterator>::skipWs(newPtr);
			if (newPtr != endPtr) {
				validatorRef.getErrors().emplace_back(error::constructError<error_classes::Validating, validate_errors::Invalid_Number_Value>(
					getUnderlyingPtr(*iter) - validatorRef.rootIter, validatorRef.endIter - validatorRef.rootIter, validatorRef.rootIter));
				return false;
			}

			return true;
		}
	};

	template<typename derived_type> struct validate_impl<json_structural_type::Bool, derived_type> {
		template<typename validator_type, typename iterator> static bool impl(iterator&& iter, validator_type& validatorRef) noexcept {
			auto newPtr = static_cast<string_view_ptr>(*iter);
			++iter;
			alignas(8) static constexpr char falseStr[]{ "false" };
			alignas(4) static constexpr char trueStr[]{ "true" };
			derailleur<optionsVal, bool, iterator>::skipWs(newPtr);
			if (*iter && std::memcmp(newPtr, trueStr, std::strlen(trueStr)) == 0) {
				newPtr += std::size(trueStr) - 1;
			} else {
				if (*iter && std::memcmp(newPtr, falseStr, std::strlen(falseStr)) == 0) {
					newPtr += std::size(falseStr) - 1;
				} else {
					validatorRef.getErrors().emplace_back(error::constructError<error_classes::Validating, validate_errors::Invalid_Bool_Value>(
						getUnderlyingPtr(*iter) - validatorRef.rootIter, validatorRef.endIter - validatorRef.rootIter, validatorRef.rootIter));
					return false;
				}
			}

			return true;
		}
	};

	template<typename derived_type> struct validate_impl<json_structural_type::Null, derived_type> {
		template<typename validator_type, typename iterator> static bool impl(iterator&& iter, validator_type& validatorRef) noexcept {
			auto newPtr = static_cast<string_view_ptr>(*iter);
			++iter;
			derailleur<optionsVal, bool, iterator>::skipWs(newPtr);
			alignas(4) static constexpr char nullStr[]{ "null" };

			if (std::memcmp(newPtr, nullStr, std::strlen(nullStr)) == 0) {
				newPtr += std::size(nullStr) - 1;
			} else {
				validatorRef.getErrors().emplace_back(error::constructError<error_classes::Validating, validate_errors::Invalid_Null_Value>(
					getUnderlyingPtr(*iter) - validatorRef.rootIter, validatorRef.endIter - validatorRef.rootIter, validatorRef.rootIter));
				return false;
			}

			return true;
		}
	};

}