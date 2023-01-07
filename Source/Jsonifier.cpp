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

	EnumConverter::operator std::vector<uint64_t>() noexcept {
		return this->vector;
	}

	EnumConverter::operator uint64_t() noexcept {
		return this->integer;
	}

	bool EnumConverter::isItAVector() noexcept {
		return this->vectorType;
	}

	inline Serializer& Serializer::operator=(Serializer&& data) noexcept {
		switch (data.type) {
			case JsonType::Object: {
				this->setValue(JsonType::Object);
				this->jsonValue.object = data.jsonValue.object;
				data.jsonValue.object = nullptr;
				data.type = JsonType::Null;
				break;
			}
			case JsonType::Array: {
				this->setValue(JsonType::Array);
				this->jsonValue.array = data.jsonValue.array;
				data.jsonValue.array = nullptr;
				data.type = JsonType::Null;
				break;
			}
			case JsonType::String: {
				this->setValue(JsonType::String);
				this->jsonValue.string = data.jsonValue.string;
				data.jsonValue.string = nullptr;
				data.type = JsonType::Null;
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

	inline Serializer& Serializer::operator=(const Serializer& data) noexcept {
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

	inline Serializer& Serializer::operator=(EnumConverter&& data) noexcept {
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

	inline Serializer& Serializer::operator=(EnumConverter& data) noexcept {
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

	inline Serializer& Serializer::operator=(std::string&& data) noexcept {
		this->setValue(JsonType::String);
		*this->jsonValue.string = std::move(data);
		this->type = JsonType::String;
		return *this;
	}

	Serializer::Serializer(std::string&& data) noexcept {
		*this = std::move(data);
	}

	inline Serializer& Serializer::operator=(const std::string& data) noexcept {
		this->setValue(JsonType::String);
		*this->jsonValue.string = data;
		this->type = JsonType::String;
		return *this;
	}

	Serializer::Serializer(const std::string& data) noexcept {
		*this = data;
	}

	inline Serializer& Serializer::operator=(const char* data) noexcept {
		this->setValue(JsonType::String);
		*this->jsonValue.string = data;
		this->type = JsonType::String;
		return *this;
	}

	Serializer::Serializer(const char* data) noexcept {
		*this = data;
	}

	inline Serializer& Serializer::operator=(double data) noexcept {
		this->jsonValue.numberDouble = data;
		this->type = JsonType::Float;
		return *this;
	}

	Serializer::Serializer(double data) noexcept {
		*this = data;
	}

	inline Serializer& Serializer::operator=(float data) noexcept {
		this->jsonValue.numberDouble = data;
		this->type = JsonType::Float;
		return *this;
	}

	Serializer::Serializer(float data) noexcept {
		*this = data;
	}

	inline Serializer& Serializer::operator=(uint64_t data) noexcept {
		this->jsonValue.numberUint = data;
		this->type = JsonType::Uint64;
		return *this;
	}

	Serializer::Serializer(uint64_t data) noexcept {
		*this = data;
	}

	inline Serializer& Serializer::operator=(uint32_t data) noexcept {
		this->jsonValue.numberUint = data;
		this->type = JsonType::Uint64;
		return *this;
	}

	Serializer::Serializer(uint32_t data) noexcept {
		*this = data;
	}

	inline Serializer& Serializer::operator=(uint16_t data) noexcept {
		this->jsonValue.numberUint = data;
		this->type = JsonType::Uint64;
		return *this;
	}

	Serializer::Serializer(uint16_t data) noexcept {
		*this = data;
	}

	inline Serializer& Serializer::operator=(uint8_t data) noexcept {
		this->jsonValue.numberUint = data;
		this->type = JsonType::Uint64;
		return *this;
	}

	Serializer::Serializer(uint8_t data) noexcept {
		*this = data;
	}

	inline Serializer& Serializer::operator=(int64_t data) noexcept {
		this->jsonValue.numberInt = data;
		this->type = JsonType::Int64;
		return *this;
	}

	Serializer::Serializer(int64_t data) noexcept {
		*this = data;
	}

	inline Serializer& Serializer::operator=(int32_t data) noexcept {
		this->jsonValue.numberInt = data;
		this->type = JsonType::Int64;
		return *this;
	}

	Serializer::Serializer(int32_t data) noexcept {
		*this = data;
	}

	inline Serializer& Serializer::operator=(int16_t data) noexcept {
		this->jsonValue.numberInt = data;
		this->type = JsonType::Int64;
		return *this;
	}

	Serializer::Serializer(int16_t data) noexcept {
		*this = data;
	}

	inline Serializer& Serializer::operator=(int8_t data) noexcept {
		this->jsonValue.numberInt = data;
		this->type = JsonType::Int64;
		return *this;
	}

	Serializer::Serializer(int8_t data) noexcept {
		*this = data;
	}

	inline Serializer& Serializer::operator=(std::nullptr_t) noexcept {
		this->type = JsonType::Null;
		return *this;
	}

	Serializer::Serializer(std::nullptr_t data) noexcept {
		*this = data;
	}

	inline Serializer& Serializer::operator=(bool data) noexcept {
		this->jsonValue.boolean = data;
		this->type = JsonType::Bool;
		return *this;
	}

	Serializer::Serializer(bool data) noexcept {
		*this = data;
	}

	inline Serializer& Serializer::operator=(JsonType typeNew) noexcept {
		this->type = typeNew;
		this->setValue(this->type);
		return *this;
	}

	Serializer::Serializer(JsonType type) noexcept {
		*this = type;
	}

	inline Serializer& Serializer::operator[](typename ObjectType::key_type key) {
		if (this->type == JsonType::Null) {
			this->setValue(JsonType::Object);
			this->type = JsonType::Object;
		}

		if (this->type == JsonType::Object) {
			auto result = this->jsonValue.object->emplace(std::move(key), Serializer{});
			return result.first->second;
		}
		throw JsonifierError{ "Sorry, but that item-key could not be produced/accessed." };
	}

	inline Serializer& Serializer::operator[](uint64_t index) {
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
		throw JsonifierError{ "Sorry, but that index could not be produced/accessed." };
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

	void Serializer::emplaceBack(const Serializer& other) noexcept {
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
		char newBuffer[5]{ static_cast<char>(EtfType::Binary_Ext) };
		storeBits(newBuffer + 1, sizeNew);
		this->writeString(newBuffer, std::size(newBuffer));
		this->writeString(bytes.data(), bytes.size());
	}

	void Serializer::appendNewFloatExt(const double FloatValue) {
		char newBuffer[9]{ static_cast<char>(EtfType::New_Float_Ext) };
		const void* punner{ &FloatValue };
		storeBits(newBuffer + 1, *static_cast<const uint64_t*>(punner));
		this->writeString(newBuffer, std::size(newBuffer));
	}

	void Serializer::appendListHeader(const uint32_t sizeNew) {
		char newBuffer[5]{ static_cast<char>(EtfType::List_Ext) };
		storeBits(newBuffer + 1, sizeNew);
		this->writeString(newBuffer, std::size(newBuffer));
	}

	void Serializer::appendMapHeader(const uint32_t sizeNew) {
		char newBuffer[5]{ static_cast<char>(EtfType::Map_Ext) };
		storeBits(newBuffer + 1, sizeNew);
		this->writeString(newBuffer, std::size(newBuffer));
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
		this->writeString(newBuffer, 1ull + 2 + static_cast<size_t>(encodedBytes));
	}

	void Serializer::appendUint32(const uint32_t value) {
		char newBuffer[5]{ static_cast<char>(EtfType::Integer_Ext) };
		storeBits(newBuffer + 1, value);
		this->writeString(newBuffer, std::size(newBuffer));
	}

	void Serializer::appendUint8(const uint8_t value) {
		char newBuffer[2]{ static_cast<char>(EtfType::Small_Integer_Ext), static_cast<char>(value) };
		this->writeString(newBuffer, std::size(newBuffer));
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
		this->writeString(newBuffer, 1ull + 2 + static_cast<size_t>(encodedBytes));
	}

	void Serializer::appendInt32(const int32_t value) {
		char newBuffer[5]{ static_cast<char>(EtfType::Integer_Ext) };
		storeBits(newBuffer + 1, value);
		this->writeString(newBuffer, std::size(newBuffer));
	}

	void Serializer::appendInt8(const int8_t value) {
		char newBuffer[2]{ static_cast<char>(EtfType::Small_Integer_Ext), static_cast<char>(value) };
		this->writeString(newBuffer, std::size(newBuffer));
	}

	void Serializer::appendBool(bool data) {
		if (data) {
			char newBuffer[6]{ static_cast<char>(EtfType::Small_Atom_Ext), static_cast<char>(4), 't', 'r', 'u', 'e' };
			this->writeString(newBuffer, std::size(newBuffer));
		} else {
			char newBuffer[7]{ static_cast<char>(EtfType::Small_Atom_Ext), static_cast<char>(5), 'f', 'a', 'l', 's', 'e' };
			this->writeString(newBuffer, std::size(newBuffer));
		}
	}

	void Serializer::appendVersion() {
		char newBuffer[1]{ static_cast<char>(formatVersion) };
		this->writeString(newBuffer, std::size(newBuffer));
	}

	void Serializer::appendNilExt() {
		this->writeCharacter(static_cast<char>(EtfType::Nil_Ext));
	}

	void Serializer::appendNil() {
		char newBuffer[5]{ static_cast<char>(EtfType::Small_Atom_Ext), static_cast<char>(3), 'n', 'i', 'l' };
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

	Serializer::~Serializer() {
		this->destroy();
	}

	Document Parser::parseJson(std::string_view string) {
		this->generateJsonIndices(reinterpret_cast<const uint8_t*>(string.data()), string.size());
		return std::forward<Document>(JsonIterator{ this });
	}

	Document Parser::parseJson(const char* string, size_t stringLength) {
		this->generateJsonIndices(reinterpret_cast<const uint8_t*>(string), stringLength);
		return std::forward<Document>(JsonIterator{ this });
	}

	Document Parser::parseJson(const std::string& string) {
		this->generateJsonIndices(reinterpret_cast<const uint8_t*>(string.data()), string.size());
		return std::forward<Document>(JsonIterator{ this });
	}

	RawJsonString::RawJsonString(const uint8_t* _buf) noexcept : stringView{ _buf } {
	}

	const char* RawJsonString::raw() const noexcept {
		return reinterpret_cast<const char*>(stringView);
	}

	bool RawJsonString::unsafeIsEqual(size_t length, std::string_view target) const noexcept {
		return (length >= target.size()) && (raw()[target.size()] == '"') && !memcmp(raw(), target.data(), target.size());
	}

	bool RawJsonString::unsafeIsEqual(std::string_view target) const noexcept {
		if (target.size() <= 256) {
			return (raw()[target.size()] == '"') && !memcmp(raw(), target.data(), target.size());
		}
		const char* r{ raw() };
		size_t pos{ 0 };
		for (; pos < target.size(); pos++) {
			if (r[pos] != target[pos]) {
				return false;
			}
		}
		if (r[pos] != '"') {
			return false;
		}
		return true;
	}

	bool RawJsonString::unsafeIsEqual(const char* target) const noexcept {
		const char* r{ raw() };
		size_t pos{ 0 };
		for (; target[pos]; pos++) {
			if (r[pos] != target[pos]) {
				return false;
			}
		}
		if (r[pos] != '"') {
			return false;
		}
		return true;
	}

	bool RawJsonString::isEqual(std::string_view target) const noexcept {
		const char* r{ raw() };
		size_t pos{ 0 };
		bool escaping{ false };
		for (; pos < target.size(); pos++) {
			if (r[pos] != target[pos]) {
				return false;
			}
			if ((target[pos] == '"') && !escaping) {
				return false;
			} else if (target[pos] == '\\') {
				escaping = !escaping;
			} else {
				escaping = false;
			}
		}
		if (r[pos] != '"') {
			return false;
		}
		return true;
	}

	bool RawJsonString::isEqual(const char* target) const noexcept {
		const char* r{ raw() };
		size_t pos{ 0 };
		bool escaping{ false };
		for (; target[pos]; pos++) {
			if (r[pos] != target[pos]) {
				return false;
			}
			if ((target[pos] == '"') && !escaping) {
				return false;
			} else if (target[pos] == '\\') {
				escaping = !escaping;
			} else {
				escaping = false;
			}
		}
		if (r[pos] != '"') {
			return false;
		}
		return true;
	}

	bool operator==(const RawJsonString& a, std::string_view c) noexcept {
		return a.unsafeIsEqual(c);
	}

	bool operator==(std::string_view c, const RawJsonString& a) noexcept {
		return a == c;
	}

	bool operator!=(const RawJsonString& a, std::string_view c) noexcept {
		return !(a == c);
	}

	bool operator!=(std::string_view c, const RawJsonString& a) noexcept {
		return !(a == c);
	}

	JsonifierResult<std::string_view> RawJsonString::unescape(JsonIterator& iter) const noexcept {
		return iter.unescape(*this);
	}

	std::ostream& operator<<(std::ostream& out, const RawJsonString& str) noexcept {
		bool inEscape = false;
		const char* s = str.raw();
		while (true) {
			switch (*s) {
				case '\\':
					inEscape = !inEscape;
					break;
				case '"':
					if (inEscape) {
						inEscape = false;
					} else {
						return out;
					}
					break;
				default:
					if (inEscape) {
						inEscape = false;
					}
			}
			out << *s;
			s++;
		}
	}

	TokenIterator::TokenIterator(const uint8_t* _buf, uint32_t* position) noexcept : stringView{ _buf }, currentPosition{ position } {
	}

	uint32_t TokenIterator::currentOffset() const noexcept {
		return *(currentPosition);
	}

	const uint8_t* TokenIterator::returnCurrentAndAdvance() noexcept {
		return &stringView[*(currentPosition++)];
	}

	const uint8_t* TokenIterator::peek(uint32_t* position) const noexcept {
		return &stringView[*position];
	}

	uint32_t TokenIterator::peekIndex(uint32_t* position) const noexcept {
		return *position;
	}

	uint32_t TokenIterator::peekLength(uint32_t* position) const noexcept {
		return *(currentPosition + 1) - *position;
	}

	const uint8_t* TokenIterator::peek(int32_t delta) const noexcept {
		return &stringView[*(currentPosition + delta)];
	}

	uint32_t TokenIterator::peekIndex(int32_t delta) const noexcept {
		return *(currentPosition + delta);
	}

	uint32_t TokenIterator::peekLength(int32_t delta) const noexcept {
		return *(currentPosition + delta + 1) - *(currentPosition + delta);
	}

	uint32_t* TokenIterator::position() const noexcept {
		return currentPosition;
	}

	void TokenIterator::setPosition(uint32_t* target_position) noexcept {
		currentPosition = target_position;
	}

	bool TokenIterator::operator==(const TokenIterator& other) const noexcept {
		return currentPosition == other.currentPosition;
	}

	bool TokenIterator::operator!=(const TokenIterator& other) const noexcept {
		return currentPosition != other.currentPosition;
	}

	bool TokenIterator::operator>(const TokenIterator& other) const noexcept {
		return currentPosition > other.currentPosition;
	}

	bool TokenIterator::operator>=(const TokenIterator& other) const noexcept {
		return currentPosition >= other.currentPosition;
	}

	bool TokenIterator::operator<(const TokenIterator& other) const noexcept {
		return currentPosition < other.currentPosition;
	}

	bool TokenIterator::operator<=(const TokenIterator& other) const noexcept {
		return currentPosition <= other.currentPosition;
	}

	JsonIterator::JsonIterator(JsonIterator&& other) noexcept
		: token(std::forward<TokenIterator>(other.token)), parser{ other.parser }, stringBuffer{ other.stringBuffer }, error{ other.error },
		  currentDepth{ other.currentDepth }, rootStructural{ other.rootStructural } {
	}

	JsonIterator& JsonIterator::operator=(JsonIterator&& other) noexcept {
		this->rootStructural = other.rootStructural;
		this->currentDepth = other.currentDepth;
		this->stringBuffer = other.stringBuffer;
		this->parser = other.parser;
		this->error = other.error;
		this->token = other.token;
		return *this;
	}

	JsonIterator::JsonIterator(Parser* _parser) noexcept
		: token(_parser->getStringView(), _parser->getStructuralIndices()), parser{ _parser }, stringBuffer{ _parser->getStringBuffer() },
		  currentDepth{ 1 }, rootStructural{ _parser->getStructuralIndices() } {};

	void JsonIterator::rewind() noexcept {
		token.setPosition(rootPosition());
		stringBuffer = parser->getStringBuffer();
		currentDepth = 1;
	}

	bool JsonIterator::balanced() const noexcept {
		TokenIterator ti(token);
		int32_t count{ 0 };
		ti.setPosition(rootPosition());
		while (ti.peek() <= peekLast()) {
			switch (*ti.returnCurrentAndAdvance()) {
				case '[':
				case '{':
					count++;
					break;
				case ']':
				case '}':
					count--;
					break;
				default:
					break;
			}
		}
		return count == 0;
	}

	inline ErrorCode JsonIterator::skipChild(size_t parent_depth) noexcept {
		if (depth() <= parent_depth) {
			return Success;
		}
		switch (*returnCurrentAndAdvance()) {
			case '[':
			case '{':
			case ':':
				break;
			case ',':
				break;
			case ']':
			case '}':
				currentDepth--;
				if (depth() <= parent_depth) {
					return Success;
				}
				break;
			case '"':
				if (*peek() == ':') {
					returnCurrentAndAdvance();
					break;
				}
				[[fallthrough]];
			default:
				currentDepth--;
				if (depth() <= parent_depth) {
					return Success;
				}
				break;
		}
		while (position() < endPosition()) {
			switch (*returnCurrentAndAdvance()) {
				case '[':
				case '{':
					currentDepth++;
					break;
				case ']':
				case '}':
					currentDepth--;
					if (depth() <= parent_depth) {
						return Success;
					}
					break;
				default:
					break;
			}
		}

		return reportError(Tape_Error, "not enough close braces");
	}

	bool JsonIterator::atRoot() const noexcept {
		return position() == rootPosition();
	}

	bool JsonIterator::isSingleToken() const noexcept {
		return parser->tapeLength == 1;
	}

	uint32_t* JsonIterator::rootPosition() const noexcept {
		return rootStructural;
	}

	void JsonIterator::assertAtDocumentDepth() const noexcept {
		assert(currentDepth == 1);
	}

	void JsonIterator::assertAtRoot() const noexcept {
		assert(currentDepth == 1);
	}

	bool JsonIterator::atEnd() const noexcept {
		return position() == endPosition();
	}

	uint32_t* JsonIterator::endPosition() const noexcept {
		size_t structuralIndexCount{ parser->getTapeLength() };
		return &parser->getStructuralIndices()[structuralIndexCount];
	}

	std::string JsonIterator::toString() const noexcept {
		if (!isAlive()) {
			return "dead JsonIterator instance";
		}
		const char* currentStructural = reinterpret_cast<const char*>(token.peek());
		return std::string("JsonIterator [ depth : ") + std::to_string(currentDepth) + std::string(", structural : '") +
			std::string(currentStructural, 1) + std::string("', offset : ") + std::to_string(token.currentOffset()) + std::string("', error : ") +
			std::to_string(error) + std::string(" ]");
	}

	JsonifierResult<const char*> JsonIterator::currentLocation() noexcept {
		if (!isAlive()) {
			if (!atRoot()) {
				return reinterpret_cast<const char*>(token.peek(-1));
			} else {
				return reinterpret_cast<const char*>(token.peek());
			}
		}
		if (atEnd()) {
			return Out_Of_Bounds;
		}
		return reinterpret_cast<const char*>(token.peek());
	}

	bool JsonIterator::isAlive() const noexcept {
		return parser;
	}

	void JsonIterator::abandon() noexcept {
		std::cout << "WERE ABANDONING!" << std::endl;
		parser = nullptr;
		currentDepth = 0;
	}

	const uint8_t* JsonIterator::returnCurrentAndAdvance() noexcept {
		return token.returnCurrentAndAdvance();
	}

	const uint8_t* JsonIterator::unsafePointer() const noexcept {
		return token.peek(0);
	}

	const uint8_t* JsonIterator::peek(int32_t delta) const noexcept {
		return token.peek(delta);
	}

	uint32_t JsonIterator::peekLength(int32_t delta) const noexcept {
		return token.peekLength(delta);
	}

	const uint8_t* JsonIterator::peek(uint32_t* position) const noexcept {
		return token.peek(position);
	}

	uint32_t JsonIterator::peekLength(uint32_t* position) const noexcept {
		return token.peekLength(position);
	}

	uint32_t* JsonIterator::lastPosition() const noexcept {
		size_t structuralIndexCount{ parser->getTapeLength() };
		assert(structuralIndexCount > 0);
		return &parser->getStructuralIndices()[structuralIndexCount - 1];
	}

	const uint8_t* JsonIterator::peekLast() const noexcept {
		return token.peek(lastPosition());
	}

	void JsonIterator::ascendTo(size_t parent_depth) noexcept {
		assert(parent_depth >= 0 && parent_depth < INT32_MAX - 1);
		assert(currentDepth == parent_depth + 1);
		currentDepth = parent_depth;
	}

	void JsonIterator::descendTo(size_t child_depth) noexcept {
		assert(child_depth >= 1 && child_depth < INT32_MAX);
		assert(currentDepth == child_depth - 1);
		currentDepth = child_depth;
	}

	size_t JsonIterator::depth() const noexcept {
		return currentDepth;
	}

	uint8_t*& JsonIterator::stringBufLoc() noexcept {
		return stringBuffer;
	}

	ErrorCode JsonIterator::reportError(ErrorCode _error, const char* message) noexcept {
		assert(_error != Success && _error != Uninitialized && _error != INCORRECT_TYPE && _error != No_Such_Field);
		error = _error;
		return error;
	}

	uint32_t* JsonIterator::position() const noexcept {
		return token.position();
	}

	JsonifierResult<std::string_view> JsonIterator::unescape(RawJsonString in) noexcept {
		uint8_t* end = parseString(in.stringView, this->stringBuffer);
		if (!end) {
			return String_Error;
		}
		std::string_view result(reinterpret_cast<const char*>(this->stringBuffer), end - this->stringBuffer);
		this->stringBuffer = end;
		return result;
	}

	void JsonIterator::reenterChild(uint32_t* position, size_t child_depth) noexcept {
#if JSONIFIER_DEVELOPMENT_CHECKS
		assert(size_t(child_depth) < parser->maxDepth());
		assert(position >= parser->startPositions[child_depth]);
#endif
		assert(child_depth >= 1 && child_depth < INT32_MAX);
		assert(currentDepth == child_depth - 1);
		token.setPosition(position);
		currentDepth = child_depth;
	}

	ErrorCode JsonIterator::optionalError(ErrorCode _error, const char* message) noexcept {
		assert(_error == Incorrect_Type || _error == No_Such_Field);
		return _error;
	}

	template<int N> bool JsonIterator::copyToBuffer(const uint8_t* json, uint32_t maxLength, uint8_t (&tmpbuf)[N]) noexcept {
		if ((N < maxLength) || (N == 0)) {
			return false;
		}
		if (maxLength > N - 1) {
			maxLength = N - 1;
		}

		std::memcpy(tmpbuf, json, maxLength);
		tmpbuf[maxLength] = ' ';
		return true;
	}

	ValueIterator::ValueIterator(JsonIterator* jsonIter, size_t depth, uint32_t* startPosition) noexcept
		: jsonIterator{ jsonIter }, currentDepth{ depth }, rootStructural{ startPosition } {
	}

	JsonifierResult<bool> ValueIterator::startObject() noexcept {
		JsonifierTry(startContainer('{', "Not an object", "object"));
		return startedObject();
	}

	JsonifierResult<bool> ValueIterator::startRootObject() noexcept {
		JsonifierTry(startContainer('{', "Not an object", "object"));
		return startedRootObject();
	}

	JsonifierResult<bool> ValueIterator::startedObject() noexcept {
#if JSONIFIER_DEVELOPMENT_CHECKS
		jsonIterator->setStartPosition(currentDepth, startPosition());
#endif
		assertAtContainerStart();
		if (*jsonIterator->peek() == '}') {
			jsonIterator->returnCurrentAndAdvance();
			endContainer();
			return false;
		}
		return true;
	}

	JsonifierResult<bool> ValueIterator::startedRootObject() noexcept {
		return startedObject();
	}

	ErrorCode ValueIterator::endContainer() noexcept {
		jsonIterator->ascendTo(depth() - 1);
		return ErrorCode::Success;
	}

	JsonifierResult<bool> ValueIterator::hasNextField() noexcept {
		assertAtNext();
		switch (*jsonIterator->returnCurrentAndAdvance()) {
			case '}':
				JsonifierTry(endContainer());
				return false;
			case ',':
				return true;
			default:
				return reportError(Tape_Error, "Missing comma between object fields");
		}
	}

	inline JsonifierResult<bool> ValueIterator::findFieldRaw(const std::string_view key) noexcept {
		ErrorCode error{};
		bool hasValue{};
		if (atFirstField()) {
			hasValue = true;
		} else if (!isOpen()) {
			if (jsonIterator->depth() < depth() - 1) {
				return Out_Of_Order_Iteration;
			}
			return false;
		} else {
			if ((error = skipChild())) {
				abandon();
				return error;
			}
			if ((error = hasNextField().get(hasValue))) {
				abandon();
				return error;
			}
#if JSONIFIER_DEVELOPMENT_CHECKS
			if (jsonIterator->startPosition(currentDepth) != startPosition()) {
				return Out_Of_Order_Iteration;
			}
#endif
		}
		while (hasValue) {
			RawJsonString actualKey{};
			if ((error = fieldKey().get(actualKey))) {
				abandon();
				return error;
			};
			if ((error = fieldValue())) {
				abandon();
				return error;
			}
			if (actualKey.unsafeIsEqual(key)) {
				return true;
			}
			JsonifierTry(skipChild());
			if ((error = hasNextField().get(hasValue))) {
				abandon();
				return error;
			}
		}
		return false;
	}

	inline JsonifierResult<bool> ValueIterator::findFieldUnorderedRaw(const std::string_view key) noexcept {
		ErrorCode error{};
		bool hasValue{};
		uint32_t* searchStart = jsonIterator->position();
		bool atFirst = atFirstField();
		if (atFirst) {
			hasValue = true;
		} else if (!isOpen()) {
#if JSONIFIER_DEVELOPMENT_CHECKS
			if (jsonIterator->depth() < depth() - 1) {
				return Out_Of_Order_Iteration;
			}
#endif
			JsonifierTry(resetObject().get(hasValue));
			atFirst = true;
		} else {
			if ((error = skipChild())) {
				abandon();
				return error;
			}
			searchStart = jsonIterator->position();
			if ((error = hasNextField().get(hasValue))) {
				abandon();
				return error;
			}
#if JSONIFIER_DEVELOPMENT_CHECKS
			if (jsonIterator->startPosition(_depth) != startPosition()) {
				return Out_Of_Order_Iteration;
			}
#endif
		}
		while (hasValue) {
			assert(jsonIterator->currentDepth == currentDepth);
			RawJsonString actualKey{};
			if ((error = fieldKey().get(actualKey))) {
				abandon();
				return error;
			};
			if ((error = fieldValue())) {
				abandon();
				return error;
			}
			if (actualKey.unsafeIsEqual(key)) {
				return true;
			}
			JsonifierTry(skipChild());
			if ((error = hasNextField().get(hasValue))) {
				this->abandon();
				return error;
			}
		}
		if (atFirst) {
			return false;
		}
		JsonifierTry(resetObject().get(hasValue));
		while (true) {
			assert(hasValue);
			assert(jsonIterator->currentDepth == currentDepth);
			RawJsonString actualKey{};
			error = fieldKey().get(actualKey);
			assert(!error);
			error = fieldValue();
			assert(!error);
			if (actualKey.unsafeIsEqual(key)) {
				return true;
			}
			JsonifierTry(skipChild());
			if (jsonIterator->position() == searchStart) {
				return false;
			}
			error = hasNextField().get(hasValue);
			assert(!error);
		}
		return false;
	}

	JsonifierResult<RawJsonString> ValueIterator::fieldKey() noexcept {
		assertAtNext();

		const uint8_t* key = jsonIterator->returnCurrentAndAdvance();
		if (*(key++) != '"') {
			return reportError(Tape_Error, "Object key is not a string");
		}
		return RawJsonString(key);
	}

	ErrorCode ValueIterator::fieldValue() noexcept {
		assertAtNext();

		if (*jsonIterator->returnCurrentAndAdvance() != ':') {
			return reportError(Tape_Error, "Missing colon in object field");
		}
		jsonIterator->descendTo(depth() + 1);
		return ErrorCode::Success;
	}

	JsonifierResult<bool> ValueIterator::startArray() noexcept {
		JsonifierTry(startContainer('[', "Not an array", "array"));
		return startedArray();
	}

	JsonifierResult<bool> ValueIterator::starRootArray() noexcept {
		JsonifierTry(startContainer('[', "Not an array", "array"));
		return startedRootArray();
	}

	std::string ValueIterator::toString() const noexcept {
		auto answer = std::string("ValueIterator [ depth : ") + std::to_string(currentDepth) + std::string(", ");
		if (jsonIterator != nullptr) {
			answer += jsonIterator->toString();
		}
		answer += std::string(" ]");
		return answer;
	}

	inline JsonifierResult<bool> ValueIterator::startedArray() noexcept {
		assertAtContainerStart();
		if (*jsonIterator->peek() == ']') {
			jsonIterator->returnCurrentAndAdvance();
			JsonifierTry(endContainer());
			return false;
		}
		jsonIterator->descendTo(depth() + 1);
#if JSONIFIER_DEVELOPMENT_CHECKS
		jsonIterator->setStartPosition(currentDepth, startPosition());
#endif
		return true;
	}

	JsonifierResult<bool> ValueIterator::startedRootArray() noexcept {
		return startedArray();
	}

	JsonifierResult<bool> ValueIterator::hasNextElement() noexcept {
		assertAtNext();
		switch (*jsonIterator->returnCurrentAndAdvance()) {
			case ']':
				JsonifierTry(endContainer());
				return false;
			case ',':
				jsonIterator->descendTo(depth() + 1);
				return true;
			default:
				return reportError(Tape_Error, "Missing comma between array elements");
		}
	}

	JsonifierResult<bool> ValueIterator::parseBool(const uint8_t* json) const noexcept {
		auto notTrue = str4ncmp(json, "true");
		auto notFalse = str4ncmp(json, "fals") | (json[4] ^ 'e');
		bool error = (notTrue && notFalse) || isNotStructuralOrWhitespace(json[notTrue ? 5 : 4]);
		if (error) {
			return incorrectTypeError("Not a boolean");
		}
		return JsonifierResult<bool>(!notTrue);
	}

	JsonifierResult<bool> ValueIterator::parseNull(const uint8_t* json) const noexcept {
		bool isNullString = !str4ncmp(json, "null") && isNotStructuralOrWhitespace(json[4]);
		if (!isNullString && json[0] == 'n') {
			return incorrectTypeError("Not a null but starts with n");
		}
		return isNullString;
	}

	JsonifierResult<std::string_view> ValueIterator::getString() noexcept {
		return getRawJsonString().unescape(jsonIter());
	}

	JsonifierResult<RawJsonString> ValueIterator::getRawJsonString() noexcept {
		auto json = peekScalar("string");
		if (*json != '"') {
			return incorrectTypeError("Not a string");
		}
		advanceScalar("string");
		return RawJsonString(json + 1);
	}

	JsonifierResult<uint64_t> ValueIterator::getUint64() noexcept {
		auto result = parseUnsigned(peekNonRootScalar("uint64"));
		if (result.error() == Success) {
			advanceNonRootScalar("uint64");
		}
		return result;
	}

	JsonifierResult<int64_t> ValueIterator::getInt64() noexcept {
		auto result = parseInteger(peekNonRootScalar("int64"));
		if (result.error() == Success) {
			advanceNonRootScalar("int64");
		}
		return result;
	}

	int64_t totalTimeNew{};
	int64_t iterationsNew{};

	JsonifierResult<double> ValueIterator::getDouble() noexcept {
		auto result = parseDouble(peekNonRootScalar("double"));
		if (result.error() == Success) {
			advanceNonRootScalar("double");
		}
		return result;
	}

	JsonifierResult<bool> ValueIterator::getBool() noexcept {
		auto result = parseBool(peekNonRootScalar("bool"));
		if (result.error() == Success) {
			advanceNonRootScalar("bool");
		}
		return result;
	}

	JsonifierResult<bool> ValueIterator::isNull() noexcept {
		bool isNullValue;
		JsonifierTry(parseNull(peekNonRootScalar("null")).get(isNullValue));
		if (isNullValue) {
			advanceNonRootScalar("null");
		}
		return isNullValue;
	}

	JsonifierResult<std::string_view> ValueIterator::getRootString() noexcept {
		return getString();
	}

	JsonifierResult<RawJsonString> ValueIterator::getRootRawJsonString() noexcept {
		return getRawJsonString();
	}

	JsonifierResult<uint64_t> ValueIterator::getRootUint64() noexcept {
		auto maxLength = peekStartLength();
		auto json = peekRootScalar("uint64");
		uint8_t tmpbuf[20 + 1];
		if (!jsonIterator->copyToBuffer(json, maxLength, tmpbuf)) {
			return NUMBER_ERROR;
		}
		auto result = parseUnsigned(tmpbuf);
		if (result.error() == Success) {
			if (!jsonIterator->isSingleToken()) {
				return Trailing_Content;
			}
			advanceRootScalar("uint64");
		}
		return result;
	}

	JsonifierResult<int64_t> ValueIterator::getRootInt64() noexcept {
		auto maxLength = peekStartLength();
		auto json = peekRootScalar("int64");
		uint8_t tmpbuf[20 + 1];
		if (!jsonIterator->copyToBuffer(json, maxLength, tmpbuf)) {
			return NUMBER_ERROR;
		}

		auto result = parseInteger(tmpbuf);
		if (result.error() == Success) {
			if (!jsonIterator->isSingleToken()) {
				return Trailing_Content;
			}
			advanceRootScalar("int64");
		}
		return result;
	}

	JsonifierResult<double> ValueIterator::getRootDouble() noexcept {
		auto maxLength = peekStartLength();
		auto json = peekRootScalar("double");
		uint8_t tmpbuf[1074 + 8 + 1];
		if (!jsonIterator->copyToBuffer(json, maxLength, tmpbuf)) {
			return NUMBER_ERROR;
		}
		auto result = parseDouble(tmpbuf);
		if (result.error() == Success) {
			if (!jsonIterator->isSingleToken()) {
				return Trailing_Content;
			}
			advanceRootScalar("double");
		}
		return result;
	}

	JsonifierResult<bool> ValueIterator::getRootBool() noexcept {
		auto maxLength = peekStartLength();
		auto json = peekRootScalar("bool");
		uint8_t tmpbuf[5 + 1];
		if (!jsonIterator->copyToBuffer(json, maxLength, tmpbuf)) {
			return incorrectTypeError("Not a boolean");
		}
		auto result = parseBool(tmpbuf);
		if (result.error() == Success) {
			if (!jsonIterator->isSingleToken()) {
				return Trailing_Content;
			}
			advanceRootScalar("bool");
		}
		return result;
	}

	bool ValueIterator::isRootNull() noexcept {
		if (!jsonIterator->isSingleToken()) {
			return false;
		}
		auto maxLength = peekStartLength();
		auto json = peekRootScalar("null");
		bool result = (maxLength >= 4 && !str4ncmp(json, "null") && (maxLength == 4 || isNotStructuralOrWhitespace(json[5])));
		if (result) {
			advanceRootScalar("null");
		}
		return result;
	}

	ErrorCode ValueIterator::skipChild() noexcept {
		assert(jsonIterator->token.currentPosition > rootStructural);
		assert(jsonIterator->currentDepth >= currentDepth);
		return jsonIterator->skipChild(depth());
	}

	ValueIterator ValueIterator::child() const noexcept {
		return { jsonIterator, depth() + 1, jsonIterator->token.position() };
	}

	bool ValueIterator::isOpen() const noexcept {
		return jsonIterator->depth() >= depth();
	}

	bool ValueIterator::atEnd() const noexcept {
		return jsonIterator->atEnd();
	}

	bool ValueIterator::atStart() const noexcept {
		return jsonIterator->token.position() == startPosition();
	}

	bool ValueIterator::atFirstField() const noexcept {
		assert(jsonIterator->token.currentPosition > rootStructural);
		return jsonIterator->token.position() == startPosition() + 1;
	}

	void ValueIterator::abandon() noexcept {
		jsonIterator->abandon();
	}

	size_t ValueIterator::depth() const noexcept {
		return currentDepth;
	}

	ErrorCode ValueIterator::error() const noexcept {
		return jsonIterator->error;
	}

	uint8_t*& ValueIterator::stringBufLoc() noexcept {
		return jsonIterator->stringBufLoc();
	}

	const JsonIterator& ValueIterator::jsonIter() const noexcept {
		return *jsonIterator;
	}

	JsonIterator& ValueIterator::jsonIter() noexcept {
		return *jsonIterator;
	}

	const uint8_t* ValueIterator::peekStart() const noexcept {
		return jsonIterator->peek(startPosition());
	}

	uint32_t ValueIterator::peekStartLength() const noexcept {
		return jsonIterator->peekLength(startPosition());
	}

	const uint8_t* ValueIterator::peekScalar(const char* type) noexcept {
		if (!isAtStart()) {
			return peekStart();
		}
		assertAtStart();
		return jsonIterator->peek();
	}

	void ValueIterator::advanceScalar(const char* type) noexcept {
		if (!isAtStart()) {
			return;
		}

		assertAtStart();
		jsonIterator->returnCurrentAndAdvance();
		jsonIterator->ascendTo(depth() - 1);
	}

	ErrorCode ValueIterator::startContainer(uint8_t start_char, const char* incorrect_type_message, const char* type) noexcept {
		const uint8_t* json{};
		if (!isAtStart()) {
			json = peekStart();
			if (*json != start_char) {
				return incorrectTypeError(incorrect_type_message);
			}
		} else {
			assertAtStart();
			json = jsonIterator->peek();
			if (*json != start_char) {
				return incorrectTypeError(incorrect_type_message);
			}
			jsonIterator->returnCurrentAndAdvance();
		}
		return ErrorCode::Success;
	}

	const uint8_t* ValueIterator::peekRootScalar(const char* type) noexcept {
		if (!isAtStart()) {
			return peekStart();
		}

		assertAtRoot();
		return jsonIterator->peek();
	}

	const uint8_t* ValueIterator::peekNonRootScalar(const char* type) noexcept {
		if (!isAtStart()) {
			return peekStart();
		}

		assertAtNonRootStart();
		return jsonIterator->peek();
	}

	void ValueIterator::advanceRootScalar(const char* type) noexcept {
		if (!isAtStart()) {
			return;
		}

		assertAtRoot();
		jsonIterator->returnCurrentAndAdvance();
		jsonIterator->ascendTo(depth() - 1);
	}

	void ValueIterator::advanceNonRootScalar(const char* type) noexcept {
		if (!isAtStart()) {
			return;
		}

		assertAtNonRootStart();
		jsonIterator->returnCurrentAndAdvance();
		jsonIterator->ascendTo(depth() - 1);
	}

	ErrorCode ValueIterator::incorrectTypeError(const char* message) const noexcept {
		return Incorrect_Type;
	}

	bool ValueIterator::isAtStart() const noexcept {
		return position() == startPosition();
	}

	bool ValueIterator::isAtKey() const noexcept {
		return currentDepth == jsonIterator->currentDepth && *jsonIterator->peek() == '"';
	}

	bool ValueIterator::isAtIteratorStart() const noexcept {
		auto delta = position() - startPosition();
		return delta == 1 || delta == 2;
	}

	void ValueIterator::assertAtStart() const noexcept {
		assert(jsonIterator->token.currentPosition == rootStructural);
		assert(jsonIterator->currentDepth == currentDepth);
		assert(currentDepth > 0);
	}

	void ValueIterator::assertAtContainerStart() const noexcept {
		assert(jsonIterator->token.currentPosition == rootStructural + 1);
		assert(jsonIterator->currentDepth == currentDepth);
		assert(currentDepth > 0);
	}

	void ValueIterator::assertAtNext() const noexcept {
		assert(jsonIterator->token.currentPosition > rootStructural);
		assert(jsonIterator->currentDepth == currentDepth);
		assert(currentDepth > 0);
	}

	void ValueIterator::moveAtStart() noexcept {
		jsonIterator->currentDepth = currentDepth;
		jsonIterator->token.setPosition(rootStructural);
	}

	void ValueIterator::moveAtContainerStart() noexcept {
		jsonIterator->currentDepth = currentDepth;
		jsonIterator->token.setPosition(rootStructural + 1);
	}

	JsonifierResult<bool> ValueIterator::resetArray() noexcept {
		moveAtContainerStart();
		return startedArray();
	}

	JsonifierResult<bool> ValueIterator::resetObject() noexcept {
		moveAtContainerStart();
		return startedObject();
	}

	void ValueIterator::assertAtChild() const noexcept {
		assert(jsonIterator->token.currentPosition > rootStructural);
		assert(jsonIterator->currentDepth == currentDepth + 1);
		assert(currentDepth > 0);
	}

	void ValueIterator::assertAtRoot() const noexcept {
		assertAtStart();
		assert(currentDepth == 1);
	}

	void ValueIterator::assertAtNonRootStart() const noexcept {
		assertAtStart();
		assert(currentDepth > 1);
	}

	void ValueIterator::assertIsValid() const noexcept {
		assert(jsonIterator != nullptr);
	}

	bool ValueIterator::isValid() const noexcept {
		return jsonIterator != nullptr;
	}

	JsonifierResult<JsonType> ValueIterator::type() const noexcept {
		switch (*peekStart()) {
			case '{':
				return JsonType::Object;
			case '[':
				return JsonType::Array;
			case '"':
				return JsonType::String;
			case 'n':
				return JsonType::Null;
			case 't':
			case 'f':
				return JsonType::Bool;
			case '-':
			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
				return JsonType::Number;
			default:
				return Tape_Error;
		}
	}

	uint32_t* ValueIterator::startPosition() const noexcept {
		return rootStructural;
	}

	uint32_t* ValueIterator::position() const noexcept {
		return jsonIterator->position();
	}

	uint32_t* ValueIterator::endPosition() const noexcept {
		return jsonIterator->endPosition();
	}

	uint32_t* ValueIterator::lastPosition() const noexcept {
		return jsonIterator->lastPosition();
	}

	ErrorCode ValueIterator::reportError(ErrorCode error, const char* message) noexcept {
		return jsonIterator->reportError(error, message);
	}
}