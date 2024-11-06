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

#include <jsonifier/HashMap.hpp>
#include <jsonifier/String.hpp>

namespace std {

	template<jsonifier::concepts::string_t string_type> struct hash<string_type> : public std::hash<std::string_view> {
		JSONIFIER_ALWAYS_INLINE uint64_t operator()(const string_type& string) const noexcept {
			return std::hash<std::string_view>::operator()(std::string_view{ string });
		}
	};
}

namespace jsonifier {

	enum class json_type : uint8_t {
		Unset  = 0,
		Object = '{',
		Array  = '[',
		String = '"',
		Number = '-',
		Bool   = 't',
		Null   = 'n',
	};

	class raw_json_data;
}

namespace jsonifier_internal {

	using object_type = std::unordered_map<jsonifier::string, jsonifier::raw_json_data>;
	using string_type = jsonifier::string;
	using array_type  = jsonifier::vector<jsonifier::raw_json_data>;
	using value_type  = std::variant<object_type, array_type, string_type, double, bool, std::nullptr_t>;

	value_type constructValueFromRawJsonData(const jsonifier::string& newData) noexcept;

	JSONIFIER_ALWAYS_INLINE jsonifier::json_type getValueType(uint8_t charToCheck) noexcept {
		if JSONIFIER_LIKELY ((isNumberType(charToCheck))) {
			return jsonifier::json_type::Number;
		} else {
			if JSONIFIER_LIKELY ((boolTable[charToCheck])) {
				return jsonifier::json_type::Bool;
			} else {
				if JSONIFIER_UNLIKELY ((charToCheck == '{')) {
					return jsonifier::json_type::Object;
				} else {
					if JSONIFIER_UNLIKELY ((charToCheck == '[')) {
						return jsonifier::json_type::Array;
					} else {
						if JSONIFIER_UNLIKELY ((charToCheck == '"')) {
							return jsonifier::json_type::String;
						} else {
							if JSONIFIER_UNLIKELY ((charToCheck == 'n')) {
								return jsonifier::json_type::Null;
							} else {
								return jsonifier::json_type::Unset;
							}
						}
					}
				}
			}
		}
	}

}

namespace jsonifier {

	class raw_json_data {
	  public:
		template<typename value_type> friend value_type constructValueFromRawJsonData(const jsonifier::string& newData);
		using object_type = std::unordered_map<jsonifier::string, raw_json_data>;
		using string_type = string;
		using array_type  = jsonifier::vector<raw_json_data>;
		using value_type  = std::variant<object_type, array_type, string_type, double, bool, std::nullptr_t>;

		JSONIFIER_ALWAYS_INLINE raw_json_data() = default;

		JSONIFIER_ALWAYS_INLINE raw_json_data& operator=(const jsonifier::string& valueNew) noexcept {
			jsonData = valueNew;
			value	 = constructValueFromRawJsonData(valueNew);
			return *this;
		}

		JSONIFIER_ALWAYS_INLINE raw_json_data(const jsonifier::string& valueNew) noexcept {
			*this = valueNew;
		}

		JSONIFIER_ALWAYS_INLINE const raw_json_data& operator[](size_t index) const {
			return operator const array_type&()[index];
		}

		JSONIFIER_ALWAYS_INLINE const raw_json_data& operator[](jsonifier::string_view key) const {
			return operator const object_type&().at(key);
		}

		JSONIFIER_ALWAYS_INLINE const char* data() const noexcept {
			return jsonData.data();
		}

		JSONIFIER_ALWAYS_INLINE char* data() noexcept {
			return jsonData.data();
		}

		JSONIFIER_ALWAYS_INLINE json_type getType() const noexcept {
			if (std::holds_alternative<object_type>(value)) {
				return json_type::Object;
			} else if (std::holds_alternative<array_type>(value)) {
				return json_type::Array;
			} else if (std::holds_alternative<string_type>(value)) {
				return json_type::String;
			} else if (std::holds_alternative<double>(value)) {
				return json_type::Number;
			} else if (std::holds_alternative<bool>(value)) {
				return json_type::Bool;
			} else if (std::holds_alternative<std::nullptr_t>(value)) {
				return json_type::Null;
			} else {
				return json_type::Unset;
			}
		}

		JSONIFIER_ALWAYS_INLINE explicit operator const object_type&() const noexcept {
			if (std::holds_alternative<object_type>(value)) {
				return std::get<object_type>(value);
			} else {
				return {};
			}
		}

		JSONIFIER_ALWAYS_INLINE explicit operator const array_type&() const noexcept {
			if (std::holds_alternative<array_type>(value)) {
				return std::get<array_type>(value);
			} else {
				return {};
			}
		}

		JSONIFIER_ALWAYS_INLINE explicit operator const string&() const noexcept {
			if (std::holds_alternative<string_type>(value)) {
				return std::get<string_type>(value);
			} else {
				return {};
			}
		}

		JSONIFIER_ALWAYS_INLINE explicit operator const double&() const noexcept {
			if (std::holds_alternative<double>(value)) {
				return std::get<double>(value);
			} else {
				return {};
			}
		}

		JSONIFIER_ALWAYS_INLINE explicit operator const size_t&() const noexcept {
			if (std::holds_alternative<double>(value)) {
				return static_cast<size_t>(std::get<double>(value));
			} else {
				return {};
			}
		}

		JSONIFIER_ALWAYS_INLINE explicit operator const int64_t&() const noexcept {
			if (std::holds_alternative<double>(value)) {
				return static_cast<int64_t>(std::get<double>(value));
			} else {
				return {};
			}
		}

		JSONIFIER_ALWAYS_INLINE explicit operator const bool&() const noexcept {
			if (std::holds_alternative<bool>(value)) {
				return std::get<bool>(value);
			} else {
				return {};
			}
		}

		JSONIFIER_ALWAYS_INLINE jsonifier::string_view rawJson() const noexcept {
			return jsonData;
		}

		JSONIFIER_ALWAYS_INLINE bool operator==(const raw_json_data& other) const noexcept {
			return jsonData == other.jsonData && value == other.value;
		}

	  protected:
		value_type value{};
		string jsonData{};
	};

	JSONIFIER_ALWAYS_INLINE std::ostream& operator<<(std::ostream& os, raw_json_data& jsonValue) noexcept {
		os << jsonValue.rawJson();
		return os;
	}

}

namespace jsonifier_internal {

	JSONIFIER_ALWAYS_INLINE value_type constructValueFromRawJsonData(const jsonifier::string& jsonData) noexcept {
		static constexpr jsonifier::parse_options optionsNew{};
		jsonifier::jsonifier_core<false> parser{};
		if (jsonData.size() > 0) {
			switch (jsonData[0]) {
				case '{': {
					jsonifier::raw_json_data::object_type results{};
					parse_context<jsonifier::jsonifier_core<false>> testContext{};
					testContext.parserPtr = &parser;
					testContext.rootIter  = jsonData.data();
					testContext.endIter	  = jsonData.data() + jsonData.size();
					testContext.iter	  = jsonData.data();
					parse_impl<false, optionsNew, object_type, parse_context<jsonifier::jsonifier_core<false>>>::impl(results, testContext);
					return results;
				}
				case '[': {
					jsonifier::raw_json_data::array_type results{};
					parse_context<jsonifier::jsonifier_core<false>> testContext{};
					testContext.parserPtr = &parser;
					testContext.rootIter  = jsonData.data();
					testContext.endIter	  = jsonData.data() + jsonData.size();
					testContext.iter	  = jsonData.data();
					parse_impl<false, optionsNew, array_type, parse_context<jsonifier::jsonifier_core<false>>>::impl(results, testContext);
					return results;
				}
				case '"': {
					if (jsonData.size() > 1) {
						return jsonifier::string{ jsonData.data() + 1, jsonData.size() - 2 };
					} else {
						return std::nullptr_t{};
					}
				}
				case 't':
					return true;
				case 'f': {
					return false;
				}
				case '0':
					[[fallthrough]];
				case '1':
					[[fallthrough]];
				case '2':
					[[fallthrough]];
				case '3':
					[[fallthrough]];
				case '4':
					[[fallthrough]];
				case '5':
					[[fallthrough]];
				case '6':
					[[fallthrough]];
				case '7':
					[[fallthrough]];
				case '8':
					[[fallthrough]];
				case '9':
					[[fallthrough]];
				case '-': {
					return strToDouble(jsonData);
				}
				default: {
					return std::nullptr_t{};
				}
			}
		} else {
			return std::nullptr_t{};
		}
	}
}