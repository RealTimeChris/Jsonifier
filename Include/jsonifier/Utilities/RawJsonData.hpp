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

#include <jsonifier/Utilities/HashMap.hpp>
#include <jsonifier/Utilities/String.hpp>

namespace jsonifier {

	class raw_json_data;

	struct json_number {
		JSONIFIER_INLINE json_number() noexcept = default;

		JSONIFIER_INLINE json_number(const string& stringNew) noexcept : rawJson{ stringNew } {};

		JSONIFIER_INLINE uint64_t getUint() const {
			return strToUint64(rawJson);
		}

		JSONIFIER_INLINE int64_t getInt() const {
			return strToInt64(rawJson);
		}

		JSONIFIER_INLINE double getDouble() const {
			return strToDouble(rawJson);
		}

		JSONIFIER_INLINE friend bool operator!=(const json_number& lhs, const json_number& rhs) {
			return lhs.rawJson != rhs.rawJson;
		}

		JSONIFIER_INLINE friend bool operator==(const json_number& lhs, const json_number& rhs) {
			return lhs.rawJson == rhs.rawJson;
		}

		string rawJson{};
	};

	class raw_json_data {
	  public:
		using object_type = std::unordered_map<string, raw_json_data>;
		using string_type = string;
		using array_type  = std::vector<raw_json_data>;
		using number_type = json_number;
		using bool_type	  = bool;
		using null_type	  = std::nullptr_t;
		using error_type  = std::monostate;
		using value_type  = std::variant<object_type, array_type, string_type, number_type, bool_type, null_type, error_type>;

		JSONIFIER_INLINE raw_json_data() noexcept {
			value = null_type{};
		}

		template<typename parser_type> JSONIFIER_INLINE raw_json_data(parser_type& parser, const string& jsonDataNew) noexcept {
			value	 = constructValueFromRawJsonData(parser, jsonDataNew);
			jsonData = jsonDataNew;
		}

		JSONIFIER_INLINE json_type getType() const noexcept {
			if (std::holds_alternative<object_type>(value)) {
				return json_type::object;
			} else if (std::holds_alternative<array_type>(value)) {
				return json_type::array;
			} else if (std::holds_alternative<string_type>(value)) {
				return json_type::string;
			} else if (std::holds_alternative<number_type>(value)) {
				return json_type::number;
			} else if (std::holds_alternative<bool_type>(value)) {
				return json_type::boolean;
			} else if (std::holds_alternative<null_type>(value)) {
				return json_type::null;
			} else {
				return json_type::unset;
			}
		}

		JSONIFIER_INLINE const object_type& getObject() const noexcept {
			return std::get<object_type>(value);
		}

		JSONIFIER_INLINE object_type& getObject() noexcept {
			return std::get<object_type>(value);
		}

		JSONIFIER_INLINE const array_type& getArray() const noexcept {
			return std::get<array_type>(value);
		}

		JSONIFIER_INLINE array_type& getArray() noexcept {
			return std::get<array_type>(value);
		}

		JSONIFIER_INLINE const string_type& getString() const noexcept {
			return std::get<string_type>(value);
		}

		JSONIFIER_INLINE string_type& getString() noexcept {
			return std::get<string_type>(value);
		}

		JSONIFIER_INLINE const number_type& getNumber() const noexcept {
			return std::get<number_type>(value);
		}

		JSONIFIER_INLINE number_type& getNumber() noexcept {
			return std::get<number_type>(value);
		}

		JSONIFIER_INLINE double getDouble() const noexcept {
			return std::get<number_type>(value).getDouble();
		}

		JSONIFIER_INLINE int64_t getInt() const noexcept {
			return std::get<number_type>(value).getInt();
		}

		JSONIFIER_INLINE uint64_t getUint() const noexcept {
			return std::get<number_type>(value).getUint();
		}

		JSONIFIER_INLINE const bool_type& getBool() const noexcept {
			return std::get<bool_type>(value);
		}

		JSONIFIER_INLINE bool_type& getBool() noexcept {
			return std::get<bool_type>(value);
		}

		template<std::integral index_type> JSONIFIER_INLINE raw_json_data& operator[](index_type&& index) noexcept {
			return (std::get<array_type>(value))[index];
		}

		template<std::integral index_type> JSONIFIER_INLINE const raw_json_data& operator[](index_type&& index) const noexcept {
			return (std::get<array_type>(value))[index];
		}

		template<std::convertible_to<string_view> key_type> JSONIFIER_INLINE raw_json_data& operator[](key_type&& key) noexcept {
			if (std::holds_alternative<null_type>(value)) {
				value = object_type{};
			}
			auto& object = std::get<object_type>(value);
			return object[key];
		}

		template<std::convertible_to<const string_view> key_type> JSONIFIER_INLINE const raw_json_data& operator[](key_type&& key) const noexcept {
			const auto& object = std::get<object_type>(value);
			return object.at(key);
		}

		template<std::convertible_to<string_view> key_type> JSONIFIER_INLINE bool contains(key_type&& key) const noexcept {
			if (!std::holds_alternative<object_type>(value)) {
				return false;
			}
			const auto& object = std::get<object_type>(value);
			return object.contains(key);
		}

		JSONIFIER_INLINE size_t size() const noexcept {
			if (std::holds_alternative<object_type>(value)) {
				return std::get<object_type>(value).size();
			} else if (std::holds_alternative<array_type>(value)) {
				return std::get<array_type>(value).size();
			} else if (std::holds_alternative<string_type>(value)) {
				return std::get<string_type>(value).size();
			} else {
				return 0;
			}
		}

		JSONIFIER_INLINE string_view rawJson() const noexcept {
			return jsonData;
		}

		JSONIFIER_INLINE const std::vector<internal::error>& getErrors() const noexcept {
			return errors;
		}

		JSONIFIER_INLINE bool operator==(const raw_json_data& other) const noexcept {
			return jsonData == other.jsonData && value == other.value;
		}

	  protected:
		std::vector<internal::error> errors{};
		value_type value{};
		string jsonData{};

		template<typename parser_type> JSONIFIER_INLINE auto constructValueFromRawJsonData(parser_type& parser, const string& jsonDataNew) noexcept {
			static constexpr parse_options optionsNew{};
			if (jsonDataNew.size() > 0) {
				switch (jsonDataNew[0]) {
					case '{': {
						typename raw_json_data::object_type results{};
						internal::parse_context<typename parser_type::derived_type, string_view_ptr> context{};
						context.parserPtr = &parser;
						context.rootIter  = jsonDataNew.data();
						context.endIter	  = jsonDataNew.data() + jsonDataNew.size();
						context.iter	  = jsonDataNew.data();
						internal::parse_impl<typename raw_json_data::object_type, internal::parse_context<typename parser_type::derived_type, string_view_ptr>, optionsNew,
							false>::impl(results, context);
						if (parser.derivedRef.getErrors().size() == 0) {
							return value_type{ results };
						} else {
							return value_type{ null_type{} };
						}
					}
					case '[': {
						typename raw_json_data::array_type results{};
						internal::parse_context<typename parser_type::derived_type, string_view_ptr> context{};
						context.parserPtr = &parser;
						context.rootIter  = jsonDataNew.data();
						context.endIter	  = jsonDataNew.data() + jsonDataNew.size();
						context.iter	  = jsonDataNew.data();
						internal::parse_impl<typename raw_json_data::array_type, internal::parse_context<typename parser_type::derived_type, string_view_ptr>, optionsNew,
							false>::impl(results, context);
						if (parser.derivedRef.getErrors().size() == 0) {
							return value_type{ results };
						} else {
							return value_type{ null_type{} };
						}
					}
					case '"': {
						typename raw_json_data::string_type results{};
						internal::parse_context<typename parser_type::derived_type, string_view_ptr> context{};
						context.parserPtr = &parser;
						context.rootIter  = jsonDataNew.data();
						context.endIter	  = jsonDataNew.data() + jsonDataNew.size();
						context.iter	  = jsonDataNew.data();
						internal::parse_impl<typename raw_json_data::string_type, internal::parse_context<typename parser_type::derived_type, string_view_ptr>, optionsNew,
							false>::impl(results, context);
						if (parser.derivedRef.getErrors().size() == 0) {
							return value_type{ results };
						} else {
							return value_type{ null_type{} };
						}
					}
					case 't':
						return value_type{ bool_type{ true } };
					case 'f': {
						return value_type{ bool_type{ false } };
					}
					case 'n': {
						return value_type{ null_type{} };
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
						return value_type{ number_type{ jsonDataNew } };
					}
					default: {
						return value_type{ null_type{} };
					}
				}
			} else {
				return value_type{ null_type{} };
			}
		}
	};

	std::ostream& operator<<(std::ostream& os, const raw_json_data& jsonValue) noexcept {
		os << jsonValue.rawJson();
		return os;
	}
}