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
	License along with this library; if not, write to the Free Software
	Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301
	USA
*/
/// https://github.com/RealTimeChris/Jsonifier
/// Feb 3, 2023

#include <jsonifier/Jsonifier.hpp>

namespace Jsonifier {

	JsonifierException::JsonifierException(const std::string& error, std::source_location location) noexcept : std::runtime_error(error) {
		std::stringstream stream{};
		stream << "Error Report: \n"
			   << "Caught in File: " << location.file_name() << " (" << std::to_string(location.line()) << ":" << std::to_string(location.column())
			   << ")"
			   << "\nThe Error: \n"
			   << error << std::endl
			   << std::endl;
		*static_cast<std::runtime_error*>(this) = std::runtime_error{ stream.str() };
	}

	EnumConverter::operator std::vector<uint64_t>() const noexcept {
		return vector;
	}

	EnumConverter::operator uint64_t() const noexcept {
		return integer;
	}

	bool EnumConverter::isItAVector() const noexcept {
		return vectorType;
	}

	Serializer& Serializer::operator=(Serializer&& data) noexcept {
		switch (data.type) {
			case JsonType::Object: {
				setValue(JsonType::Object);
				*jsonValue.object = std::forward<ObjectType>(*data.jsonValue.object);
				break;
			}
			case JsonType::Array: {
				setValue(JsonType::Array);
				*jsonValue.array = std::forward<ArrayType>(*data.jsonValue.array);
				break;
			}
			case JsonType::String: {
				setValue(JsonType::String);
				*jsonValue.string = std::forward<StringType>(*data.jsonValue.string);
				break;
			}
			case JsonType::Float: {
				jsonValue.numberDouble = data.jsonValue.numberDouble;
				break;
			}
			case JsonType::Uint64: {
				jsonValue.numberUint = data.jsonValue.numberUint;
				break;
			}
			case JsonType::Int64: {
				jsonValue.numberInt = data.jsonValue.numberInt;
				break;
			}
			case JsonType::Bool: {
				jsonValue.boolean = data.jsonValue.boolean;
				break;
			}
		}
		string = std::move(data.string);
		type = data.type;
		return *this;
	}

	Serializer::Serializer(Serializer&& data) noexcept {
		*this = std::move(data);
	}

	Serializer& Serializer::operator=(const Serializer& data) noexcept {
		switch (data.type) {
			case JsonType::Object: {
				setValue(JsonType::Object);
				*jsonValue.object = *data.jsonValue.object;
				break;
			}
			case JsonType::Array: {
				setValue(JsonType::Array);
				*jsonValue.array = *data.jsonValue.array;
				break;
			}
			case JsonType::String: {
				setValue(JsonType::String);
				*jsonValue.string = *data.jsonValue.string;
				break;
			}
			case JsonType::Float: {
				jsonValue.numberDouble = data.jsonValue.numberDouble;
				break;
			}
			case JsonType::Uint64: {
				jsonValue.numberUint = data.jsonValue.numberUint;
				break;
			}
			case JsonType::Int64: {
				jsonValue.numberInt = data.jsonValue.numberInt;
				break;
			}
			case JsonType::Bool: {
				jsonValue.boolean = data.jsonValue.boolean;
				break;
			}
		}
		string = data.string;
		type = data.type;
		return *this;
	}

	Serializer::Serializer(const Serializer& data) noexcept {
		*this = data;
	}

	Serializer::operator std::string&&() noexcept {
		return std::move(string);
	}

	Serializer::operator std::string() noexcept {
		return string;
	}

	JsonType Serializer::getType() noexcept {
		return type;
	}

	void Serializer::refreshString(JsonifierSerializeType opCode) {
		string.clear();
		if (opCode == JsonifierSerializeType::Etf) {
			appendVersion();
			serializeJsonToEtfString(this);
		} else {
			serializeJsonToJsonString(this);
		}
	}

	Serializer& Serializer::operator=(EnumConverter&& data) noexcept {
		if (data.isItAVector()) {
			setValue(JsonType::Array);
			for (auto& value: data.operator std::vector<uint64_t>()) {
				jsonValue.array->emplace_back(std::move(value));
			}
		} else {
			jsonValue.numberUint = uint64_t{ data };
			type = JsonType::Uint64;
		}
		return *this;
	}

	Serializer::Serializer(EnumConverter&& data) noexcept {
		*this = std::move(data);
	}

	Serializer& Serializer::operator=(const EnumConverter& data) noexcept {
		if (data.isItAVector()) {
			setValue(JsonType::Array);
			for (auto& value: data.operator std::vector<uint64_t>()) {
				jsonValue.array->emplace_back(value);
			}
		} else {
			jsonValue.numberUint = uint64_t{ data };
			type = JsonType::Uint64;
		}
		return *this;
	}

	Serializer::Serializer(const EnumConverter& data) noexcept {
		*this = data;
	}

	Serializer& Serializer::operator=(std::string&& data) noexcept {
		setValue(JsonType::String);
		*jsonValue.string = std::move(data);
		type = JsonType::String;
		return *this;
	}

	Serializer::Serializer(std::string&& data) noexcept {
		*this = std::move(data);
	}

	Serializer& Serializer::operator=(const std::string& data) noexcept {
		setValue(JsonType::String);
		*jsonValue.string = data;
		type = JsonType::String;
		return *this;
	}

	Serializer::Serializer(const std::string& data) noexcept {
		*this = data;
	}

	Serializer& Serializer::operator=(const char* data) noexcept {
		setValue(JsonType::String);
		*jsonValue.string = data;
		type = JsonType::String;
		return *this;
	}

	Serializer::Serializer(const char* data) noexcept {
		*this = data;
	}

	Serializer& Serializer::operator=(double data) noexcept {
		jsonValue.numberDouble = data;
		type = JsonType::Float;
		return *this;
	}

	Serializer::Serializer(double data) noexcept {
		*this = data;
	}

	Serializer& Serializer::operator=(float data) noexcept {
		jsonValue.numberDouble = data;
		type = JsonType::Float;
		return *this;
	}

	Serializer::Serializer(float data) noexcept {
		*this = data;
	}

	Serializer& Serializer::operator=(uint64_t data) noexcept {
		jsonValue.numberUint = data;
		type = JsonType::Uint64;
		return *this;
	}

	Serializer::Serializer(uint64_t data) noexcept {
		*this = data;
	}

	Serializer& Serializer::operator=(uint32_t data) noexcept {
		jsonValue.numberUint = data;
		type = JsonType::Uint64;
		return *this;
	}

	Serializer::Serializer(uint32_t data) noexcept {
		*this = data;
	}

	Serializer& Serializer::operator=(uint16_t data) noexcept {
		jsonValue.numberUint = data;
		type = JsonType::Uint64;
		return *this;
	}

	Serializer::Serializer(uint16_t data) noexcept {
		*this = data;
	}

	Serializer& Serializer::operator=(uint8_t data) noexcept {
		jsonValue.numberUint = data;
		type = JsonType::Uint64;
		return *this;
	}

	Serializer::Serializer(uint8_t data) noexcept {
		*this = data;
	}

	Serializer& Serializer::operator=(int64_t data) noexcept {
		jsonValue.numberInt = data;
		type = JsonType::Int64;
		return *this;
	}

	Serializer::Serializer(int64_t data) noexcept {
		*this = data;
	}

	Serializer& Serializer::operator=(int32_t data) noexcept {
		jsonValue.numberInt = data;
		type = JsonType::Int64;
		return *this;
	}

	Serializer::Serializer(int32_t data) noexcept {
		*this = data;
	}

	Serializer& Serializer::operator=(int16_t data) noexcept {
		jsonValue.numberInt = data;
		type = JsonType::Int64;
		return *this;
	}

	Serializer::Serializer(int16_t data) noexcept {
		*this = data;
	}

	Serializer& Serializer::operator=(int8_t data) noexcept {
		jsonValue.numberInt = data;
		type = JsonType::Int64;
		return *this;
	}

	Serializer::Serializer(int8_t data) noexcept {
		*this = data;
	}

	Serializer& Serializer::operator=(std::nullptr_t) noexcept {
		type = JsonType::Null;
		return *this;
	}

	Serializer::Serializer(std::nullptr_t data) noexcept {
		*this = data;
	}

	Serializer& Serializer::operator=(bool data) noexcept {
		jsonValue.boolean = data;
		type = JsonType::Bool;
		return *this;
	}

	Serializer::Serializer(bool data) noexcept {
		*this = data;
	}

	Serializer& Serializer::operator=(JsonType typeNew) noexcept {
		type = typeNew;
		setValue(type);
		return *this;
	}

	Serializer::Serializer(JsonType type) noexcept {
		*this = type;
	}

	Serializer& Serializer::operator[](typename ObjectType::key_type key) {
		if (type == JsonType::Null) {
			setValue(JsonType::Object);
			type = JsonType::Object;
		}

		if (type == JsonType::Object) {
			std::pair result = jsonValue.object->emplace(std::move(key), Serializer{});
			return result.first->second;
		}
		throw JsonifierException{ "Sorry, but that item-key could not be produced/accessed." };
	}

	Serializer& Serializer::operator[](uint64_t index) {
		if (type == JsonType::Null) {
			setValue(JsonType::Array);
			type = JsonType::Array;
		}

		if (type == JsonType::Array) {
			if (index >= jsonValue.array->size()) {
				jsonValue.array->resize(index + 1);
			}

			return jsonValue.array->operator[](index);
		}
		throw JsonifierException{ "Sorry, but that index could not be produced/accessed." };
	}

	void Serializer::emplaceBack(Serializer&& other) noexcept {
		if (type == JsonType::Null) {
			setValue(JsonType::Array);
			type = JsonType::Array;
		}

		if (type == JsonType::Array) {
			jsonValue.array->emplace_back(std::move(other));
		}
	}

	void Serializer::emplaceBack(Serializer& other) noexcept {
		if (type == JsonType::Null) {
			setValue(JsonType::Array);
			type = JsonType::Array;
		}

		if (type == JsonType::Array) {
			jsonValue.array->emplace_back(other);
		}
	}

	void Serializer::serializeJsonToEtfString(const Serializer* dataToParse) {
		switch (dataToParse->type) {
			case JsonType::Object: {
				return writeEtfObject(*dataToParse->jsonValue.object);
			}
			case JsonType::Array: {
				return writeEtfArray(*dataToParse->jsonValue.array);
			}
			case JsonType::String: {
				return writeEtfString(*dataToParse->jsonValue.string);
			}
			case JsonType::Float: {
				return writeEtfFloat(dataToParse->jsonValue.numberDouble);
			}
			case JsonType::Uint64: {
				return writeEtfUint(dataToParse->jsonValue.numberUint);
			}
			case JsonType::Int64: {
				return writeEtfInt(dataToParse->jsonValue.numberInt);
			}
			case JsonType::Bool: {
				return writeEtfBool(dataToParse->jsonValue.boolean);
			}
			case JsonType::Null: {
				return writeEtfNull();
			}
		}
	}

	void Serializer::serializeJsonToJsonString(const Serializer* dataToParse) {
		switch (dataToParse->type) {
			case JsonType::Object: {
				return writeJsonObject(*dataToParse->jsonValue.object);
			}
			case JsonType::Array: {
				return writeJsonArray(*dataToParse->jsonValue.array);
			}
			case JsonType::String: {
				return writeJsonString(*dataToParse->jsonValue.string);
			}
			case JsonType::Float: {
				return writeJsonFloat(dataToParse->jsonValue.numberDouble);
			}
			case JsonType::Uint64: {
				return writeJsonInt(dataToParse->jsonValue.numberUint);
			}
			case JsonType::Int64: {
				return writeJsonInt(dataToParse->jsonValue.numberInt);
			}
			case JsonType::Bool: {
				return writeJsonBool(dataToParse->jsonValue.boolean);
			}
			case JsonType::Null: {
				return writeJsonNull();
			}
		}
	}

	void Serializer::writeJsonObject(const ObjectType& objectNew) {
		if (objectNew.empty()) {
			writeString("{}", 2);
			return;
		}
		writeCharacter('{');

		int32_t index{};
		for (auto& [key, value]: objectNew) {
			writeJsonString(key);
			writeCharacter(':');
			serializeJsonToJsonString(&value);

			if (index != objectNew.size() - 1) {
				writeCharacter(',');
			}
			++index;
		}

		writeCharacter('}');
	}

	void Serializer::writeJsonArray(const ArrayType& arrayNew) {
		if (arrayNew.empty()) {
			writeString("[]", 2);
			return;
		}

		writeCharacter('[');

		int32_t index{};
		for (auto& value: arrayNew) {
			serializeJsonToJsonString(&value);
			if (index != arrayNew.size() - 1) {
				writeCharacter(',');
			}
			++index;
		}

		writeCharacter(']');
	}

	void Serializer::writeJsonString(const StringType& stringNew) {
		writeCharacter('"');
		writeString(stringNew.data(), stringNew.size());
		writeCharacter('"');
	}

	void Serializer::writeJsonFloat(const FloatType x) {
		std::string floatValue = std::to_string(x);
		writeString(floatValue.data(), floatValue.size());
	}

	void Serializer::writeJsonBool(const BoolType jsonValueNew) {
		if (jsonValueNew) {
			writeString("true", 4);
		} else {
			writeString("false", 5);
		}
	}

	void Serializer::writeJsonNull() {
		writeString("null", 4);
	}

	void Serializer::writeEtfObject(const ObjectType& jsonData) {
		appendMapHeader(static_cast<uint32_t>(jsonData.size()));
		for (auto& [key, value]: jsonData) {
			appendBinaryExt(key, static_cast<uint32_t>(key.size()));
			serializeJsonToEtfString(&value);
		}
	}

	void Serializer::writeEtfArray(const ArrayType& jsonData) {
		appendListHeader(static_cast<uint32_t>(jsonData.size()));
		for (auto& value: jsonData) {
			serializeJsonToEtfString(&value);
		}
		appendNilExt();
	}

	void Serializer::writeEtfString(const StringType& jsonData) {
		appendBinaryExt(jsonData, static_cast<uint32_t>(jsonData.size()));
	}

	void Serializer::writeEtfUint(const UintType jsonData) {
		if (jsonData <= std::numeric_limits<uint8_t>::max() && jsonData >= std::numeric_limits<uint8_t>::min()) {
			appendUint8(static_cast<uint8_t>(jsonData));
		} else if (jsonData <= std::numeric_limits<uint32_t>::max() && jsonData >= std::numeric_limits<uint32_t>::min()) {
			appendUint32(static_cast<uint32_t>(jsonData));
		} else {
			appendUint64(jsonData);
		}
	}

	void Serializer::writeEtfInt(const IntType jsonData) {
		if (jsonData <= std::numeric_limits<int8_t>::max() && jsonData >= std::numeric_limits<int8_t>::min()) {
			appendInt8(static_cast<int8_t>(jsonData));
		} else if (jsonData <= std::numeric_limits<int32_t>::max() && jsonData >= std::numeric_limits<int32_t>::min()) {
			appendInt32(static_cast<int32_t>(jsonData));
		} else {
			appendInt64(jsonData);
		}
	}

	void Serializer::writeEtfFloat(const FloatType jsonData) {
		appendNewFloatExt(jsonData);
	}

	void Serializer::writeEtfBool(const BoolType jsonData) {
		appendBool(jsonData);
	}

	void Serializer::writeEtfNull() {
		appendNil();
	}

	void Serializer::writeString(const char* data, size_t length) {
		string.append(data, length);
	}

	void Serializer::writeCharacter(const char charValue) {
		string.push_back(charValue);
	}

	bool operator==(const Serializer& lhs, const Serializer& rhs) {
		if (lhs.type != rhs.type) {
			return false;
		}
		switch (rhs.type) {
			case JsonType::Object: {
				if (*lhs.jsonValue.object != *rhs.jsonValue.object) {
					return false;
				}
				break;
			}
			case JsonType::Array: {
				if (*lhs.jsonValue.array != *rhs.jsonValue.array) {
					return false;
				}
				break;
			}
			case JsonType::String: {
				if (*lhs.jsonValue.string != *rhs.jsonValue.string) {
					return false;
				}
				break;
			}
			case JsonType::Float: {
				if (lhs.jsonValue.numberDouble != rhs.jsonValue.numberDouble) {
					return false;
				}
				break;
			}
			case JsonType::Uint64: {
				if (lhs.jsonValue.numberUint != rhs.jsonValue.numberUint) {
					return false;
				}
				break;
			}
			case JsonType::Int64: {
				if (lhs.jsonValue.numberInt != rhs.jsonValue.numberInt) {
					return false;
				}
				break;
			}
			case JsonType::Bool: {
				if (lhs.jsonValue.boolean != rhs.jsonValue.boolean) {
					return false;
				}
				break;
			}
		}
		return true;
	}

	void Serializer::appendBinaryExt(const std::string& bytes, uint32_t sizeNew) {
		char newBuffer[5]{ static_cast<char>(EtfType::Binary_Ext) };
		storeBits(newBuffer + 1, sizeNew);
		writeString(newBuffer, std::size(newBuffer));
		writeString(bytes.data(), bytes.size());
	}

	void Serializer::appendNewFloatExt(const double FloatValue) {
		char newBuffer[9]{ static_cast<char>(EtfType::New_Float_Ext) };
		const void* punner{ &FloatValue };
		storeBits(newBuffer + 1, *static_cast<const uint64_t*>(punner));
		writeString(newBuffer, std::size(newBuffer));
	}

	void Serializer::appendListHeader(const uint32_t sizeNew) {
		char newBuffer[5]{ static_cast<char>(EtfType::List_Ext) };
		storeBits(newBuffer + 1, sizeNew);
		writeString(newBuffer, std::size(newBuffer));
	}

	void Serializer::appendMapHeader(const uint32_t sizeNew) {
		char newBuffer[5]{ static_cast<char>(EtfType::Map_Ext) };
		storeBits(newBuffer + 1, sizeNew);
		writeString(newBuffer, std::size(newBuffer));
	}

	void Serializer::appendUint64(uint64_t value) {
		char newBuffer[11]{ static_cast<char>(EtfType::Small_Big_Ext) };
		char encodedBytes{};
		while (value > 0) {
			newBuffer[3 + encodedBytes] = value & 0xFF;
			value >>= 8;
			++encodedBytes;
		}
		newBuffer[1] = encodedBytes;
		newBuffer[2] = 0;
		writeString(newBuffer, 1ull + 2ull + static_cast<size_t>(encodedBytes));
	}

	void Serializer::appendUint32(const uint32_t value) {
		char newBuffer[5]{ static_cast<char>(EtfType::Integer_Ext) };
		storeBits(newBuffer + 1, value);
		writeString(newBuffer, std::size(newBuffer));
	}

	void Serializer::appendUint8(const uint8_t value) {
		char newBuffer[2]{ static_cast<char>(EtfType::Small_Integer_Ext), static_cast<char>(value) };
		writeString(newBuffer, std::size(newBuffer));
	}

	void Serializer::appendInt64(int64_t value) {
		char newBuffer[11]{ static_cast<char>(EtfType::Small_Big_Ext) };
		char encodedBytes{};
		while (value > 0) {
			newBuffer[3 + encodedBytes] = value & 0xFF;
			value >>= 8;
			++encodedBytes;
		}
		newBuffer[1] = encodedBytes;
		if (value >= 0) {
			newBuffer[2] = 0;
		} else {
			newBuffer[2] = 1;
		}
		writeString(newBuffer, 1ull + 2ull + static_cast<size_t>(encodedBytes));
	}

	void Serializer::appendInt32(const int32_t value) {
		char newBuffer[5]{ static_cast<char>(EtfType::Integer_Ext) };
		storeBits(newBuffer + 1, value);
		writeString(newBuffer, std::size(newBuffer));
	}

	void Serializer::appendInt8(const int8_t value) {
		char newBuffer[2]{ static_cast<char>(EtfType::Small_Integer_Ext), static_cast<char>(value) };
		writeString(newBuffer, std::size(newBuffer));
	}

	void Serializer::appendBool(bool data) {
		if (data) {
			char newBuffer[6]{ static_cast<char>(EtfType::Small_Atom_Ext), static_cast<char>(4), 't', 'r', 'u', 'e' };
			writeString(newBuffer, std::size(newBuffer));

		} else {
			char newBuffer[7]{ static_cast<char>(EtfType::Small_Atom_Ext), static_cast<char>(5), 'f', 'a', 'l', 's', 'e' };
			writeString(newBuffer, std::size(newBuffer));
		}
	}

	void Serializer::appendVersion() {
		char newBuffer[1]{ static_cast<char>(formatVersion) };
		writeString(newBuffer, std::size(newBuffer));
	}

	void Serializer::appendNilExt() {
		writeCharacter(static_cast<char>(EtfType::Nil_Ext));
	}

	void Serializer::appendNil() {
		char newBuffer[5]{ static_cast<char>(EtfType::Small_Atom_Ext), static_cast<char>(3), 'n', 'i', 'l' };
		writeString(newBuffer, std::size(newBuffer));
	}

	void Serializer::setValue(JsonType typeNew) {
		destroy();
		type = typeNew;
		switch (type) {
			case JsonType::Object: {
				AllocatorType<ObjectType> allocator{};
				jsonValue.object = AllocatorTraits<ObjectType>::allocate(allocator, 1);
				AllocatorTraits<ObjectType>::construct(allocator, jsonValue.object);
				break;
			}
			case JsonType::Array: {
				AllocatorType<ArrayType> allocator{};
				jsonValue.array = AllocatorTraits<ArrayType>::allocate(allocator, 1);
				AllocatorTraits<ArrayType>::construct(allocator, jsonValue.array);
				break;
			}
			case JsonType::String: {
				AllocatorType<StringType> allocator{};
				jsonValue.string = AllocatorTraits<StringType>::allocate(allocator, 1);
				AllocatorTraits<StringType>::construct(allocator, jsonValue.string);
				break;
			}
		}
	}

	void Serializer::destroy() noexcept {
		switch (type) {
			case JsonType::Object: {
				AllocatorType<ObjectType> allocator{};
				AllocatorTraits<ObjectType>::destroy(allocator, jsonValue.object);
				AllocatorTraits<ObjectType>::deallocate(allocator, jsonValue.object, 1);
				break;
			}
			case JsonType::Array: {
				AllocatorType<ArrayType> allocator{};
				AllocatorTraits<ArrayType>::destroy(allocator, jsonValue.array);
				AllocatorTraits<ArrayType>::deallocate(allocator, jsonValue.array, 1);
				break;
			}
			case JsonType::String: {
				AllocatorType<StringType> allocator{};
				AllocatorTraits<StringType>::destroy(allocator, jsonValue.string);
				AllocatorTraits<StringType>::deallocate(allocator, jsonValue.string, 1);
				break;
			}
		}
	}

	Serializer::~Serializer() {
		destroy();
	}
}