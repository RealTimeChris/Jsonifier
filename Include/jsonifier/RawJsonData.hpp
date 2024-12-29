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

	class raw_json_data;
}

namespace jsonifier {

	struct json_number {
		json_number() noexcept = default;

		json_number(const string& stringNew) noexcept : rawJson{ stringNew } {};

		JSONIFIER_INLINE uint64_t getUint() {
			return strToUint64(rawJson);
		}

		JSONIFIER_INLINE int64_t getInt() {
			return strToInt64(rawJson);
		}

		JSONIFIER_INLINE double getDouble() {
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
		using array_type  = vector<raw_json_data>;
		using number_type = json_number;
		using bool_type	  = bool;
		using value_type  = std::variant<std::unique_ptr<object_type>, std::unique_ptr<array_type>, std::unique_ptr<string_type>, std::unique_ptr<number_type>,
			 std::unique_ptr<bool_type>, std::nullptr_t>;

		JSONIFIER_INLINE raw_json_data() noexcept {
			value = std::nullptr_t{};
		}

		template<typename parser_type> JSONIFIER_INLINE raw_json_data(parser_type& parser, const jsonifier::string& jsonDataNew) noexcept {
			value	 = constructValueFromRawJsonData(parser, jsonDataNew);
			jsonData = jsonDataNew;
		}

		JSONIFIER_INLINE json_type getType() const noexcept {
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

		[[nodiscard]] JSONIFIER_INLINE bool& getBool() noexcept {
			return *std::get<std::unique_ptr<bool>>(value);
		}

		[[nodiscard]] JSONIFIER_INLINE double getDouble() noexcept {
			return std::get<std::unique_ptr<number_type>>(value)->getDouble();
		}

		[[nodiscard]] JSONIFIER_INLINE int64_t getInt() noexcept {
			return std::get<std::unique_ptr<number_type>>(value)->getInt();
		}

		[[nodiscard]] JSONIFIER_INLINE uint64_t getUint() noexcept {
			return std::get<std::unique_ptr<number_type>>(value)->getUint();
		}

		[[nodiscard]] JSONIFIER_INLINE string& getString() noexcept {
			return *std::get<std::unique_ptr<string>>(value);
		}

		[[nodiscard]] JSONIFIER_INLINE object_type& getObject() noexcept {
			return *std::get<std::unique_ptr<object_type>>(value);
		}

		[[nodiscard]] JSONIFIER_INLINE array_type& getArray() noexcept {
			return *std::get<std::unique_ptr<array_type>>(value);
		}

		template<std::integral index_type> JSONIFIER_INLINE raw_json_data& operator[](index_type&& index) noexcept {
			return (*std::get<std::unique_ptr<array_type>>(value))[index];
		}

		template<std::integral index_type> JSONIFIER_INLINE const raw_json_data& operator[](index_type&& index) const noexcept {
			return (*std::get<std::unique_ptr<array_type>>(value))[index];
		}

		template<std::convertible_to<std::string_view> key_type> JSONIFIER_INLINE raw_json_data& operator[](key_type&& key) noexcept {
			if (std::holds_alternative<std::nullptr_t>(value)) {
				value = std::make_unique<object_type>();
			}
			auto& object = *std::get<std::unique_ptr<object_type>>(value);
			return object[key];
		}

		template<std::convertible_to<const std::string_view> key_type> JSONIFIER_INLINE const raw_json_data& operator[](key_type&& key) const noexcept {
			const auto& object = *std::get<std::unique_ptr<object_type>>(value);
			return object.at(key);
		}

		template<std::convertible_to<std::string_view> key_type> [[nodiscard]] JSONIFIER_INLINE bool contains(key_type&& key) const noexcept {
			if (!std::holds_alternative<std::unique_ptr<object_type>>(value)) {
				return false;
			}
			const auto& object = *std::get<std::unique_ptr<object_type>>(value);
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
		JSONIFIER_INLINE auto constructValueFromRawJsonData(parser_type& parser, const jsonifier::string& jsonDataNew) noexcept {
			static constexpr jsonifier::parse_options optionsNew{};
			if (jsonDataNew.size() > 0) {
				switch (jsonDataNew[0]) {
					case '{': {
						std::unique_ptr<typename jsonifier::raw_json_data::object_type> results{ std::make_unique<typename jsonifier::raw_json_data::object_type>() };
						jsonifier_internal::parse_context<typename parser_type::derived_type, const char*> testContext{};
						testContext.parserPtr = &parser;
						testContext.rootIter  = jsonDataNew.data();
						testContext.endIter	  = jsonDataNew.data() + jsonDataNew.size();
						testContext.iter	  = jsonDataNew.data();
						jsonifier_internal::object_val_parser<std::string, jsonifier_internal::parse_context<typename parser_type::derived_type, const char*>, optionsNew,
							false>::impl(*results, testContext);
						return results;
					}
					case '[': {
						std::unique_ptr<typename jsonifier::raw_json_data::array_type> results{ std::make_unique<typename jsonifier::raw_json_data::array_type>() };
						jsonifier_internal::parse_context<typename parser_type::derived_type, const char*> testContext{};
						testContext.parserPtr = &parser;
						testContext.rootIter  = jsonDataNew.data();
						testContext.endIter	  = jsonDataNew.data() + jsonDataNew.size();
						testContext.iter	  = jsonDataNew.data();
						jsonifier_internal::array_val_parser<std::string, jsonifier_internal::parse_context<typename parser_type::derived_type, const char*>, optionsNew,
							false>::impl(*results, testContext);
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

	JSONIFIER_INLINE std::ostream& operator<<(std::ostream& os, const raw_json_data& jsonValue) noexcept {
		os << jsonValue.rawJson();
		return os;
	}
}