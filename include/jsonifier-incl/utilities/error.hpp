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

#include <jsonifier-incl/utilities/get_enum_name.hpp>
#include <jsonifier-incl/utilities/simd.hpp>

namespace jsonifier::internal {

	enum class status_classes : uint8_t {
		unset,
		parsing,
		serializing,
		minifying,
		prettifying,
		validating,
		count,
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

	enum class serialize_statuses : uint8_t {
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

	JSONIFIER_INLINE string_view getErrorTypeName(status_classes errorClassNew, uint64_t errorTypeNew) noexcept {
		switch (static_cast<uint64_t>(errorClassNew)) {
			case static_cast<uint64_t>(status_classes::parsing): {
				return getName(static_cast<parse_statuses>(errorTypeNew));
			}
			case static_cast<uint64_t>(status_classes::serializing): {
				return getName(static_cast<serialize_statuses>(errorTypeNew));
			}
			case static_cast<uint64_t>(status_classes::prettifying): {
				return getName(static_cast<prettify_statuses>(errorTypeNew));
			}
			case static_cast<uint64_t>(status_classes::minifying): {
				return getName(static_cast<minify_statuses>(errorTypeNew));
			}
			case static_cast<uint64_t>(status_classes::validating): {
				return getName(static_cast<validate_statuses>(errorTypeNew));
			}
			default: {
				return "Unknown Type.";
			}
		}
	}

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
			errorType			= typeNew;
			errorClass			= errorClassNew;
			uint64_t errorIndex = static_cast<uint64_t>(errorPos - rootIter);

			uint64_t line{ 0 };
			uint64_t localIndex{ 0 };
			string context{};

			if (errorPos && endIter && errorPos < endIter) {
				uint64_t errorLength = std::min(static_cast<uint64_t>(16ULL), static_cast<uint64_t>(endIter - errorPos));
				int64_t reportIndex	 = static_cast<int64_t>(errorIndex);
				if (static_cast<uint64_t>(reportIndex) >= errorLength) {
					reportIndex = static_cast<int64_t>(errorLength) - 1;
				}
				string_view view{ errorPos, errorLength };
				using V				   = std::decay_t<decltype(view[0])>;
				const auto start	   = std::begin(view) + reportIndex;
				line				   = uint64_t(std::count(std::begin(view), start, static_cast<V>('\n')) + 1);
				const auto rstart	   = std::rbegin(view) + static_cast<int64_t>(view.size()) - reportIndex - 1ll;
				const auto prevNewLine = std::find((std::min)(rstart + 1, std::rend(view)), std::rend(view), static_cast<V>('\n'));
				localIndex			   = static_cast<uint64_t>(std::distance(rstart, prevNewLine) - 1ll);
				auto endIndex{ std::end(view) - start >= 64 ? 64 : std::end(view) - start };
				context = string{ start, static_cast<uint64_t>(endIndex) };
				for (auto& c: context) {
					if (c == '\t') {
						c = ' ';
					}
				}
			}

			std::stringstream stream{};
			stream << "Error of Class: " << getName(errorClass) << ", of Type: " << getErrorTypeName(errorClass, errorType) << ", at global index: " << std::to_string(errorIndex)
				   << ", on line: " << std::to_string(line) << ", at local index: " << std::to_string(localIndex);
			if (!context.empty()) {
				stream << "\nHere's some of the string's values: " << collectValues(context) << string{ "\nThe Values: " } + context;
			}
			stream << string{ "\nIn file: " } << sourceLocation.file_name() << ", at: " << std::to_string(sourceLocation.line()) << ":" << std::to_string(sourceLocation.column())
				   << ", in function: " << sourceLocation.function_name() << "().\n";
			finalMessage = stream.str();
		}

		template<status_classes errorClassNew> static error constructError(auto typeNew, string_view_ptr rootIter, string_view_ptr errorPos, string_view_ptr endIter,
			const std::source_location& sourceLocation = std::source_location::current()) noexcept {
			return { sourceLocation, errorClassNew, rootIter, errorPos, endIter, static_cast<uint64_t>(typeNew) };
		}

		template<status_classes errorClassNew, auto typeNew> static error constructError(string_view_ptr rootIter, string_view_ptr errorPos, string_view_ptr endIter,
			const std::source_location& sourceLocation = std::source_location::current()) noexcept {
			return { sourceLocation, errorClassNew, rootIter, errorPos, endIter, static_cast<uint64_t>(typeNew) };
		}

		template<typename error_class> operator error_class() const noexcept {
			return static_cast<error_class>(errorType);
		}

		operator bool() const noexcept {
			return errorType != 0;
		}

		bool operator==(const error& rhs) const noexcept {
			return errorType == rhs.errorType && errorClass == rhs.errorClass && finalMessage == rhs.finalMessage;
		}

		const string& reportError() const noexcept {
			return finalMessage;
		}

	  protected:
		status_classes errorClass{};
		string finalMessage{};
		uint64_t errorType{};

		static string collectValues(const string& inputValues) {
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
