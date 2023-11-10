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
			return jsonifier_internal::fnv1aHash(string, 0);
		}
	};

}

namespace jsonifier {

	enum class json_type : uint8_t { Unset = 0, Object = 0x7Bu, Array = 0x5Bu, String = 0x22u, Number = 0x2Du, Bool = 0x74u, Null = 0x6Eu };

	class raw_json_data;

}

namespace jsonifier_internal {

	inline std::unordered_map<jsonifier::string_view, jsonifier::json_type> typeMap{ { "", jsonifier::json_type::Unset }, { "Object", jsonifier::json_type::Object },
		{ "Array", jsonifier::json_type::Array }, { "String", jsonifier::json_type::String }, { "Number", jsonifier::json_type::Number }, { "Bool", jsonifier::json_type::Bool },
		{ "Null", jsonifier::json_type::Null } };

}

namespace jsonifier {

	class raw_json_data {
	  public:
		template<typename value_type> friend inline value_type constructValueFromRawJsonData(const jsonifier::string& jsonData);
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

		JSONIFIER_INLINE json_type getType() const {
			if (jsonData.size() > 0) {
				return jsonifier_internal::typeMap[jsonifier_internal::getValueType(static_cast<uint8_t>(jsonData[0]))];
			} else {
				return jsonifier_internal::typeMap[""];
			}
		}

		JSONIFIER_INLINE explicit operator object_type() const {
			if (getType() == json_type::Object) {
				std::unordered_map<jsonifier::string, jsonifier::raw_json_data> results{};
				if (jsonData.size() > 0) {
					jsonifier::string::const_iterator iter = jsonData.begin();
					jsonifier::string::const_iterator end  = jsonData.end();

					jsonifier_internal::derailleur::skipWs(iter);
					jsonifier_internal::derailleur::collectCharacter('{', iter, end);
					auto newCount = jsonifier_internal::derailleur::countValueElements(iter, end);
					for (uint64_t x = 0; x < newCount && iter < end; ++x) {
						jsonifier_internal::derailleur::skipWs(iter);
						auto newKey = jsonifier_internal::derailleur::collectKey(iter, end);
						jsonifier_internal::derailleur::skipWs(iter);
						jsonifier_internal::derailleur::collectCharacter(':', iter, end);
						jsonifier_internal::derailleur::skipWs(iter);
						results.emplace(newKey, jsonifier_internal::derailleur::collectValue(iter, end));
						jsonifier_internal::derailleur::skipWs(iter);
						jsonifier_internal::derailleur::collectCharacter(',', iter, end);
					}
				}
				return results;
			} else {
				return {};
			}
		}

		JSONIFIER_INLINE explicit operator array_type() const {
			if (getType() == json_type::Array) {
				jsonifier::vector<jsonifier::raw_json_data> results{};
				if (jsonData.size() > 0) {
					jsonifier::string::const_iterator iter = jsonData.begin();
					jsonifier::string::const_iterator end  = jsonData.end();

					jsonifier_internal::derailleur::skipWs(iter);
					jsonifier_internal::derailleur::collectCharacter('[', iter, end);
					auto newCount = jsonifier_internal::derailleur::countValueElements(iter, end);
					for (uint64_t x = 0; x < newCount && iter < end; ++x) {
						jsonifier_internal::derailleur::skipWs(iter);
						results.emplace_back(jsonifier_internal::derailleur::collectValue(iter, end));
						jsonifier_internal::derailleur::skipWs(iter);
						jsonifier_internal::derailleur::collectCharacter(',', iter, end);
					}
				}
				return results;
			} else {
				return {};
			}
		}

		JSONIFIER_INLINE explicit operator string() const {
			if (getType() == json_type::String) {
				if (jsonData.size() > 1) {
					return { jsonData.data() + 1, jsonData.size() - 2 };
				} else {
					return {};
				}
			} else {
				return {};
			}
		}

		JSONIFIER_INLINE explicit operator double() const {
			if (getType() == json_type::Number) {
				if (jsonData.size() > 0) {
					return strToDouble(jsonData);
				} else {
					return {};
				}
			} else {
				return {};
			}
		}

		JSONIFIER_INLINE explicit operator uint64_t() const {
			if (getType() == json_type::Number) {
				if (jsonData.size() > 0) {
					return strToUint64(jsonData);
				} else {
					return {};
				}
			} else {
				return {};
			}
		}

		JSONIFIER_INLINE explicit operator int64_t() const {
			if (getType() == json_type::Number) {
				if (jsonData.size() > 0) {
					return strToInt64(jsonData);
				} else {
					return {};
				}
			} else {
				return {};
			}
		}

		JSONIFIER_INLINE explicit operator bool() const {
			if (getType() == json_type::Bool) {
				if (jsonData == "true") {
					return true;
				} else {
					return false;
				}
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

		JSONIFIER_INLINE bool operator==(const array_type& other) const {
			return operator array_type() == other;
		}

		JSONIFIER_INLINE bool operator==(const object_type& other) const {
			return operator object_type() == other;
		}

		JSONIFIER_INLINE bool operator==(const jsonifier::string& other) const {
			return operator jsonifier::string() == other;
		}

		template<jsonifier::concepts::num_t number_type> JSONIFIER_INLINE bool operator==(const number_type& other) const {
			if constexpr (concepts::float_t<number_type>) {
				return operator double() == other;
			} else if constexpr (concepts::unsigned_t<number_type>) {
				return operator uint64_t() == other;
			} else {
				return operator int64_t() == other;
			}
		}

		JSONIFIER_INLINE bool operator==(const bool& other) const {
			return operator bool() == other;
		}

	  protected:
		string jsonData{};
	};

	JSONIFIER_INLINE std::ostream& operator<<(std::ostream& os, raw_json_data& jsonValue) {
		os << jsonValue.rawJson();
		return os;
	}

}

