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
/// Feb 3, 2023
#pragma once

#include <jsonifier-incl/utilities/json_entity.hpp>
#include <jsonifier-incl/simd/avx_validation.hpp>
#include <jsonifier-incl/simd/neon_validation.hpp>
#include <jsonifier-incl/parsing/validator.hpp>
#include <jsonifier-incl/utilities/hash_map.hpp>
#include <jsonifier-incl/utilities/number_utils.hpp>
#include <jsonifier-incl/utilities/string.hpp>
#include <jsonifier-incl/utilities/error.hpp>
#include <jsonifier-incl/utilities/simd.hpp>

namespace jsonifier::internal {

	template<json_structural_characters json_structural_character, typename iterator_type> struct iterator_interface;

	template<typename iterator_type> struct iterator_interface<json_structural_characters::l_crl_bracket, iterator_type> {
		static constexpr jsonifier::internal::array<jsonifier::internal::parse_statuses, 256> genTable() {
			jsonifier::internal::array<jsonifier::internal::parse_statuses, 256> return_values{};
			return_values.fill(jsonifier::internal::parse_statuses::invalid_string_characters);
			return_values[static_cast<uint8_t>('}')] = jsonifier::internal::parse_statuses::success;
			return_values[static_cast<uint8_t>('"')] = jsonifier::internal::parse_statuses::success;
			return return_values;
		}
		static constexpr jsonifier::internal::array<jsonifier::internal::parse_statuses, 256> statusValues{ genTable() };
	};

	template<typename iterator_type> struct iterator_interface<json_structural_characters::l_sqr_bracket, iterator_type> {
		static constexpr jsonifier::internal::array<jsonifier::internal::parse_statuses, 256> genTable() {
			jsonifier::internal::array<jsonifier::internal::parse_statuses, 256> return_values{};
			return_values.fill(jsonifier::internal::parse_statuses::unfinished_input);
			return_values[static_cast<uint8_t>(':')] = jsonifier::internal::parse_statuses::missing_array_end;
			return_values[static_cast<uint8_t>('}')] = jsonifier::internal::parse_statuses::missing_array_end;
			return_values[static_cast<uint8_t>(']')] = jsonifier::internal::parse_statuses::success;
			return_values[static_cast<uint8_t>('"')] = jsonifier::internal::parse_statuses::success;
			return_values[static_cast<uint8_t>('{')] = jsonifier::internal::parse_statuses::success;
			return_values[static_cast<uint8_t>('[')] = jsonifier::internal::parse_statuses::success;
			return_values[static_cast<uint8_t>('t')] = jsonifier::internal::parse_statuses::success;
			return_values[static_cast<uint8_t>('f')] = jsonifier::internal::parse_statuses::success;
			return_values[static_cast<uint8_t>('n')] = jsonifier::internal::parse_statuses::success;
			return_values[static_cast<uint8_t>('-')] = jsonifier::internal::parse_statuses::success;
			for (size_t i = '0'; i <= '9'; ++i) {
				return_values[static_cast<uint8_t>(i)] = jsonifier::internal::parse_statuses::success;
			}
			return return_values;
		}
		static constexpr jsonifier::internal::array<jsonifier::internal::parse_statuses, 256> statusValues{ genTable() };
	};

	template<typename iterator_type> struct iterator_interface<json_structural_characters::colon, iterator_type> {
		static constexpr jsonifier::internal::array<jsonifier::internal::parse_statuses, 256> genTable() {
			jsonifier::internal::array<jsonifier::internal::parse_statuses, 256> return_values{};
			return_values.fill(jsonifier::internal::parse_statuses::missing_colon);
			return_values[static_cast<uint8_t>(',')] = jsonifier::internal::parse_statuses::invalid_null_value;
			return_values[static_cast<uint8_t>('}')] = jsonifier::internal::parse_statuses::missing_object_end;
			return_values[static_cast<uint8_t>(']')] = jsonifier::internal::parse_statuses::missing_array_end;
			for (size_t i = 0; i < 256; ++i) {
				if (i >= 'a' && i <= 'z') {
					if (i != 't' && i != 'f' && i != 'n') {
						return_values[i] = jsonifier::internal::parse_statuses::invalid_null_value;
					}
				}
			}
			return_values[static_cast<uint8_t>('"')] = jsonifier::internal::parse_statuses::success;
			return_values[static_cast<uint8_t>('{')] = jsonifier::internal::parse_statuses::success;
			return_values[static_cast<uint8_t>('[')] = jsonifier::internal::parse_statuses::success;
			return_values[static_cast<uint8_t>('t')] = jsonifier::internal::parse_statuses::success;
			return_values[static_cast<uint8_t>('f')] = jsonifier::internal::parse_statuses::success;
			return_values[static_cast<uint8_t>('n')] = jsonifier::internal::parse_statuses::success;
			return_values[static_cast<uint8_t>('-')] = jsonifier::internal::parse_statuses::success;
			for (size_t i = '0'; i <= '9'; ++i) {
				return_values[static_cast<uint8_t>(i)] = jsonifier::internal::parse_statuses::success;
			}
			return return_values;
		}
		static constexpr jsonifier::internal::array<jsonifier::internal::parse_statuses, 256> statusValues{ genTable() };
	};

	template<typename iterator_type> struct iterator_interface<json_structural_characters::comma, iterator_type> {
		static constexpr jsonifier::internal::array<jsonifier::internal::parse_statuses, 256> genTable() {
			jsonifier::internal::array<jsonifier::internal::parse_statuses, 256> return_values{};
			return_values.fill(jsonifier::internal::parse_statuses::unfinished_input);
			return_values[static_cast<uint8_t>('}')] = jsonifier::internal::parse_statuses::invalid_string_characters;
			return_values[static_cast<uint8_t>(']')] = jsonifier::internal::parse_statuses::unfinished_input;
			return_values[static_cast<uint8_t>(',')] = jsonifier::internal::parse_statuses::unfinished_input;
			return_values[static_cast<uint8_t>('"')] = jsonifier::internal::parse_statuses::success;
			return_values[static_cast<uint8_t>('{')] = jsonifier::internal::parse_statuses::success;
			return_values[static_cast<uint8_t>('[')] = jsonifier::internal::parse_statuses::success;
			return_values[static_cast<uint8_t>('t')] = jsonifier::internal::parse_statuses::success;
			return_values[static_cast<uint8_t>('f')] = jsonifier::internal::parse_statuses::success;
			return_values[static_cast<uint8_t>('n')] = jsonifier::internal::parse_statuses::success;
			return_values[static_cast<uint8_t>('-')] = jsonifier::internal::parse_statuses::success;
			for (size_t i = '0'; i <= '9'; ++i) {
				return_values[static_cast<uint8_t>(i)] = jsonifier::internal::parse_statuses::success;
			}
			return return_values;
		}
		static constexpr jsonifier::internal::array<jsonifier::internal::parse_statuses, 256> statusValues{ genTable() };
	};

	template<typename iterator_type> struct iterator_interface<json_structural_characters::r_crl_bracket, iterator_type> {
		static constexpr jsonifier::internal::array<jsonifier::internal::parse_statuses, 256> genTable() {
			jsonifier::internal::array<jsonifier::internal::parse_statuses, 256> return_values{};
			return_values.fill(jsonifier::internal::parse_statuses::unexpected_string_end);
			return_values[static_cast<uint8_t>('"')] = jsonifier::internal::parse_statuses::missing_comma;
			return_values[static_cast<uint8_t>('{')] = jsonifier::internal::parse_statuses::missing_comma;
			return_values[static_cast<uint8_t>('[')] = jsonifier::internal::parse_statuses::missing_comma;
			return_values[static_cast<uint8_t>('t')] = jsonifier::internal::parse_statuses::missing_comma;
			return_values[static_cast<uint8_t>('f')] = jsonifier::internal::parse_statuses::missing_comma;
			return_values[static_cast<uint8_t>('n')] = jsonifier::internal::parse_statuses::missing_comma;
			return_values[static_cast<uint8_t>('-')] = jsonifier::internal::parse_statuses::missing_comma;
			for (size_t i = '0'; i <= '9'; ++i) {
				return_values[static_cast<uint8_t>(i)] = jsonifier::internal::parse_statuses::missing_comma;
			}
			return_values[static_cast<uint8_t>('}')]  = jsonifier::internal::parse_statuses::success;
			return_values[static_cast<uint8_t>(']')]  = jsonifier::internal::parse_statuses::success;
			return_values[static_cast<uint8_t>(',')]  = jsonifier::internal::parse_statuses::success;
			return_values[static_cast<uint8_t>('\0')] = jsonifier::internal::parse_statuses::success;
			return return_values;
		}
		static constexpr jsonifier::internal::array<jsonifier::internal::parse_statuses, 256> statusValues{ genTable() };
	};

	template<typename iterator_type> struct iterator_interface<json_structural_characters::r_sqr_bracket, iterator_type> {
		static constexpr jsonifier::internal::array<jsonifier::internal::parse_statuses, 256> genTable() {
			jsonifier::internal::array<jsonifier::internal::parse_statuses, 256> return_values{};
			return_values.fill(jsonifier::internal::parse_statuses::unexpected_string_end);
			return_values[static_cast<uint8_t>('"')] = jsonifier::internal::parse_statuses::missing_comma;
			return_values[static_cast<uint8_t>('{')] = jsonifier::internal::parse_statuses::missing_comma;
			return_values[static_cast<uint8_t>('[')] = jsonifier::internal::parse_statuses::missing_comma;
			return_values[static_cast<uint8_t>('t')] = jsonifier::internal::parse_statuses::missing_comma;
			return_values[static_cast<uint8_t>('f')] = jsonifier::internal::parse_statuses::missing_comma;
			return_values[static_cast<uint8_t>('n')] = jsonifier::internal::parse_statuses::missing_comma;
			return_values[static_cast<uint8_t>('-')] = jsonifier::internal::parse_statuses::missing_comma;
			for (size_t i = '0'; i <= '9'; ++i) {
				return_values[static_cast<uint8_t>(i)] = jsonifier::internal::parse_statuses::missing_comma;
			}
			return_values[static_cast<uint8_t>('}')]  = jsonifier::internal::parse_statuses::success;
			return_values[static_cast<uint8_t>(']')]  = jsonifier::internal::parse_statuses::success;
			return_values[static_cast<uint8_t>(',')]  = jsonifier::internal::parse_statuses::success;
			return_values[static_cast<uint8_t>('\0')] = jsonifier::internal::parse_statuses::success;
			return return_values;
		}
		static constexpr jsonifier::internal::array<jsonifier::internal::parse_statuses, 256> statusValues{ genTable() };
	};

	template<typename iterator_type> struct iterator_interface<json_structural_characters::post_primitive_state, iterator_type> {
		static constexpr jsonifier::internal::array<jsonifier::internal::parse_statuses, 256> genTable() {
			jsonifier::internal::array<jsonifier::internal::parse_statuses, 256> return_values{};
			return_values.fill(jsonifier::internal::parse_statuses::missing_comma);
			return_values[static_cast<uint8_t>('\0')] = jsonifier::internal::parse_statuses::success;
			return_values[static_cast<uint8_t>(',')]  = jsonifier::internal::parse_statuses::success;
			return_values[static_cast<uint8_t>('}')]  = jsonifier::internal::parse_statuses::success;
			return_values[static_cast<uint8_t>(']')]  = jsonifier::internal::parse_statuses::success;
			return_values[static_cast<uint8_t>(' ')]  = jsonifier::internal::parse_statuses::success;
			return_values[static_cast<uint8_t>('\t')] = jsonifier::internal::parse_statuses::success;
			return_values[static_cast<uint8_t>('\n')] = jsonifier::internal::parse_statuses::success;
			return_values[static_cast<uint8_t>('\r')] = jsonifier::internal::parse_statuses::success;
			return return_values;
		}
		static constexpr jsonifier::internal::array<jsonifier::internal::parse_statuses, 256> statusValues{ genTable() };
	};

	template<typename derived_type, typename iterator_type, parse_options parseOpts> struct json_iterator;

	template<typename derived_type, parse_options parseOpts> struct json_iterator<derived_type, string_view_ptr, parseOpts> {
		using size_type		= uint64_t;
		using iterator_type = string_view_ptr;

		JSONIFIER_INLINE json_iterator(derived_type* parserPtrNew, iterator_type rootIterNew, iterator_type endIterNew)
			: parserPtr{ parserPtrNew }, rootIter{ rootIterNew }, endIter{ endIterNew }, iter{ rootIterNew } {
		}

		JSONIFIER_INLINE size_type getRemainingLength() const noexcept {
			return static_cast<size_type>(endIter - iter);
		}

		JSONIFIER_INLINE void printValues(std::source_location location = std::source_location::current()) noexcept {
			std::cout << "File: " << location.file_name() << ", Line: " << location.line() << std::endl;
			if (iter) {
				std::cout << "iter: " << currentChar() << std::endl;
				std::cout << "string: " << std::string_view{ iter, 32 } << std::endl;
			}
			std::cout << "index: " << iter - rootIter << std::endl;
			std::cout << "current object depth: " << currentObjectDepth << std::endl;
			std::cout << "current array depth: " << currentArrayDepth << std::endl;
		}

		JSONIFIER_INLINE size_type getTotalLength() const noexcept {
			return static_cast<size_type>(endIter - rootIter);
		}

		JSONIFIER_INLINE bool atEnd() const noexcept {
			return iter >= endIter || !iter;
		}

		JSONIFIER_INLINE bool anyInput() noexcept {
			if (rootIter != endIter) {
				return true;
			} else {
				reportError<parse_statuses::no_input>();
				return false;
			}
		}

		JSONIFIER_INLINE bool getState() const noexcept {
			return (currentArrayDepth > 0 || currentObjectDepth > 0);
		}

		JSONIFIER_INLINE void skipWs() noexcept {
			while (iter < endIter && whitespaceTable[static_cast<uint8_t>(*iter)]) {
				++iter;
			}
		}

		JSONIFIER_INLINE char currentChar() const noexcept {
			return *iter;
		}

		JSONIFIER_INLINE char nextChar() const noexcept {
			return *(iter + 1);
		}

		JSONIFIER_INLINE bool atEndNext() const noexcept {
			return iter == endIter;
		}

		template<json_structural_characters to> JSONIFIER_INLINE void incrementDepth() {
			if constexpr (to == json_structural_characters::l_crl_bracket) {
				++currentObjectDepth;
			} else if constexpr (to == json_structural_characters::r_crl_bracket) {
				--currentObjectDepth;
			} else if constexpr (to == json_structural_characters::l_sqr_bracket) {
				++currentArrayDepth;
			} else if constexpr (to == json_structural_characters::r_sqr_bracket) {
				--currentArrayDepth;
			}
		}

		template<json_structural_characters to> constexpr bool advanceTo(std::source_location location = std::source_location::current()) {
			if (!atEnd()) {
				if (currentChar() == static_cast<char>(to)) {
					if constexpr (to == json_structural_characters::l_crl_bracket) {
						++currentObjectDepth;
					} else if constexpr (to == json_structural_characters::r_crl_bracket) {
						--currentObjectDepth;
					} else if constexpr (to == json_structural_characters::l_sqr_bracket) {
						++currentArrayDepth;
					} else if constexpr (to == json_structural_characters::r_sqr_bracket) {
						--currentArrayDepth;
					}
					++iter;
					if constexpr (!parseOpts.minified) {
						skipWs();
					}
					return true;
				} else {
					reportError<failed_iterator_statuses[static_cast<uint8_t>(to)]>(location);
					return false;
				}
			} else {
				reportError<parse_statuses::unexpected_string_end>(location);
				return false;
			}
		}

		template<json_structural_characters to> constexpr bool advanceMaybe() {
			if (!atEnd()) {
				if (currentChar() == static_cast<char>(to)) {
					if constexpr (to == json_structural_characters::l_crl_bracket) {
						++currentObjectDepth;
					} else if constexpr (to == json_structural_characters::r_crl_bracket) {
						--currentObjectDepth;
					} else if constexpr (to == json_structural_characters::l_sqr_bracket) {
						++currentArrayDepth;
					} else if constexpr (to == json_structural_characters::r_sqr_bracket) {
						--currentArrayDepth;
					}
					++iter;
					if constexpr (!parseOpts.minified) {
						skipWs();
					}
					return true;
				} else {
					return false;
				}
			} else {
				return false;
			}
		}

		template<concepts::bool_t value_type> JSONIFIER_INLINE void parseBoolValue(value_type& value) noexcept {
			const char* result = parseBool(value, iter, endIter);
			if JSONIFIER_UNLIKELY (!result) {
				reportError<parse_statuses::invalid_bool_value>();
				return;
			}
			iter = result;
			if constexpr (!parseOpts.minified) {
				skipWs();
			}
			return;
		}

		JSONIFIER_INLINE void parseNullValue() noexcept {
			const char* result = parseNull(iter, endIter);
			if JSONIFIER_UNLIKELY (!result) {
				reportError<parse_statuses::invalid_null_value>();
				return;
			}
			iter = result;
			if constexpr (!parseOpts.minified) {
				skipWs();
			}
			return;
		}

		template<concepts::num_t value_type> JSONIFIER_INLINE void parseNumberValue(value_type& value) noexcept {
			const char* result = jsonifier::internal::parseNumber(value, iter, endIter);
			if JSONIFIER_UNLIKELY (!result) {
				reportError<parse_statuses::invalid_number_value>();
				return;
			}
			iter = result;
			if constexpr (!parseOpts.minified) {
				skipWs();
			}
		}

		template<concepts::string_t value_type> JSONIFIER_INLINE void parseStringValue(value_type& value) noexcept {
			const char* result = parseString<parseOpts>(value, *this);
			if JSONIFIER_UNLIKELY (!result) {
				reportError<parse_statuses::invalid_string_characters>();
				return;
			}
			iter = result;
			if constexpr (!parseOpts.minified) {
				skipWs();
			}
			return;
		}

		JSONIFIER_INLINE bool checkForCompletion() noexcept {
			if (currentArrayDepth > 0) {
				reportError<parse_statuses::missing_array_end>();
				return false;
			}
			if (currentObjectDepth > 0) {
				reportError<parse_statuses::missing_object_end>();
				return false;
			}
			if constexpr (!parseOpts.minified) {
				skipWs();
			}
			if JSONIFIER_UNLIKELY (iter < endIter) {
				reportError<parse_statuses::unfinished_input>();
				return false;
			}
			return parserPtr->getErrors().size() == 0;
		}

		JSONIFIER_INLINE void skipToNextValue() noexcept {
			if (atEnd())
				return;
			const char first = currentChar();
			if (first == '{' || first == '[') {
				int64_t depth{ 1 };
				++iter;
				while (!atEnd() && depth > 0) {
					const char c = currentChar();
					if (c == '"') {
						skipString();
						if (!atEnd())
							++iter;
						continue;
					}
					if (c == '{' || c == '[')
						++depth;
					else if (c == '}' || c == ']')
						--depth;
					++iter;
				}
			} else if (first == '"') {
				skipString();
				if (!atEnd())
					++iter;
			} else {
				while (!atEnd() && currentChar() != ',' && currentChar() != '}' && currentChar() != ']') {
					++iter;
				}
			}
			if constexpr (!parseOpts.minified) {
				skipWs();
			}
		}

		JSONIFIER_INLINE void skipString() noexcept {
			++iter;
			const auto newLength = static_cast<uint64_t>(endIter - iter);
			skipStringImpl(iter, newLength);
			if constexpr (!parseOpts.minified) {
				skipWs();
			}
		}

		JSONIFIER_INLINE void skipKey() noexcept {
			skipString();
			++iter;
			if constexpr (!parseOpts.minified) {
				skipWs();
			}
		}

		template<parse_statuses status> JSONIFIER_INLINE void reportError(const std::source_location& loc = std::source_location::current()) noexcept {
			parserPtr->template reportError<status>(*this, loc);
		}

		JSONIFIER_INLINE void reportError(parse_statuses status, const std::source_location& loc = std::source_location::current()) noexcept {
			parserPtr->reportError(status, *this, loc);
		}

		int64_t currentObjectDepth{};
		int64_t currentArrayDepth{};
		derived_type* parserPtr{};
		iterator_type rootIter{};
		iterator_type endIter{};
		iterator_type iter{};
	};

	template<typename derived_type, parse_options parseOpts> struct json_iterator<derived_type, structural_index_ptr, parseOpts> {
		using size_type		= uint64_t;
		using iterator_type = structural_index_ptr;

		JSONIFIER_INLINE json_iterator(derived_type* parserPtrNew, iterator_type rootIterNew, iterator_type endIterNew, string_view_ptr stringRootNew, string_view_ptr rawEndNew)
			: stringRoot{ stringRootNew }, rawEndIter{ rawEndNew }, parserPtr{ parserPtrNew }, rootIter{ rootIterNew }, endIter{ endIterNew }, iter{ rootIterNew } {
		}

		JSONIFIER_INLINE size_type getRemainingLength() const noexcept {
			return static_cast<size_type>(*endIter - *iter);
		}

		JSONIFIER_INLINE size_type getTotalLength() const noexcept {
			return static_cast<size_type>(*endIter - *rootIter);
		}

		JSONIFIER_INLINE char currentChar() const noexcept {
			if (!atEnd()) {
				return stringRoot[*iter];
			} else {
				return '\0';
			}
		}

		JSONIFIER_INLINE bool atEnd() const noexcept {
			return iter >= endIter;
		}

		JSONIFIER_INLINE bool anyInput() noexcept {
			if (rootIter != endIter) {
				return true;
			} else {
				reportError<parse_statuses::no_input>();
				return false;
			}
		}

		JSONIFIER_INLINE bool getState() const noexcept {
			return (currentArrayDepth > 0 || currentObjectDepth > 0);
		}

		JSONIFIER_INLINE void skipWs() noexcept {
			++iter;
		}

		JSONIFIER_INLINE operator bool() const noexcept {
			return getState();
		}

		template<json_structural_characters to> JSONIFIER_INLINE void incrementDepth() {
			if constexpr (to == json_structural_characters::l_crl_bracket) {
				++currentObjectDepth;
			} else if constexpr (to == json_structural_characters::r_crl_bracket) {
				--currentObjectDepth;
			} else if constexpr (to == json_structural_characters::l_sqr_bracket) {
				++currentArrayDepth;
			} else if constexpr (to == json_structural_characters::r_sqr_bracket) {
				--currentArrayDepth;
			}
		}

		template<json_structural_characters to> constexpr bool advanceTo(std::source_location location = std::source_location::current()) {
			if (!atEnd()) {
				if (currentChar() == static_cast<char>(to)) {
					if constexpr (to == json_structural_characters::l_crl_bracket) {
						++currentObjectDepth;
					} else if constexpr (to == json_structural_characters::r_crl_bracket) {
						--currentObjectDepth;
					} else if constexpr (to == json_structural_characters::l_sqr_bracket) {
						++currentArrayDepth;
					} else if constexpr (to == json_structural_characters::r_sqr_bracket) {
						--currentArrayDepth;
					}
					++iter;
					return true;
				} else {
					reportError<failed_iterator_statuses[static_cast<uint8_t>(to)]>(location);
					return false;
				}
			} else {
				reportError<parse_statuses::unexpected_string_end>(location);
				return false;
			}
		}

		template<json_structural_characters to> constexpr bool advanceMaybe() {
			if (!atEnd()) {
				if (currentChar() == static_cast<char>(to)) {
					if constexpr (to == json_structural_characters::l_crl_bracket) {
						++currentObjectDepth;
					} else if constexpr (to == json_structural_characters::r_crl_bracket) {
						--currentObjectDepth;
					} else if constexpr (to == json_structural_characters::l_sqr_bracket) {
						++currentArrayDepth;
					} else if constexpr (to == json_structural_characters::r_sqr_bracket) {
						--currentArrayDepth;
					}
					++iter;
					return true;
				} else {
					return false;
				}
			} else {
				return false;
			}
		}

		template<concepts::bool_t value_type> JSONIFIER_INLINE parse_statuses parseBoolValue(value_type& value) noexcept {
			const char* result = parseBool(value, &stringRoot[*iter], &stringRoot[*endIter]);
			if JSONIFIER_UNLIKELY (!result) {
				reportError<parse_statuses::invalid_bool_value>();
				skipToNextValue();
				return parse_statuses::invalid_bool_value;
			}
			if JSONIFIER_UNLIKELY (result != rawEndIter) {
				if JSONIFIER_UNLIKELY (*result == '\0') {
					reportError<parse_statuses::unfinished_input>();
					skipToNextValue();
					return parse_statuses::unfinished_input;
				}
				const auto status = iterator_interface<json_structural_characters::post_primitive_state, iterator_type>::statusValues[static_cast<uint8_t>(*result)];
				if JSONIFIER_UNLIKELY (status != parse_statuses::success) {
					reportError(status);
					skipToNextValue();
					return status;
				}
			}
			++iter;
			return parse_statuses::success;
		}

		JSONIFIER_INLINE parse_statuses parseNullValue() noexcept {
			const char* result = parseNull(&stringRoot[*iter], &stringRoot[*endIter]);
			if JSONIFIER_UNLIKELY (!result) {
				reportError<parse_statuses::invalid_null_value>();
				skipToNextValue();
				return parse_statuses::invalid_null_value;
			}
			if JSONIFIER_UNLIKELY (result != rawEndIter) {
				const auto status = iterator_interface<json_structural_characters::post_primitive_state, iterator_type>::statusValues[static_cast<uint8_t>(*result)];
				if JSONIFIER_UNLIKELY (status != parse_statuses::success) {
					reportError(status);
					skipToNextValue();
					return status;
				}
			}
			++iter;
			return parse_statuses::success;
		}

		template<concepts::num_t value_type> JSONIFIER_INLINE parse_statuses parseNumberValue(value_type& value) noexcept {
			const char* numStart = &stringRoot[*iter];
			const char* numEnd	 = &stringRoot[*(iter + 1)];
			const char* result	 = jsonifier::internal::parseNumber(value, numStart, numEnd);
			if JSONIFIER_UNLIKELY (!result) {
				reportError<parse_statuses::invalid_number_value>();
				skipToNextValue();
				return parse_statuses::invalid_number_value;
			}
			if JSONIFIER_UNLIKELY (*result == '\0' && result != rawEndIter) {
				reportError<parse_statuses::unfinished_input>();
				skipToNextValue();
				return parse_statuses::unfinished_input;
			}
			const auto status = iterator_interface<json_structural_characters::post_primitive_state, iterator_type>::statusValues[static_cast<uint8_t>(*result)];
			if JSONIFIER_UNLIKELY (status != parse_statuses::success) {
				reportError(status);
				skipToNextValue();
				return status;
			}
			++iter;
			return parse_statuses::success;
		}

		template<concepts::string_t value_type> JSONIFIER_INLINE parse_statuses parseStringValue(value_type& value) noexcept {
			if JSONIFIER_UNLIKELY (atEnd() || currentChar() != '"') {
				reportError<parse_statuses::invalid_string_characters>();
				skipToNextValue();
				return parse_statuses::invalid_string_characters;
			}
			const char* result = parseStringPartial<parseOpts>(value, *this);
			if JSONIFIER_UNLIKELY (!result) {
				reportError<parse_statuses::invalid_string_characters>();
				skipToNextValue();
				return parse_statuses::invalid_string_characters;
			}
			return parse_statuses::success;
		}

		JSONIFIER_INLINE bool checkForCompletion() noexcept {
			if (currentArrayDepth > 0) {
				reportError<parse_statuses::missing_array_end>();
				return false;
			}
			if (currentObjectDepth > 0) {
				reportError<parse_statuses::missing_object_end>();
				return false;
			}
			if JSONIFIER_UNLIKELY (iter < endIter) {
				reportError<parse_statuses::unfinished_input>();
				return false;
			}
			if JSONIFIER_UNLIKELY (endIter > rootIter) {
				const char* lastTapePos = &stringRoot[*(endIter - 1)];
				if JSONIFIER_UNLIKELY (lastTapePos < rawEndIter) {
					return parserPtr->getErrors().size() == 0;
				}
			}
			return parserPtr->getErrors().size() == 0;
		}

		JSONIFIER_INLINE void skipToNextValue() noexcept {
			if (atEnd()) {
				return;
			}
			const char first = currentChar();
			if (first == '{' || first == '[') {
				int64_t depth{ 1 };
				++iter;
				while (!atEnd() && depth > 0) {
					const char c = currentChar();
					if (c == '{' || c == '[') {
						++depth;
					} else if (c == '}' || c == ']') {
						--depth;
					}
					++iter;
				}
			} else {
				++iter;
			}
		}

		JSONIFIER_INLINE void skipString() noexcept {
			++iter;
		}

		JSONIFIER_INLINE void skipKey() noexcept {
			++iter;
		}

		template<parse_statuses status> JSONIFIER_INLINE void reportError(const std::source_location& loc = std::source_location::current()) noexcept {
			parserPtr->template reportError<status>(*this, loc);
		}

		JSONIFIER_INLINE void reportError(parse_statuses status, const std::source_location& loc = std::source_location::current()) noexcept {
			parserPtr->reportError(status, *this, loc);
		}

		string_view_ptr stringRoot{};
		string_view_ptr rawEndIter{};
		int64_t currentObjectDepth{};
		int64_t currentArrayDepth{};
		derived_type* parserPtr{};
		iterator_type rootIter{};
		iterator_type endIter{};
		iterator_type iter{};
	};

	template<typename value_type>
	concept partial_reading_context_t = requires(jsonifier::internal::remove_cvref_t<value_type> value) {
		{ value.getState() } -> std::same_as<bool>;
	};

	JSONIFIER_INLINE constexpr uint64_t str_len(const char* input) noexcept {
		uint64_t return_val{};
		while (input[return_val] != '\0') {
			++return_val;
		}
		return return_val;
	}

	template<concepts::pointer_t value_type> JSONIFIER_INLINE static string_view_ptr getEndIter(value_type value) noexcept {
		return value + str_len(value);
	}

	template<concepts::pointer_t value_type> JSONIFIER_INLINE static string_view_ptr getBeginIter(value_type value) noexcept {
		return std::bit_cast<string_view_ptr>(value);
	}

	template<concepts::has_data value_type> JSONIFIER_INLINE static string_view_ptr getEndIter(value_type& value) noexcept {
		return std::bit_cast<string_view_ptr>(value.data() + value.size());
	}

	template<concepts::has_data value_type> JSONIFIER_INLINE static string_view_ptr getBeginIter(value_type& value) noexcept {
		return std::bit_cast<string_view_ptr>(value.data());
	}


	template<typename value_type, typename context_type, parse_options optionsNew> struct parse_partial_impl;
	template<typename value_type, typename context_type, parse_options optionsNew> struct parse_impl;

	template<parse_options options> struct parse {
		template<typename value_type_new, typename context_type> JSONIFIER_INLINE static void impl(value_type_new&& value, context_type&& context) noexcept {
			using value_type = remove_cvref_t<value_type_new>;
			if constexpr (options.partialRead) {
				parse_partial_impl<value_type, context_type, options>::impl(value, context);
			} else {
				parse_impl<value_type, context_type, options>::impl(value, context);
			}
		}
	};

	template<typename derived_type_new> class parser {
	  public:
		friend class jsonifier::raw_json_data;
		template<const auto options, typename context_type> friend struct derailleur;

		using derived_type = derived_type_new;

		parser& operator=(const parser& other) = delete;
		parser(const parser& other)			   = delete;

		template<parse_options options = parse_options{}, typename comparison_type, typename buffer_type>
		inline bool parseJsonForComparison(comparison_type&& object, buffer_type&& in) noexcept {
			auto rootIter = getBeginIter(in);
			auto endIter  = getEndIter(in);
			derivedRef.section.template reset<options.minified>(rootIter, static_cast<uint64_t>(endIter - rootIter));
			json_iterator<parser, structural_index_ptr, options> context{ this, derivedRef.section.begin(), derivedRef.section.end(), rootIter, endIter };
			object.indices.resize(derivedRef.section.getTapecount());
			std::copy_n(derivedRef.section.begin(), object.indices.size(), object.indices.data());
			context.rootIter   = derivedRef.section.begin();
			context.iter	   = derivedRef.section.begin();
			context.endIter	   = derivedRef.section.end();
			context.stringRoot = rootIter;
			context.parserPtr  = this;
			auto newSize	   = static_cast<uint64_t>((context.endIter - context.iter) / 2);
			if (derivedRef.stringBuffer.size() < newSize) {
				derivedRef.stringBuffer.resize(newSize);
			}
			if constexpr (options.validateJson) {
				if (!derivedRef.validateJson(in)) {
					return false;
				}
			}
			derivedRef.errors.clear();
			if JSONIFIER_UNLIKELY (!context.iter || context.iter == context.endIter) {
				reportError<parse_statuses::no_input>(context);
				return false;
			}
			return derivedRef.errors.size() > 0 ? false : true;
		}

		template<parse_options options = parse_options{}, typename value_type, typename buffer_type> inline bool parseJson(value_type&& object, buffer_type&& in) noexcept {
			if constexpr (options.partialRead) {
				static constexpr parse_options optionsNew{ options };
				auto rootIter = getBeginIter(in);
				auto endIter  = getEndIter(in);
				derivedRef.section.template reset<options.minified>(rootIter, static_cast<uint64_t>(endIter - rootIter));
				json_iterator<parser, structural_index_ptr, options> context{ this, derivedRef.section.begin(), derivedRef.section.end(), rootIter, endIter };
				auto newSize = context.getTotalLength() / 2;
				if (derivedRef.stringBuffer.size() < newSize) {
					derivedRef.stringBuffer.resize(newSize);
				}
				if constexpr (options.validateJson) {
					if (!derivedRef.validateJson(in)) {
						return false;
					}
				}
				derivedRef.errors.clear();
				if JSONIFIER_UNLIKELY (!context.anyInput()) {
					return false;
				}
				parse<optionsNew>::impl(object, context);
				return context.checkForCompletion();
			} else {
				static constexpr parse_options optionsNew{ options };
				json_iterator<parser, string_view_ptr, options> context{ this, getBeginIter(in), getEndIter(in) };
				auto newSize = context.getTotalLength() / 2;
				if (derivedRef.stringBuffer.size() < newSize) {
					derivedRef.stringBuffer.resize(newSize);
				}
				if constexpr (options.validateJson) {
					if (!derivedRef.validateJson(in)) {
						return false;
					}
				}
				derivedRef.errors.clear();
				if JSONIFIER_UNLIKELY (!context.anyInput()) {
					return false;
				}
				parse<optionsNew>::impl(object, context);
				return context.checkForCompletion();
			}
		}

		template<typename value_type, parse_options options = parse_options{}, typename buffer_type> inline bool parseManyJson(value_type&& object, buffer_type&& in) noexcept {
			static constexpr parse_options optionsNew{ options };
			auto rootIter = getBeginIter(in);
			auto endIter  = getEndIter(in);
			derivedRef.section.template reset<options.minified>(rootIter, static_cast<uint64_t>(endIter - rootIter));
			json_iterator<parser, structural_index_ptr, options> context{ this, derivedRef.section.begin(), derivedRef.section.end(), rootIter, endIter };
			context.rootIter   = derivedRef.section.begin();
			context.iter	   = derivedRef.section.begin();
			context.endIter	   = derivedRef.section.end();
			context.stringRoot = rootIter;
			context.parserPtr  = this;
			auto newSize	   = static_cast<uint64_t>((endIter - rootIter) / 2);
			if (derivedRef.stringBuffer.size() < newSize) {
				derivedRef.stringBuffer.resize(newSize);
			}
			if constexpr (options.validateJson) {
				if (!derivedRef.validateJson(in)) {
					return false;
				}
			}
			derivedRef.errors.clear();
			if JSONIFIER_UNLIKELY (context.iter >= context.endIter) {
				reportError<parse_statuses::no_input>(context);
				return false;
			}
			while (context.iter < context.endIter) {
				auto before = context.iter;
				parse<optionsNew>::impl(object.emplace_back(), context);
				if JSONIFIER_UNLIKELY (context.iter == before) {
					object.pop_back();
					reportError<parse_statuses::unfinished_input>(context);
					break;
				}
			}
			return (context.currentObjectDepth != 0)						  ? (reportError<parse_statuses::missing_object_end>(context), false)
				: (context.currentArrayDepth != 0)							  ? (reportError<parse_statuses::missing_array_end>(context), false)
				: (context.iter < context.endIter && !optionsNew.partialRead) ? (reportError<parse_statuses::unfinished_input>(context), false)
				: derivedRef.errors.size() > 0								  ? false
																			  : true;
		}

		template<typename value_type, parse_options options = parse_options{}, concepts::string_t buffer_type> inline value_type parseJson(buffer_type&& in) noexcept {
			if constexpr (options.partialRead) {
				static constexpr parse_options optionsNew{ options };
				auto rootIter = getBeginIter(in);
				auto endIter  = getEndIter(in);
				json_iterator<parser, string_view_ptr, options> context{ this, getBeginIter(in), getEndIter(in), rootIter };
				derivedRef.section.template reset<options.minified>(rootIter, static_cast<uint64_t>(endIter - rootIter));
				context.rootIter   = derivedRef.section.begin();
				context.iter	   = derivedRef.section.begin();
				context.endIter	   = derivedRef.section.end();
				context.stringRoot = rootIter;
				context.parserPtr  = this;
				auto newSize	   = static_cast<uint64_t>((context.endIter - context.iter) / 2);
				if (derivedRef.stringBuffer.size() < newSize) {
					derivedRef.stringBuffer.resize(newSize);
				}
				if constexpr (options.validateJson) {
					if (!derivedRef.validateJson(in)) {
						return jsonifier::internal::remove_cvref_t<value_type>{};
					}
				}
				derivedRef.errors.clear();
				if JSONIFIER_UNLIKELY (!context.iter || context.iter == context.endIter) {
					reportError<parse_statuses::no_input>(context);
					return jsonifier::internal::remove_cvref_t<value_type>{};
				}
				value_type object{};
				parse<optionsNew>::impl(object, context);
				return derivedRef.errors.size() > 0 ? jsonifier::internal::remove_cvref_t<value_type>{} : object;
			} else {
				static constexpr parse_options optionsNew{ options };
				json_iterator<parser, string_view_ptr, options> context{ this, getBeginIter(in), getEndIter(in) };
				auto newSize = context.getTotalLength() / 2;
				if (derivedRef.stringBuffer.size() < newSize) {
					derivedRef.stringBuffer.resize(newSize);
				}
				if constexpr (options.validateJson) {
					if (!derivedRef.validateJson(in)) {
						return false;
					}
				}
				derivedRef.errors.clear();
				if JSONIFIER_UNLIKELY (!context.anyInput()) {
					return false;
				}
				value_type object;
				parse<optionsNew>::impl(object, context);
				return object;
			}
		}

		template<auto parseError, typename iterator_type, parse_options parseOpts>
		void reportError(json_iterator<parser, iterator_type, parseOpts>& context, const std::source_location& sourceLocation = std::source_location::current()) noexcept {
			if constexpr (std::is_same_v<iterator_type, structural_index_ptr>) {
				derivedRef.errors.emplace_back(
					error::constructError<status_classes::Parsing, parseError>(*context.iter, *context.endIter - *context.rootIter, context.stringRoot, sourceLocation));
			} else {
				derivedRef.errors.emplace_back(error::constructError<status_classes::Parsing, parseError>(context.iter - context.rootIter, context.endIter - context.rootIter,
					context.rootIter, sourceLocation));
			}
		}

		template<typename iterator_type, parse_options parseOpts> void reportError(auto parseError, json_iterator<parser, iterator_type, parseOpts>& context,
			const std::source_location& sourceLocation = std::source_location::current()) noexcept {
			if constexpr (std::is_same_v<iterator_type, structural_index_ptr>) {
				derivedRef.errors.emplace_back(
					error::constructError<status_classes::Parsing>(parseError, *context.iter, *context.endIter - *context.rootIter, context.stringRoot, sourceLocation));
			} else {
				derivedRef.errors.emplace_back(error::constructError<status_classes::Parsing>(parseError, context.iter - context.rootIter, context.endIter - context.rootIter,
					context.rootIter, sourceLocation));
			}
		}

		const std::vector<internal::error>& getErrors() const noexcept {
			return derivedRef.getErrors();
		}

		JSONIFIER_INLINE auto& getStringBuffer() noexcept {
			return derivedRef.stringBuffer;
		}

	  protected:
		derived_type& derivedRef{ initializeSelfRef() };

		parser() noexcept : derivedRef{ initializeSelfRef() } {
		}

		derived_type& initializeSelfRef() noexcept {
			return *static_cast<derived_type*>(this);
		}

		~parser() noexcept = default;
	};
}
