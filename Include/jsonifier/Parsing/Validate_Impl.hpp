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

#include <jsonifier/Parsing/Validator.hpp>

namespace jsonifier::internal {

	inline static constexpr parse_options optionsVal{};

	template<typename derived_type> struct validate_impl<json_structural_type::object_start, derived_type> {
		template<typename validator_type, typename iterator> inline static bool impl(iterator&& iter, iterator&& end, validator_type& validatorRef) noexcept {
			if JSONIFIER_LIKELY (*iter && **iter == '{') {
				++iter;
				if JSONIFIER_UNLIKELY (*iter && **iter == '}') {
					++iter;
					return true;
				}

				while (*iter < *end) {
					if JSONIFIER_LIKELY (validate_impl<json_structural_type::string, derived_type>::impl(iter, validatorRef)) {
						if JSONIFIER_LIKELY (*iter && **iter == ':') {
							++iter;
							if JSONIFIER_LIKELY (validator<derived_type>::impl(iter, end, validatorRef)) {
								if JSONIFIER_LIKELY (*iter && **iter == ',') {
									++iter;
								} else {
									if (!*iter || **iter == '}') {
										++iter;
										return true;
									} else {
										validatorRef.getErrors().emplace_back(error::constructError<error_classes::Validating, validate_errors::Missing_Comma_Or_Closing_Brace>(
											getUnderlyingPtr(*iter) - validatorRef.rootIter, validatorRef.endIter - validatorRef.rootIter, validatorRef.rootIter));
										return false;
									}
								}
							} else {
								return false;
							}
						} else {
							validatorRef.getErrors().emplace_back(error::constructError<error_classes::Validating, validate_errors::Missing_Colon>(
								getUnderlyingPtr(*iter) - validatorRef.rootIter, validatorRef.endIter - validatorRef.rootIter, validatorRef.rootIter));
							return false;
						}
					} else {
						return false;
					}
				}
				return false;
			} else {
				validatorRef.getErrors().emplace_back(error::constructError<error_classes::Validating, validate_errors::Missing_Object_Start>(
					getUnderlyingPtr(*iter) - validatorRef.rootIter, validatorRef.endIter - validatorRef.rootIter, validatorRef.rootIter));
				return false;
			}
		}
	};

	template<typename derived_type> struct validate_impl<json_structural_type::array_start, derived_type> {
		template<typename validator_type, typename iterator> inline static bool impl(iterator&& iter, iterator&& end, validator_type& validatorRef) noexcept {
			if JSONIFIER_LIKELY (*iter && **iter == '[') {
				++iter;
				if JSONIFIER_UNLIKELY (*iter && **iter == ']') {
					++iter;
					return true;
				}

				while (*iter) {
					if JSONIFIER_LIKELY (validator<derived_type>::impl(iter, end, validatorRef)) {
						if JSONIFIER_LIKELY (*iter && **iter == ',') {
							++iter;
						} else {
							if (*iter && **iter == ']') {
								++iter;
								return true;
							} else {
								validatorRef.getErrors().emplace_back(error::constructError<error_classes::Validating, validate_errors::Missing_Comma_Or_Closing_Brace>(
									getUnderlyingPtr(*iter) - validatorRef.rootIter, validatorRef.endIter - validatorRef.rootIter, validatorRef.rootIter));
								return false;
							}
						}
					} else {
						return false;
					}
				}
				return false;
			} else {
				validatorRef.getErrors().emplace_back(error::constructError<error_classes::Validating, validate_errors::Missing_Array_Start>(
					getUnderlyingPtr(*iter) - validatorRef.rootIter, validatorRef.endIter - validatorRef.rootIter, validatorRef.rootIter));
				return false;
			}
		}
	};

	template<typename derived_type> struct validate_impl<json_structural_type::string, derived_type> {
		template<typename validator_type, typename iterator> JSONIFIER_INLINE static bool impl(iterator&& iter, validator_type& validatorRef) noexcept {
			if JSONIFIER_LIKELY (*iter && **iter == '"') {
				auto newPtr = *iter;
				++iter;
				auto endPtr = *iter;
				newPtr		= string_parser<optionsVal, decltype(newPtr), decltype(validatorRef.getStringBuffer().data())>::impl(newPtr, validatorRef.getStringBuffer().data(),
						 static_cast<size_t>(endPtr - newPtr));
				if JSONIFIER_LIKELY (newPtr) {
					return true;
				}
				JSONIFIER_ELSE_UNLIKELY(else) {
					validatorRef.getErrors().emplace_back(error::constructError<error_classes::Validating, validate_errors::Invalid_String_Characters>(
						getUnderlyingPtr(*iter) - validatorRef.rootIter, validatorRef.endIter - validatorRef.rootIter, validatorRef.rootIter));
					return false;
				}
			}
			JSONIFIER_ELSE_UNLIKELY(else) {
				validatorRef.getErrors().emplace_back(error::constructError<error_classes::Validating, validate_errors::Invalid_String_Characters>(
					getUnderlyingPtr(*iter) - validatorRef.rootIter, validatorRef.endIter - validatorRef.rootIter, validatorRef.rootIter));
				return false;
			}
		}
	};

	template<typename derived_type> struct validate_impl<json_structural_type::number, derived_type> {
		JSONIFIER_INLINE static bool consumeChar(char expected, string_view_ptr& newerPtr) {
			if (*newerPtr == expected) {
				++newerPtr;
				return true;
			}
			return false;
		};

		JSONIFIER_INLINE static bool consumeDigits(string_view_ptr& newerPtr, uint64_t min_count = 1) {
			uint64_t count = 0;
			while (JSONIFIER_IS_DIGIT(*newerPtr)) {
				++newerPtr;
				++count;
			}
			return count >= min_count;
		};

		JSONIFIER_INLINE static void consumeSign(string_view_ptr& newerPtr) {
			if (*newerPtr == '-' || *newerPtr == '+') {
				++newerPtr;
			}
			return;
		};

		template<typename validator_type, typename iterator> JSONIFIER_INLINE static bool impl(iterator&& iter, validator_type& validatorRef) noexcept {
			auto newPtr = *iter;
			++iter;
			if JSONIFIER_LIKELY (*iter && (*newPtr != 0x30u || !numberTable[static_cast<uint64_t>(*(newPtr + 1))])) {
				consumeSign(newPtr);

				consumeDigits(newPtr);

				if (consumeChar(0x2Eu, newPtr)) {
					if (!*iter || !consumeDigits(newPtr)) {
						validatorRef.getErrors().emplace_back(error::constructError<error_classes::Validating, validate_errors::Invalid_Number_Value>(
							getUnderlyingPtr(*iter) - validatorRef.rootIter, validatorRef.endIter - validatorRef.rootIter, validatorRef.rootIter));
						return false;
					}
				}
				if (consumeChar(0x65u, newPtr) || consumeChar(0x45u, newPtr)) {
					consumeSign(newPtr);
					if (!*iter || consumeDigits(newPtr)) {
						validatorRef.getErrors().emplace_back(error::constructError<error_classes::Validating, validate_errors::Invalid_Number_Value>(
							getUnderlyingPtr(*iter) - validatorRef.rootIter, validatorRef.endIter - validatorRef.rootIter, validatorRef.rootIter));
						return false;
					}
				}
				return true;
			} else {
				validatorRef.getErrors().emplace_back(error::constructError<error_classes::Validating, validate_errors::Invalid_Number_Value>(
					getUnderlyingPtr(*iter) - validatorRef.rootIter, validatorRef.endIter - validatorRef.rootIter, validatorRef.rootIter));
				return false;
			}
		}
	};

	template<typename derived_type> struct validate_impl<json_structural_type::boolean, derived_type> {
		template<typename validator_type, typename iterator> JSONIFIER_INLINE static bool impl(iterator&& iter, validator_type& validatorRef) noexcept {
			if JSONIFIER_LIKELY ((*iter + 4) < validatorRef.endIter && validateBool(*iter)) {
				++iter;
				return true;
			} else {
				validatorRef.getErrors().emplace_back(error::constructError<error_classes::Validating, validate_errors::Invalid_Bool_Value>(
					getUnderlyingPtr(*iter) - validatorRef.rootIter, validatorRef.endIter - validatorRef.rootIter, validatorRef.rootIter));
				return false;
			}
		}
	};

	template<typename derived_type> struct validate_impl<json_structural_type::null, derived_type> {
		template<typename validator_type, typename iterator> JSONIFIER_INLINE static bool impl(iterator&& iter, validator_type& validatorRef) noexcept {
			if JSONIFIER_LIKELY (validateNull(*iter)) {
				++iter;
				return true;
			} else {
				validatorRef.getErrors().emplace_back(error::constructError<error_classes::Validating, validate_errors::Invalid_Null_Value>(
					getUnderlyingPtr(*iter) - validatorRef.rootIter, validatorRef.endIter - validatorRef.rootIter, validatorRef.rootIter));
				return false;
			}
		}
	};

}