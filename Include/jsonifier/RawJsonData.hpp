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

namespace jsonifier {

	class raw_json_data;

	struct json_number {
		json_number() noexcept = default;

		json_number(const string& stringNew) noexcept : rawJson{ stringNew } {};

		uint64_t getUint() const {
			return strToUint64(rawJson);
		}

		int64_t getInt() const {
			return strToInt64(rawJson);
		}

		double getDouble() const {
			return strToDouble(rawJson);
		}

		friend bool operator!=(const json_number& lhs, const json_number& rhs) {
			return lhs.rawJson != rhs.rawJson;
		}

		friend bool operator==(const json_number& lhs, const json_number& rhs) {
			return lhs.rawJson == rhs.rawJson;
		}

		string rawJson{};
	};

	class raw_json_data {
	  public:
		using object_type = std::unordered_map<string, raw_json_data>;
		using string_type = string;
		using array_type  = vector<raw_json_data>;
		using number_type = json_number;
		using bool_type	  = bool;
		using value_type  = std::variant<std::unique_ptr<object_type>, std::unique_ptr<array_type>, std::unique_ptr<string_type>, std::unique_ptr<number_type>,
			 std::unique_ptr<bool_type>, std::nullptr_t>;

		raw_json_data() noexcept {
			value = std::nullptr_t{};
		}

		template<typename parser_type> raw_json_data(parser_type& parser, const string& jsonDataNew) noexcept {
			value	 = constructValueFromRawJsonData(parser, jsonDataNew);
			jsonData = jsonDataNew;
		}

		json_type getType() const noexcept {
			if (std::holds_alternative<std::unique_ptr<object_type>>(value)) {
				return json_type::object;
			} else if (std::holds_alternative<std::unique_ptr<array_type>>(value)) {
				return json_type::array;
			} else if (std::holds_alternative<std::unique_ptr<string_type>>(value)) {
				return json_type::string;
			} else if (std::holds_alternative<std::unique_ptr<number_type>>(value)) {
				return json_type::number;
			} else if (std::holds_alternative<std::unique_ptr<bool_type>>(value)) {
				return json_type::boolean;
			} else if (std::holds_alternative<std::nullptr_t>(value)) {
				return json_type::null;
			} else {
				return json_type::unset;
			}
		}

		const object_type& getObject() const noexcept {
			return *std::get<std::unique_ptr<object_type>>(value);
		}

		object_type& getObject() noexcept {
			return *std::get<std::unique_ptr<object_type>>(value);
		}

		const array_type& getArray() const noexcept {
			return *std::get<std::unique_ptr<array_type>>(value);
		}

		array_type& getArray() noexcept {
			return *std::get<std::unique_ptr<array_type>>(value);
		}

		const string& getString() const noexcept {
			return *std::get<std::unique_ptr<string>>(value);
		}

		string& getString() noexcept {
			return *std::get<std::unique_ptr<string>>(value);
		}

		const number_type& getNumber() const noexcept {
			return *std::get<std::unique_ptr<number_type>>(value);
		}

		number_type& getNumber() noexcept {
			return *std::get<std::unique_ptr<number_type>>(value);
		}

		double getDouble() const noexcept {
			return std::get<std::unique_ptr<number_type>>(value)->getDouble();
		}

		int64_t getInt() const noexcept {
			return std::get<std::unique_ptr<number_type>>(value)->getInt();
		}

		uint64_t getUint() const noexcept {
			return std::get<std::unique_ptr<number_type>>(value)->getUint();
		}

		const bool_type& getBool() const noexcept {
			return *std::get<std::unique_ptr<bool_type>>(value);
		}

		bool_type& getBool() noexcept {
			return *std::get<std::unique_ptr<bool_type>>(value);
		}

		template<std::integral index_type> raw_json_data& operator[](index_type&& index) noexcept {
			return (*std::get<std::unique_ptr<array_type>>(value))[index];
		}

		template<std::integral index_type> const raw_json_data& operator[](index_type&& index) const noexcept {
			return (*std::get<std::unique_ptr<array_type>>(value))[index];
		}

		template<std::convertible_to<string_view> key_type> raw_json_data& operator[](key_type&& key) noexcept {
			if (std::holds_alternative<std::nullptr_t>(value)) {
				value = std::make_unique<object_type>();
			}
			auto& object = *std::get<std::unique_ptr<object_type>>(value);
			return object[key];
		}

		template<std::convertible_to<const string_view> key_type> const raw_json_data& operator[](key_type&& key) const noexcept {
			const auto& object = *std::get<std::unique_ptr<object_type>>(value);
			return object.at(key);
		}

		template<std::convertible_to<string_view> key_type> bool contains(key_type&& key) const noexcept {
			if (!std::holds_alternative<std::unique_ptr<object_type>>(value)) {
				return false;
			}
			const auto& object = *std::get<std::unique_ptr<object_type>>(value);
			return object.contains(key);
		}

		size_t size() const noexcept {
			if (std::holds_alternative<std::unique_ptr<object_type>>(value)) {
				return std::get<std::unique_ptr<object_type>>(value).get()->size();
			} else if (std::holds_alternative<std::unique_ptr<array_type>>(value)) {
				return std::get<std::unique_ptr<array_type>>(value).get()->size();
			} else if (std::holds_alternative<std::unique_ptr<string_type>>(value)) {
				return std::get<std::unique_ptr<string_type>>(value).get()->size();
			} else {
				return 0;
			}
		}

		string_view rawJson() const noexcept {
			return jsonData;
		}

		bool operator==(const raw_json_data& other) const noexcept {
			return jsonData == other.jsonData && value == other.value;
		}

	  protected:
		value_type value{};
		string jsonData{};

		template<typename parser_type> auto constructValueFromRawJsonData(parser_type& parser, const string& jsonDataNew) noexcept {
			static constexpr parse_options optionsNew{};
			if (jsonDataNew.size() > 0) {
				switch (jsonDataNew[0]) {
					case '{': {
						std::unique_ptr<typename raw_json_data::object_type> results{ std::make_unique<typename raw_json_data::object_type>() };
						internal::parse_context<typename parser_type::derived_type, string_view_ptr> testContext{};
						testContext.parserPtr = &parser;
						testContext.rootIter  = jsonDataNew.data();
						testContext.endIter	  = jsonDataNew.data() + jsonDataNew.size();
						testContext.iter	  = jsonDataNew.data();
						internal::object_val_parser<string, internal::parse_context<typename parser_type::derived_type, string_view_ptr>, optionsNew, false>::impl(*results,
							testContext);
						return results;
					}
					case '[': {
						std::unique_ptr<typename raw_json_data::array_type> results{ std::make_unique<typename raw_json_data::array_type>() };
						internal::parse_context<typename parser_type::derived_type, string_view_ptr> testContext{};
						testContext.parserPtr = &parser;
						testContext.rootIter  = jsonDataNew.data();
						testContext.endIter	  = jsonDataNew.data() + jsonDataNew.size();
						testContext.iter	  = jsonDataNew.data();
						internal::array_val_parser<string, internal::parse_context<typename parser_type::derived_type, string_view_ptr>, optionsNew, false>::impl(*results,
							testContext);
						return results;
					}
					case '"': {
						if (jsonDataNew.size() > 1) {
							return std::make_unique<string_type>(jsonDataNew.data() + 1, jsonDataNew.size() - 2);
						} else {
							return std::nullptr_t{};
						}
					}
					case 't':
						return std::make_unique<bool_type>(true);
					case 'f': {
						return std::make_unique<bool_type>(false);
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
						return std::make_unique<number_type>(jsonDataNew);
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

	std::ostream& operator<<(std::ostream& os, const raw_json_data& jsonValue) noexcept {
		os << jsonValue.rawJson();
		return os;
	}
}