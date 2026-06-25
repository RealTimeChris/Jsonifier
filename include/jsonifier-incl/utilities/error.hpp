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

#pragma once

#include <jsonifier-incl/utilities/string_view.hpp>
#include <jsonifier-incl/utilities/simd.hpp>

namespace jsonifier::internal {

	enum class status_classes : uint8_t {
		unset,
		parsing,
		serializing,
		minifying,
		prettifying,
		validating,
	};

	enum class parse_statuses : uint8_t {
		success,
		missing_key_start,
		missing_object_start,
		missing_object_end,
		missing_array_start,
		missing_array_end,
		invalid_string_characters,
		missing_colon,
		missing_comma,
		invalid_number_value,
		invalid_null_value,
		invalid_bool_value,
		no_input,
		unfinished_input,
		unexpected_string_end,
		unexpected_end_of_input,
		exceeded_max_depth,
		unexpected_token,
		illegal_control_character,
		count,
	};

	enum class serialize_status : uint8_t {
		success,
		count,
	};

	enum class minify_statuses : uint8_t {
		success,
		no_input,
		invalid_string_length,
		invalid_number_value,
		incorrect_structural_index,
		count,
	};

	enum class prettify_statuses : uint8_t {
		success,
		no_input,
		exceeded_max_depth,
		incorrect_structural_index,
		count,
	};

	enum class validate_statuses : uint8_t {
		success,
		missing_object_start,
		missing_object_end,
		missing_array_start,
		missing_array_end,
		invalid_string_characters,
		missing_colon,
		missing_comma,
		invalid_number_value,
		invalid_null_value,
		invalid_bool_value,
		invalid_escape_characters,
		missing_comma_or_closing_brace,
		no_input,
		count,
	};

	static constexpr array<string_view_ptr, static_cast<size_t>(serialize_status::count)> serializeErrorNames{ { "success" } };

	static constexpr array<string_view_ptr, static_cast<size_t>(minify_statuses::count)> minifyErrorNames{ { "success", "no_input", "invalid_string_length", "invalid_number_value",
		"incorrect_structural_index" } };

	static constexpr array<string_view_ptr, static_cast<size_t>(prettify_statuses::count)> prettifyErrorNames{ { "success", "no_input", "exceeded_max_depth",
		"incorrect_structural_index" } };

	static constexpr array<string_view_ptr, static_cast<size_t>(validate_statuses::count)> validateErrorNames{ { "success", "missing_object_start", "missing_object_end",
		"missing_array_start", "missing_array_end", "invalid_string_characters", "missing_colon", "missing_comma", "invalid_number_value", "invalid_null_value",
		"invalid_bool_value", "invalid_escape_characters", "missing_comma_or_closing_brace", "no_input" } };

	static constexpr array<string_view_ptr, static_cast<size_t>(parse_statuses::count)> parseStatusNames{ { "success", "missing_key_start", "missing_object_start",
		"missing_object_end", "missing_array_start", "missing_array_end", "invalid_string_characters", "missing_colon", "missing_comma", "invalid_number_value",
		"invalid_null_value", "invalid_bool_value", "no_input", "unfinished_input", "unexpected_string_end", "unexpected_end_of_input", "exceeded_max_depth", "unexpected_token",
		"illegal_control_character" } };

	JSONIFIER_INLINE std::string printBytes(char b) {
		switch (b) {
			case '\a':
				return "\\a";
			case '\b':
				return "\\b";
				break;
			case '\f':
				return "\\f";
				break;
			case '\n':
				return "\\n";
				break;
			case '\r':
				return "\\r";
				break;
			case '\t':
				return "\\t";
				break;
			case '\v':
				return "\\v";
				break;
			case '\\':
				return "\\\\";
				break;
			case '\'':
				return "\\'";
				break;
			case '\"':
				return "\\\"";
				break;
			case '\0':
				return "\\0";
				break;
			default:
				if (std::isprint(static_cast<unsigned char>(b))) {
					return std::string{ b };
				} else {
					std::stringstream stream{};
					stream << "\\x" << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(b) << std::dec;
					return stream.str();
				}
				break;
		}
	}

	template<typename error_type> constexpr string_view_ptr errorName(status_classes cls, error_type codeNew) noexcept {
		uint64_t code{ static_cast<uint64_t>(codeNew) };
		switch (static_cast<uint64_t>(cls)) {
			case static_cast<uint64_t>(status_classes::parsing):
				return code < parseStatusNames.size() ? parseStatusNames[code] : "Unknown_Parse_Status";
			case static_cast<uint64_t>(status_classes::serializing):
				return code < serializeErrorNames.size() ? serializeErrorNames[code] : "Unknown_Serialize_Status";
			case static_cast<uint64_t>(status_classes::minifying):
				return code < minifyErrorNames.size() ? minifyErrorNames[code] : "Unknown_Minify_Status";
			case static_cast<uint64_t>(status_classes::prettifying):
				return code < prettifyErrorNames.size() ? prettifyErrorNames[code] : "Unknown_Prettify_Status";
			case static_cast<uint64_t>(status_classes::validating):
				return code < validateErrorNames.size() ? validateErrorNames[code] : "Unknown_Validate_Status";
			case static_cast<uint64_t>(status_classes::unset):
				[[fallthrough]];
			default:
				return "unset";
		}
	}

	inline static std::ostream& operator<<(std::ostream& os, serialize_status error) {
		os << errorName(status_classes::serializing, error);
		return os;
	}

	inline static std::ostream& operator<<(std::ostream& os, validate_statuses error) {
		os << errorName(status_classes::validating, error);
		return os;
	}

	inline static std::ostream& operator<<(std::ostream& os, minify_statuses error) {
		os << errorName(status_classes::minifying, error);
		return os;
	}

	inline static std::ostream& operator<<(std::ostream& os, prettify_statuses error) {
		os << errorName(status_classes::prettifying, error);
		return os;
	}

	inline static std::ostream& operator<<(std::ostream& os, parse_statuses error) {
		os << errorName(status_classes::parsing, error);
		return os;
	}

	enum class json_structural_type : int8_t {
		unset		 = 0,
		string		 = '"',
		comma		 = ',',
		number		 = '-',
		colon		 = ':',
		array_start	 = '[',
		array_end	 = ']',
		null		 = 'n',
		boolean		 = 't',
		object_start = '{',
		object_end	 = '}',
		error		 = -1,
		type_count	 = 12
	};

	inline static string convertChar(char value) {
		switch (value) {
			[[unlikely]] case '\b': { return R"(\b)"; }
			[[unlikely]] case '\t': { return R"(\t)"; }
			[[unlikely]] case '\n': { return R"(\n)"; }
			[[unlikely]] case '\f': { return R"(\f)"; }
			[[unlikely]] case '\r': { return R"(\r)"; }
			[[unlikely]] case '"': { return R"(\")"; }
			[[unlikely]] case '\\': { return R"(\\)"; }
			[[unlikely]] case '\0': { return R"(\0)"; }
			[[likely]] default: { return string{ value }; }
		}
	}

	static constexpr array<bool, 256> boolTable{ []() constexpr {
		array<bool, 256> returnValues{};
		returnValues['t'] = true;
		returnValues['f'] = true;
		return returnValues;
	}() };

	class error {
	  public:
		template<parse_options parseOpts, typename iterator_type, typename string_buffer_type> friend struct json_iterator;
		error() noexcept = default;

		error(std::source_location sourceLocation, status_classes errorClassNew, string_view_ptr rootIter, string_view_ptr errorPos, string_view_ptr endIter,
			uint64_t typeNew) noexcept {
			location   = sourceLocation;
			errorClass = errorClassNew;
			errorIndex = static_cast<uint64_t>(errorPos - rootIter);
			errorType  = typeNew;

			if (!errorPos || !endIter || errorPos >= endIter) {
				return;
			}

			errorLength = std::min(static_cast<uint64_t>(16ULL), static_cast<uint64_t>(endIter - errorPos));

			int64_t reportIndex = static_cast<int64_t>(errorIndex);
			if (static_cast<uint64_t>(reportIndex) >= errorLength) {
				reportIndex = static_cast<int64_t>(errorLength) - 1;
			}

			string_view view{ errorPos, errorLength };
			using V				   = std::decay_t<decltype(view[0])>;
			const auto start	   = std::begin(view) + reportIndex;
			line				   = uint64_t(std::count(std::begin(view), start, static_cast<V>('\n')) + 1);
			const auto rstart	   = std::rbegin(view) + static_cast<int64_t>(view.size()) - reportIndex - 1ll;
			const auto prevNewLine = std::find(( std::min )(rstart + 1, std::rend(view)), std::rend(view), static_cast<V>('\n'));
			localIndex			   = static_cast<uint64_t>(std::distance(rstart, prevNewLine) - 1ll);

			auto endIndex{ std::end(view) - start >= 64 ? 64 : std::end(view) - start };
			context = string{ start, static_cast<uint64_t>(endIndex) };
			for (auto& c: context) {
				if (c == '\t') {
					c = ' ';
				}
			}
		}

		template<status_classes errorClassNew> static error constructError(auto typeNew, string_view_ptr rootIter, string_view_ptr errorPos, string_view_ptr endIter,
			const std::source_location& sourceLocation = std::source_location::current()) noexcept {
			return { sourceLocation, errorClassNew, rootIter, errorPos, endIter, static_cast<uint64_t>(typeNew) };
		}

		template<status_classes errorClassNew, auto typeNew> static error constructError(string_view_ptr rootIter, string_view_ptr errorPos, string_view_ptr endIter,
			const std::source_location& sourceLocation = std::source_location::current()) noexcept {
			return { sourceLocation, errorClassNew, rootIter, errorPos, endIter, static_cast<uint64_t>(typeNew) };
		}

		operator uint64_t() const noexcept {
			return errorType;
		}

		operator parse_statuses() const noexcept {
			return static_cast<parse_statuses>(errorType);
		}

		operator bool() const noexcept {
			return errorType != 0;
		}

		bool operator==(const error& rhs) const noexcept {
			return errorType == rhs.errorType && errorIndex == rhs.errorIndex;
		}

		string reportError() const noexcept {
			string returnValue{ string{ "Error of Type: " } + errorName(errorClass, errorType) + ", at global index: " + std::to_string(errorIndex) +
				", on line: " + std::to_string(line) + ", at local index: " + std::to_string(localIndex) };
			if (!context.empty()) {
				returnValue += "\nHere's some of the string's values: " + collectValues(context) + string{ "\nThe Values: " } + context;
			}
			returnValue += string{ "\nIn file: " } + location.file_name() + ", at: " + std::to_string(location.line()) + ":" + std::to_string(location.column()) +
				", in function: " + location.function_name() + "().\n";
			return returnValue;
		}

	  protected:
		std::source_location location{};
		status_classes errorClass{};
		uint64_t errorLength{};
		uint64_t errorIndex{};
		uint64_t localIndex{};
		uint64_t errorType{};
		string context{};
		uint64_t line{};
		string collectValues(const string& inputValues) const {
			string output{};
			for (uint64_t i = 0; i < 32 && i < inputValues.size(); ++i) {
				output += string{ "'" } + printBytes(inputValues[i]) + string{ "' " };
			}
			return output;
		}
	};

	inline static std::ostream& operator<<(std::ostream& os, const error& errorNew) noexcept {
		os << errorNew.reportError();
		return os;
	}


}
