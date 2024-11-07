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

	struct unset {
		JSONIFIER_ALWAYS_INLINE bool operator==(const unset&) const {
			return true;
		}
	};

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
	using value_type  = std::variant<object_type, array_type, string_type, double, bool, std::nullptr_t, jsonifier::unset>;

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
		template<typename value_type> friend value_type constructValueFromRawJsonData(const string& newData);
		using object_type = std::unordered_map<string, raw_json_data>;
		using string_type = string;
		using array_type  = vector<raw_json_data>;
		using value_type  = std::variant<object_type, array_type, string_type, double, bool, std::nullptr_t, unset>;

		JSONIFIER_ALWAYS_INLINE raw_json_data() {
			value = unset{};
		};

		JSONIFIER_ALWAYS_INLINE raw_json_data& operator=(const string& valueNew) noexcept {
			value	 = constructValueFromRawJsonData(valueNew);
			jsonData = valueNew;
			return *this;
		}

		JSONIFIER_ALWAYS_INLINE raw_json_data(const string& valueNew) noexcept {
			*this = valueNew;
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

		template<class value_type> [[nodiscard]] value_type& get() {
			return std::get<value_type>(value);
		}

		template<class value_type> [[nodiscard]] const value_type& get() const {
			return std::get<value_type>(value);
		}

		template<std::integral index_type> raw_json_data& operator[](index_type&& index) {
			return std::get<array_type>(value)[index];
		}

		template<std::integral index_type> const raw_json_data& operator[](index_type&& index) const {
			return std::get<array_type>(value)[index];
		}

		template<std::convertible_to<std::string_view> key_type> raw_json_data& operator[](key_type&& key) {
			if (std::holds_alternative<std::nullptr_t>(value)) {
				value = object_type{};
			}
			auto& object = std::get<object_type>(value);
			auto iter	 = object.find(static_cast<string_type>(key));
			if (iter == object.end()) {
				iter = object.insert(std::make_pair(static_cast<string_type>(key), raw_json_data{})).first;
			}
			return iter->second;
		}

		template<std::convertible_to<std::string_view> key_type> const raw_json_data& operator[](key_type&& key) const {
			auto& object = std::get<object_type>(value);
			auto iter	 = object.find(static_cast<string_type>(key));
			if (iter == object.end()) {
			}
			return iter->second;
		}

		template<std::convertible_to<std::string_view> key_type> [[nodiscard]] bool contains(key_type&& key) const {
			if (!std::holds_alternative<object_type>(value)) {
				return false;
			}
			auto& object = std::get<object_type>(value);
			auto iter	 = object.find(static_cast<string_type>(key));
			return iter != object.end();
		}

		JSONIFIER_ALWAYS_INLINE string_view rawJson() const noexcept {
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
				case 'n': {
					return std::nullptr_t{};
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
					return jsonifier::unset{};
				}
			}
		} else {
			return jsonifier::unset{};
		}
	}
}