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
/// https://github.com/RealTimeChris/Jsonifier
/// Feb 20, 2023
#pragma once

#include <jsonifier/String.hpp>

namespace Jsonifier {

	struct RawJsonData {
		inline RawJsonData() noexcept = default;

		inline RawJsonData& operator=(bool value) noexcept {
			if (value) {
				jsonData = "true";
			} else {
				jsonData = "false";
			}
			return *this;
		}

		inline RawJsonData(bool value) noexcept {
			*this = value;
		}

		inline RawJsonData& operator=(double value) noexcept {
			jsonData = String{ std::to_string(value) };
			return *this;
		}

		inline RawJsonData(double value) noexcept {
			*this = value;
		}

		inline RawJsonData& operator=(int64_t value) noexcept {
			jsonData = String{ std::to_string(value) };
			return *this;
		}

		inline RawJsonData(int64_t value) noexcept {
			*this = value;
		}

		inline RawJsonData& operator=(const Jsonifier::String& value) noexcept {
			jsonData = value;
			return *this;
		}

		inline RawJsonData(const Jsonifier::String& value) noexcept {
			*this = value;
		}

		inline char* data() noexcept {
			return jsonData.data();
		}

		inline void resize(uint64_t sizeNew) noexcept {
			jsonData.resize(sizeNew);
		}

		inline explicit operator StringView() noexcept {
			return { jsonData.data(), jsonData.size() };
		}

		inline operator String() noexcept {
			String newString{};
			newString.resize(jsonData.size());
			std::memcpy(newString.data(), jsonData.data(), jsonData.size());
			return newString;
		}

		inline operator std::string() noexcept {
			return jsonData.operator std::string();
		}

		inline bool operator==(const RawJsonData& other) const {
			return jsonData == other.jsonData;
		}

		inline RawJsonData operator+(const String& other) {
			jsonData += other;
			return *this;
		}

	  protected:
		String jsonData{};
	};

	inline std::ostream& operator<<(std::ostream& os, RawJsonData& jsonValue) noexcept {
		os << jsonValue.operator std::string();
		return os;
	}

}
