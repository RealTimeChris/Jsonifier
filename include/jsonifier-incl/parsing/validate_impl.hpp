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

#include <jsonifier-incl/parsing/validator.hpp>

namespace jsonifier::internal {

	static constexpr parse_options optionsVal{ [] {
		parse_options return_value{};
		return_value.validateUtf8 = true;
		return return_value;
	}() };

	template<typename derived_type> struct validate_impl<json_structural_type::object_start, derived_type> {
		template<typename context_type> inline static bool impl(context_type& context) noexcept {
			if JSONIFIER_LIKELY (context.template checkChar<'{'>()) {
				++context.currentIterPtr();
				if JSONIFIER_UNLIKELY (context.template checkChar<'}'>()) {
					++context.currentIterPtr();
					return true;
				}

				while (context.notAtEndPre()) {
					if JSONIFIER_LIKELY (validate_impl<json_structural_type::string, derived_type>::impl(context)) {
						if JSONIFIER_LIKELY (context.template checkChar<':'>()) {
							++context.currentIterPtr();
							if JSONIFIER_LIKELY (validator<derived_type>::impl(context)) {
								if JSONIFIER_LIKELY (context.template checkChar<','>()) {
									++context.currentIterPtr();
								} else {
									if (context.template checkChar<'}'>()) {
										++context.currentIterPtr();
										return true;
									} else {
										return false;
									}
								}
							} else {
								return false;
							}
						} else {
							return false;
						}
					} else {
						return false;
					}
				}
				return false;
			} else {
				return false;
			}
		}
	};

	template<typename derived_type> struct validate_impl<json_structural_type::array_start, derived_type> {
		template<typename context_type> inline static bool impl(context_type& context) noexcept {
			if JSONIFIER_LIKELY (context.template checkChar<'['>()) {
				++context.currentIterPtr();
				if JSONIFIER_UNLIKELY (context.template checkChar<']'>()) {
					++context.currentIterPtr();
					return true;
				}
				while (context.notAtEndPre()) {
					if JSONIFIER_LIKELY (validator<derived_type>::impl(context)) {
						if JSONIFIER_LIKELY (context.template checkChar<','>()) {
							++context.currentIterPtr();
						} else {
							if (context.template checkChar<']'>()) {
								++context.currentIterPtr();
								return true;
							} else {
								return false;
							}
						}
					} else {
						return false;
					}
				}
				return false;
			} else {
				return false;
			}
		}
	};

	template<typename derived_type> struct validate_impl<json_structural_type::string, derived_type> {
		template<typename context_type> inline static bool impl(context_type& context) noexcept {
			if JSONIFIER_LIKELY (context.template checkChar<'"'>()) {
				auto newPtr = context.currentPtr();
				++context.currentIterPtr();
				auto endPtr = context.notAtEndPre() ? context.currentPtr() : (newPtr + (context.endIterPtr() - context.currentIterPtr()));
				newPtr		= string_parser<optionsVal>::impl(newPtr, context.getStringBuffer().data(), static_cast<uint64_t>(endPtr - newPtr));
				if JSONIFIER_LIKELY (newPtr) {
					return true;
				}
				JSONIFIER_ELSE_UNLIKELY(else) {
					return false;
				}
			}
			JSONIFIER_ELSE_UNLIKELY(else) {
				return false;
			}
		}
	};

	template<typename derived_type> struct validate_impl<json_structural_type::number, derived_type> {
		inline static bool consumeChar(char expected, string_view_ptr& newerPtr) {
			if (*newerPtr == expected) {
				++newerPtr;
				return true;
			}
			return false;
		}

		inline static bool consumeDigits(string_view_ptr& newerPtr, uint64_t minCount = 1) {
			uint64_t count = 0;
			while (JSONIFIER_IS_DIGIT(*newerPtr)) {
				++newerPtr;
				++count;
			}
			return count >= minCount;
		}

		inline static void consumeSign(string_view_ptr& newerPtr) {
			if (*newerPtr == '-' || *newerPtr == '+') {
				++newerPtr;
			}
			return;
		}

		template<typename context_type> inline static bool impl(context_type& context) noexcept {
			auto newPtr = context.currentPtr();
			++context.currentIterPtr();
			if JSONIFIER_LIKELY (context.notAtEndPre() && (*newPtr != 0x30u || !numberTable[static_cast<uint64_t>(*(newPtr + 1))])) {
				consumeSign(newPtr);
				consumeDigits(newPtr);
				if (consumeChar(0x2Eu, newPtr)) {
					if (!context.notAtEndPre() || !consumeDigits(newPtr)) {
						return false;
					}
				}
				if (consumeChar(0x65u, newPtr) || consumeChar(0x45u, newPtr)) {
					consumeSign(newPtr);
				}
				return true;
			} else {
				return false;
			}
		}
	};

	template<typename derived_type> struct validate_impl<json_structural_type::boolean, derived_type> {
		template<typename context_type> inline static bool impl(context_type& context) noexcept {
			if JSONIFIER_LIKELY (context.notAtEndPre() && validateBool(context.currentPtr())) {
				++context.currentIterPtr();
				return true;
			} else {
				return false;
			}
		}
	};

	template<typename derived_type> struct validate_impl<json_structural_type::null, derived_type> {
		template<typename context_type> inline static bool impl(context_type& context) noexcept {
			if JSONIFIER_LIKELY (context.notAtEndPre() && validateNull(context.currentPtr())) {
				++context.currentIterPtr();
				return true;
			} else {
				return false;
			}
		}
	};

}
