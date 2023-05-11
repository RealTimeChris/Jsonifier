/*
	Jsonifier - For parsing and serializing Json - very rapidly.
	Copyright (C) 2023 Chris M. (RealTimeChris)

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
	Lesser General Public License for more details.

	You should have received a copy of the GNU Lesser General Public
	License along with this library; if not, Serialize to the Free Software
	Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301
	USA
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

		inline void resize(size_t newSize) noexcept {
			jsonData.resize(newSize);
		}

		inline explicit operator StringView() noexcept {
			return { jsonData.data(), jsonData.size() };
		}

		inline explicit operator String() noexcept {
			String newString{};
			newString.resize(jsonData.size());
			std::copy(jsonData.data(), jsonData.data() + jsonData.size(), newString.data());
			return newString;
		}

		inline operator std::string() noexcept {
			return jsonData.operator std::string();
		}

		inline bool operator==(const RawJsonData& other) const {
			return jsonData == other.jsonData;
		}

		inline bool operator!=(const RawJsonData& other) const {
			return !(*this == other);
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
