#include "Jsonifier.hpp"

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

	EnumConverter::operator std::vector<uint64_t>() noexcept {
		return this->vector;
	}

	EnumConverter::operator uint64_t() noexcept {
		return this->integer;
	}

	bool EnumConverter::isItAVector() noexcept {
		return this->vectorType;
	}

	Serializer& Serializer::operator=(Serializer&& data) noexcept {
		switch (data.type) {
			case JsonType::Object: {
				this->setValue(JsonType::Object);
				*this->jsonValue.object = std::move(*data.jsonValue.object);
				break;
			}
			case JsonType::Array: {
				this->setValue(JsonType::Array);
				*this->jsonValue.array = std::move(*data.jsonValue.array);
				break;
			}
			case JsonType::String: {
				this->setValue(JsonType::String);
				*this->jsonValue.string = std::move(*data.jsonValue.string);
				break;
			}
			case JsonType::Float: {
				this->jsonValue.numberDouble = data.jsonValue.numberDouble;
				break;
			}
			case JsonType::Uint64: {
				this->jsonValue.numberUint = data.jsonValue.numberUint;
				break;
			}
			case JsonType::Int64: {
				this->jsonValue.numberInt = data.jsonValue.numberInt;
				break;
			}
			case JsonType::Bool: {
				this->jsonValue.boolean = data.jsonValue.boolean;
				break;
			}
		}
		this->string = std::move(data.string);
		this->type = data.type;
		return *this;
	}

	Serializer& Serializer::operator=(const Serializer& data) noexcept {
		switch (data.type) {
			case JsonType::Object: {
				this->setValue(JsonType::Object);
				*this->jsonValue.object = *data.jsonValue.object;
				break;
			}
			case JsonType::Array: {
				this->setValue(JsonType::Array);
				*this->jsonValue.array = *data.jsonValue.array;
				break;
			}
			case JsonType::String: {
				this->setValue(JsonType::String);
				*this->jsonValue.string = *data.jsonValue.string;
				break;
			}
			case JsonType::Float: {
				this->jsonValue.numberDouble = data.jsonValue.numberDouble;
				break;
			}
			case JsonType::Uint64: {
				this->jsonValue.numberUint = data.jsonValue.numberUint;
				break;
			}
			case JsonType::Int64: {
				this->jsonValue.numberInt = data.jsonValue.numberInt;
				break;
			}
			case JsonType::Bool: {
				this->jsonValue.boolean = data.jsonValue.boolean;
				break;
			}
		}
		this->string = data.string;
		this->type = data.type;
		return *this;
	}

	Serializer::Serializer(const Serializer& data) noexcept {
		*this = data;
	}

	Serializer::operator std::string&&() noexcept {
		return std::move(this->string);
	}

	Serializer::operator std::string() noexcept {
		return this->string;
	}

	JsonType Serializer::getType() noexcept {
		return this->type;
	}

	void Serializer::refreshString(JsonifierSerializeType opCode) {
		this->string.clear();
		if (opCode == JsonifierSerializeType::Etf) {
			this->appendVersion();
			this->serializeJsonToEtfString(this);
		} else {
			this->serializeJsonToJsonString(this);
		}
	}

	Serializer& Serializer::operator=(EnumConverter&& data) noexcept {
		if (data.isItAVector()) {
			this->setValue(JsonType::Array);
			for (auto& value: data.operator std::vector<uint64_t>()) {
				this->jsonValue.array->emplace_back(std::move(value));
			}
		} else {
			this->jsonValue.numberUint = uint64_t{ data };
			this->type = JsonType::Uint64;
		}
		return *this;
	}

	Serializer::Serializer(EnumConverter&& data) noexcept {
		*this = std::move(data);
	}

	Serializer& Serializer::operator=(EnumConverter& data) noexcept {
		if (data.isItAVector()) {
			this->setValue(JsonType::Array);
			for (auto& value: data.operator std::vector<uint64_t>()) {
				this->jsonValue.array->emplace_back(value);
			}
		} else {
			this->jsonValue.numberUint = uint64_t{ data };
			this->type = JsonType::Uint64;
		}
		return *this;
	}

	Serializer::Serializer(EnumConverter& data) noexcept {
		*this = data;
	}

	Serializer& Serializer::operator=(std::string&& data) noexcept {
		this->setValue(JsonType::String);
		*this->jsonValue.string = std::move(data);
		this->type = JsonType::String;
		return *this;
	}

	Serializer::Serializer(std::string&& data) noexcept {
		*this = std::move(data);
	}

	Serializer& Serializer::operator=(const std::string& data) noexcept {
		this->setValue(JsonType::String);
		*this->jsonValue.string = data;
		this->type = JsonType::String;
		return *this;
	}

	Serializer::Serializer(const std::string& data) noexcept {
		*this = data;
	}

	Serializer& Serializer::operator=(const char* data) noexcept {
		this->setValue(JsonType::String);
		*this->jsonValue.string = data;
		this->type = JsonType::String;
		return *this;
	}

	Serializer::Serializer(const char* data) noexcept {
		*this = data;
	}

	Serializer& Serializer::operator=(double data) noexcept {
		this->jsonValue.numberDouble = data;
		this->type = JsonType::Float;
		return *this;
	}

	Serializer::Serializer(double data) noexcept {
		*this = data;
	}

	Serializer& Serializer::operator=(float data) noexcept {
		this->jsonValue.numberDouble = data;
		this->type = JsonType::Float;
		return *this;
	}

	Serializer::Serializer(float data) noexcept {
		*this = data;
	}

	Serializer& Serializer::operator=(uint64_t data) noexcept {
		this->jsonValue.numberUint = data;
		this->type = JsonType::Uint64;
		return *this;
	}

	Serializer::Serializer(uint64_t data) noexcept {
		*this = data;
	}

	Serializer& Serializer::operator=(uint32_t data) noexcept {
		this->jsonValue.numberUint = data;
		this->type = JsonType::Uint64;
		return *this;
	}

	Serializer::Serializer(uint32_t data) noexcept {
		*this = data;
	}

	Serializer& Serializer::operator=(uint16_t data) noexcept {
		this->jsonValue.numberUint = data;
		this->type = JsonType::Uint64;
		return *this;
	}

	Serializer::Serializer(uint16_t data) noexcept {
		*this = data;
	}

	Serializer& Serializer::operator=(uint8_t data) noexcept {
		this->jsonValue.numberUint = data;
		this->type = JsonType::Uint64;
		return *this;
	}

	Serializer::Serializer(uint8_t data) noexcept {
		*this = data;
	}

	Serializer& Serializer::operator=(int64_t data) noexcept {
		this->jsonValue.numberInt = data;
		this->type = JsonType::Int64;
		return *this;
	}

	Serializer::Serializer(int64_t data) noexcept {
		*this = data;
	}

	Serializer& Serializer::operator=(int32_t data) noexcept {
		this->jsonValue.numberInt = data;
		this->type = JsonType::Int64;
		return *this;
	}

	Serializer::Serializer(int32_t data) noexcept {
		*this = data;
	}

	Serializer& Serializer::operator=(int16_t data) noexcept {
		this->jsonValue.numberInt = data;
		this->type = JsonType::Int64;
		return *this;
	}

	Serializer::Serializer(int16_t data) noexcept {
		*this = data;
	}

	Serializer& Serializer::operator=(int8_t data) noexcept {
		this->jsonValue.numberInt = data;
		this->type = JsonType::Int64;
		return *this;
	}

	Serializer::Serializer(int8_t data) noexcept {
		*this = data;
	}

	Serializer& Serializer::operator=(std::nullptr_t) noexcept {
		this->type = JsonType::Null;
		return *this;
	}

	Serializer::Serializer(std::nullptr_t data) noexcept {
		*this = data;
	}

	Serializer& Serializer::operator=(bool data) noexcept {
		this->jsonValue.boolean = data;
		this->type = JsonType::Bool;
		return *this;
	}

	Serializer::Serializer(bool data) noexcept {
		*this = data;
	}

	Serializer& Serializer::operator=(JsonType typeNew) noexcept {
		this->type = typeNew;
		this->setValue(this->type);
		return *this;
	}

	Serializer::Serializer(JsonType type) noexcept {
		*this = type;
	}

	Serializer& Serializer::operator[](typename ObjectType::key_type key) {
		if (this->type == JsonType::Null) {
			this->setValue(JsonType::Object);
			this->type = JsonType::Object;
		}
		if (this->type == JsonType::Object) {
			auto result = this->jsonValue.object->emplace(std::move(key), Serializer{});
			return result.first->second;
		}
		throw JsonifierException{ "Sorry, but that item-key could not be produced/accessed." };
	}

	Serializer& Serializer::operator[](uint64_t index) {
		if (this->type == JsonType::Null) {
			this->setValue(JsonType::Array);
			this->type = JsonType::Array;
		}
		if (this->type == JsonType::Array) {
			if (index >= this->jsonValue.array->size()) {
				this->jsonValue.array->resize(index + 1);
			}
			return this->jsonValue.array->operator[](index);
		}
		throw JsonifierException{ "Sorry, but that index could not be produced/accessed." };
	}

	void Serializer::emplaceBack(Serializer&& other) noexcept {
		if (this->type == JsonType::Null) {
			this->setValue(JsonType::Array);
			this->type = JsonType::Array;
		}
		if (this->type == JsonType::Array) {
			this->jsonValue.array->emplace_back(std::move(other));
		}
	}

	void Serializer::emplaceBack(Serializer& other) noexcept {
		if (this->type == JsonType::Null) {
			this->setValue(JsonType::Array);
			this->type = JsonType::Array;
		}
		if (this->type == JsonType::Array) {
			this->jsonValue.array->emplace_back(other);
		}
	}

	void Serializer::serializeJsonToEtfString(const Serializer* dataToParse) {
		switch (dataToParse->type) {
			case JsonType::Object: {
				return this->writeEtfObject(*dataToParse->jsonValue.object);
			}
			case JsonType::Array: {
				return this->writeEtfArray(*dataToParse->jsonValue.array);
			}
			case JsonType::String: {
				return this->writeEtfString(*dataToParse->jsonValue.string);
			}
			case JsonType::Float: {
				return this->writeEtfFloat(dataToParse->jsonValue.numberDouble);
			}
			case JsonType::Uint64: {
				return this->writeEtfUint(dataToParse->jsonValue.numberUint);
			}
			case JsonType::Int64: {
				return this->writeEtfInt(dataToParse->jsonValue.numberInt);
			}
			case JsonType::Bool: {
				return this->writeEtfBool(dataToParse->jsonValue.boolean);
			}
			case JsonType::Null: {
				return this->writeEtfNull();
			}
		}
	}

	void Serializer::serializeJsonToJsonString(const Serializer* dataToParse) {
		switch (dataToParse->type) {
			case JsonType::Object: {
				return this->writeJsonObject(*dataToParse->jsonValue.object);
			}
			case JsonType::Array: {
				return this->writeJsonArray(*dataToParse->jsonValue.array);
			}
			case JsonType::String: {
				return this->writeJsonString(*dataToParse->jsonValue.string);
			}
			case JsonType::Float: {
				return this->writeJsonFloat(dataToParse->jsonValue.numberDouble);
			}
			case JsonType::Uint64: {
				return this->writeJsonInt(dataToParse->jsonValue.numberUint);
			}
			case JsonType::Int64: {
				return this->writeJsonInt(dataToParse->jsonValue.numberInt);
			}
			case JsonType::Bool: {
				return this->writeJsonBool(dataToParse->jsonValue.boolean);
			}
			case JsonType::Null: {
				return this->writeJsonNull();
			}
		}
	}

	void Serializer::writeJsonObject(const ObjectType& objectNew) {
		if (objectNew.empty()) {
			this->writeString("{}", 2);
			return;
		}
		this->writeCharacter('{');

		int32_t index{};
		for (auto& [key, value]: objectNew) {
			this->writeJsonString(key);
			this->writeCharacter(':');
			this->serializeJsonToJsonString(&value);

			if (index != objectNew.size() - 1) {
				this->writeCharacter(',');
			}
			++index;
		}
		this->writeCharacter('}');
	}

	void Serializer::writeJsonArray(const ArrayType& arrayNew) {
		if (arrayNew.empty()) {
			this->writeString("[]", 2);
			return;
		}
		this->writeCharacter('[');

		int32_t index{};
		for (auto& value: arrayNew) {
			this->serializeJsonToJsonString(&value);
			if (index != arrayNew.size() - 1) {
				this->writeCharacter(',');
			}
			++index;
		}
		this->writeCharacter(']');
	}

	void Serializer::writeJsonString(const StringType& stringNew) {
		this->writeCharacter('"');
		this->writeString(stringNew.data(), stringNew.size());
		this->writeCharacter('"');
	}

	void Serializer::writeJsonFloat(const FloatType x) {
		auto floatValue = std::to_string(x);
		this->writeString(floatValue.data(), floatValue.size());
	}

	void Serializer::writeJsonBool(const BoolType jsonValueNew) {
		if (jsonValueNew) {
			this->writeString("true", 4);
		} else {
			this->writeString("false", 5);
		}
	}

	void Serializer::writeJsonNull() {
		this->writeString("null", 4);
	}

	void Serializer::writeEtfObject(const ObjectType& jsonData) {
		this->appendMapHeader(static_cast<uint32_t>(jsonData.size()));
		for (auto& [key, value]: jsonData) {
			this->appendBinaryExt(key, static_cast<uint32_t>(key.size()));
			this->serializeJsonToEtfString(&value);
		}
	}

	void Serializer::writeEtfArray(const ArrayType& jsonData) {
		this->appendListHeader(static_cast<uint32_t>(jsonData.size()));
		for (auto& value: jsonData) {
			this->serializeJsonToEtfString(&value);
		}
		this->appendNilExt();
	}

	void Serializer::writeEtfString(const StringType& jsonData) {
		this->appendBinaryExt(jsonData, static_cast<uint32_t>(jsonData.size()));
	}

	void Serializer::writeEtfUint(const UintType jsonData) {
		if (jsonData >= std::numeric_limits<uint8_t>::min() && jsonData <= std::numeric_limits<uint8_t>::max()) {
			this->appendUint8(static_cast<uint8_t>(jsonData));
		} else if (jsonData >= std::numeric_limits<uint32_t>::min() && jsonData <= std::numeric_limits<uint32_t>::max()) {
			this->appendUint32(static_cast<uint32_t>(jsonData));
		} else {
			this->appendUint64(jsonData);
		}
	}

	void Serializer::writeEtfInt(const IntType jsonData) {
		if (jsonData >= std::numeric_limits<int8_t>::min() && jsonData <= std::numeric_limits<int8_t>::max()) {
			this->appendInt8(static_cast<int8_t>(jsonData));
		} else if (jsonData >= std::numeric_limits<int32_t>::min() && jsonData <= std::numeric_limits<int32_t>::max()) {
			this->appendInt32(static_cast<int32_t>(jsonData));
		} else {
			this->appendInt64(jsonData);
		}
	}

	void Serializer::writeEtfFloat(const FloatType jsonData) {
		this->appendNewFloatExt(jsonData);
	}

	void Serializer::writeEtfBool(const BoolType jsonData) {
		this->appendBool(jsonData);
	}

	void Serializer::writeEtfNull() {
		this->appendNil();
	}

	void Serializer::writeString(const char* data, size_t length) {
		this->string.append(data, length);
	}

	void Serializer::writeCharacter(const char charValue) {
		this->string.push_back(charValue);
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
		char newBuffer[5]{ static_cast<int8_t>(EtfType::Binary_Ext) };
		storeBits(newBuffer + 1, sizeNew);
		this->writeString(newBuffer, std::size(newBuffer));
		this->writeString(bytes.data(), bytes.size());
	}

	void Serializer::appendUint64(uint64_t value) {
		char newBuffer[11]{ static_cast<int8_t>(EtfType::Small_Big_Ext) };
		char encodedBytes{};
		while (value > 0) {
			newBuffer[3 + encodedBytes] = value & 0xFF;
			value >>= 8;
			++encodedBytes;
		}
		newBuffer[1] = encodedBytes;
		newBuffer[2] = 0;
		this->writeString(newBuffer, 1ull + 2ull + static_cast<size_t>(encodedBytes));
	}

	void Serializer::appendInt64(int64_t value) {
		char newBuffer[11]{ static_cast<int8_t>(EtfType::Small_Big_Ext) };
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
		this->writeString(newBuffer, 1ull + 2ull + static_cast<size_t>(encodedBytes));
	}

	void Serializer::appendNewFloatExt(const double FloatValue) {
		char newBuffer[9]{ static_cast<uint8_t>(EtfType::New_Float_Ext) };
		const void* punner{ &FloatValue };
		storeBits(newBuffer + 1, *static_cast<const uint64_t*>(punner));
		this->writeString(newBuffer, std::size(newBuffer));
	}

	void Serializer::appendUint8(const uint8_t value) {
		char newBuffer[2]{ static_cast<uint8_t>(EtfType::Small_Integer_Ext), static_cast<char>(value) };
		this->writeString(newBuffer, std::size(newBuffer));
	}

	void Serializer::appendInt8(const int8_t value) {
		char newBuffer[2]{ static_cast<uint8_t>(EtfType::Small_Integer_Ext), static_cast<char>(value) };
		this->writeString(newBuffer, std::size(newBuffer));
	}

	void Serializer::appendUint32(const uint32_t value) {
		char newBuffer[5]{ static_cast<uint8_t>(EtfType::Integer_Ext) };
		storeBits(newBuffer + 1, value);
		this->writeString(newBuffer, std::size(newBuffer));
	}

	void Serializer::appendInt32(const int32_t value) {
		char newBuffer[5]{ static_cast<uint8_t>(EtfType::Integer_Ext) };
		storeBits(newBuffer + 1, value);
		this->writeString(newBuffer, std::size(newBuffer));
	}

	void Serializer::appendListHeader(const uint32_t sizeNew) {
		char newBuffer[5]{ static_cast<uint8_t>(EtfType::List_Ext) };
		storeBits(newBuffer + 1, sizeNew);
		this->writeString(newBuffer, std::size(newBuffer));
	}

	void Serializer::appendMapHeader(const uint32_t sizeNew) {
		char newBuffer[5]{ static_cast<uint8_t>(EtfType::Map_Ext) };
		storeBits(newBuffer + 1, sizeNew);
		this->writeString(newBuffer, std::size(newBuffer));
	}

	void Serializer::appendBool(bool data) {
		if (data) {
			char newBuffer[6]{ static_cast<uint8_t>(EtfType::Small_Atom_Ext), static_cast<uint8_t>(4), 't', 'r', 'u', 'e' };
			this->writeString(newBuffer, std::size(newBuffer));
		} else {
			char newBuffer[7]{ static_cast<uint8_t>(EtfType::Small_Atom_Ext), static_cast<uint8_t>(5), 'f', 'a', 'l', 's', 'e' };
			this->writeString(newBuffer, std::size(newBuffer));
		}
	}

	void Serializer::appendVersion() {
		char newBuffer[1]{ static_cast<int8_t>(formatVersion) };
		this->writeString(newBuffer, std::size(newBuffer));
	}

	void Serializer::appendNilExt() {
		this->writeCharacter(static_cast<uint8_t>(EtfType::Nil_Ext));
	}

	void Serializer::appendNil() {
		char newBuffer[5]{ static_cast<uint8_t>(EtfType::Small_Atom_Ext), static_cast<uint8_t>(3), 'n', 'i', 'l' };
		this->writeString(newBuffer, std::size(newBuffer));
	}

	void Serializer::setValue(JsonType typeNew) {
		this->destroy();
		this->type = typeNew;
		switch (this->type) {
			case JsonType::Object: {
				AllocatorType<ObjectType> allocator{};
				this->jsonValue.object = AllocatorTraits<ObjectType>::allocate(allocator, 1);
				AllocatorTraits<ObjectType>::construct(allocator, this->jsonValue.object);
				break;
			}
			case JsonType::Array: {
				AllocatorType<ArrayType> allocator{};
				this->jsonValue.array = AllocatorTraits<ArrayType>::allocate(allocator, 1);
				AllocatorTraits<ArrayType>::construct(allocator, this->jsonValue.array);
				break;
			}
			case JsonType::String: {
				AllocatorType<StringType> allocator{};
				this->jsonValue.string = AllocatorTraits<StringType>::allocate(allocator, 1);
				AllocatorTraits<StringType>::construct(allocator, this->jsonValue.string);
				break;
			}
		}
	}

	size_t Serializer::size() noexcept {
		switch (this->type) {
			case JsonType::Object: {
				return this->jsonValue.object->size();
			}
			case JsonType::Array: {
				return this->jsonValue.array->size();
			}
			case JsonType::String: {
				return this->jsonValue.string->size();
			}
			default: {
				return 1;
			}
		}
	}

	void Serializer::destroy() noexcept {
		switch (this->type) {
			case JsonType::Object: {
				AllocatorType<ObjectType> allocator{};
				AllocatorTraits<ObjectType>::destroy(allocator, this->jsonValue.object);
				AllocatorTraits<ObjectType>::deallocate(allocator, this->jsonValue.object, 1);
				break;
			}
			case JsonType::Array: {
				AllocatorType<ArrayType> allocator{};
				AllocatorTraits<ArrayType>::destroy(allocator, this->jsonValue.array);
				AllocatorTraits<ArrayType>::deallocate(allocator, this->jsonValue.array, 1);
				break;
			}
			case JsonType::String: {
				AllocatorType<StringType> allocator{};
				AllocatorTraits<StringType>::destroy(allocator, this->jsonValue.string);
				AllocatorTraits<StringType>::deallocate(allocator, this->jsonValue.string, 1);
				break;
			}
		}
	}

	Serializer::~Serializer() noexcept {
		this->destroy();
	}
}