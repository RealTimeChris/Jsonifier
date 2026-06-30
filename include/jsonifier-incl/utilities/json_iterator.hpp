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

#include <jsonifier-incl/utilities/string_view.hpp>
#include <jsonifier-incl/utilities/simd.hpp>
#include <jsonifier-incl/utilities/string_utils.hpp>
#include <source_location>
#include <unordered_map>
#include <exception>
#include <algorithm>
#include <string>

namespace jsonifier::internal {

	template<jsonifier::concepts::bool_t bool_type> JSONIFIER_INLINE static string_view_ptr parseBool(bool_type& value, string_view_ptr iter, string_view_ptr endIter) noexcept;
	JSONIFIER_INLINE static string_view_ptr parseNull(string_view_ptr iter JSONIFIER_LIFETIME_BOUND, string_view_ptr endIter) noexcept;

#undef IN

	static constexpr std::array<const char*, static_cast<size_t>(parse_statuses::count)> parseStatusNames{ { "success", "missing_object_start", "missing_object_end",
		"missing_array_start", "missing_array_end", "invalid_string_characters", "missing_colon", "missing_comma", "invalid_number_value", "invalid_null_value",
		"invalid_bool_value", "no_input", "unfinished_input", "unexpected_string_end", "unexpected_end_of_input", "unexpected_token", "illegal_control_character" } };

	template<typename error_type> constexpr const char* errorName(error_type code_new) noexcept {
		uint64_t code{ static_cast<uint64_t>(code_new) };
		return code < parseStatusNames.size() ? parseStatusNames[code] : "Unknown_Parse_Status";
	}

	inline static std::ostream& operator<<(std::ostream& os, parse_statuses error) {
		os << errorName(error);
		return os;
	}

	struct error_data {
		parse_statuses parseStatus;
		const char* errorRoot;
		uint64_t errorLength;
		uint64_t errorIndex;
	};

	inline static std::ostream& operator<<(std::ostream& os, const error_data& error) {
		os << "Error: " << std::endl;
		os << "Error Index: " << error.errorIndex << std::endl;
		os << "Type: " << error.parseStatus << std::endl;
		os << "Values: " << std::endl;
		os << "-------" << std::endl;
		if (error.errorLength > 0 && error.errorRoot) {
			for (uint64_t x = 0; x < error.errorLength - 1; ++x) {
				os << std::hex << "0x" << +static_cast<uint8_t>(error.errorRoot[x]);
				if (x < error.errorLength - 1) {
					os << ", ";
				}
			}
		}
		os << std::dec << std::endl;
		return os;
	}

	enum class jsonifier_classes : int8_t {
		C_ERROR = -1,
		C_SPACE, /* space */
		C_WHITE, /* other whitespace */
		C_LCURB, /* {  */
		C_RCURB, /* } */
		C_LSQRB, /* [ */
		C_RSQRB, /* ] */
		C_COLON, /* : */
		C_COMMA, /* , */
		C_QUOTE, /* " */
		C_BACKS, /* \ */
		C_SLASH, /* / */
		C_PLUS, /* + */
		C_MINUS, /* - */
		C_POINT, /* . */
		C_ZERO, /* 0 */
		C_DIGIT, /* 123456789 */
		C_LOW_A, /* a */
		C_LOW_B, /* b */
		C_LOW_C, /* c */
		C_LOW_D, /* d */
		C_LOW_E, /* e */
		C_LOW_F, /* f */
		C_LOW_L, /* l */
		C_LOW_N, /* n */
		C_LOW_R, /* r */
		C_LOW_S, /* s */
		C_LOW_T, /* t */
		C_LOW_U, /* u */
		C_ABCDF, /* ABCDF */
		C_E, /* E */
		C_ETC, /* everything else */
		NR_CLASSES = 31,
	};

	enum class jsonifier_states : int8_t {
		n9 = -9,
		n8 = -8,
		n7 = -7,
		n6 = -6,
		n5 = -5,
		n4 = -4,
		n3 = -3,
		n2 = -2,
		ER = -1,
		GO, /* start    */
		OK, /* ok       */
		OB, /* object   */
		KE, /* key      */
		CO, /* colon    */
		VA, /* value    */
		AR, /* array    */
		ST, /* string   */
		ES, /* escape   */
		U1, /* u1       */
		U2, /* u2       */
		U3, /* u3       */
		U4, /* u4       */
		MI, /* minus    */
		ZE, /* zero     */
		IN, /* integer  */
		FR, /* fraction */
		FS, /* fraction */
		E1, /* e        */
		E2, /* ex       */
		E3, /* exp      */
		T1, /* tr       */
		T2, /* tru      */
		T3, /* true     */
		F1, /* fa       */
		F2, /* fal      */
		F3, /* fals     */
		F4, /* false    */
		N1, /* nu       */
		N2, /* nul      */
		N3, /* null     */
		NR_STATES = 31,
	};

	static constexpr std::array<parse_statuses, static_cast<uint64_t>(jsonifier_states::NR_STATES)> jsonifierParseStatuses{ [] {
		std::array<parse_statuses, static_cast<uint64_t>(jsonifier_states::NR_STATES)> return_values{};
		return_values.fill(parse_statuses::unexpected_end_of_input);
		return_values[static_cast<uint64_t>(jsonifier_states::MI)] = parse_statuses::unfinished_input;
		return_values[static_cast<uint64_t>(jsonifier_states::IN)] = parse_statuses::unfinished_input;
		return_values[static_cast<uint64_t>(jsonifier_states::ZE)] = parse_statuses::unfinished_input;
		return_values[static_cast<uint64_t>(jsonifier_states::FR)] = parse_statuses::unfinished_input;
		return_values[static_cast<uint64_t>(jsonifier_states::FS)] = parse_statuses::unfinished_input;
		return_values[static_cast<uint64_t>(jsonifier_states::E1)] = parse_statuses::unfinished_input;
		return_values[static_cast<uint64_t>(jsonifier_states::E2)] = parse_statuses::unfinished_input;
		return_values[static_cast<uint64_t>(jsonifier_states::E3)] = parse_statuses::unfinished_input;
		return_values[static_cast<uint64_t>(jsonifier_states::ST)] = parse_statuses::unexpected_string_end;
		return_values[static_cast<uint64_t>(jsonifier_states::ES)] = parse_statuses::unexpected_string_end;
		return_values[static_cast<uint64_t>(jsonifier_states::U1)] = parse_statuses::unexpected_string_end;
		return_values[static_cast<uint64_t>(jsonifier_states::U2)] = parse_statuses::unexpected_string_end;
		return_values[static_cast<uint64_t>(jsonifier_states::U3)] = parse_statuses::unexpected_string_end;
		return_values[static_cast<uint64_t>(jsonifier_states::U4)] = parse_statuses::unexpected_string_end;
		return_values[static_cast<uint64_t>(jsonifier_states::T1)] = parse_statuses::unfinished_input;
		return_values[static_cast<uint64_t>(jsonifier_states::T2)] = parse_statuses::unfinished_input;
		return_values[static_cast<uint64_t>(jsonifier_states::T3)] = parse_statuses::unfinished_input;
		return_values[static_cast<uint64_t>(jsonifier_states::F1)] = parse_statuses::unfinished_input;
		return_values[static_cast<uint64_t>(jsonifier_states::F2)] = parse_statuses::unfinished_input;
		return_values[static_cast<uint64_t>(jsonifier_states::F3)] = parse_statuses::unfinished_input;
		return_values[static_cast<uint64_t>(jsonifier_states::F4)] = parse_statuses::unfinished_input;
		return_values[static_cast<uint64_t>(jsonifier_states::N1)] = parse_statuses::unfinished_input;
		return_values[static_cast<uint64_t>(jsonifier_states::N2)] = parse_statuses::unfinished_input;
		return_values[static_cast<uint64_t>(jsonifier_states::N3)] = parse_statuses::unfinished_input;
		return return_values;
	}() };

	template<typename v_type>
	concept jsonifier_classes_types = std::same_as<jsonifier_classes, std::remove_cvref_t<v_type>>;

	template<typename v_type>
	concept jsonifier_states_types = std::same_as<jsonifier_states, std::remove_cvref_t<v_type>>;

	template<typename v_type>
	concept integral_or_operable_enum_types = jsonifier_classes_types<v_type> || jsonifier_states_types<v_type> || concepts::integral_types<v_type>;

	template<integral_or_operable_enum_types v_type_01, integral_or_operable_enum_types v_type_02> constexpr bool operator<=(v_type_01 val_01, v_type_02 val_02) {
		return static_cast<int64_t>(val_01) <= static_cast<int64_t>(val_02);
	}

	template<integral_or_operable_enum_types v_type_01, integral_or_operable_enum_types v_type_02> constexpr bool operator>=(v_type_01 val_01, v_type_02 val_02) {
		return static_cast<int64_t>(val_01) >= static_cast<int64_t>(val_02);
	}

	template<integral_or_operable_enum_types v_type_01, integral_or_operable_enum_types v_type_02> constexpr bool operator<(v_type_01 val_01, v_type_02 val_02) {
		return static_cast<int64_t>(val_01) < static_cast<int64_t>(val_02);
	}

	template<integral_or_operable_enum_types v_type_01, integral_or_operable_enum_types v_type_02> constexpr bool operator>(v_type_01 val_01, v_type_02 val_02) {
		return static_cast<int64_t>(val_01) > static_cast<int64_t>(val_02);
	}

	// clang-format off

	static constexpr jsonifier_classes jsonifierAsciiClasses[128] = {
		jsonifier_classes::C_ERROR, jsonifier_classes::C_ERROR, jsonifier_classes::C_ERROR, jsonifier_classes::C_ERROR, jsonifier_classes::C_ERROR, jsonifier_classes::C_ERROR, jsonifier_classes::C_ERROR, jsonifier_classes::C_ERROR, jsonifier_classes::C_ERROR,
		jsonifier_classes::C_WHITE, jsonifier_classes::C_WHITE, jsonifier_classes::C_ERROR, jsonifier_classes::C_ERROR, jsonifier_classes::C_WHITE, jsonifier_classes::C_ERROR, jsonifier_classes::C_ERROR, jsonifier_classes::C_ERROR, jsonifier_classes::C_ERROR,
		jsonifier_classes::C_ERROR, jsonifier_classes::C_ERROR, jsonifier_classes::C_ERROR, jsonifier_classes::C_ERROR, jsonifier_classes::C_ERROR, jsonifier_classes::C_ERROR, jsonifier_classes::C_ERROR, jsonifier_classes::C_ERROR, jsonifier_classes::C_ERROR,
		jsonifier_classes::C_ERROR, jsonifier_classes::C_ERROR, jsonifier_classes::C_ERROR, jsonifier_classes::C_ERROR, jsonifier_classes::C_ERROR,

		jsonifier_classes::C_SPACE, jsonifier_classes::C_ETC, jsonifier_classes::C_QUOTE, jsonifier_classes::C_ETC, jsonifier_classes::C_ETC, jsonifier_classes::C_ETC, jsonifier_classes::C_ETC, jsonifier_classes::C_ETC, jsonifier_classes::C_ETC, jsonifier_classes::C_ETC,
		jsonifier_classes::C_ETC, jsonifier_classes::C_PLUS, jsonifier_classes::C_COMMA, jsonifier_classes::C_MINUS, jsonifier_classes::C_POINT, jsonifier_classes::C_SLASH, jsonifier_classes::C_ZERO, jsonifier_classes::C_DIGIT, jsonifier_classes::C_DIGIT, jsonifier_classes::C_DIGIT,
		jsonifier_classes::C_DIGIT, jsonifier_classes::C_DIGIT, jsonifier_classes::C_DIGIT, jsonifier_classes::C_DIGIT, jsonifier_classes::C_DIGIT, jsonifier_classes::C_DIGIT, jsonifier_classes::C_COLON, jsonifier_classes::C_ETC, jsonifier_classes::C_ETC, jsonifier_classes::C_ETC,
		jsonifier_classes::C_ETC, jsonifier_classes::C_ETC,

		jsonifier_classes::C_ETC, jsonifier_classes::C_ABCDF, jsonifier_classes::C_ABCDF, jsonifier_classes::C_ABCDF, jsonifier_classes::C_ABCDF, jsonifier_classes::C_E, jsonifier_classes::C_ABCDF, jsonifier_classes::C_ETC, jsonifier_classes::C_ETC, jsonifier_classes::C_ETC,
		jsonifier_classes::C_ETC, jsonifier_classes::C_ETC, jsonifier_classes::C_ETC, jsonifier_classes::C_ETC, jsonifier_classes::C_ETC, jsonifier_classes::C_ETC, jsonifier_classes::C_ETC, jsonifier_classes::C_ETC, jsonifier_classes::C_ETC, jsonifier_classes::C_ETC, jsonifier_classes::C_ETC,
		jsonifier_classes::C_ETC, jsonifier_classes::C_ETC, jsonifier_classes::C_ETC, jsonifier_classes::C_ETC, jsonifier_classes::C_ETC, jsonifier_classes::C_ETC, jsonifier_classes::C_LSQRB, jsonifier_classes::C_BACKS, jsonifier_classes::C_RSQRB, jsonifier_classes::C_ETC,
		jsonifier_classes::C_ETC,

		jsonifier_classes::C_ETC, jsonifier_classes::C_LOW_A, jsonifier_classes::C_LOW_B, jsonifier_classes::C_LOW_C, jsonifier_classes::C_LOW_D, jsonifier_classes::C_LOW_E, jsonifier_classes::C_LOW_F, jsonifier_classes::C_ETC, jsonifier_classes::C_ETC, jsonifier_classes::C_ETC,
		jsonifier_classes::C_ETC, jsonifier_classes::C_ETC, jsonifier_classes::C_LOW_L, jsonifier_classes::C_ETC, jsonifier_classes::C_LOW_N, jsonifier_classes::C_ETC, jsonifier_classes::C_ETC, jsonifier_classes::C_ETC, jsonifier_classes::C_LOW_R, jsonifier_classes::C_LOW_S,
		jsonifier_classes::C_LOW_T, jsonifier_classes::C_LOW_U, jsonifier_classes::C_ETC, jsonifier_classes::C_ETC, jsonifier_classes::C_ETC, jsonifier_classes::C_ETC, jsonifier_classes::C_ETC, jsonifier_classes::C_LCURB, jsonifier_classes::C_ETC, jsonifier_classes::C_RCURB,
		jsonifier_classes::C_ETC, jsonifier_classes::C_ETC
	};

	static constexpr jsonifier_states jsonifierStateTransitionTable[static_cast<uint64_t>(jsonifier_states::NR_STATES)][static_cast<uint64_t>(jsonifier_classes::NR_CLASSES)] = {
		/*                                         white                                                                                                                                                                                                                                                                                            1-9                                                                                                                                                                                                                                                                                                                ABCDF					                   etc
						   space                     |                     {                      }                      [                     ]                     :                   ,                      "                     \                       /                      +                     -                      .                   0                      |                     a                    b                    c                   d                      e                     f                       l                       n                     r                  s                      t                     u                    |                     E                     |    */
		/*start  GO*/ { jsonifier_states::GO, jsonifier_states::GO, jsonifier_states::n6, jsonifier_states::ER, jsonifier_states::n5, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ST, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::MI, jsonifier_states::ER, jsonifier_states::ZE, jsonifier_states::IN, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::F1, jsonifier_states::ER, jsonifier_states::N1, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::T1, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER},
		/*ok     OK*/ { jsonifier_states::OK, jsonifier_states::OK, jsonifier_states::ER, jsonifier_states::n8, jsonifier_states::ER, jsonifier_states::n7, jsonifier_states::ER, jsonifier_states::n3, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER},
		/*object OB*/ { jsonifier_states::OB, jsonifier_states::OB, jsonifier_states::ER, jsonifier_states::n9, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ST, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER},
		/*key    KE*/ { jsonifier_states::KE, jsonifier_states::KE, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ST, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER},
		/*colon  CO*/ { jsonifier_states::CO, jsonifier_states::CO, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::n2, jsonifier_states::ER, jsonifier_states::IN, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER},
		/*value  VA*/ { jsonifier_states::VA, jsonifier_states::VA, jsonifier_states::n6, jsonifier_states::ER, jsonifier_states::n5, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ST, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::MI, jsonifier_states::ER, jsonifier_states::ZE, jsonifier_states::IN, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::F1, jsonifier_states::ER, jsonifier_states::N1, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::T1, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER},
		/*array  AR*/ { jsonifier_states::AR, jsonifier_states::AR, jsonifier_states::n6, jsonifier_states::ER, jsonifier_states::n5, jsonifier_states::n7, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ST, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::MI, jsonifier_states::ER, jsonifier_states::ZE, jsonifier_states::IN, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::F1, jsonifier_states::ER, jsonifier_states::N1, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::T1, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER},
		/*string ST*/ { jsonifier_states::ST, jsonifier_states::ER, jsonifier_states::ST, jsonifier_states::ST, jsonifier_states::ST, jsonifier_states::ST, jsonifier_states::ST, jsonifier_states::ST, jsonifier_states::n4, jsonifier_states::ES, jsonifier_states::ST, jsonifier_states::ST, jsonifier_states::ST, jsonifier_states::ST, jsonifier_states::ST, jsonifier_states::ST, jsonifier_states::ST, jsonifier_states::ST, jsonifier_states::ST, jsonifier_states::ST, jsonifier_states::ST, jsonifier_states::ST, jsonifier_states::ST, jsonifier_states::ST, jsonifier_states::ST, jsonifier_states::ST, jsonifier_states::ST, jsonifier_states::ST, jsonifier_states::ST, jsonifier_states::ST, jsonifier_states::ST},
		/*escape ES*/ { jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ST, jsonifier_states::ST, jsonifier_states::ST, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ST, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ST, jsonifier_states::ER, jsonifier_states::ST, jsonifier_states::ST, jsonifier_states::ER, jsonifier_states::ST, jsonifier_states::U1, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER},
		/*u1     U1*/ { jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::U2, jsonifier_states::U2, jsonifier_states::U2, jsonifier_states::U2, jsonifier_states::U2, jsonifier_states::U2, jsonifier_states::U2, jsonifier_states::U2, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::U2, jsonifier_states::U2, jsonifier_states::ER},
		/*u2     U2*/ { jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::U3, jsonifier_states::U3, jsonifier_states::U3, jsonifier_states::U3, jsonifier_states::U3, jsonifier_states::U3, jsonifier_states::U3, jsonifier_states::U3, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::U3, jsonifier_states::U3, jsonifier_states::ER},
		/*u3     U3*/ { jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::U4, jsonifier_states::U4, jsonifier_states::U4, jsonifier_states::U4, jsonifier_states::U4, jsonifier_states::U4, jsonifier_states::U4, jsonifier_states::U4, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::U4, jsonifier_states::U4, jsonifier_states::ER},
		/*u4     U4*/ { jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ST, jsonifier_states::ST, jsonifier_states::ST, jsonifier_states::ST, jsonifier_states::ST, jsonifier_states::ST, jsonifier_states::ST, jsonifier_states::ST, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ST, jsonifier_states::ST, jsonifier_states::ER},
		/*minus  MI*/ { jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ZE, jsonifier_states::IN, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER},
		/*zero   ZE*/ { jsonifier_states::OK, jsonifier_states::OK, jsonifier_states::ER, jsonifier_states::n8, jsonifier_states::ER, jsonifier_states::n7, jsonifier_states::ER, jsonifier_states::n3, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::FR, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::E1, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::E1, jsonifier_states::ER},
		/*ine    IN*/ { jsonifier_states::OK, jsonifier_states::OK, jsonifier_states::ER, jsonifier_states::n8, jsonifier_states::ER, jsonifier_states::n7, jsonifier_states::ER, jsonifier_states::n3, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::FR, jsonifier_states::IN, jsonifier_states::IN, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::E1, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::E1, jsonifier_states::ER},
		/*frac   FR*/ { jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::FS, jsonifier_states::FS, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER},
		/*fracs  FS*/ { jsonifier_states::OK, jsonifier_states::OK, jsonifier_states::ER, jsonifier_states::n8, jsonifier_states::ER, jsonifier_states::n7, jsonifier_states::ER, jsonifier_states::n3, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::FS, jsonifier_states::FS, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::E1, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::E1, jsonifier_states::ER},
		/*e      E1*/ { jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::E2, jsonifier_states::E2, jsonifier_states::ER, jsonifier_states::E3, jsonifier_states::E3, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER},
		/*ex     E2*/ { jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::E3, jsonifier_states::E3, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER},
		/*exp    E3*/ { jsonifier_states::OK, jsonifier_states::OK, jsonifier_states::ER, jsonifier_states::n8, jsonifier_states::ER, jsonifier_states::n7, jsonifier_states::ER, jsonifier_states::n3, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::E3, jsonifier_states::E3, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER},
		/*tr     T1*/ { jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::T2, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER},
		/*tru    T2*/ { jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::T3, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER},
		/*true   T3*/ { jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::OK, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER},
		/*fa     F1*/ { jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::F2, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER},
		/*fal    F2*/ { jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::F3, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER},
		/*fals   F3*/ { jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::F4, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER},
		/*false  F4*/ { jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::OK, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER},
		/*nu     N1*/ { jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::N2, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER},
		/*nul    N2*/ { jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::N3, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER},
		/*null   N3*/ { jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::OK, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER, jsonifier_states::ER}
	};

	// clang-format on

	enum class jsonifier_modes : int8_t {
		MODE_ARRAY	= 1 << 0,
		MODE_DONE	= 1 << 1,
		MODE_KEY	= 1 << 2,
		MODE_OBJECT = 1 << 3,
	};

	static constexpr std::array<bool, 256> jsonifierWhitespaceTable{ [] {
		std::array<bool, 256> return_values{};
		return_values[' ']	= true;
		return_values['\t'] = true;
		return_values['\n'] = true;
		return_values['\r'] = true;
		return return_values;
	}() };

	struct parse_options_new {
		bool partialRead{};
		bool minified{};
		bool knownOrder{};
		bool continueOnError{};
		bool validateJson{};
		int64_t maxDepth{ 1024 };
		uint64_t maxStackSize{ 1024 };
	};

	static consteval std::array<std::array<jsonifier_states, 256>, static_cast<uint64_t>(jsonifier_states::NR_STATES)> genTable() {
		std::array<std::array<jsonifier_states, 256>, static_cast<uint64_t>(jsonifier_states::NR_STATES)> return_values{};
		for (uint64_t stateLocal = 0; stateLocal < static_cast<uint64_t>(jsonifier_states::NR_STATES); ++stateLocal) {
			jsonifier_states etcState = jsonifierStateTransitionTable[stateLocal][static_cast<uint64_t>(jsonifier_classes::C_ETC)];
			for (uint64_t byteVal = 0; byteVal < 256; ++byteVal) {
				if (byteVal >= 128) {
					return_values[stateLocal][byteVal] = etcState;
				} else {
					jsonifier_classes classVal = jsonifierAsciiClasses[byteVal];
					if (classVal < 0) {
						return_values[stateLocal][byteVal] = jsonifier_states::ER;
					} else {
						return_values[stateLocal][byteVal] = jsonifierStateTransitionTable[stateLocal][static_cast<uint64_t>(classVal)];
					}
				}
			}
		}
		return return_values;
	}

	static consteval std::array<std::array<jsonifier_states, static_cast<uint64_t>(jsonifier_classes::NR_CLASSES)>, static_cast<uint64_t>(jsonifier_states::NR_STATES)>
	genStructuralBaseTable() {
		std::array<std::array<jsonifier_states, static_cast<uint64_t>(jsonifier_classes::NR_CLASSES)>, static_cast<uint64_t>(jsonifier_states::NR_STATES)> return_values{};
		for (uint64_t stateLocal = 0; stateLocal < static_cast<uint64_t>(jsonifier_states::NR_STATES); ++stateLocal) {
			for (uint64_t classLocal = 0; classLocal < static_cast<uint64_t>(jsonifier_classes::NR_CLASSES); ++classLocal) {
				return_values[stateLocal][classLocal] = jsonifierStateTransitionTable[stateLocal][classLocal];
			}
		}
		constexpr uint64_t quoteClass = static_cast<uint64_t>(jsonifier_classes::C_QUOTE);
		for (uint64_t stateLocal = 0; stateLocal < static_cast<uint64_t>(jsonifier_states::NR_STATES); ++stateLocal) {
			if (return_values[stateLocal][quoteClass] == jsonifier_states::ST) {
				return_values[stateLocal][quoteClass] = jsonifier_states::n4;
			}
		}
		constexpr std::array<jsonifier_states, 6> stringStates{ { jsonifier_states::ST, jsonifier_states::ES, jsonifier_states::U1, jsonifier_states::U2, jsonifier_states::U3,
			jsonifier_states::U4 } };
		for (jsonifier_states stateLocal: stringStates) {
			for (uint64_t classLocal = 0; classLocal < static_cast<uint64_t>(jsonifier_classes::NR_CLASSES); ++classLocal) {
				return_values[static_cast<uint64_t>(stateLocal)][classLocal] = jsonifier_states::ER;
			}
		}
		constexpr std::array<uint64_t, 6> scalarClasses{ { static_cast<uint64_t>(jsonifier_classes::C_MINUS), static_cast<uint64_t>(jsonifier_classes::C_ZERO),
			static_cast<uint64_t>(jsonifier_classes::C_DIGIT), static_cast<uint64_t>(jsonifier_classes::C_LOW_T), static_cast<uint64_t>(jsonifier_classes::C_LOW_F),
			static_cast<uint64_t>(jsonifier_classes::C_LOW_N) } };
		constexpr std::array<uint64_t, 3> valueStates{ { static_cast<uint64_t>(jsonifier_states::GO), static_cast<uint64_t>(jsonifier_states::VA),
			static_cast<uint64_t>(jsonifier_states::AR) } };
		for (uint64_t stateLocal: valueStates) {
			for (uint64_t classLocal: scalarClasses) {
				return_values[stateLocal][classLocal] = jsonifier_states::OK;
			}
		}
		constexpr std::array<jsonifier_states, 17> scalarInteriorStates{ { jsonifier_states::MI, jsonifier_states::ZE, jsonifier_states::IN, jsonifier_states::FR,
			jsonifier_states::FS, jsonifier_states::E1, jsonifier_states::E2, jsonifier_states::E3, jsonifier_states::T1, jsonifier_states::T2, jsonifier_states::T3,
			jsonifier_states::F1, jsonifier_states::F2, jsonifier_states::F3, jsonifier_states::F4, jsonifier_states::N1, jsonifier_states::N2 } };
		for (jsonifier_states stateLocal: scalarInteriorStates) {
			for (uint64_t classLocal = 0; classLocal < static_cast<uint64_t>(jsonifier_classes::NR_CLASSES); ++classLocal) {
				return_values[static_cast<uint64_t>(stateLocal)][classLocal] = jsonifier_states::ER;
			}
		}
		return return_values;
	}

	static constexpr auto jsonifierStructuralStateTransitionTable{ genStructuralBaseTable() };

	template<typename table_type>
	static consteval std::array<std::array<jsonifier_states, 256>, static_cast<uint64_t>(jsonifier_states::NR_STATES)> genTable(const table_type& baseTable) {
		std::array<std::array<jsonifier_states, 256>, static_cast<uint64_t>(jsonifier_states::NR_STATES)> return_values{};
		for (uint64_t stateLocal = 0; stateLocal < static_cast<uint64_t>(jsonifier_states::NR_STATES); ++stateLocal) {
			jsonifier_states etcState = baseTable[stateLocal][static_cast<uint64_t>(jsonifier_classes::C_ETC)];
			for (uint64_t byteVal = 0; byteVal < 256; ++byteVal) {
				if (byteVal >= 128) {
					return_values[stateLocal][byteVal] = etcState;
				} else {
					jsonifier_classes classVal = jsonifierAsciiClasses[byteVal];
					if (classVal < 0) {
						return_values[stateLocal][byteVal] = jsonifier_states::ER;
					} else {
						return_values[stateLocal][byteVal] = baseTable[stateLocal][static_cast<uint64_t>(classVal)];
					}
				}
			}
		}
		return return_values;
	}

	static constexpr std::array<std::array<jsonifier_states, 256>, static_cast<uint64_t>(jsonifier_states::NR_STATES)> jsonifierByteTransitionTable{ genTable(
		jsonifierStateTransitionTable) };
	static constexpr std::array<std::array<jsonifier_states, 256>, static_cast<uint64_t>(jsonifier_states::NR_STATES)> jsonifierStructuralByteTransitionTable{ genTable(
		jsonifierStructuralStateTransitionTable) };
	struct iterator_state {
		jsonifier_classes jsonifier_class{};
		jsonifier_states jsonifier_state{};
		jsonifier_modes jsonifier_mode{};
		char stringValue{};
	};

	template<parse_options_new parseOpts, typename iterator> struct jsonifier_iterator;

	template<parse_options_new parseOpts> struct jsonifier_iterator<parseOpts, string_view_ptr> {
		using pointer = string_view_ptr;
		bool valid{ true };
		jsonifier_states state{ jsonifier_states::GO };
		int64_t top{ -1 };
		jsonifier_modes stack[parseOpts.maxStackSize]{};
		string_view_ptr rootIter{};
		string_view_ptr endIter{};
		string_view_ptr iter{};
		std::vector<error_data> errors{};

		JSONIFIER_INLINE pointer begin() const noexcept {
			return rootIter;
		}
		JSONIFIER_INLINE pointer end() const noexcept {
			return endIter;
		}
		JSONIFIER_INLINE bool hasNext() const noexcept {
			return iter < endIter;
		}

		JSONIFIER_INLINE jsonifier_iterator(string_view_ptr rootIterNew, string_view_ptr endIterNew) noexcept
			: rootIter{ rootIterNew }, endIter{ endIterNew }, iter{ rootIterNew } {
			++top;
			stack[top] = jsonifier_modes::MODE_DONE;
		}

		JSONIFIER_INLINE jsonifier_iterator& operator++() noexcept {
			if (iter < endIter) {
				jsonifierIterate();
				++iter;
				if constexpr (!parseOpts.minified) {
					while (iter < endIter && jsonifierWhitespaceTable[static_cast<uint8_t>(*iter)]) {
						++iter;
					}
				}
			}
			return *this;
		}

		JSONIFIER_INLINE operator bool() noexcept {
			return valid != false;
		}

		JSONIFIER_INLINE void reject(parse_statuses parseStatus) noexcept {
			error_data new_error{};
			new_error.errorIndex  = static_cast<uint64_t>(iter - rootIter);
			new_error.errorRoot	  = iter;
			new_error.parseStatus = parseStatus;
			new_error.errorLength = std::min(static_cast<uint64_t>(16ULL), static_cast<uint64_t>(endIter - iter));
			errors.emplace_back(new_error);
			if constexpr (!parseOpts.continueOnError) {
				valid = false;
			} else {
				skipToNextValue();
			}
		}

		JSONIFIER_INLINE void jsonifierIterate() noexcept {
			int32_t next_char = static_cast<uint8_t>(*iter);
			jsonifier_states next_state;
			if (next_char == 0) {
				if (iter + 1 < endIter) {
					return reject(parse_statuses::illegal_control_character);
				}
				return reject(truncationStatus(state));
			}
			next_state = jsonifierByteTransitionTable[static_cast<uint64_t>(state)][static_cast<uint64_t>(next_char)];
			if (next_state >= 0) {
				state = next_state;
			} else {
				switch (static_cast<uint64_t>(next_state)) {
					case static_cast<uint64_t>(jsonifier_states::n9):
						if (!pop<jsonifier_modes::MODE_KEY>()) {
							return reject<parse_statuses::invalid_string_characters>();
						}
						state = jsonifier_states::OK;
						break;

					case static_cast<uint64_t>(jsonifier_states::n8):
						if (!pop<jsonifier_modes::MODE_OBJECT>()) {
							return reject<parse_statuses::missing_object_end>();
						}
						state = jsonifier_states::OK;
						break;

					case static_cast<uint64_t>(jsonifier_states::n7):
						if (!pop<jsonifier_modes::MODE_ARRAY>()) {
							return reject<parse_statuses::missing_array_end>();
						}
						state = jsonifier_states::OK;
						break;

					case static_cast<uint64_t>(jsonifier_states::n6):
						if (!push<jsonifier_modes::MODE_KEY>()) {
							return reject<parse_statuses::invalid_string_characters>();
						}
						state = jsonifier_states::OB;
						break;

					case static_cast<uint64_t>(jsonifier_states::n5):
						if (!push<jsonifier_modes::MODE_ARRAY>()) {
							return reject<parse_statuses::missing_array_start>();
						}
						state = jsonifier_states::AR;
						break;

					case static_cast<uint64_t>(jsonifier_states::n4):
						switch (static_cast<uint64_t>(stack[top])) {
							case static_cast<uint64_t>(jsonifier_modes::MODE_KEY):
								state = jsonifier_states::CO;
								break;
							case static_cast<uint64_t>(jsonifier_modes::MODE_ARRAY):
							case static_cast<uint64_t>(jsonifier_modes::MODE_OBJECT):
								state = jsonifier_states::OK;
								break;
							case static_cast<uint64_t>(jsonifier_modes::MODE_DONE):
								state = jsonifier_states::OK;
								break;
							default:
								return reject<parse_statuses::missing_object_start>();
						}
						break;
					case static_cast<uint64_t>(jsonifier_states::n3):
						switch (static_cast<uint64_t>(stack[top])) {
							case static_cast<uint64_t>(jsonifier_modes::MODE_OBJECT):
								if (!pop<jsonifier_modes::MODE_OBJECT>() || !push<jsonifier_modes::MODE_KEY>()) {
									return reject<parse_statuses::missing_comma>();
								}
								state = jsonifier_states::KE;
								break;
							case static_cast<uint64_t>(jsonifier_modes::MODE_ARRAY):
								state = jsonifier_states::VA;
								break;
							default:
								return reject<parse_statuses::missing_comma>();
						}
						break;

					case static_cast<uint64_t>(jsonifier_states::n2):
						if (!pop<jsonifier_modes::MODE_KEY>() || !push<jsonifier_modes::MODE_OBJECT>()) {
							return reject<parse_statuses::missing_colon>();
						}
						state = jsonifier_states::VA;
						break;
					default:
						return reject<parse_statuses::unexpected_token>();
				}
			}
			return;
		}

		template<jsonifier::concepts::bool_t bool_type> JSONIFIER_INLINE void parseBoolAndAdvance(bool_type& value) noexcept {
			string_view_ptr result = parseBool(value, iter, endIter);
			if JSONIFIER_LIKELY (result != nullptr) {
				iter = result;
			} else {
				reject(parse_statuses::invalid_bool_value);
			}
		}

		JSONIFIER_INLINE void parseNullAndAdvance() noexcept {
			string_view_ptr result = parseNull(iter, endIter);
			if JSONIFIER_LIKELY (result != nullptr) {
				iter = result;
			} else {
				reject(parse_statuses::invalid_null_value);
			}
		}

		JSONIFIER_INLINE bool skipToNextValue() {
			int64_t relativeDepth{};
			bool inString{};
			while (iter < endIter) {
				const uint8_t current = static_cast<uint8_t>(*iter);
				if (inString) {
					if (current == '\\') {
						++iter;
						if (iter < endIter) {
							++iter;
						}
						continue;
					}
					if (current == '"') {
						inString = false;
					}
					++iter;
					continue;
				}
				switch (current) {
					case '"':
						inString = true;
						++iter;
						continue;
					case '{':
					case '[':
						++relativeDepth;
						++iter;
						continue;
					case '}':
					case ']':
						if (relativeDepth == 0) {
							if (current == '}') {
								if (!pop<jsonifier_modes::MODE_OBJECT>() && !pop<jsonifier_modes::MODE_KEY>()) {
									return false;
								}
							} else {
								if (!pop<jsonifier_modes::MODE_ARRAY>()) {
									return false;
								}
							}
							state = jsonifier_states::OK;
							valid = true;
							++iter;
							return true;
						}
						--relativeDepth;
						++iter;
						continue;
					case ',':
						if (relativeDepth == 0) {
							if (top >= 0 && stack[top] == jsonifier_modes::MODE_OBJECT) {
								if (!pop<jsonifier_modes::MODE_OBJECT>() || !push<jsonifier_modes::MODE_KEY>()) {
									return false;
								}
								state = jsonifier_states::KE;
							} else if (top >= 0 && stack[top] == jsonifier_modes::MODE_ARRAY) {
								state = jsonifier_states::VA;
							} else {
								return false;
							}
							valid = true;
							++iter;
							return true;
						}
						++iter;
						continue;
					default:
						++iter;
						continue;
				}
			}
			return false;
		}

		JSONIFIER_INLINE parse_statuses truncationStatus(jsonifier_states stateNew) noexcept {
			return jsonifierParseStatuses[static_cast<uint64_t>(stateNew)];
		}

		JSONIFIER_INLINE bool jsonifierIterateDone() noexcept {
			if (valid != true) {
				return false;
			}
			bool stateOk = state == jsonifier_states::OK || state == jsonifier_states::IN || state == jsonifier_states::ZE || state == jsonifier_states::FS ||
				state == jsonifier_states::E3 || state == jsonifier_states::T3 || state == jsonifier_states::F4 || state == jsonifier_states::N3;
			if (!stateOk) {
				error_data new_error{};
				new_error.errorIndex  = static_cast<uint64_t>(iter - rootIter);
				new_error.errorRoot	  = iter;
				new_error.errorLength = std::min(static_cast<uint64_t>(16ULL), static_cast<uint64_t>(endIter - iter));
				new_error.parseStatus = truncationStatus(state);
				errors.emplace_back(new_error);
				return false;
			}
			if (top > 0) {
				error_data new_error{};
				new_error.errorIndex  = static_cast<uint64_t>(iter - rootIter);
				new_error.errorRoot	  = iter;
				new_error.errorLength = std::min(static_cast<uint64_t>(16ULL), static_cast<uint64_t>(endIter - iter));
				new_error.parseStatus =
					static_cast<uint64_t>(stack[top]) & static_cast<uint64_t>(jsonifier_modes::MODE_ARRAY) ? parse_statuses::missing_array_end : parse_statuses::missing_object_end;
				errors.emplace_back(new_error);
				return false;
			}
			return pop<jsonifier_modes::MODE_DONE>() && errors.size() == 0;
		}

		template<parse_statuses parseStatus> JSONIFIER_INLINE void reject() noexcept {
			error_data new_error{};
			new_error.errorIndex  = static_cast<uint64_t>(iter - rootIter);
			new_error.errorRoot	  = iter;
			new_error.parseStatus = parseStatus;
			new_error.errorLength = std::min(static_cast<uint64_t>(16ULL), static_cast<uint64_t>(endIter - iter));
			errors.emplace_back(new_error);
			if constexpr (!parseOpts.continueOnError) {
				valid = false;
			} else {
				skipToNextValue();
			}
		}

		template<jsonifier_modes mode> JSONIFIER_INLINE bool push() noexcept {
			++top;
			if (top >= parseOpts.maxDepth) {
				return false;
			}
			stack[top] = mode;
			return true;
		}

		template<jsonifier_modes mode> JSONIFIER_INLINE bool pop() noexcept {
			if (top < 0 || stack[top] != mode) {
				return false;
			}
			--top;
			return true;
		}
	};

	template<parse_options_new parseOpts> struct jsonifier_iterator<parseOpts, structural_index_ptr> {
		using pointer		 = structural_index_ptr;
		using string_pointer = string_view_ptr;
		bool valid{ true };
		jsonifier_states state{ jsonifier_states::GO };
		int64_t top{ -1 };
		jsonifier_modes stack[parseOpts.maxStackSize]{};
		string_pointer stringRootIter{};
		string_pointer stringEndIter{};
		pointer rootIter{};
		pointer endIter{};
		pointer iter{};
		std::vector<error_data> errors{};

		JSONIFIER_INLINE pointer begin() const noexcept {
			return rootIter;
		}
		JSONIFIER_INLINE pointer end() const noexcept {
			return endIter;
		}
		JSONIFIER_INLINE bool hasNext() const noexcept {
			return iter < endIter;
		}

		JSONIFIER_INLINE jsonifier_iterator(pointer rootIterNew, pointer endIterNew, string_pointer stringRootIterNew, string_pointer stringEndIterNew) noexcept
			: stringRootIter{ stringRootIterNew }, stringEndIter{ stringEndIterNew }, rootIter{ rootIterNew }, endIter{ endIterNew }, iter{ rootIterNew } {
			++top;
			stack[top] = jsonifier_modes::MODE_DONE;
		}

		JSONIFIER_INLINE jsonifier_iterator& operator++() noexcept {
			if (iter < endIter) {
				jsonifierIterate();
				++iter;
			}
			return *this;
		}

		JSONIFIER_INLINE operator bool() noexcept {
			return valid != false;
		}

		JSONIFIER_INLINE void reject(parse_statuses parseStatus) noexcept {
			error_data new_error{};
			if (iter) {
				new_error.errorIndex = static_cast<uint64_t>(*iter);
				if (stringRootIter) {
					new_error.errorRoot	  = &stringRootIter[*iter];
					new_error.errorLength = std::min(static_cast<uint64_t>(16ULL), static_cast<uint64_t>(stringEndIter - new_error.errorRoot));
				}
			}
			new_error.parseStatus = parseStatus;
			errors.emplace_back(new_error);
			if constexpr (!parseOpts.continueOnError) {
				valid = false;
			} else {
				skipToNextValue();
			}
		}

		JSONIFIER_INLINE void jsonifierIterate() noexcept {
			int32_t next_char = static_cast<uint8_t>(stringRootIter[*iter]);
			jsonifier_states next_state;
			if (next_char == 0) {
				if (iter + 1 < endIter) {
					return reject(parse_statuses::illegal_control_character);
				}
				return reject(truncationStatus(state));
			}
			next_state = jsonifierStructuralByteTransitionTable[static_cast<uint64_t>(state)][static_cast<uint64_t>(next_char)];
			if (next_state >= 0) {
				state = next_state;
			} else {
				switch (static_cast<uint64_t>(next_state)) {
					case static_cast<uint64_t>(jsonifier_states::n9):
						if (!pop<jsonifier_modes::MODE_KEY>()) {
							return reject<parse_statuses::invalid_string_characters>();
						}
						state = jsonifier_states::OK;
						break;

					case static_cast<uint64_t>(jsonifier_states::n8):
						if (!pop<jsonifier_modes::MODE_OBJECT>()) {
							return reject<parse_statuses::missing_object_end>();
						}
						state = jsonifier_states::OK;
						break;

					case static_cast<uint64_t>(jsonifier_states::n7):
						if (!pop<jsonifier_modes::MODE_ARRAY>()) {
							return reject<parse_statuses::missing_array_end>();
						}
						state = jsonifier_states::OK;
						break;

					case static_cast<uint64_t>(jsonifier_states::n6):
						if (!push<jsonifier_modes::MODE_KEY>()) {
							return reject<parse_statuses::invalid_string_characters>();
						}
						state = jsonifier_states::OB;
						break;

					case static_cast<uint64_t>(jsonifier_states::n5):
						if (!push<jsonifier_modes::MODE_ARRAY>()) {
							return reject<parse_statuses::missing_array_start>();
						}
						state = jsonifier_states::AR;
						break;

					case static_cast<uint64_t>(jsonifier_states::n4):
						switch (static_cast<uint64_t>(stack[top])) {
							case static_cast<uint64_t>(jsonifier_modes::MODE_KEY):
								state = jsonifier_states::CO;
								break;
							case static_cast<uint64_t>(jsonifier_modes::MODE_ARRAY):
							case static_cast<uint64_t>(jsonifier_modes::MODE_OBJECT):
								state = jsonifier_states::OK;
								break;
							case static_cast<uint64_t>(jsonifier_modes::MODE_DONE):
								state = jsonifier_states::OK;
								break;
							default:
								return reject<parse_statuses::missing_object_start>();
						}
						break;
					case static_cast<uint64_t>(jsonifier_states::n3):
						switch (static_cast<uint64_t>(stack[top])) {
							case static_cast<uint64_t>(jsonifier_modes::MODE_OBJECT):
								if (!pop<jsonifier_modes::MODE_OBJECT>() || !push<jsonifier_modes::MODE_KEY>()) {
									return reject<parse_statuses::missing_comma>();
								}
								state = jsonifier_states::KE;
								break;
							case static_cast<uint64_t>(jsonifier_modes::MODE_ARRAY):
								state = jsonifier_states::VA;
								break;
							default:
								return reject<parse_statuses::missing_comma>();
						}
						break;

					case static_cast<uint64_t>(jsonifier_states::n2):
						if (!pop<jsonifier_modes::MODE_KEY>() || !push<jsonifier_modes::MODE_OBJECT>()) {
							return reject<parse_statuses::missing_colon>();
						}
						state = jsonifier_states::VA;
						break;
					default:
						return reject<parse_statuses::unexpected_token>();
				}
			}
			return;
		}

		template<jsonifier::concepts::bool_t bool_type> JSONIFIER_INLINE void parseBoolAndAdvance(bool_type& value) noexcept {
			string_view_ptr result = parseBool(value, &stringRootIter[*iter], &stringRootIter[*endIter]);
			if JSONIFIER_LIKELY (result != nullptr) {
				++iter;
			} else {
				reject(parse_statuses::invalid_bool_value);
			}
		}

		JSONIFIER_INLINE void parseNullAndAdvance() noexcept {
			string_view_ptr result = parseNull(&stringRootIter[*iter], &stringRootIter[*endIter]);
			if JSONIFIER_LIKELY (result != nullptr) {
				++iter;
			} else {
				reject(parse_statuses::invalid_null_value);
			}
		}

		JSONIFIER_INLINE bool skipToNextValue() {
			int64_t relativeDepth{};
			while (iter < endIter) {
				const uint8_t current = static_cast<uint8_t>(stringRootIter[*iter]);
				switch (current) {
					case '"':
						++iter;
						continue;
					case '{':
					case '[':
						++relativeDepth;
						++iter;
						continue;
					case '}':
					case ']':
						if (relativeDepth == 0) {
							if (current == '}') {
								if (!pop<jsonifier_modes::MODE_OBJECT>() && !pop<jsonifier_modes::MODE_KEY>()) {
									return false;
								}
							} else {
								if (!pop<jsonifier_modes::MODE_ARRAY>()) {
									return false;
								}
							}
							state = jsonifier_states::OK;
							valid = true;
							++iter;
							return true;
						}
						--relativeDepth;
						++iter;
						continue;
					case ',':
						if (relativeDepth == 0) {
							if (top >= 0 && stack[top] == jsonifier_modes::MODE_OBJECT) {
								if (!pop<jsonifier_modes::MODE_OBJECT>() || !push<jsonifier_modes::MODE_KEY>()) {
									return false;
								}
								state = jsonifier_states::KE;
							} else if (top >= 0 && stack[top] == jsonifier_modes::MODE_ARRAY) {
								state = jsonifier_states::VA;
							} else {
								return false;
							}
							valid = true;
							++iter;
							return true;
						}
						++iter;
						continue;
					default:
						++iter;
						continue;
				}
			}
			return false;
		}

		JSONIFIER_INLINE parse_statuses truncationStatus(jsonifier_states stateNew) noexcept {
			return jsonifierParseStatuses[static_cast<uint64_t>(stateNew)];
		}

		JSONIFIER_INLINE bool jsonifierIterateDone() noexcept {
			if (valid != true) {
				return false;
			}
			bool stateOk = state == jsonifier_states::OK || state == jsonifier_states::IN || state == jsonifier_states::ZE || state == jsonifier_states::FS ||
				state == jsonifier_states::E3 || state == jsonifier_states::T3 || state == jsonifier_states::F4 || state == jsonifier_states::N3;
			if (!stateOk) {
				error_data new_error{};
				if (iter) {
					new_error.errorIndex  = static_cast<uint64_t>(*iter);
					new_error.errorLength = std::min(static_cast<uint64_t>(16ULL), static_cast<uint64_t>(stringEndIter - &stringRootIter[*iter]));
					if (stringRootIter) {
						new_error.errorRoot = &stringRootIter[*iter];
					}
				}
				new_error.parseStatus = truncationStatus(state);
				errors.emplace_back(new_error);
				return false;
			}
			if (top > 0) {
				error_data new_error{};
				if (iter) {
					new_error.errorIndex  = static_cast<uint64_t>(*iter);
					new_error.errorLength = std::min(static_cast<uint64_t>(16ULL), static_cast<uint64_t>(stringEndIter - &stringRootIter[*iter]));
					if (stringRootIter) {
						new_error.errorRoot = &stringRootIter[*iter];
					}
				}
				new_error.parseStatus = stack[top] == jsonifier_modes::MODE_ARRAY ? parse_statuses::missing_array_end : parse_statuses::missing_object_end;
				errors.emplace_back(new_error);
				return false;
			}
			return pop<jsonifier_modes::MODE_DONE>() && errors.size() == 0;
		}

		template<parse_statuses parseStatus> JSONIFIER_INLINE void reject() noexcept {
			error_data new_error{};
			if (iter) {
				new_error.errorIndex  = static_cast<uint64_t>(*iter);
				new_error.errorLength = std::min(static_cast<uint64_t>(16ULL), static_cast<uint64_t>(stringEndIter - &stringRootIter[*iter]));
				if (stringRootIter) {
					new_error.errorRoot = &stringRootIter[*iter];
				}
			}
			new_error.parseStatus = parseStatus;
			errors.emplace_back(new_error);
			if constexpr (!parseOpts.continueOnError) {
				valid = false;
			} else {
				skipToNextValue();
			}
		}

		template<jsonifier_modes mode> JSONIFIER_INLINE bool push() noexcept {
			++top;
			if (top >= parseOpts.maxDepth) {
				return false;
			}
			stack[top] = mode;
			return true;
		}

		template<jsonifier_modes mode> JSONIFIER_INLINE bool pop() noexcept {
			if (top < 0 || stack[top] != mode) {
				return false;
			}
			--top;
			return true;
		}
	};
}
