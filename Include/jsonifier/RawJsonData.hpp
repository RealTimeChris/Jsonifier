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

#include <jsonifier/String.hpp>

namespace jsonifier {

	class raw_json_data {
	  public:
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
			jsonData = string{ std::to_string(value) };
			return *this;
		}

		inline raw_json_data(double value) {
			*this = value;
		}

		inline raw_json_data& operator=(int64_t value) {
			jsonData = string{ std::to_string(value) };
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

		inline char* data() {
			return jsonData.data();
		}

		inline void resize(uint64_t sizeNew) {
			jsonData.resize(sizeNew);
		}

		inline explicit operator string_view() {
			return { jsonData.data(), jsonData.size() };
		}

		inline operator string() const {
			string newstring{};
			newstring.resize(jsonData.size());
			std::memcpy(newstring.data(), jsonData.data(), jsonData.size());
			return newstring;
		}

		inline operator std::string() const {
			return jsonData.operator std::string();
		}

		inline bool operator==(const raw_json_data& other) const {
			return jsonData == other.jsonData;
		}

		inline raw_json_data operator+(const string& other) {
			jsonData += other;
			return *this;
		}

	  protected:
		string jsonData{};
	};

	inline std::ostream& operator<<(std::ostream& os, raw_json_data& jsonValue) {
		os << jsonValue.operator std::string();
		return os;
	}

}
