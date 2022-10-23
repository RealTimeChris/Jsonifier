#include "../Include/Jsonifier.hpp"

namespace Jsonifier {

	EnumConverter::operator std::vector<uint64_t>() const noexcept {
		return this->vector;
	}

	EnumConverter::operator uint64_t() const noexcept {
		return this->integer;
	}

	bool EnumConverter::isItAVector() const noexcept {
		return this->vectorType;
	}

	Jsonifier& Jsonifier::operator=(Jsonifier&& data) noexcept {
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

	Jsonifier::Jsonifier(Jsonifier&& data) noexcept {
		*this = std::move(data);
	}

	Jsonifier& Jsonifier::operator=(const Jsonifier& data) noexcept {
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

	Jsonifier::Jsonifier(const Jsonifier& data) noexcept {
		*this = data;
	}

	Jsonifier::operator std::string&&() noexcept {
		return std::move(this->string);
	}

	Jsonifier::operator std::string() noexcept {
		return this->string;
	}

	JsonType Jsonifier::getType() noexcept {
		return this->type;
	}

	void Jsonifier::refreshString(JsonifierSerializeType opCode) {
		this->string.clear();
		if (opCode == JsonifierSerializeType::Etf) {
			this->appendVersion();
			this->serializeJsonToEtfString(this);
		} else {
			this->serializeJsonToJsonString(this);
		}
	}

	Jsonifier& Jsonifier::operator=(EnumConverter&& data) noexcept {
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

	Jsonifier::Jsonifier(EnumConverter&& data) noexcept {
		*this = std::move(data);
	}

	Jsonifier& Jsonifier::operator=(const EnumConverter& data) noexcept {
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

	Jsonifier::Jsonifier(const EnumConverter& data) noexcept {
		*this = data;
	}

	Jsonifier& Jsonifier::operator=(std::string&& data) noexcept {
		this->setValue(JsonType::String);
		*this->jsonValue.string = std::move(data);
		this->type = JsonType::String;
		return *this;
	}

	Jsonifier::Jsonifier(std::string&& data) noexcept {
		*this = std::move(data);
	}

	Jsonifier& Jsonifier::operator=(const std::string& data) noexcept {
		this->setValue(JsonType::String);
		*this->jsonValue.string = data;
		this->type = JsonType::String;
		return *this;
	}

	Jsonifier::Jsonifier(const std::string& data) noexcept {
		*this = data;
	}

	Jsonifier& Jsonifier::operator=(const char* data) noexcept {
		this->setValue(JsonType::String);
		*this->jsonValue.string = data;
		this->type = JsonType::String;
		return *this;
	}

	Jsonifier::Jsonifier(const char* data) noexcept {
		*this = data;
	}

	Jsonifier& Jsonifier::operator=(double data) noexcept {
		this->jsonValue.numberDouble = data;
		this->type = JsonType::Float;
		return *this;
	}

	Jsonifier::Jsonifier(double data) noexcept {
		*this = data;
	}

	Jsonifier& Jsonifier::operator=(float data) noexcept {
		this->jsonValue.numberDouble = data;
		this->type = JsonType::Float;
		return *this;
	}

	Jsonifier::Jsonifier(float data) noexcept {
		*this = data;
	}

	Jsonifier& Jsonifier::operator=(uint64_t data) noexcept {
		this->jsonValue.numberUint = data;
		this->type = JsonType::Uint64;
		return *this;
	}

	Jsonifier::Jsonifier(uint64_t data) noexcept {
		*this = data;
	}

	Jsonifier& Jsonifier::operator=(uint32_t data) noexcept {
		this->jsonValue.numberUint = data;
		this->type = JsonType::Uint64;
		return *this;
	}

	Jsonifier::Jsonifier(uint32_t data) noexcept {
		*this = data;
	}

	Jsonifier& Jsonifier::operator=(uint16_t data) noexcept {
		this->jsonValue.numberUint = data;
		this->type = JsonType::Uint64;
		return *this;
	}

	Jsonifier::Jsonifier(uint16_t data) noexcept {
		*this = data;
	}

	Jsonifier& Jsonifier::operator=(uint8_t data) noexcept {
		this->jsonValue.numberUint = data;
		this->type = JsonType::Uint64;
		return *this;
	}

	Jsonifier::Jsonifier(uint8_t data) noexcept {
		*this = data;
	}

	Jsonifier& Jsonifier::operator=(int64_t data) noexcept {
		this->jsonValue.numberInt = data;
		this->type = JsonType::Int64;
		return *this;
	}

	Jsonifier::Jsonifier(int64_t data) noexcept {
		*this = data;
	}

	Jsonifier& Jsonifier::operator=(int32_t data) noexcept {
		this->jsonValue.numberInt = data;
		this->type = JsonType::Int64;
		return *this;
	}

	Jsonifier::Jsonifier(int32_t data) noexcept {
		*this = data;
	}

	Jsonifier& Jsonifier::operator=(int16_t data) noexcept {
		this->jsonValue.numberInt = data;
		this->type = JsonType::Int64;
		return *this;
	}

	Jsonifier::Jsonifier(int16_t data) noexcept {
		*this = data;
	}

	Jsonifier& Jsonifier::operator=(int8_t data) noexcept {
		this->jsonValue.numberInt = data;
		this->type = JsonType::Int64;
		return *this;
	}

	Jsonifier::Jsonifier(int8_t data) noexcept {
		*this = data;
	}

	Jsonifier& Jsonifier::operator=(std::nullptr_t) noexcept {
		this->type = JsonType::Null;
		return *this;
	}

	Jsonifier::Jsonifier(std::nullptr_t data) noexcept {
		*this = data;
	}

	Jsonifier& Jsonifier::operator=(bool data) noexcept {
		this->jsonValue.boolean = data;
		this->type = JsonType::Bool;
		return *this;
	}

	Jsonifier::Jsonifier(bool data) noexcept {
		*this = data;
	}

	Jsonifier& Jsonifier::operator=(JsonType typeNew) noexcept {
		this->type = typeNew;
		this->setValue(this->type);
		return *this;
	}

	Jsonifier::Jsonifier(JsonType type) noexcept {
		*this = type;
	}

	Jsonifier& Jsonifier::operator[](typename ObjectType::key_type key) {
		if (this->type == JsonType::Null) {
			this->setValue(JsonType::Object);
			this->type = JsonType::Object;
		}

		if (this->type == JsonType::Object) {
			auto result = this->jsonValue.object->emplace(std::move(key), Jsonifier{});
			return result.first->second;
		}
		throw std::runtime_error{ "Sorry, but that item-key could not be produced/accessed." };
	}

	Jsonifier& Jsonifier::operator[](uint64_t index) {
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
		throw std::runtime_error{ "Sorry, but that index could not be produced/accessed." };
	}

	void Jsonifier::emplaceBack(Jsonifier&& other) noexcept {
		if (this->type == JsonType::Null) {
			this->setValue(JsonType::Array);
			this->type = JsonType::Array;
		}

		if (this->type == JsonType::Array) {
			this->jsonValue.array->emplace_back(std::move(other));
		}
	}

	void Jsonifier::emplaceBack(Jsonifier& other) noexcept {
		if (this->type == JsonType::Null) {
			this->setValue(JsonType::Array);
			this->type = JsonType::Array;
		}

		if (this->type == JsonType::Array) {
			this->jsonValue.array->emplace_back(other);
		}
	}

	void Jsonifier::serializeJsonToEtfString(const Jsonifier* dataToParse) {
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

	void Jsonifier::serializeJsonToJsonString(const Jsonifier* dataToParse) {
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

	void Jsonifier::writeJsonObject(const ObjectType& objectNew) {
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
			index++;
		}

		this->writeCharacter('}');
	}

	void Jsonifier::writeJsonArray(const ArrayType& arrayNew) {
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
			index++;
		}

		this->writeCharacter(']');
	}

	void Jsonifier::writeJsonString(const StringType& stringNew) {
		this->writeCharacter('\"');
		for (auto& value: stringNew) {
			switch (static_cast<std::uint8_t>(value)) {
				case 0x08: {
					this->writeCharacter('b');
					break;
				}
				case 0x09: {
					this->writeCharacter('t');
					break;
				}
				case 0x0A: {
					this->writeCharacter('n');
					break;
				}
				case 0x0C: {
					this->writeCharacter('f');
					break;
				}
				case 0x0D: {
					this->writeCharacter('r');
					break;
				}
				case 0x22: {
					this->writeCharacter('\"');
					break;
				}
				case 0x5C: {
					this->writeCharacter('\\');
					break;
				}
				default: {
					this->writeCharacter(value);
					break;
				}
			}
		}
		this->writeCharacter('\"');
	}

	void Jsonifier::writeJsonFloat(const FloatType x) {
		auto floatValue = std::to_string(x);
		this->writeString(floatValue.data(), floatValue.size());
	}

	void Jsonifier::writeJsonBool(const BoolType jsonValueNew) {
		if (jsonValueNew) {
			this->writeString("true", 4);
		} else {
			this->writeString("false", 5);
		}
	}

	void Jsonifier::writeJsonNull() {
		this->writeString("null", 4);
	}

	void Jsonifier::writeEtfObject(const ObjectType& jsonData) {
		this->appendMapHeader(static_cast<uint32_t>(jsonData.size()));
		for (auto& [key, value]: jsonData) {
			this->appendBinaryExt(key, static_cast<uint32_t>(key.size()));
			this->serializeJsonToEtfString(&value);
		}
	}

	void Jsonifier::writeEtfArray(const ArrayType& jsonData) {
		this->appendListHeader(static_cast<uint32_t>(jsonData.size()));
		for (auto& value: jsonData) {
			this->serializeJsonToEtfString(&value);
		}
		this->appendNilExt();
	}

	void Jsonifier::writeEtfString(const StringType& jsonData) {
		this->appendBinaryExt(jsonData, static_cast<uint32_t>(jsonData.size()));
	}

	void Jsonifier::writeEtfUint(const UintType jsonData) {
		if (jsonData <= 255) {
			this->appendSmallIntegerExt(static_cast<uint8_t>(jsonData));
		} else if (jsonData <= std::numeric_limits<uint32_t>::max()) {
			this->appendIntegerExt(static_cast<uint32_t>(jsonData));
		} else {
			this->appendUnsignedLongLong(jsonData);
		}
	}

	void Jsonifier::writeEtfInt(const IntType jsonData) {
		if (jsonData <= 127 && jsonData >= -127) {
			this->appendSmallIntegerExt(static_cast<uint8_t>(jsonData));
		} else if (jsonData <= std::numeric_limits<int32_t>::max() && jsonData >= std::numeric_limits<int32_t>::min()) {
			this->appendIntegerExt(static_cast<uint32_t>(jsonData));
		} else {
			this->appendUnsignedLongLong(static_cast<uint64_t>(jsonData));
		}
	}

	void Jsonifier::writeEtfFloat(const FloatType jsonData) {
		this->appendNewFloatExt(jsonData);
	}

	void Jsonifier::writeEtfBool(const BoolType jsonData) {
		this->appendBool(jsonData);
	}

	void Jsonifier::writeEtfNull() {
		this->appendNil();
	}

	void Jsonifier::writeString(const char* data, std::size_t length) {
		this->string.append(data, length);
	}

	void Jsonifier::writeCharacter(const char charValue) {
		this->string.push_back(charValue);
	}

	bool operator==(const Jsonifier& lhs, const Jsonifier& rhs) {
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

	void Jsonifier::appendBinaryExt(const std::string& bytes, uint32_t sizeNew) {
		char newBuffer[5]{};
		newBuffer[0] = static_cast<int8_t>(EtfType::Binary_Ext);
		storeBits(newBuffer + 1, sizeNew);
		this->writeString(newBuffer, std::size(newBuffer));
		this->writeString(bytes.data(), bytes.size());
	}

	void Jsonifier::appendUnsignedLongLong(uint64_t value) {
		char newBuffer[11];
		newBuffer[0] = static_cast<int8_t>(EtfType::Small_Big_Ext);

		char encodedBytes{};
		while (value > 0) {
			newBuffer[3 + encodedBytes] = value & 0xFF;
			value >>= 8;
			encodedBytes++;
		}
		newBuffer[1] = encodedBytes;
		newBuffer[2] = 0;

		this->writeString(newBuffer, 1 + 2 + encodedBytes);
	}

	void Jsonifier::appendNewFloatExt(const double FloatValue) {
		char newBuffer[9]{};
		newBuffer[0] = static_cast<unsigned char>(EtfType::New_Float_Ext);
		const void* punner{ &FloatValue };
		storeBits(newBuffer + 1, *static_cast<const uint64_t*>(punner));
		this->writeString(newBuffer, std::size(newBuffer));
	}

	void Jsonifier::appendSmallIntegerExt(const uint8_t value) {
		char newBuffer[2]{};
		newBuffer[0] = static_cast<uint8_t>(EtfType::Small_Integer_Ext);
		newBuffer[1] = value;
		this->writeString(newBuffer, std::size(newBuffer));
	}

	void Jsonifier::appendIntegerExt(const uint32_t value) {
		char newBuffer[5]{};
		newBuffer[0] = static_cast<uint8_t>(EtfType::Integer_Ext);
		storeBits(newBuffer + 1, value);
		this->writeString(newBuffer, std::size(newBuffer));
	}

	void Jsonifier::appendListHeader(const uint32_t sizeNew) {
		char newBuffer[5]{};
		newBuffer[0] = static_cast<uint8_t>(EtfType::List_Ext);
		storeBits(newBuffer + 1, sizeNew);
		this->writeString(newBuffer, std::size(newBuffer));
	}

	void Jsonifier::appendMapHeader(const uint32_t sizeNew) {
		char newBuffer[5]{};
		newBuffer[0] = static_cast<uint8_t>(EtfType::Map_Ext);
		storeBits(newBuffer + 1, sizeNew);
		this->writeString(newBuffer, std::size(newBuffer));
	}

	void Jsonifier::appendBool(bool data) {
		if (data) {
			char newBuffer[3]{};
			newBuffer[0] = static_cast<uint8_t>(EtfType::Atom_Ext);
			storeBits(newBuffer + 1, static_cast<uint16_t>(4));
			this->writeString(newBuffer, std::size(newBuffer));
			this->writeString("true", 4);

		} else {
			char newBuffer[3]{};
			newBuffer[0] = static_cast<uint8_t>(EtfType::Atom_Ext);
			storeBits(newBuffer + 1, static_cast<uint16_t>(5));
			this->writeString(newBuffer, std::size(newBuffer));
			this->writeString("false", 5);
		}
	}

	void Jsonifier::appendVersion() {
		char newBuffer[1]{};
		newBuffer[0] = static_cast<int8_t>(formatVersion);
		this->writeString(newBuffer, std::size(newBuffer));
	}

	void Jsonifier::appendNilExt() {
		this->writeCharacter(static_cast<uint8_t>(EtfType::Nil_Ext));
	}

	void Jsonifier::appendNil() {
		char newBuffer[3]{};
		newBuffer[0] = static_cast<uint8_t>(EtfType::Atom_Ext);
		storeBits(newBuffer + 1, static_cast<uint16_t>(3));
		this->writeString(newBuffer, std::size(newBuffer));
		this->writeString("nil", 3);
	}

	void Jsonifier::setValue(JsonType typeNew) {
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

	void Jsonifier::destroy() noexcept {
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

	Jsonifier::~Jsonifier() noexcept {
		this->destroy();
	}
}