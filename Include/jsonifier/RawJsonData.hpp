/*
	MIT License

	Copyright (c) 2023 RealTimeChris

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

#include <jsonifier/Derailleur.hpp>
#include <jsonifier/HashMap.hpp>
#include <jsonifier/String.hpp>

namespace std {

	template<jsonifier::concepts::string_t string_type> struct hash<string_type> {
		JSONIFIER_INLINE uint64_t operator()(const string_type& string) const {
			return jsonifier_internal::fnv1aHashRt(string.data(), string.size());
		}
	};

}

namespace jsonifier {

	enum class json_type : uint8_t { Unset = 0, Object = 0x7Bu, Array = 0x5Bu, String = 0x22u, Number = 0x2Du, Bool = 0x74u, Null = 0x6Eu };

	class raw_json_data;
}

namespace jsonifier_internal {

	template<typename value_type> JSONIFIER_INLINE value_type constructValueFromRawJsonData(const jsonifier::string& newData);

	template<> JSONIFIER_INLINE std::unordered_map<jsonifier::string, jsonifier::raw_json_data>
	constructValueFromRawJsonData<std::unordered_map<jsonifier::string, jsonifier::raw_json_data>>(const jsonifier::string& newData);

	template<>
	JSONIFIER_INLINE jsonifier::vector<jsonifier::raw_json_data> constructValueFromRawJsonData<jsonifier::vector<jsonifier::raw_json_data>>(const jsonifier::string& newData);

	template<> JSONIFIER_INLINE jsonifier::string constructValueFromRawJsonData<jsonifier::string>(const jsonifier::string& newData);

	template<> JSONIFIER_INLINE double constructValueFromRawJsonData<double>(const jsonifier::string& newData);

	template<> JSONIFIER_INLINE uint64_t constructValueFromRawJsonData<uint64_t>(const jsonifier::string& newData);

	template<> JSONIFIER_INLINE int64_t constructValueFromRawJsonData<int64_t>(const jsonifier::string& newData);

	template<> JSONIFIER_INLINE bool constructValueFromRawJsonData<bool>(const jsonifier::string& newData);

	JSONIFIER_INLINE jsonifier::json_type getValueType(uint8_t charToCheck) {
		if (jsonifier_internal::isNumberType(charToCheck)) [[likely]] {
			return jsonifier::json_type::Number;
		} else if (jsonifier_internal::boolTable[charToCheck]) [[likely]] {
			return jsonifier::json_type::Bool;
		} else if (charToCheck == 0x7B) [[unlikely]] {
			return jsonifier::json_type::Object;
		} else if (charToCheck == 0x5B) [[unlikely]] {
			return jsonifier::json_type::Array;
		} else if (charToCheck == 0x22u) [[unlikely]] {
			return jsonifier::json_type::String;
		} else if (charToCheck == 0x6Eu) [[unlikely]] {
			return jsonifier::json_type::Null;
		} else {
			return jsonifier::json_type::Unset;
		}
	}

}

namespace jsonifier {

	class raw_json_data {
	  public:
		template<typename value_type> friend JSONIFIER_INLINE value_type constructValueFromRawJsonData(const jsonifier::string& newData);
		using object_type = std::unordered_map<jsonifier::string, raw_json_data>;
		using array_type  = jsonifier::vector<raw_json_data>;

		JSONIFIER_INLINE raw_json_data() = default;

		JSONIFIER_INLINE raw_json_data& operator=(const jsonifier::string& value) {
			jsonData = value;
			return *this;
		}

		JSONIFIER_INLINE raw_json_data(const jsonifier::string& value) {
			*this = value;
		}

		JSONIFIER_INLINE const char* data() const {
			return jsonData.data();
		}

		JSONIFIER_INLINE char* data() {
			return jsonData.data();
		}

		JSONIFIER_INLINE json_type getType() const {
			if (jsonData.size() > 0) {
				return jsonifier_internal::getValueType(static_cast<uint8_t>(jsonData[0]));
			} else {
				return json_type::Unset;
			}
		}

		JSONIFIER_INLINE void resize(uint64_t newSize) {
			jsonData.resize(newSize);
		}

		JSONIFIER_INLINE explicit operator object_type() {
			if (getType() == json_type::Object) {
				return constructValueFromRawJsonData<object_type>(jsonData);
			} else {
				return {};
			}
		}

		JSONIFIER_INLINE explicit operator array_type() {
			if (getType() == json_type::Array) {
				return constructValueFromRawJsonData<array_type>(jsonData);
			} else {
				return {};
			}
		}

		JSONIFIER_INLINE explicit operator string() const {
			if (getType() == json_type::String) {
				return constructValueFromRawJsonData<string>(jsonData);
			} else {
				return {};
			}
		}

		JSONIFIER_INLINE explicit operator double() const {
			if (getType() == json_type::Number) {
				return constructValueFromRawJsonData<double>(jsonData);
			} else {
				return {};
			}
		}

		JSONIFIER_INLINE explicit operator uint64_t() const {
			if (getType() == json_type::Number) {
				return constructValueFromRawJsonData<uint64_t>(jsonData);
			} else {
				return {};
			}
		}

		JSONIFIER_INLINE explicit operator int64_t() const {
			if (getType() == json_type::Number) {
				return constructValueFromRawJsonData<int64_t>(jsonData);
			} else {
				return {};
			}
		}

		JSONIFIER_INLINE explicit operator bool() const {
			if (getType() == json_type::Bool) {
				return constructValueFromRawJsonData<bool>(jsonData);
			} else {
				return {};
			}
		}

		JSONIFIER_INLINE jsonifier::string_view rawJson() {
			return jsonData;
		}

		JSONIFIER_INLINE bool operator==(const raw_json_data& other) const {
			return jsonData == other.jsonData;
		}

	  protected:
		string jsonData{};
	};

	JSONIFIER_INLINE std::ostream& operator<<(std::ostream& os, raw_json_data& jsonValue) {
		os << jsonValue.rawJson();
		return os;
	}

}

namespace jsonifier_internal {

	template<> JSONIFIER_INLINE std::unordered_map<jsonifier::string, jsonifier::raw_json_data>
	constructValueFromRawJsonData<std::unordered_map<jsonifier::string, jsonifier::raw_json_data>>(const jsonifier::string& jsonData) {
		jsonifier::raw_json_data::object_type results{};
		if (jsonData.size() > 0) {
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
				skipToNextValue(newIter02, endIter01);
				jsonifier::string newString{};
				auto newSize = newIter02 - newIter01;
				if (endValue) {
					--newSize;
				}
				newString.resize(static_cast<uint64_t>(newSize));
				std::copy(newIter01.operator->(), newIter01.operator->() + static_cast<uint64_t>(newSize), newString.data());
				newIter01 = newIter02;
				return newString;
			};

			auto collectKey = [&]() {
				while (newIter02 < endIter01) {
					if (*newIter02 == '"' && *(newIter01 - 1) != '\\') {
						++newIter02;
						break;
					}
					++newIter02;
				}
				newIter01 = newIter02 + 1;
				while (newIter01 < endIter01) {
					if (*newIter01 == '"' && *(newIter01 - 1) != '\\') {
						break;
					}
					++newIter01;
				}
				jsonifier::string newString{};
				auto newSize = newIter01 - newIter02;
				newString.resize(static_cast<uint64_t>(newSize));
				std::copy(newIter02.operator->(), newIter02.operator->() + static_cast<uint64_t>(newSize), newString.data());
				newIter01 = newIter02;
				return newString;
			};

			skipWs(newIter01);
			auto newCount = countValueElements<0x7Bu, 0x7Du>(newIter02, endIter01);
			collectCharacter(0x7Bu);
			for (uint64_t x = 0; x < newCount && newIter02 < endIter01 && newIter01 < endIter01; ++x) {
				skipWs(newIter01);
				auto newKey = collectKey();
				skipWs(newIter01);
				collectCharacter(0x3A);
				skipWs(newIter01);
				bool endValue{ x == newCount - 1 };
				results[newKey] = collectValue(endValue);
				skipWs(newIter01);
				collectCharacter(0x2Cu);
			}
		}
		return results;
	}

	template<>
	JSONIFIER_INLINE jsonifier::vector<jsonifier::raw_json_data> constructValueFromRawJsonData<jsonifier::vector<jsonifier::raw_json_data>>(const jsonifier::string& jsonData) {
		jsonifier::raw_json_data::array_type results{};
		if (jsonData.size() > 0) {
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
				skipToNextValue(newIter02, endIter01);
				jsonifier::string newString{};
				auto newSize = newIter02 - newIter01;
				if (endValue) {
					--newSize;
				}
				newString.resize(static_cast<uint64_t>(newSize));
				std::copy(newIter01.operator->(), newIter01.operator->() + static_cast<uint64_t>(newSize), newString.data());
				newIter01 = newIter02;
				return newString;
			};

			skipWs(newIter01);
			auto newCount = countValueElements<0x5Bu, 0x5Du>(newIter02, endIter01);
			collectCharacter(0x5Bu);
			for (uint64_t x = 0; x < newCount && newIter02 < endIter01 && newIter01 < endIter01; ++x) {
				skipWs(newIter01);
				bool endValue{ x == newCount - 1 };
				results.emplace_back(collectValue(endValue));
				skipWs(newIter01);
				collectCharacter(0x2Cu);
			}
		}
		return results;
	}

	template<> JSONIFIER_INLINE jsonifier::string constructValueFromRawJsonData<jsonifier::string>(const jsonifier::string& jsonData) {
		if (jsonData.size() > 1) {
			return { jsonData.data() + 1, jsonData.size() - 2 };
		} else {
			return {};
		}
	}

	template<> JSONIFIER_INLINE double constructValueFromRawJsonData<double>(const jsonifier::string& jsonData) {
		if (jsonData.size() > 0) {
			return strToDouble(jsonData);
		} else {
			return {};
		}
	}

	template<> JSONIFIER_INLINE uint64_t constructValueFromRawJsonData<uint64_t>(const jsonifier::string& jsonData) {
		if (jsonData.size() > 0) {
			return strToUint64(jsonData);
		} else {
			return {};
		}
	}

	template<> JSONIFIER_INLINE int64_t constructValueFromRawJsonData<int64_t>(const jsonifier::string& jsonData) {
		if (jsonData.size() > 0) {
			return strToInt64(jsonData);
		} else {
			return {};
		}
	}

	template<> JSONIFIER_INLINE bool constructValueFromRawJsonData<bool>(const jsonifier::string& jsonData) {
		if (jsonData == "true") {
			return true;
		} else {
			return false;
		}
	}
}