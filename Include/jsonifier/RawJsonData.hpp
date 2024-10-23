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
			return std::hash<std::string_view>::operator()(static_cast<std::string_view>(string));
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

	using object_type  = std::unordered_map<jsonifier::string, jsonifier::raw_json_data>;
	using array_type   = jsonifier::vector<jsonifier::raw_json_data>;
	using float_type   = double;
	using bool_type	   = bool;
	using uint_type	   = size_t;
	using int_type	   = int64_t;
	using string_type  = jsonifier::string;
	using size_type	   = size_t;
	using variant_type = std::variant<object_type, array_type, string_type, float_type, uint_type, int_type, bool_type, std::nullptr_t>;

	variant_type constructValueFromRawJsonData(const jsonifier::string& newData) noexcept;

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
		using object_type  = std::unordered_map<jsonifier::string, raw_json_data>;
		using array_type   = jsonifier::vector<raw_json_data>;
		using float_type   = double;
		using bool_type	   = bool;
		using uint_type	   = size_t;
		using int_type	   = int64_t;
		using string_type  = jsonifier::string;
		using size_type	   = size_t;
		using variant_type = std::variant<object_type, array_type, string_type, float_type, uint_type, int_type, bool_type, std::nullptr_t>;
		friend variant_type constructValueFromRawJsonData(const jsonifier::string& newData);
		variant_type value{};

		JSONIFIER_ALWAYS_INLINE auto begin() const {
			return std::get<object_type>(value).begin();
		}

		JSONIFIER_ALWAYS_INLINE auto end() const {
			return std::get<object_type>(value).end();
		}

		JSONIFIER_ALWAYS_INLINE raw_json_data() = default;

		JSONIFIER_ALWAYS_INLINE raw_json_data& operator=(const jsonifier::string& valueNew) noexcept {
			value	 = constructValueFromRawJsonData(valueNew);
			jsonData = valueNew;
			return *this;
		}

		JSONIFIER_ALWAYS_INLINE raw_json_data(const jsonifier::string& valueNew) noexcept : value{ constructValueFromRawJsonData(valueNew) }, jsonData{ valueNew } {};

		template<typename value_type> JSONIFIER_ALWAYS_INLINE bool holds() const {
			return std::holds_alternative<value_type>(value);
		}

		JSONIFIER_ALWAYS_INLINE const char* data() const noexcept {
			return jsonData.data();
		}

		JSONIFIER_ALWAYS_INLINE char* data() noexcept {
			return jsonData.data();
		}

		JSONIFIER_ALWAYS_INLINE const raw_json_data& operator[](size_type index) const noexcept {
			return std::get<array_type>(value)[index];
		}

		JSONIFIER_ALWAYS_INLINE raw_json_data& operator[](size_type index) noexcept {
			return std::get<array_type>(value)[index];
		}

		JSONIFIER_ALWAYS_INLINE raw_json_data& operator[](jsonifier::string_view key) noexcept {
			return std::get<object_type>(value)[key];
		}

		JSONIFIER_ALWAYS_INLINE const raw_json_data& operator[](jsonifier::string_view key) const noexcept {
			return std::get<object_type>(value).at(key);
		}

		JSONIFIER_ALWAYS_INLINE json_type getType() const noexcept {
			if (jsonData.size() > 0) {
				return jsonifier_internal::getValueType(static_cast<uint8_t>(jsonData[0]));
			} else {
				return json_type::Unset;
			}
		}

		JSONIFIER_ALWAYS_INLINE void resize(uint64_t newSize) noexcept {
			jsonData.resize(newSize);
		}

		JSONIFIER_ALWAYS_INLINE explicit operator object_type() const noexcept {
			if (holds<object_type>()) {
				return std::get<object_type>(value);
			} else {
				return {};
			}
		}

		JSONIFIER_ALWAYS_INLINE explicit operator array_type() const noexcept {
			if (holds<array_type>()) {
				return std::get<array_type>(value);
			} else {
				return {};
			}
		}

		template<jsonifier::concepts::string_t value_type> JSONIFIER_ALWAYS_INLINE explicit operator value_type() const noexcept {
			if (holds<jsonifier::string>()) {
				return static_cast<value_type>(std::get<jsonifier::string>(value));
			} else {
				return {};
			}
		}

		template<jsonifier::concepts::float_type value_type> JSONIFIER_ALWAYS_INLINE explicit operator value_type() const noexcept {
			if (holds<double>()) {
				return static_cast<value_type>(std::get<double>(value));
			} else {
				return {};
			}
		}

		template<jsonifier::concepts::unsigned_type value_type> JSONIFIER_ALWAYS_INLINE explicit operator value_type() const noexcept {
			if (holds<double>()) {
				return static_cast<value_type>(std::get<double>(value));
			} else {
				return {};
			}
		}

		template<jsonifier::concepts::signed_type value_type> JSONIFIER_ALWAYS_INLINE explicit operator value_type() const noexcept {
			if (holds<double>()) {
				return static_cast<value_type>(std::get<double>(value));
			} else {
				return {};
			}
		}

		template<jsonifier::concepts::bool_t value_type> JSONIFIER_ALWAYS_INLINE explicit operator value_type() const noexcept {
			if (holds<bool>()) {
				return static_cast<value_type>(std::get<bool>(value));
			} else {
				return {};
			}
		}

		JSONIFIER_ALWAYS_INLINE jsonifier::string_view rawJson() const noexcept {
			return jsonData;
		}

		JSONIFIER_ALWAYS_INLINE bool operator==(const raw_json_data& other) const noexcept {
			return jsonData == other.jsonData;
		}

	  protected:
		string jsonData{};
	};

	JSONIFIER_ALWAYS_INLINE std::ostream& operator<<(std::ostream& os, const raw_json_data& jsonValue) noexcept {
		os << jsonValue.rawJson();
		return os;
	}

}

namespace jsonifier_internal {

	variant_type constructValueFromRawJsonData(const jsonifier::string& jsonData) noexcept {
		static constexpr jsonifier::parse_options optionsNew{};
		if (jsonData.size() > 0) {
			switch (jsonData[0]) {
				case '{': {
					jsonifier::raw_json_data::object_type results{};
					jsonifier::string::const_iterator newIter01 = jsonData.begin();
					jsonifier::string::const_iterator newIter02 = jsonData.begin() + 1;
					jsonifier::string::const_iterator endIter01 = jsonData.end();

					auto collectCharacter = [&](uint8_t character) {
						newIter02 = newIter01;
						while (newIter01 < endIter01) {
							if (*newIter01 == character) {
								++newIter01;
								break;
							}
							++newIter01;
						}
					};

					auto collectValue = [&](bool endValue) {
						newIter02 = newIter01;
						derailleur<optionsNew, parse_context<bool>>::skipToNextValue(newIter01, endIter01);
						jsonifier::string newString{};
						auto newSize = (newIter01 - newIter02);
						if (endValue) {
							newSize -= 3;
						}
						newString.resize(static_cast<uint64_t>(newSize));
						std::copy(newIter02.operator->(), newIter02.operator->() + static_cast<uint64_t>(newSize), newString.data());
						return jsonifier::raw_json_data{ newString };
					};

					auto collectKey = [&]() {
						newIter02 = newIter01;
						derailleur<optionsNew, parse_context<bool>>::skipString(newIter01, endIter01);
						jsonifier::string newString{};
						auto newSize = (newIter01 - newIter02) - 1;
						if (static_cast<int64_t>(newSize) >= 0) {
							newString.resize(static_cast<uint64_t>(newSize));
							std::copy(newIter02.operator->() + 1, newIter02.operator->() + (static_cast<uint64_t>(newSize)), newString.data());
						}
						newString.resize(static_cast<uint64_t>(newString.size() - 1));
						return newString;
					};

					derailleur<optionsNew, parse_context<bool>>::skipWs(newIter01);
					size_t newCount{ derailleur<optionsNew, parse_context<bool>>::countValueElements<'{', '}'>(newIter02, endIter01) };
					collectCharacter('{');
					for (uint64_t x = 0; x < newCount && newIter02 < endIter01 && newIter01 < endIter01; ++x) {
						derailleur<optionsNew, parse_context<bool>>::skipWs(newIter01);
						auto newKey = collectKey();
						derailleur<optionsNew, parse_context<bool>>::skipWs(newIter01);
						collectCharacter(':');
						derailleur<optionsNew, parse_context<bool>>::skipWs(newIter01);
						bool endValue{ x == newCount - 1 };
						results[newKey] = collectValue(endValue);
						derailleur<optionsNew, parse_context<bool>>::skipWs(newIter01);
						collectCharacter(',');
					}
					return variant_type{ results };
				}
				case '[': {
					jsonifier::raw_json_data::array_type results{};
					jsonifier::string::const_iterator newIter01 = jsonData.begin();
					jsonifier::string::const_iterator newIter02 = jsonData.begin() + 1;
					jsonifier::string::const_iterator endIter01 = jsonData.end();

					auto collectCharacter = [&](uint8_t character) {
						newIter02 = newIter01;
						while (newIter01 < endIter01) {
							if (*newIter01 == character) {
								++newIter01;
								break;
							}
							++newIter01;
						}
					};

					auto collectValue = [&](bool endValue) {
						newIter02 = newIter01;
						++newIter02;
						derailleur<optionsNew, parse_context<bool>>::skipToNextValue(newIter02, endIter01);
						jsonifier::string newString{};
						auto newSize = newIter02 - newIter01;
						if (endValue) {
							--newSize;
						}
						newString.resize(static_cast<uint64_t>(newSize));
						std::copy(newIter01.operator->(), newIter01.operator->() + static_cast<uint64_t>(newSize), newString.data());
						newIter02 = newIter01;
						return jsonifier::raw_json_data{ newString };
					};

					derailleur<optionsNew, parse_context<bool>>::skipWs(newIter01);
					size_t newCount{ derailleur<optionsNew, parse_context<bool>>::countValueElements<'[', ']'>(newIter02, endIter01) };
					collectCharacter('[');
					for (uint64_t x = 0; x < newCount; ++x) {
						derailleur<optionsNew, parse_context<bool>>::skipWs(newIter01);
						bool endValue{ x == newCount - 1 };
						results.emplace_back(collectValue(endValue));
						derailleur<optionsNew, parse_context<bool>>::skipWs(newIter01);
						collectCharacter(',');
					}
					return variant_type{ results };
				}
				case '"': {
					jsonifier::string returnValues{};
					parse_context<jsonifier::jsonifier_core<true>> context{};
					context.iter	 = jsonData.data();
					context.endIter	 = jsonData.data() + jsonData.size();
					context.rootIter = jsonData.data();
					derailleur<optionsNew, parse_context<jsonifier::jsonifier_core<true>>>::parseStringNr(returnValues, context);
					return variant_type{ returnValues };
				}
				case '0':
				case '1':
				case '2':
				case '3':
				case '4':
				case '5':
				case '6':
				case '7':
				case '8':
				case '9':
				case '-': {
					if (jsonData.size() > 0) {
						return variant_type{ strToDouble(jsonData) };
					} else {
						return variant_type{};
					}
				}
				case 't':
				case 'f': {
					if (jsonData == "true") {
						return variant_type{ true };
					} else {
						return variant_type{ false };
					}
				}
				case 'n': {
					return variant_type{ nullptr };
				}
			}
		} else {
			return variant_type{ nullptr };
		}
	}


}