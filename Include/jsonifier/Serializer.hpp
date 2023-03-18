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
	License along with this library; if not, Write to the Free Software
	Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301
	USA
*/
/// https://github.com/RealTimeChris/Jsonifier
/// Feb 3, 2023
#pragma once

#include <jsonifier/Base.hpp>
#include <jsonifier/ToJson.hpp>
#include <jsonifier/String.hpp>

namespace Jsonifier {

	template<typename OTy> class StringReference : public StringView {
	  public:
		inline constexpr StringReference(OTy& stringReference) : string{ stringReference } {
		}

		inline constexpr void writeCharacter(const char value) {
			if (this->string.size() < this->currentSize + 1) {
				string.resize((std::max)(string.size() * 2, string.size() + 1));
			}
			this->string[this->currentSize++] = value;
		}

		inline constexpr void reserveSpace(size_t length) {
			if (this->string.size() < this->currentSize + length) {
				string.resize((std::max)(string.size() * 2, string.size() + length));
			}
		}

		inline constexpr void addSpace(size_t amount) {
			currentSize += amount;
		}

		inline constexpr char* data() {
			return string.data() + currentSize;
		}

		inline constexpr size_t getCurrentSize() {
			return this->currentSize;
		}

		inline constexpr void writeCharacters(const char* data, size_t length) {
			if (this->string.size() < this->currentSize + length) {
				string.resize((std::max)(string.size() * 2, string.size() + length));
			}
			Jsonifier::memcpy(this->string.data() + this->currentSize, data, length);
			this->currentSize += length;
		}

	  protected:
		size_t currentSize{};
		OTy& string{};
	};

	class Serializer {
	  public:
		inline constexpr Serializer() noexcept = default;

		template<typename OTy> inline void serializeJson(OTy& data, auto& buffer) {
			size_t index{};
			Write::op(data, buffer, index);
			buffer.resize(index);
		}
	};
}