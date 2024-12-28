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
#include <jsonifier/JsonEntity.hpp>
#include <jsonifier/String.hpp>

namespace std {

	template<jsonifier::concepts::string_t string_type> struct hash<string_type> : public std::hash<std::string_view> {
		JSONIFIER_INLINE uint64_t operator()(const string_type& string) const noexcept {
			return std::hash<std::string_view>::operator()(std::string_view{ string });
		}
	};
}

namespace jsonifier {

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

		JSONIFIER_INLINE operator uint64_t() const {
			return strToUint64(rawJson);
		}

		JSONIFIER_INLINE operator int64_t() const {
			return strToInt64(rawJson);
		}

		JSONIFIER_INLINE operator double() const {
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
		using array_type  = vector<raw_json_data>;
		using string_type = string;
		using number_type = json_number;
		using bool_type	  = bool;
		using null_type	  = std::nullptr_t;
		using value_type  = std::variant<std::monostate, object_type, array_type, string_type, number_type, bool_type, null_type>;

		JSONIFIER_INLINE raw_json_data() noexcept {
		}

		JSONIFIER_INLINE raw_json_data& operator=(raw_json_data&& other) noexcept {
			jsonData.swap(other.jsonData);
			value.swap(other.value);
			return *this;
		}

		JSONIFIER_INLINE raw_json_data(raw_json_data&& other) noexcept {
			*this = jsonifier_internal::move(other);
		}

		JSONIFIER_INLINE raw_json_data& operator=(const raw_json_data& other) noexcept {
			jsonData = other.jsonData;
			switch (other.getType()) {
				case json_type::object: {
					value.emplace<object_type>(other.getObject());
					break;
				}
				case json_type::array: {
					value.emplace<array_type>(other.getArray());
					break;
				}
				case json_type::string: {
					value.emplace<string_type>(other.getString());
					break;
				}
				case json_type::number: {
					value.emplace<number_type>(other.getNumber());
					break;
				}
				case json_type::boolean: {
					value.emplace<bool_type>(other.getBool());
					break;
				}
				case json_type::null: {
					value.emplace<null_type>();
					break;
				}
				case json_type::accessor: {
					break;
				}
				case json_type::custom: {
					break;
				}
				case json_type::unset: {
					break;
				}
			}
			return *this;
		}

		JSONIFIER_INLINE raw_json_data(const raw_json_data& other) {
			*this = other;
		}

		template<typename parser_type> JSONIFIER_INLINE raw_json_data(parser_type& parser, const jsonifier::string& jsonDataNew) noexcept {
			constructValueFromRawJsonData(parser, jsonDataNew);
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

		JSONIFIER_INLINE const bool_type& getBool() const noexcept {
			return std::get<bool_type>(value);
		}

		JSONIFIER_INLINE bool_type& getBool() noexcept {
			return std::get<bool_type>(value);
		}

		JSONIFIER_INLINE double getDouble() const noexcept {
			return static_cast<double>(static_cast<const number_type&>(std::get<number_type>(value)));
		}

		JSONIFIER_INLINE int64_t getInt() const noexcept {
			return static_cast<int64_t>(static_cast<const number_type&>(std::get<number_type>(value)));
		}

		JSONIFIER_INLINE uint64_t getUint() const noexcept {
			return static_cast<uint64_t>(static_cast<const number_type&>(std::get<number_type>(value)));
		}

		JSONIFIER_INLINE const number_type& getNumber() const noexcept {
			return std::get<number_type>(value);
		}

		JSONIFIER_INLINE number_type& getNumber() noexcept {
			return std::get<number_type>(value);
		}

		JSONIFIER_INLINE const string_type& getString() const noexcept {
			return std::get<string_type>(value);
		}

		JSONIFIER_INLINE string_type& getString() noexcept {
			return std::get<string_type>(value);
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

		template<std::integral index_type> JSONIFIER_INLINE const raw_json_data& operator[](index_type&& index) const noexcept {
			return (std::get<array_type>(value))[index];
		}

		template<std::integral index_type> JSONIFIER_INLINE raw_json_data& operator[](index_type&& index) noexcept {
			return (std::get<array_type>(value))[index];
		}

		template<std::convertible_to<const std::string_view> key_type> JSONIFIER_INLINE const raw_json_data& operator[](key_type&& key) const noexcept {
			const object_type& object = static_cast<const object_type&>(std::get<object_type>(value));
			return object.at(key);
		}

		template<std::convertible_to<std::string_view> key_type> JSONIFIER_INLINE raw_json_data& operator[](key_type&& key) noexcept {
			if (std::holds_alternative<null_type>(value)) {
				value.emplace<object_type>();
			}
			object_type& object = std::get<object_type>(value);
			return object[key];
		}

		template<std::convertible_to<std::string_view> key_type> JSONIFIER_INLINE bool contains(key_type&& key) const noexcept {
			if (!std::holds_alternative<object_type>(value)) {
				return false;
			}
			const object_type& object = std::get<object_type>(value);
			return object.contains(key);
		}

		JSONIFIER_INLINE string_view rawJson() const noexcept {
			return jsonData;
		}

		JSONIFIER_INLINE bool operator==(const raw_json_data& other) const noexcept {
			return jsonData == other.jsonData && value == other.value;
		}

		JSONIFIER_INLINE ~raw_json_data(){};

	  protected:
		value_type value{};
		string jsonData{};

		template<typename parser_type> JSONIFIER_INLINE void constructValueFromRawJsonData(parser_type& parser, const jsonifier::string& jsonDataNew) noexcept {
			static constexpr jsonifier::parse_options optionsNew{};
			if (jsonDataNew.size() > 0) {
				switch (jsonDataNew[0]) {
					case '{': {
						object_type results{};
						jsonifier_internal::parse_context<typename parser_type::derived_type, const char*, std::string> testContext{};
						testContext.parserPtr = &parser;
						testContext.rootIter  = jsonDataNew.data();
						testContext.endIter	  = jsonDataNew.data() + jsonDataNew.size();
						testContext.iter	  = jsonDataNew.data();
						jsonifier_internal::object_val_parser<object_type, jsonifier_internal::parse_context<typename parser_type::derived_type, const char*, std::string>,
							optionsNew, jsonifier_internal::getParseValue<std::remove_cvref_t<object_type>>(), 0, jsonifier_internal::maxFieldDepth<object_type>(),
							false>::impl(results, testContext);
						value.emplace<object_type>(jsonifier_internal::move(results));
						return;
					}
					case '[': {
						array_type results{};
						jsonifier_internal::parse_context<typename parser_type::derived_type, const char*, std::string> testContext{};
						testContext.parserPtr = &parser;
						testContext.rootIter  = jsonDataNew.data();
						testContext.endIter	  = jsonDataNew.data() + jsonDataNew.size();
						testContext.iter	  = jsonDataNew.data();
						jsonifier_internal::array_val_parser<array_type, jsonifier_internal::parse_context<typename parser_type::derived_type, const char*, std::string>,
							optionsNew, jsonifier_internal::getParseValue<std::remove_cvref_t<object_type>>(), 0, jsonifier_internal::maxFieldDepth<array_type>(),
							false>::impl(results, testContext);
						value.emplace<array_type>(jsonifier_internal::move(results));
						return;
					}
					case '"': {
						if (jsonDataNew.size() > 1) {
							value.emplace<string_type>(string_type{ jsonDataNew.data() + 1, jsonDataNew.size() - 2 });
							return;
						} else {
							value.emplace<null_type>();
							return;
						}
					}
					case 't': {
						value.emplace<bool_type>(true);
						return;
					}
					case 'f': {
						value.emplace<bool_type>(false);
						return;
					}
					case 'n': {
						value.emplace<null_type>();
						return;
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
						value.emplace<number_type>(number_type{ jsonDataNew });
						return;
					}
					default: {
						value.emplace<null_type>();
						return;
					}
				}
			} else {
				value.emplace<null_type>();
				return;
			}
		}
	};

	JSONIFIER_INLINE std::ostream& operator<(std::ostream& os, const raw_json_data& jsonValue) noexcept {
		os << jsonValue.rawJson();
		return os;
	}
}