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
		switch (data.typeValue) {
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
		this->typeValue = data.typeValue;
		return *this;
	}

	Jsonifier::Jsonifier(Jsonifier&& data) noexcept {
		*this = std::move(data);
	}

	Jsonifier& Jsonifier::operator=(const Jsonifier& data) noexcept {
		switch (data.typeValue) {
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
			case JsonType::Int64: {
				this->jsonValue.numberInt = data.jsonValue.numberInt;
				break;
			}
			case JsonType::Uint64: {
				this->jsonValue.numberUint = data.jsonValue.numberUint;
				break;
			}
			case JsonType::Bool: {
				this->jsonValue.boolean = data.jsonValue.boolean;
				break;
			}
		}
		this->string = data.string;
		this->typeValue = data.typeValue;
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

	JsonType Jsonifier::type() noexcept {
		return this->typeValue;
	}

	void Jsonifier::refreshString(JsonifierSerializeType OpCode) {
		this->string.clear();
		if (OpCode == JsonifierSerializeType::Etf) {
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
			this->typeValue = JsonType::Uint64;
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
			this->typeValue = JsonType::Uint64;
		}
		return *this;
	}

	Jsonifier::Jsonifier(const EnumConverter& data) noexcept {
		*this = data;
	}

	Jsonifier& Jsonifier::operator=(std::string&& data) noexcept {
		this->setValue(JsonType::String);
		*this->jsonValue.string = std::move(data);
		this->typeValue = JsonType::String;
		return *this;
	}

	Jsonifier::Jsonifier(std::string&& data) noexcept {
		*this = std::move(data);
	}

	Jsonifier& Jsonifier::operator=(const std::string& data) noexcept {
		this->setValue(JsonType::String);
		*this->jsonValue.string = data;
		this->typeValue = JsonType::String;
		return *this;
	}

	Jsonifier::Jsonifier(const std::string& data) noexcept {
		*this = data;
	}

	Jsonifier& Jsonifier::operator=(const char* data) noexcept {
		this->setValue(JsonType::String);
		*this->jsonValue.string = data;
		this->typeValue = JsonType::String;
		return *this;
	}

	Jsonifier::Jsonifier(const char* data) noexcept {
		*this = data;
	}

	Jsonifier& Jsonifier::operator=(double data) noexcept {
		this->jsonValue.numberDouble = data;
		this->typeValue = JsonType::Float;
		return *this;
	}

	Jsonifier::Jsonifier(double data) noexcept {
		*this = data;
	}

	Jsonifier& Jsonifier::operator=(float data) noexcept {
		this->jsonValue.numberDouble = data;
		this->typeValue = JsonType::Float;
		return *this;
	}

	Jsonifier::Jsonifier(float data) noexcept {
		*this = data;
	}

	Jsonifier& Jsonifier::operator=(uint64_t data) noexcept {
		this->jsonValue.numberUint = data;
		this->typeValue = JsonType::Uint64;
		return *this;
	}

	Jsonifier::Jsonifier(uint64_t data) noexcept {
		*this = data;
	}

	Jsonifier& Jsonifier::operator=(uint32_t data) noexcept {
		this->jsonValue.numberUint = data;
		this->typeValue = JsonType::Uint64;
		return *this;
	}

	Jsonifier::Jsonifier(uint32_t data) noexcept {
		*this = data;
	}

	Jsonifier& Jsonifier::operator=(uint16_t data) noexcept {
		this->jsonValue.numberUint = data;
		this->typeValue = JsonType::Uint64;
		return *this;
	}

	Jsonifier::Jsonifier(uint16_t data) noexcept {
		*this = data;
	}

	Jsonifier& Jsonifier::operator=(uint8_t data) noexcept {
		this->jsonValue.numberUint = data;
		this->typeValue = JsonType::Uint64;
		return *this;
	}

	Jsonifier::Jsonifier(uint8_t data) noexcept {
		*this = data;
	}

	Jsonifier& Jsonifier::operator=(int64_t data) noexcept {
		this->jsonValue.numberInt = data;
		this->typeValue = JsonType::Int64;
		return *this;
	}

	Jsonifier::Jsonifier(int64_t data) noexcept {
		*this = data;
	}

	Jsonifier& Jsonifier::operator=(int32_t data) noexcept {
		this->jsonValue.numberInt = data;
		this->typeValue = JsonType::Int64;
		return *this;
	}

	Jsonifier::Jsonifier(int32_t data) noexcept {
		*this = data;
	}

	Jsonifier& Jsonifier::operator=(int16_t data) noexcept {
		this->jsonValue.numberInt = data;
		this->typeValue = JsonType::Int64;
		return *this;
	}

	Jsonifier::Jsonifier(int16_t data) noexcept {
		*this = data;
	}

	Jsonifier& Jsonifier::operator=(int8_t data) noexcept {
		this->jsonValue.numberInt = data;
		this->typeValue = JsonType::Int64;
		return *this;
	}

	Jsonifier::Jsonifier(int8_t data) noexcept {
		*this = data;
	}

	Jsonifier& Jsonifier::operator=(std::nullptr_t) noexcept {
		this->typeValue = JsonType::Null;
		return *this;
	}

	Jsonifier::Jsonifier(std::nullptr_t data) noexcept {
		*this = data;
	}

	Jsonifier& Jsonifier::operator=(bool data) noexcept {
		this->jsonValue.boolean = data;
		this->typeValue = JsonType::Bool;
		return *this;
	}

	Jsonifier::Jsonifier(bool data) noexcept {
		*this = data;
	}

	Jsonifier& Jsonifier::operator=(JsonType TypeNew) noexcept {
		this->typeValue = TypeNew;
		this->setValue(this->typeValue);
		return *this;
	}

	Jsonifier::Jsonifier(JsonType typeValue) noexcept {
		*this = typeValue;
	}

	Jsonifier& Jsonifier::operator[](typename ObjectType::key_type key) {
		if (this->typeValue == JsonType::Null) {
			this->setValue(JsonType::Object);
			this->typeValue = JsonType::Object;
		}

		if (this->typeValue == JsonType::Object) {
			auto result = this->jsonValue.object->emplace(std::move(key), Jsonifier{});
			return result.first->second;
		}
		throw std::runtime_error{ "Sorry, but that item-key could not be produced/accessed." };
	}

	Jsonifier& Jsonifier::operator[](uint64_t index) {
		if (this->typeValue == JsonType::Null) {
			this->setValue(JsonType::Array);
			this->typeValue = JsonType::Array;
		}

		if (this->typeValue == JsonType::Array) {
			if (index >= this->jsonValue.array->size()) {
				this->jsonValue.array->resize(index + 1);
			}

			return this->jsonValue.array->operator[](index);
		}
		throw std::runtime_error{ "Sorry, but that index could not be produced/accessed." };
	}

	template<> Jsonifier::ObjectType& Jsonifier::get() {
		return *this->jsonValue.object;
	}

	template<> Jsonifier::ArrayType& Jsonifier::get() {
		return *this->jsonValue.array;
	}

	template<> Jsonifier::StringType& Jsonifier::get() {
		return *this->jsonValue.string;
	}

	template<> Jsonifier::FloatType& Jsonifier::get() {
		return this->jsonValue.numberDouble;
	}

	template<> Jsonifier::UintType& Jsonifier::get() {
		return this->jsonValue.numberUint;
	}

	template<> Jsonifier::IntType& Jsonifier::get() {
		return this->jsonValue.numberInt;
	}

	template<> Jsonifier::BoolType& Jsonifier::get() {
		return this->jsonValue.boolean;
	}

	void Jsonifier::emplaceBack(Jsonifier&& data) noexcept {
		if (this->typeValue == JsonType::Null) {
			this->setValue(JsonType::Array);
			this->typeValue = JsonType::Array;
		}

		if (this->typeValue == JsonType::Array) {
			this->jsonValue.array->emplace_back(std::move(data));
		}
	}

	void Jsonifier::emplaceBack(Jsonifier& data) noexcept {
		if (this->typeValue == JsonType::Null) {
			this->setValue(JsonType::Array);
			this->typeValue = JsonType::Array;
		}

		if (this->typeValue == JsonType::Array) {
			this->jsonValue.array->emplace_back(data);
		}
	}

	void Jsonifier::serializeJsonToEtfString(const Jsonifier* dataToParse) {
		switch (dataToParse->typeValue) {
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
		switch (dataToParse->typeValue) {
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

	void Jsonifier::writeJsonObject(const Jsonifier::ObjectType& ObjectNew) {
		if (ObjectNew.empty()) {
			this->writeString("{}", 2);
			return;
		}
		this->writeCharacter('{');

		int32_t Index{};
		for (auto& [key, value]: ObjectNew) {
			this->writeJsonString(key);
			this->writeCharacter(':');
			this->serializeJsonToJsonString(&value);

			if (Index != ObjectNew.size() - 1) {
				this->writeCharacter(',');
			}
			Index++;
		}

		this->writeCharacter('}');
	}

	void Jsonifier::writeJsonArray(const Jsonifier::ArrayType& Array) {
		if (Array.empty()) {
			this->writeString("[]", 2);
			return;
		}

		this->writeCharacter('[');

		int32_t Index{};
		for (auto& value: Array) {
			this->serializeJsonToJsonString(&value);
			if (Index != Array.size() - 1) {
				this->writeCharacter(',');
			}
			Index++;
		}

		this->writeCharacter(']');
	}

	void Jsonifier::writeJsonString(const Jsonifier::StringType& StringNew) {
		this->writeCharacter('\"');
		for (auto& value: StringNew) {
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

	void Jsonifier::writeJsonFloat(const Jsonifier::FloatType x) {
		auto newFloat = std::to_string(x);
		this->writeString(newFloat.data(), newFloat.size());
	}

	void Jsonifier::writeJsonBool(const Jsonifier::BoolType ValueNew) {
		if (ValueNew) {
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

	void Jsonifier::writeCharacter(const char Char) {
		this->string.push_back(Char);
	}

	bool operator==(const Jsonifier& lhs, const Jsonifier& rhs) {
		if (lhs.typeValue != rhs.typeValue) {
			return false;
		}
		switch (rhs.typeValue) {
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
		std::string bufferNew{ static_cast<uint8_t>(EtfType::Binary_Ext) };
		storeBits(bufferNew, sizeNew);
		this->writeString(bufferNew.data(), bufferNew.size());
		this->writeString(bytes.data(), bytes.size());
	}

	void Jsonifier::appendUnsignedLongLong(const uint64_t value) {
		std::string bufferNew{};
		uint64_t ValueNew = value;
		bufferNew.resize(static_cast<uint64_t>(1) + 2 + sizeof(uint64_t));
		bufferNew[0] = static_cast<uint8_t>(EtfType::Small_Big_Ext);
		StopWatch StopWatch{ std::chrono::milliseconds{ 1500 } };
		uint8_t bytesToEncode = 0;
		while (ValueNew > 0) {
			if (StopWatch.hasTimePassed()) {
				break;
			}
			bufferNew[static_cast<uint64_t>(3) + bytesToEncode] = ValueNew & 0xF;
			ValueNew >>= 8;
			bytesToEncode++;
		}
		bufferNew[1] = bytesToEncode;
		bufferNew[2] = 0;
		this->writeString(bufferNew.data(), bufferNew.size());
	}

	void Jsonifier::appendNewFloatExt(const double FloatValue) {
		std::string bufferNew{ static_cast<unsigned char>(EtfType::New_Float_Ext) };
		const void* punner{ &FloatValue };
		storeBits(bufferNew, *static_cast<const uint64_t*>(punner));
		this->writeString(bufferNew.data(), bufferNew.size());
	}

	void Jsonifier::appendSmallIntegerExt(const uint8_t value) {
		std::string bufferNew{ static_cast<uint8_t>(EtfType::Small_Integer_Ext), static_cast<char>(value) };
		this->writeString(bufferNew.data(), bufferNew.size());
	}

	void Jsonifier::appendIntegerExt(const uint32_t value) {
		std::string bufferNew{ static_cast<uint8_t>(EtfType::Integer_Ext) };
		storeBits(bufferNew, value);
		this->writeString(bufferNew.data(), bufferNew.size());
	}

	void Jsonifier::appendListHeader(const uint32_t sizeNew) {
		std::string bufferNew{ static_cast<uint8_t>(EtfType::List_Ext) };
		storeBits(bufferNew, sizeNew);
		this->writeString(bufferNew.data(), bufferNew.size());
	}

	void Jsonifier::appendMapHeader(const uint32_t sizeNew) {
		std::string bufferNew{ static_cast<uint8_t>(EtfType::Map_Ext) };
		storeBits(bufferNew, sizeNew);
		this->writeString(bufferNew.data(), bufferNew.size());
	}

	void Jsonifier::appendBool(bool data) {
		if (data) {
			this->writeCharacter(static_cast<uint8_t>(EtfType::Small_Atom_Ext));
			this->writeCharacter(4);
			this->writeString("true", 4);

		} else {
			this->writeCharacter(static_cast<uint8_t>(EtfType::Small_Atom_Ext));
			this->writeCharacter(5);
			this->writeString("false", 5);
		}
	}

	void Jsonifier::appendVersion() {
		std::string bufferNew{ static_cast<int8_t>(formatVersion) };
		this->writeString(bufferNew.data(), bufferNew.size());
	}

	void Jsonifier::appendNilExt() {
		this->writeCharacter(static_cast<uint8_t>(EtfType::Nil_Ext));
	}

	void Jsonifier::appendNil() {
		this->writeCharacter(static_cast<uint8_t>(EtfType::Small_Atom_Ext));
		this->writeCharacter(3);
		this->writeString("nil", 3);
	}

	void Jsonifier::setValue(JsonType TypeNew) {
		this->destroy();
		this->typeValue = TypeNew;
		switch (this->typeValue) {
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
		switch (this->typeValue) {
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