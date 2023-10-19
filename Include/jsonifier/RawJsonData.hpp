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

	template<> struct hash<jsonifier::string_view> {
		inline uint64_t operator()(jsonifier::string_view string) const {
			return jsonifier_internal::fnv1aHash(string);
		}
	};

	template<> struct hash<jsonifier::string> {
		inline uint64_t operator()(const jsonifier::string& string) const {
			return jsonifier_internal::fnv1aHash(string);
		}
	};

}

namespace jsonifier {

	enum class json_type {
		Unset  = '\0',
		Object = 0x7B,
		Array  = '[',
		String = '"',
		Number = '-' | '0' | '1' | '2' | '3' | '4' | '5' | '6' | '7' | '8' | '9',
		Bool   = 't' | 'f',
		Null   = 'n',
	};

	inline std::unordered_map<jsonifier::string_view, json_type> typeMap{ { "", json_type::Unset }, { "Object", json_type::Object }, { "Array", json_type::Array },
		{ "String", json_type::String }, { "Number", json_type::Number }, { "Bool", json_type::Bool }, { "Null", json_type::Null } };

	class raw_json_data {
	  public:
		using object_type = std::unordered_map<jsonifier::string, raw_json_data>;
		using array_type  = jsonifier::vector<raw_json_data>;

		inline raw_json_data() = default;

		inline raw_json_data& operator=(bool value) {
			if (value) {
				jsonData = "true";
			} else {
				jsonData = "false";
			}
			return *this;
		}

		inline raw_json_data(bool value) {
			*this = value;
		}

		inline raw_json_data& operator=(double value) {
			jsonData = toString(value);
			return *this;
		}

		inline raw_json_data(double value) {
			*this = value;
		}

		inline raw_json_data& operator=(int64_t value) {
			jsonData = toString(value);
			return *this;
		}

		inline raw_json_data(int64_t value) {
			*this = value;
		}

		inline raw_json_data& operator=(const jsonifier::string& value) {
			jsonData = value;
			return *this;
		}

		inline raw_json_data(const jsonifier::string& value) {
			*this = value;
		}

		inline const char* data() const {
			return jsonData.data();
		}

		inline char* data() {
			return jsonData.data();
		}

		inline json_type getType() {
			if (jsonData.size() > 0) {
				return typeMap[jsonifier_internal::getValueType(jsonData[0])];
			} else {
				return typeMap[""];
			}
		}

		inline void resize(uint64_t sizeNew) {
			jsonData.resize(sizeNew);
		}

		inline explicit operator object_type() {
			object_type results{};
			if (jsonData.size() > 0) {
				jsonifier::string::iterator newIter01 = jsonData.begin();
				jsonifier::string::iterator newIter02 = jsonData.begin() + 1;
				jsonifier::string::iterator endIter01 = jsonData.end();
				jsonifier_internal::derailleur::skipToNextValue(endIter01, endIter01);
				auto newCount = jsonifier_internal::derailleur::countValueElements(newIter01 + 1, endIter01);
				for (uint64_t x = 0; x < newCount; ++x) {
					auto newIter03 = newIter02;
					if (x == 0) {
						++newIter02;
						++newIter02;
						jsonifier_internal::derailleur::skipKey(newIter02, endIter01);
						auto stringSize = static_cast<uint64_t>(newIter02.operator->() - newIter03.operator->() - 2);
						auto newKey		= jsonifier::string{ newIter03.operator->() + 1, stringSize };
						newIter03		= newIter02 + 1;
						jsonifier_internal::derailleur::skipToNextValue(newIter02, endIter01);
						results.emplace(std::pair<jsonifier::string, raw_json_data>{ newKey,
							raw_json_data{ jsonifier::string{ newIter03.operator->(), static_cast<uint64_t>(newIter02.operator->() - newIter03.operator->()) } } });
					} else if (x == newCount - 1) {
						++newIter02;
						jsonifier_internal::derailleur::skipKey(newIter02, endIter01);
						auto stringSize = static_cast<uint64_t>(newIter02.operator->() - newIter03.operator->() - 3);
						auto newKey		= jsonifier::string{ newIter03.operator->() + 2, stringSize };

						newIter03 = newIter02 + 1;
						jsonifier_internal::derailleur::skipToNextValue(newIter02, endIter01);
						results.emplace(std::pair<jsonifier::string, raw_json_data>{ newKey,
							raw_json_data{ jsonifier::string{ newIter03.operator->(), static_cast<uint64_t>(newIter02.operator->() - newIter03.operator->() - 1) } } });
					} else {
						++newIter02;
						jsonifier_internal::derailleur::skipKey(newIter02, endIter01);
						auto stringSize = static_cast<uint64_t>(newIter02.operator->() - newIter03.operator->() - 3);
						auto newKey		= jsonifier::string{ newIter03.operator->() + 2, stringSize };
						newIter03		= newIter02 + 1;
						jsonifier_internal::derailleur::skipToNextValue(newIter02, endIter01);
						results.emplace(std::pair<jsonifier::string, raw_json_data>{ newKey,
							raw_json_data{ jsonifier::string{ newIter03.operator->(), static_cast<uint64_t>(newIter02.operator->() - newIter03.operator->()) } } });
					}
				}
				return results;
			}
			return {};
		}

		inline explicit operator array_type() {
			array_type results{};
			if (jsonData.size() > 0) {
				jsonifier::string::iterator newIter01 = jsonData.begin();
				jsonifier::string::iterator newIter02 = jsonData.begin() + 1;
				jsonifier::string::iterator endIter01 = jsonData.end();
				jsonifier_internal::derailleur::skipToNextValue(endIter01, endIter01);
				auto newCount = jsonifier_internal::derailleur::countValueElements(newIter01 + 1, endIter01);
				for (uint64_t x = 0; x < newCount; ++x) {
					auto newIter03 = newIter02;
					jsonifier_internal::derailleur::skipToNextValue(newIter02, endIter01);
					++newIter02;
					if (x == newCount - 1) {
						results.emplace_back(
							raw_json_data{ jsonifier::string{ newIter03.operator->(), static_cast<uint64_t>(newIter02.operator->() - newIter03.operator->() - 2) } });
					} else {
						results.emplace_back(
							raw_json_data{ jsonifier::string{ newIter03.operator->(), static_cast<uint64_t>(newIter02.operator->() - newIter03.operator->() - 1) } });
					}
				}
				return results;
			}
			return {};
		}

		inline explicit operator string_view() {
			if (jsonData.size() > 0) {
				return { jsonData.data() + 1, jsonData.size() - 1 };
			} else {
				return {};
			}
		}

		inline explicit operator string() const {
			if (jsonData.size() > 0) {
				return { jsonData.data() + 1, jsonData.size() - 1 };
			} else {
				return {};
			}
		}

		inline explicit operator double() const {
			if (jsonData.size() > 0) {
				return strToDouble(jsonData);
			} else {
				return {};
			}
		}

		inline explicit operator uint64_t() const {
			if (jsonData.size() > 0) {
				return strToUint64(jsonData);
			} else {
				return {};
			}
		}

		inline explicit operator int64_t() const {
			if (jsonData.size() > 0) {
				return strToInt64(jsonData);
			} else {
				return {};
			}
		}

		inline explicit operator bool() const {
			if (jsonData == "true") {
				return true;
			} else {
				return false;
			}
		}

		inline jsonifier::string_view rawJson() {
			return jsonData;
		}

		inline bool operator==(const raw_json_data& other) const {
			return jsonData == other.jsonData;
		}

	  protected:
		string jsonData{};
	};

	inline std::ostream& operator<<(std::ostream& os, raw_json_data& jsonValue) {
		os << jsonValue.rawJson();
		return os;
	}

}
