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
		JSONIFIER_INLINE uint64_t operator()(const string_type& string) const noexcept {
			return std::hash<std::string_view>::operator()(std::string_view{ string });
		}
	};
}

namespace jsonifier {

	enum class json_token_type : uint8_t {
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

namespace jsonifier {

	class raw_json_data {
	  public:
		using object_type = std::unordered_map<string, raw_json_data>;
		using string_type = string;
		using array_type  = vector<raw_json_data>;
		using value_type  = std::variant<object_type, array_type, string_type, double, bool, std::nullptr_t>;

		JSONIFIER_INLINE raw_json_data() noexcept {
			value = std::nullptr_t{};
		}

		template<typename parser_type> JSONIFIER_INLINE raw_json_data(parser_type& parser, const jsonifier::string& jsonDataNew) noexcept {
			value	 = constructValueFromRawJsonData(parser, jsonDataNew);
			jsonData = jsonDataNew;
		}

		JSONIFIER_INLINE json_token_type getType() const noexcept {
			if (std::holds_alternative<object_type>(value)) {
				return json_token_type::Object;
			} else if (std::holds_alternative<array_type>(value)) {
				return json_token_type::Array;
			} else if (std::holds_alternative<string_type>(value)) {
				return json_token_type::String;
			} else if (std::holds_alternative<double>(value)) {
				return json_token_type::Number;
			} else if (std::holds_alternative<bool>(value)) {
				return json_token_type::Bool;
			} else if (std::holds_alternative<std::nullptr_t>(value)) {
				return json_token_type::Null;
			} else {
				return json_token_type::Unset;
			}
		}

		template<typename value_type> [[nodiscard]] JSONIFIER_INLINE value_type& get() noexcept {
			return std::get<value_type>(value);
		}

		template<typename value_type> [[nodiscard]] JSONIFIER_INLINE const value_type& get() const noexcept {
			return std::get<value_type>(value);
		}

		[[nodiscard]] JSONIFIER_INLINE bool& getBool() noexcept {
			return std::get<bool>(value);
		}

		[[nodiscard]] JSONIFIER_INLINE double& getDouble() noexcept {
			return std::get<double>(value);
		}

		[[nodiscard]] JSONIFIER_INLINE int64_t getInt() noexcept {
			return static_cast<int64_t>(std::get<double>(value));
		}

		[[nodiscard]] JSONIFIER_INLINE uint64_t getUint() noexcept {
			return static_cast<uint64_t>(std::get<double>(value));
		}

		[[nodiscard]] JSONIFIER_INLINE string& getString() noexcept {
			return std::get<string>(value);
		}

		[[nodiscard]] JSONIFIER_INLINE object_type& getObject() noexcept {
			return std::get<object_type>(value);
		}

		[[nodiscard]] JSONIFIER_INLINE array_type& getArray() noexcept {
			return std::get<array_type>(value);
		}

		template<jsonifier::concepts::integer_t value_type> [[nodiscard]] JSONIFIER_INLINE value_type get() noexcept {
			return static_cast<value_type>(std::get<double>(value));
		}

		template<jsonifier::concepts::integer_t value_type> [[nodiscard]] JSONIFIER_INLINE const value_type& get() const noexcept {
			return static_cast<value_type>(std::get<double>(value));
		}

		template<jsonifier::concepts::string_t value_type> [[nodiscard]] JSONIFIER_INLINE value_type get() noexcept {
			return static_cast<value_type>(std::get<string>(value));
		}

		template<jsonifier::concepts::string_t value_type> [[nodiscard]] JSONIFIER_INLINE const value_type& get() const noexcept {
			return static_cast<value_type>(std::get<string>(value));
		}

		template<std::integral index_type> JSONIFIER_INLINE raw_json_data& operator[](index_type&& index) noexcept {
			return std::get<array_type>(value)[index];
		}

		template<std::integral index_type> JSONIFIER_INLINE const raw_json_data& operator[](index_type&& index) const noexcept {
			return std::get<array_type>(value)[index];
		}

		template<std::convertible_to<std::string_view> key_type> JSONIFIER_INLINE raw_json_data& operator[](key_type&& key) noexcept {
			if (std::holds_alternative<std::nullptr_t>(value)) {
				value = object_type{};
			}
			auto& object = std::get<object_type>(value);
			return object[key];
		}

		template<std::convertible_to<const std::string_view> key_type> JSONIFIER_INLINE const raw_json_data& operator[](key_type&& key) const noexcept {
			const auto& object = std::get<object_type>(value);
			return object.at(key);
		}

		template<std::convertible_to<std::string_view> key_type> [[nodiscard]] JSONIFIER_INLINE bool contains(key_type&& key) const noexcept {
			if (!std::holds_alternative<object_type>(value)) {
				return false;
			}
			const auto& object = std::get<object_type>(value);
			return object.contains(key);
		}

		JSONIFIER_INLINE string_view rawJson() const noexcept {
			return jsonData;
		}

		JSONIFIER_INLINE bool operator==(const raw_json_data& other) const noexcept {
			return jsonData == other.jsonData && value == other.value;
		}

	  protected:
		value_type value{};
		string jsonData{};

		template<typename parser_type>
		JSONIFIER_INLINE typename jsonifier::raw_json_data::value_type constructValueFromRawJsonData(parser_type& parser, const jsonifier::string& jsonDataNew) noexcept {
			static constexpr jsonifier::parse_options optionsNew{};
			if (jsonDataNew.size() > 0) {
				switch (jsonDataNew[0]) {
					case '{': {
						typename jsonifier::raw_json_data::object_type results{};
						jsonifier_internal::parse_context<typename parser_type::derived_type, const char*> testContext{};
						testContext.parserPtr = &parser;
						testContext.rootIter  = jsonDataNew.data();
						testContext.endIter	  = jsonDataNew.data() + jsonDataNew.size();
						testContext.iter	  = jsonDataNew.data();
						jsonifier_internal::object_val_parser<std::string, jsonifier_internal::parse_context<typename parser_type::derived_type, const char*>, optionsNew,
							false>::impl(results, testContext);
						return results;
					}
					case '[': {
						typename jsonifier::raw_json_data::array_type results{};
						jsonifier_internal::parse_context<typename parser_type::derived_type, const char*> testContext{};
						testContext.parserPtr = &parser;
						testContext.rootIter  = jsonDataNew.data();
						testContext.endIter	  = jsonDataNew.data() + jsonDataNew.size();
						testContext.iter	  = jsonDataNew.data();
						jsonifier_internal::array_val_parser<std::string, jsonifier_internal::parse_context<typename parser_type::derived_type, const char*>, optionsNew,
							false>::impl(results, testContext);
						return results;
					}
					case '"': {
						if (jsonDataNew.size() > 1) {
							return jsonifier::string{ jsonDataNew.data() + 1, jsonDataNew.size() - 2 };
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
						return strToDouble(jsonDataNew);
					}
					default: {
						return std::nullptr_t{};
					}
				}
			} else {
				return std::nullptr_t{};
			}
		}
	};

	JSONIFIER_INLINE std::ostream& operator<<(std::ostream& os, const raw_json_data& jsonValue) noexcept {
		os << jsonValue.rawJson();
		return os;
	}
}