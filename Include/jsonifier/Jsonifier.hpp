#pragma once

#pragma warning(push)
#pragma warning(disable : 4244)
#pragma warning(disable : 4251)

#include <source_location>
#include <unordered_map>
#include <immintrin.h>
#include <string_view>
#include <functional>
#include <concepts>
#include <iostream>
#include <vector>
#include <atomic>
#include <memory>
#include <thread>
#include <chrono>
#include <bitset>
#include <array>
#include <deque>
#include <map>

#include "NumberParsingUtils.hpp"
#include "StringParsingUtils.hpp"
#include "Simd.hpp"
#include "Object.hpp"
#include "Array.hpp"
#include "Document.hpp"
#include "Field.hpp"

namespace Jsonifier {

	template<typename OTy> class ObjectBuffer {
	  public:
		using AllocatorType = std::allocator<OTy>;
		using AllocatorTraits = std::allocator_traits<AllocatorType>;

		inline ObjectBuffer& operator=(ObjectBuffer&&) = delete;
		inline ObjectBuffer(ObjectBuffer&&) = delete;

		inline ObjectBuffer& operator=(const ObjectBuffer&) = delete;
		inline ObjectBuffer(const ObjectBuffer&) = delete;

		inline ObjectBuffer() noexcept = default;

		inline OTy& operator[](size_t index) noexcept {
			return this->objects[index];
		}

		inline operator OTy*() noexcept {
			return this->objects;
		}

		inline void reset(size_t newSize) noexcept {
			this->deallocate();
			if (newSize != 0) {
				AllocatorType allocator{};
				this->objects = AllocatorTraits::allocate(allocator, newSize);
				this->currentSize = newSize;
			}
		}

		inline ~ObjectBuffer() noexcept {
			this->deallocate();
		}

	  protected:
		size_t currentSize{};
		OTy* objects{};

		inline void deallocate() {
			if (this->currentSize > 0 && this->objects) {
				AllocatorType allocator{};
				AllocatorTraits::deallocate(allocator, this->objects, this->currentSize);
				this->currentSize = 0;
			}
		}
	};

	template<typename RTy> inline void reverseByteOrder(RTy& net) {
		if constexpr (std::endian::native == std::endian::little) {
			switch (sizeof(RTy)) {
				case 1: {
					return;
				}
				case 2: {
					net =
						_mm256_extract_epi16(_mm256_shuffle_epi8(_mm256_insert_epi16(__m256i{}, net, 0), _mm256_insert_epi16(__m256i{}, 0x01, 0)), 0);
					break;
				}
				case 4: {
					net = _mm256_extract_epi32(
						_mm256_shuffle_epi8(_mm256_insert_epi32(__m256i{}, net, 0), _mm256_insert_epi32(__m256i{}, 0x10203, 0)), 0);
					break;
				}
				case 8: {
					net = _mm256_extract_epi64(
						_mm256_shuffle_epi8(_mm256_insert_epi64(__m256i{}, net, 0), _mm256_insert_epi64(__m256i{}, 0x102030405060708, 0)), 0);
					break;
				}
				default:
					return;
			}
		}
	}

	template<typename RTy> inline void storeBits(char* to, RTy num) {
		uint8_t byteSize{ 8 };
		reverseByteOrder<RTy>(num);
		for (uint32_t x = 0; x < sizeof(RTy); ++x) {
			to[x] = static_cast<uint8_t>(num >> (byteSize * x));
		}
	}

	template<typename TTy> class StopWatch {
	  public:
		using HRClock = std::chrono::high_resolution_clock;

		inline StopWatch() = delete;

		inline StopWatch<TTy>& operator=(const StopWatch<TTy>& data) {
			this->maxNumberOfMs.store(data.maxNumberOfMs.load());
			this->startTime.store(data.startTime.load());
			return *this;
		}

		inline StopWatch(const StopWatch<TTy>& data) {
			*this = data;
		}

		inline StopWatch(TTy maxNumberOfMsNew) {
			this->maxNumberOfMs.store(maxNumberOfMsNew);
			this->startTime.store(std::chrono::duration_cast<TTy>(HRClock::now().time_since_epoch()));
		}

		inline TTy totalTimePassed() {
			TTy currentTime = std::chrono::duration_cast<TTy>(HRClock::now().time_since_epoch());
			TTy elapsedTime = currentTime - this->startTime.load();
			return elapsedTime;
		}

		inline TTy getTotalWaitTime() {
			return this->maxNumberOfMs.load();
		}

		inline bool hasTimePassed() {
			TTy currentTime = std::chrono::duration_cast<TTy>(HRClock::now().time_since_epoch());
			TTy elapsedTime = currentTime - this->startTime.load();
			if (elapsedTime >= this->maxNumberOfMs.load()) {
				return true;
			} else {
				return false;
			}
		}

		inline void resetTimer() {
			this->startTime.store(std::chrono::duration_cast<TTy>(HRClock::now().time_since_epoch()));
		}

	  protected:
		std::atomic<TTy> maxNumberOfMs{ TTy{ 0 } };
		std::atomic<TTy> startTime{ TTy{ 0 } };
	};

	const uint8_t formatVersion{ 131 };

	enum class EtfType : uint8_t {
		New_Float_Ext = 70,
		Small_Integer_Ext = 97,
		Integer_Ext = 98,
		Atom_Ext = 100,
		Nil_Ext = 106,
		String_Ext = 107,
		List_Ext = 108,
		Binary_Ext = 109,
		Small_Big_Ext = 110,
		Small_Atom_Ext = 115,
		Map_Ext = 116,
	};

	template<typename Ty>
	concept IsEnum = std::is_enum<Ty>::value;

	struct EnumConverter {
		template<IsEnum EnumType> inline EnumConverter& operator=(const std::vector<EnumType>& data) {
			for (auto& Value: data) {
				this->vector.emplace_back(std::move(static_cast<uint64_t>(Value)));
			}
			return *this;
		};

		template<IsEnum EnumType> inline EnumConverter(const std::vector<EnumType>& data) {
			*this = data;
		};

		template<IsEnum EnumType> inline EnumConverter& operator=(EnumType data) {
			this->integer = static_cast<uint64_t>(data);
			return *this;
		};

		template<IsEnum EnumType> inline EnumConverter(EnumType data) {
			*this = data;
		};

		operator std::vector<uint64_t>() noexcept;

		operator uint64_t() noexcept;

		bool isItAVector() noexcept;

	  protected:
		std::vector<uint64_t> vector{};
		bool vectorType{};
		uint64_t integer{};
	};

	enum class JsonifierSerializeType { Etf = 0, Json = 1 };

	class Jsonifier_Dll Serializer;

	template<typename Ty>
	concept IsConvertibleToJsonifier = std::convertible_to<Ty, Serializer>;

	class Jsonifier_Dll Serializer {
	  public:
		using MapAllocatorType = std::allocator<std::pair<const std::string, Serializer>>;
		template<typename OTy> using AllocatorType = std::allocator<OTy>;
		template<typename OTy> using AllocatorTraits = std::allocator_traits<AllocatorType<OTy>>;
		using ObjectType = std::map<std::string, Serializer, std::less<>, MapAllocatorType>;
		using ArrayType = std::vector<Serializer, AllocatorType<Serializer>>;
		using StringType = std::string;
		using FloatType = double;
		using UintType = uint64_t;
		using IntType = int64_t;
		using BoolType = bool;

		union JsonValue {
			inline JsonValue() noexcept = default;
			inline JsonValue& operator=(JsonValue&&) noexcept = delete;
			inline JsonValue(JsonValue&&) noexcept = delete;
			inline JsonValue& operator=(const JsonValue&) noexcept = delete;
			inline JsonValue(const JsonValue&) noexcept = delete;
			ObjectType* object;
			StringType* string;
			ArrayType* array;
			FloatType numberDouble;
			UintType numberUint;
			IntType numberInt;
			BoolType boolean;
		};

		Serializer() noexcept = default;

		template<IsConvertibleToJsonifier OTy> Serializer& operator=(std::vector<OTy>&& data) noexcept {
			this->setValue(JsonType::Array);
			for (auto& Value: data) {
				this->jsonValue.array->push_back(std::move(Value));
			}
			return *this;
		}

		template<IsConvertibleToJsonifier OTy> inline Serializer(std::vector<OTy>&& data) noexcept {
			*this = std::move(data);
		}

		template<IsConvertibleToJsonifier OTy> inline Serializer& operator=(std::vector<OTy>& data) noexcept {
			this->setValue(JsonType::Array);
			for (auto& Value: data) {
				this->jsonValue.array->push_back(Value);
			}
			return *this;
		}

		template<IsConvertibleToJsonifier OTy> inline Serializer(std::vector<OTy>& data) noexcept {
			*this = data;
		}

		template<IsConvertibleToJsonifier KTy, IsConvertibleToJsonifier OTy>
		inline Serializer& operator=(std::unordered_map<KTy, OTy>&& data) noexcept {
			this->setValue(JsonType::Object);
			for (auto& [key, value]: data) {
				(*this->jsonValue.object)[key] = std::move(value);
			}
			return *this;
		}

		template<IsConvertibleToJsonifier KTy, IsConvertibleToJsonifier OTy> inline Serializer(std::unordered_map<KTy, OTy>&& data) noexcept {
			*this = std::move(data);
		};

		template<IsConvertibleToJsonifier KTy, IsConvertibleToJsonifier OTy>
		inline Serializer& operator=(std::unordered_map<KTy, OTy>& data) noexcept {
			this->setValue(JsonType::Object);
			for (auto& [key, value]: data) {
				(*this->jsonValue.object)[key] = value;
			}
			return *this;
		}

		template<IsConvertibleToJsonifier KTy, IsConvertibleToJsonifier OTy> inline Serializer(std::unordered_map<KTy, OTy>& data) noexcept {
			*this = data;
		};

		template<IsConvertibleToJsonifier KTy, IsConvertibleToJsonifier OTy> inline Serializer& operator=(std::map<KTy, OTy>&& data) noexcept {
			this->setValue(JsonType::Object);
			for (auto& [key, value]: data) {
				(*this->jsonValue.object)[key] = std::move(value);
			}
			return *this;
		}

		template<IsConvertibleToJsonifier KTy, IsConvertibleToJsonifier OTy> inline Serializer(std::map<KTy, OTy>&& data) noexcept {
			*this = std::move(data);
		};

		template<IsConvertibleToJsonifier KTy, IsConvertibleToJsonifier OTy> inline Serializer& operator=(std::map<KTy, OTy>& data) noexcept {
			this->setValue(JsonType::Object);
			for (auto& [key, value]: data) {
				(*this->jsonValue.object)[key] = value;
			}
			return *this;
		}

		template<IsConvertibleToJsonifier KTy, IsConvertibleToJsonifier OTy> inline Serializer(std::map<KTy, OTy>& data) noexcept {
			*this = data;
		};

		template<IsEnum Ty> inline Serializer& operator=(Ty data) noexcept {
			this->jsonValue.numberUint = static_cast<uint64_t>(data);
			this->type = JsonType::Uint64;
			return *this;
		}

		template<IsEnum Ty> inline Serializer(Ty data) noexcept {
			*this = data;
		}

		Serializer& operator=(Serializer&& data) noexcept;

		Serializer& operator=(const Serializer& data) noexcept;

		Serializer(const Serializer& data) noexcept;

		operator std::string&&() noexcept;

		operator std::string() noexcept;

		void refreshString(JsonifierSerializeType OpCode);

		Serializer& operator=(EnumConverter&& data) noexcept;
		Serializer(EnumConverter&& data) noexcept;

		Serializer& operator=(EnumConverter& data) noexcept;
		Serializer(EnumConverter& data) noexcept;

		Serializer& operator=(std::string&& data) noexcept;
		Serializer(std::string&& data) noexcept;

		Serializer& operator=(const std::string& data) noexcept;
		Serializer(const std::string& data) noexcept;

		Serializer& operator=(const char* data) noexcept;
		Serializer(const char* data) noexcept;

		Serializer& operator=(double data) noexcept;
		Serializer(double data) noexcept;

		Serializer& operator=(float data) noexcept;
		Serializer(float data) noexcept;

		Serializer& operator=(uint64_t data) noexcept;
		Serializer(uint64_t data) noexcept;

		Serializer& operator=(uint32_t data) noexcept;
		Serializer(uint32_t data) noexcept;

		Serializer& operator=(uint16_t data) noexcept;
		Serializer(uint16_t data) noexcept;

		Serializer& operator=(uint8_t data) noexcept;
		Serializer(uint8_t data) noexcept;

		Serializer& operator=(int64_t data) noexcept;
		Serializer(int64_t data) noexcept;

		Serializer& operator=(int32_t data) noexcept;
		Serializer(int32_t data) noexcept;

		Serializer& operator=(int16_t data) noexcept;
		Serializer(int16_t data) noexcept;

		Serializer& operator=(int8_t data) noexcept;
		Serializer(int8_t data) noexcept;

		Serializer& operator=(bool data) noexcept;
		Serializer(bool data) noexcept;

		Serializer& operator=(JsonType TypeNew) noexcept;
		Serializer(JsonType type) noexcept;

		Serializer& operator=(std::nullptr_t) noexcept;
		Serializer(std::nullptr_t data) noexcept;

		Serializer& operator[](typename ObjectType::key_type key);

		Serializer& operator[](uint64_t index);

		template<typename Ty> inline const Ty& getValue() const {
			return Ty{};
		}

		template<typename Ty> inline Ty& getValue() {
			return Ty{};
		}

		size_t size() noexcept;

		JsonType getType() noexcept;

		void emplaceBack(Serializer&& data) noexcept;
		void emplaceBack(Serializer& data) noexcept;

		~Serializer() noexcept;

	  protected:
		JsonType type{ JsonType::Null };
		JsonValue jsonValue{};
		std::string string{};

		void serializeJsonToEtfString(const Serializer* dataToParse);

		void serializeJsonToJsonString(const Serializer* dataToParse);

		void writeJsonObject(const ObjectType& ObjectNew);

		void writeJsonArray(const ArrayType& Array);

		void writeJsonString(const StringType& StringNew);

		void writeJsonFloat(const FloatType x);

		template<typename NumberType,
			std::enable_if_t<
				std::is_integral<NumberType>::value || std::is_same<NumberType, uint64_t>::value || std::is_same<NumberType, int64_t>::value, int> =
				0>
		inline void writeJsonInt(NumberType Int) {
			auto IntNew = std::to_string(Int);
			this->writeString(IntNew.data(), IntNew.size());
		}

		void writeJsonBool(const BoolType ValueNew);

		void writeJsonNull();

		void writeEtfObject(const ObjectType& jsonData);

		void writeEtfArray(const ArrayType& jsonData);

		void writeEtfString(const StringType& jsonData);

		void writeEtfUint(const UintType jsonData);

		void writeEtfInt(const IntType jsonData);

		void writeEtfFloat(const FloatType jsonData);

		void writeEtfBool(const BoolType jsonData);

		void writeEtfNull();

		void writeString(const char* data, size_t length);

		void writeCharacter(const char Char);

		void appendBinaryExt(const std::string& bytes, const uint32_t sizeNew);

		void appendNewFloatExt(const double FloatValue);

		void appendListHeader(const uint32_t sizeNew);

		void appendMapHeader(const uint32_t sizeNew);

		void appendUint64(const uint64_t value);

		void appendUint32(const uint32_t value);

		void appendUint8(const uint8_t value);

		void appendInt64(const int64_t value);

		void appendInt32(const int32_t value);

		void appendInt8(const int8_t value);

		void appendBool(bool data);

		void appendVersion();

		void appendNilExt();

		void appendNil();

		void setValue(JsonType TypeNew);

		void destroy() noexcept;

		friend bool operator==(const Serializer& lhs, const Serializer& rhs);
	};

	template<> inline const Serializer::ObjectType& Serializer::getValue() const {
		return *this->jsonValue.object;
	}

	template<> inline const Serializer::ArrayType& Serializer::getValue() const {
		return *this->jsonValue.array;
	}

	template<> inline const Serializer::StringType& Serializer::getValue() const {
		return *this->jsonValue.string;
	}

	template<> inline const Serializer::FloatType& Serializer::getValue() const {
		return this->jsonValue.numberDouble;
	}

	template<> inline const Serializer::UintType& Serializer::getValue() const {
		return this->jsonValue.numberUint;
	}

	template<> inline const Serializer::IntType& Serializer::getValue() const {
		return this->jsonValue.numberInt;
	}

	template<> inline const Serializer::BoolType& Serializer::getValue() const {
		return this->jsonValue.boolean;
	}

	template<> inline Serializer::ObjectType& Serializer::getValue() {
		return *this->jsonValue.object;
	}

	template<> inline Serializer::ArrayType& Serializer::getValue() {
		return *this->jsonValue.array;
	}

	template<> inline Serializer::StringType& Serializer::getValue() {
		return *this->jsonValue.string;
	}

	template<> inline Serializer::FloatType& Serializer::getValue() {
		return this->jsonValue.numberDouble;
	}

	template<> inline Serializer::UintType& Serializer::getValue() {
		return this->jsonValue.numberUint;
	}

	template<> inline Serializer::IntType& Serializer::getValue() {
		return this->jsonValue.numberInt;
	}

	template<> inline Serializer::BoolType& Serializer::getValue() {
		return this->jsonValue.boolean;
	}

	inline int64_t totalTimePassed{};
	inline int64_t iterationCount{};

	class Jsonifier_Dll Parser {
	  public:
		friend class JsonIterator;
		inline Parser& operator=(Parser&&) = default;
		inline Parser(Parser&&) = default;
		inline Parser& operator=(const Parser&) = delete;
		inline Parser(const Parser&) = delete;
		inline Parser(){};

		inline Document parseJson(const char* string, size_t stringLength);
		inline Document parseJson(const std::string& string);		

	  protected:
		ObjectBuffer<uint32_t> structuralIndexes{};
		ObjectBuffer<uint8_t> stringBuffer{};
		SimdStringSection section{};
		size_t stringLengthRaw{};
		size_t allocatedSpace{};
		uint8_t* stringView{};
		size_t tapeLength{};

		inline uint8_t* getStringView() {
			return this->stringView;
		}

		inline uint8_t* getStringBuffer() {
			return this->stringBuffer;
		}

		inline uint32_t* getStructuralIndices() {
			return this->structuralIndexes;
		}

		inline size_t& getTapeLength() {
			return this->tapeLength;
		}

		inline uint64_t round(int64_t a, int64_t n) {
			return (((a) + (( n )-1)) & ~(( n )-1));
		}

		inline ErrorCode allocate(const uint8_t* stringViewNew) noexcept {
			if (this->stringLengthRaw == 0) {
				return ErrorCode::Success;
			}
			this->stringBuffer.reset(round(5 * this->stringLengthRaw / 3 + 256, 256));
			this->structuralIndexes.reset(round(this->stringLengthRaw + 3, 256));
			this->allocatedSpace = round(5 * this->stringLengthRaw / 3 + 256, 256);
			this->stringView = ( uint8_t* )stringViewNew;
			if (!(this->structuralIndexes && this->stringBuffer)) {
				this->stringBuffer.reset(0);
				this->structuralIndexes.reset(0);
				return ErrorCode::Mem_Alloc_Error;
			}

			return ErrorCode::Success;
		}

		inline void generateJsonIndices(const uint8_t* stringNew, size_t stringLength) {
			if (stringNew) {
				if (stringLength == 0) {
					throw JsonifierException{ "Failed to parse as the string size is 0." };
				}

				this->stringLengthRaw = stringLength;
				if (this->allocatedSpace < round(5 * this->stringLengthRaw / 3 + 256, 256)) {
					if (this->allocate(stringNew) != ErrorCode::Success) {
						throw JsonifierException{ "Failed to allocate properly!" };
					}
				}

				this->stringView = ( uint8_t* )stringNew;
				this->section.reset();
				//iterationCount++;
				StringBlockReader<256> stringReader{ this->stringView, this->stringLengthRaw };
				//StopWatch stopWatch{ std::chrono::nanoseconds{ 1 } };
				size_t tapeCurrentIndex{ 0 };
				while (stringReader.hasFullBlock()) {
					this->section.submitDataForProcessing(stringReader.fullBlock());
					section.getStructuralIndices(this->structuralIndexes, tapeCurrentIndex, this->stringLengthRaw);
					stringReader.advance();
				}
				uint8_t block[256];
				stringReader.getRemainder(block);
				this->section.submitDataForProcessing(block);
				this->section.getStructuralIndices(this->structuralIndexes, tapeCurrentIndex, this->stringLengthRaw);
				//totalTimePassed += stopWatch.totalTimePassed().count();
				this->getTapeLength() = tapeCurrentIndex;
				//std::cout << "TIME FOR STAGE1: " << totalTimePassed / iterationCount << std::endl;
				//for (size_t x = 0; x < this->tapeLength; ++x) {
				//std::cout << "CURRENT INDEX: " << this->structuralIndexes[x]
				//<< ", THE INDEX'S VALUE: " << this->stringView[this->structuralIndexes[x]] << std::endl;
				//}
			}
		}
	};

	inline Document Parser::parseJson(const char* string, size_t stringLength) {
		this->generateJsonIndices(reinterpret_cast<const uint8_t*>(string), stringLength);
		return std::forward<Document>(JsonIterator{ this });
	}

	inline Document Parser::parseJson(const std::string& string) {
		this->generateJsonIndices(reinterpret_cast<const uint8_t*>(string.data()), string.size());
		return std::forward<Document>(JsonIterator{ this });
	}

	inline Document::Document(JsonIterator&& _iter) noexcept : iterator{ std::forward<JsonIterator>(_iter) } {
	}

	inline Document Document::start(JsonIterator&& iterator) noexcept {
		return Document(std::forward<JsonIterator>(iterator));
	}

	inline void Document::rewind() noexcept {
		this->iterator.rewind();
	}

	inline JsonifierResult<const char*> Document::currentLocation() noexcept {
		return this->iterator.currentLocation();
	}

	inline int32_t Document::currentDepth() const noexcept {
		return this->iterator.depth();
	}

	inline bool Document::isAlive() noexcept {
		return this->iterator.isAlive();
	}

	inline ValueIterator Document::resumeValueIterator() noexcept {
		return ValueIterator(&this->iterator, 1, this->iterator.rootPosition());
	}

	inline ValueIterator Document::getRootValueIterator() noexcept {
		return this->resumeValueIterator();
	}

	inline JsonifierResult<Object> Document::startOrResumeObject() noexcept {
		if (iterator.atRoot()) {
			return this->getObject();
		} else {
			return Object::resume(this->resumeValueIterator());
		}
	}

	inline JsonifierResult<Value> Document::getValue() noexcept {
		this->iterator.assertAtDocumentDepth();
		switch (*iterator.peek()) {
			case '[':
			case '{':
				return Value(this->getRootValueIterator());
			default:
				return ErrorCode::Empty;
		}
	}

	inline JsonifierResult<Array> Document::getArray() & noexcept {
		auto Value = this->getRootValueIterator();
		return Array::startRoot(Value);
	}

	inline JsonifierResult<Object> Document::getObject() & noexcept {
		auto Value = this->getRootValueIterator();
		return Object::startRoot(Value);
	}

	inline JsonifierResult<uint64_t> Document::getUint64() noexcept {
		return this->getRootValueIterator().getRootUint64();
	}

	inline JsonifierResult<int64_t> Document::getInt64() noexcept {
		return this->getRootValueIterator().getRootInt64();
	}

	inline JsonifierResult<double> Document::getDouble() noexcept {
		return this->getRootValueIterator().getRootDouble();
	}

	inline JsonifierResult<std::string_view> Document::getString() noexcept {
		return this->getRootValueIterator().getRootString();
	}

	inline JsonifierResult<RawJsonString> Document::getRawJsonString() noexcept {
		return this->getRootValueIterator().getRootRawJsonString();
	}

	inline JsonifierResult<bool> Document::getBool() noexcept {
		return this->getRootValueIterator().getRootBool();
	}

	inline JsonifierResult<bool> Document::isNull() noexcept {
		return this->getRootValueIterator().isRootNull();
	}

	inline JsonIterator::JsonIterator(JsonIterator&& other) noexcept
		: token(std::forward<TokenIterator>(other.token)), parser{ other.parser }, stringBuffer{ other.stringBuffer }, error{ other.error },
		  currentDepth{ other.currentDepth }, rootStructural{ other.rootStructural } {
		other.parser = nullptr;
	}

	inline JsonIterator& JsonIterator::operator=(JsonIterator&& other) noexcept {
		this->rootStructural = other.rootStructural;
		this->currentDepth = other.currentDepth;
		this->stringBuffer = other.stringBuffer;
		this->parser = other.parser;
		this->error = other.error;
		this->token = other.token;
		other.parser = nullptr;
		return *this;
	}

	inline JsonIterator::JsonIterator(Parser* _parser) noexcept
		: token(_parser->getStringView(), _parser->getStructuralIndices()), parser{ _parser }, stringBuffer{ _parser->getStringBuffer() },
		  currentDepth{ 1 }, rootStructural{ _parser->getStructuralIndices() } {};


	inline void JsonIterator::rewind() noexcept {
		this->token.setPosition(rootPosition());
		this->stringBuffer = parser->getStringBuffer();
		this->currentDepth = 1;
	}

	inline bool JsonIterator::balanced() const noexcept {
		TokenIterator ti(token);
		int32_t count{ 0 };
		ti.setPosition(this->rootPosition());
		while (ti.peek() <= this->peekLast()) {
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

	inline ErrorCode JsonIterator::skipChild(size_t parentDepth) noexcept {
		if (static_cast<size_t>(this->depth()) <= parentDepth) {
			return ErrorCode::Success;
		}
		switch (*this->returnCurrentAndAdvance()) {
			case '[':
			case '{':
			case ':':
				break;
			case ',':
				break;
			case ']':
			case '}':
				this->currentDepth--;
				if (static_cast<size_t>(this->depth()) <= parentDepth) {
					return ErrorCode::Success;
				}
				break;
			case '"':
				if (*this->peek() == ':') {
					this->returnCurrentAndAdvance();
					break;
				}
				[[fallthrough]];
			default:
				this->currentDepth--;
				if (static_cast<size_t>(this->depth()) <= parentDepth) {
					return ErrorCode::Success;
				}
				break;
		}

		while (this->position() < this->endPosition()) {
			switch (*this->returnCurrentAndAdvance()) {
				case '[':
				case '{':
					this->currentDepth++;
					break;
				case ']':
				case '}':
					this->currentDepth--;
					if (static_cast<size_t>(this->depth()) <= parentDepth) {
						return ErrorCode::Success;
					}
					break;
				default:
					break;
			}
		}

		return reportError(ErrorCode::Tape_Error, "not enough close braces");
	}

	inline bool JsonIterator::atRoot() const noexcept {
		return this->position() == this->rootPosition();
	}

	inline bool JsonIterator::isSingleToken() const noexcept {
		return this->parser->getTapeLength() == 1;
	}

	inline void JsonIterator::assertAtDocumentDepth() const noexcept {
		assert(this->currentDepth == 1);
	}

	inline void JsonIterator::assertAtRoot() const noexcept {
		assert(this->currentDepth == 1);
	}

	inline bool JsonIterator::atEnd() const noexcept {
		return this->position() == this->endPosition();
	}

	inline uint32_t* JsonIterator::rootPosition() const noexcept {
		return this->rootStructural;
	}

	inline uint32_t* JsonIterator::endPosition() const noexcept {
		size_t structuralIndexCount{ this->parser->getTapeLength() };
		return &this->parser->getStructuralIndices()[structuralIndexCount];
	}

	inline std::string JsonIterator::toString() const noexcept {
		if (!this->isAlive()) {
			return "dead JsonIterator instance";
		}
		const char* current_structural = reinterpret_cast<const char*>(this->token.peek());
		return std::string("JsonIterator [ depth : ") + std::to_string(this->currentDepth) + std::string(", structural : '") +
			std::string(current_structural, 1) + std::string("', offset : ") + std::to_string(this->token.currentOffset()) +
			std::string("', error : ") + std::to_string(( int32_t )this->error) + std::string(" ]");
	}

	inline JsonifierResult<const char*> JsonIterator::currentLocation() noexcept {
		if (!this->isAlive()) {
			if (!this->atRoot()) {
				return reinterpret_cast<const char*>(this->token.peek(-1));
			} else {
				return reinterpret_cast<const char*>(this->token.peek());
			}
		}
		if (this->atEnd()) {
			return ErrorCode::Out_Of_Bounds;
		}
		return reinterpret_cast<const char*>(this->token.peek());
	}

	inline bool JsonIterator::isAlive() const noexcept {
		return this->parser;
	}

	inline void JsonIterator::abandon() noexcept {
		this->parser = nullptr;
		this->currentDepth = 0;
	}

	inline const uint8_t* JsonIterator::returnCurrentAndAdvance() noexcept {
		auto newPtr = this->token.returnCurrentAndAdvance();
		return newPtr;
	}

	inline const uint8_t* JsonIterator::unsafePointer() const noexcept {
		return this->token.peek(0);
	}

	inline const uint8_t* JsonIterator::peek(int32_t delta) const noexcept {
		return this->token.peek(delta);
	}

	inline uint32_t JsonIterator::peekLength(int32_t delta) const noexcept {
		return this->token.peekLength(delta);
	}

	inline const uint8_t* JsonIterator::peek(uint32_t* position) const noexcept {
		return this->token.peek(position);
	}

	inline uint32_t JsonIterator::peekLength(uint32_t* position) const noexcept {
		return this->token.peekLength(position);
	}

	inline uint32_t* JsonIterator::lastPosition() const noexcept {
		size_t structuralIndexCount{ this->parser->getTapeLength() };
		assert(structuralIndexCount > 0);
		return &this->parser->getStructuralIndices()[structuralIndexCount - 1];
	}

	inline const uint8_t* JsonIterator::peekLast() const noexcept {
		return this->token.peek(lastPosition());
	}

	inline void JsonIterator::ascendTo(size_t parentDepth) noexcept {
		assert(parentDepth >= 0 && parentDepth < INT32_MAX - 1);
		assert(this->currentDepth == parentDepth + 1);
		this->currentDepth = parentDepth;
	}

	inline void JsonIterator::descendTo(size_t childDepth) noexcept {
		assert(childDepth >= 1 && childDepth < INT32_MAX);
		assert(this->currentDepth == childDepth - 1);
		this->currentDepth = childDepth;
	}

	inline size_t JsonIterator::depth() const noexcept {
		return this->currentDepth;
	}

	inline uint8_t*& JsonIterator::stringBufLoc() noexcept {
		return this->stringBuffer;
	}

	inline ErrorCode JsonIterator::reportError(ErrorCode _error, const char* message) noexcept {
		assert(_error != ErrorCode::Success && _error != Uninitialized && _error != Incorrect_Type && _error != No_Such_Field);
		this->error = _error;
		return this->error;
	}

	inline uint32_t* JsonIterator::position() const noexcept {
		return this->token.position();
	}

	inline JsonifierResult<std::string_view> JsonIterator::unescape(RawJsonString in) noexcept {
		uint8_t* end = StringParser::parseString(in.stringView, this->stringBuffer);
		if (!end) {
			return String_Error;
		}
		std::string_view result(reinterpret_cast<const char*>(this->stringBuffer), end - this->stringBuffer);
		this->stringBuffer = end;
		return result;
	}

	inline void JsonIterator::reenterChild(uint32_t* position, size_t childDepth) noexcept {
		assert(childDepth >= 1 && childDepth < INT32_MAX);
		assert(this->currentDepth == childDepth - 1);
	}

	inline ErrorCode JsonIterator::optionalError(ErrorCode _error, const char* message) noexcept {
		assert(_error == Incorrect_Type || _error == No_Such_Field);
		return _error;
	}

	template<int N> inline bool JsonIterator::copyToBuffer(const uint8_t* json, uint32_t max_len, uint8_t (&tmpbuf)[N]) noexcept {
		if ((N < max_len) || (N == 0)) {
			return false;
		}
		if (max_len > N - 1) {
			max_len = N - 1;
		}
		std::memcpy(tmpbuf, json, max_len);
		tmpbuf[max_len] = ' ';
		return true;
	}

	inline Field::Field() noexcept : std::pair<RawJsonString, Value>(nullptr, ValueIterator{ this->second.iterator }){};

	inline Field::Field(RawJsonString key, Value&& value) noexcept : std::pair<RawJsonString, Value>(key, std::forward<Value>(value)) {
	}

	inline JsonifierResult<Field> Field::start(const ValueIterator& parent_iter, RawJsonString key) noexcept {
		return Field(key, parent_iter.child());
	}

	inline JsonifierResult<std::string_view> Field::unescapedKey() noexcept {
		assert(this->first.stringView != nullptr);
		JsonifierResult<std::string_view> answer = this->first.unescape(this->second.iterator.jsonIter());
		this->first.consume();
		return answer;
	}

	inline RawJsonString Field::key() const noexcept {
		assert(this->first.stringView != nullptr);
		return this->first;
	}

	inline JsonifierResult<Field> Field::start(ValueIterator& parent_iter) noexcept {
		RawJsonString key;
		if (auto error = parent_iter.fieldKey().get(key)) {
			return error;
		}
		if (auto error = parent_iter.fieldValue()) {
			return error;
		}
		return Field::start(parent_iter, key);
	}

	inline Value& Field::value() & noexcept {
		return second;
	}

	inline Value Field::value() && noexcept {
		return std::forward<Field>(*this).second;
	}

	JsonifierResult<Value> Object::findFieldUnordered(const std::string_view key) && noexcept {
		bool hasValue;
		if (auto error = this->iterator.findFieldUnorderedRaw(key).get(hasValue)) {
			return error;
		}
		if (!hasValue) {
			return No_Such_Field;
		}
		return Value(this->iterator.child());
	}

	inline JsonifierResult<Value> Object::findFieldUnordered(const std::string_view key) & noexcept {
		bool hasValue;
		if (auto error = this->iterator.findFieldUnorderedRaw(key).get(hasValue)) {
			return error;
		}
		if (!hasValue) {
			return No_Such_Field;
		}
		return Value(this->iterator.child());
	}

	inline JsonifierResult<Value> Object::operator[](std::string_view key) & noexcept {
		return this->findFieldUnordered(key);
	}

	inline JsonifierResult<Value> Object::operator[](std::string_view key) && noexcept {
		return std::forward<Object>(*this).findFieldUnordered(key);
	}

	inline JsonifierResult<Value> Object::findField(const std::string_view key) && noexcept {
		bool hasValue;
		this->iterator.findFieldRaw(key).get(hasValue);
		if (!hasValue) {
			return No_Such_Field;
		}
		return Value(this->iterator.child());
	}

	inline JsonifierResult<Object> Object::start(ValueIterator& iterator) noexcept {
		iterator.startObject().error();
		return Object(iterator);
	}

	inline JsonifierResult<Object> Object::startRoot(ValueIterator& iterator) noexcept {
		iterator.startRootObject().error();
		return Object(iterator);
	}

	inline ErrorCode Object::consume() noexcept {
		if (this->iterator.isAtKey()) {
			RawJsonString actualKey;
			auto error = this->iterator.fieldKey().get(actualKey);
			if (error) {
				this->iterator.abandon();
				return error;
			};
			if ((error = this->iterator.fieldValue())) {
				this->iterator.abandon();
				return error;
			}
		}
		auto error_skip = this->iterator.jsonIter().skipChild(static_cast<size_t>(this->iterator.depth()) - 1);
		if (error_skip) {
			this->iterator.abandon();
		}
		return error_skip;
	}

	inline JsonifierResult<std::string_view> Object::rawJson() noexcept {
		const uint8_t* starting_point{ this->iterator.peekStart() };
		auto error = consume();
		if (error) {
			return error;
		}
		const uint8_t* final_point{ this->iterator.jsonIterator->peek(0) };
		return std::string_view(reinterpret_cast<const char*>(starting_point), size_t(final_point - starting_point));
	}

	inline JsonifierResult<Object> Object::started(ValueIterator& iterator) noexcept {
		iterator.startedObject().error();
		return Object(iterator);
	}

	inline Object Object::resume(const ValueIterator& iterator) noexcept {
		return iterator;
	}

	inline Object::Object(const ValueIterator& _iter) noexcept : iterator{ _iter } {
	}

	inline JsonifierResult<ObjectIterator> Object::begin() noexcept {
		return ObjectIterator(this->iterator);
	}

	inline JsonifierResult<ObjectIterator> Object::end() noexcept {
		return ObjectIterator(this->iterator);
	}

	inline JsonifierResult<size_t> Object::countFields() & noexcept {
		size_t count{ 0 };
		for (const auto& v: *this) {
			count++;
		}
		if (this->iterator.error()) {
			return this->iterator.error();
		}
		this->iterator.resetObject();
		return count;
	}

	inline JsonifierResult<bool> Object::isEmpty() & noexcept {
		bool is_not_empty;
		auto error = this->iterator.resetObject().get(is_not_empty);
		if (error) {
			return error;
		}
		return !is_not_empty;
	}

	inline JsonifierResult<bool> Object::reset() & noexcept {
		return this->iterator.resetObject();
	}

	inline JsonifierResult<ObjectIterator>::JsonifierResult(ObjectIterator&& value) noexcept
		: ImplementationJsonifierResultBase<ObjectIterator>(std::forward<ObjectIterator>(value)) {
		this->first.iterator.assertIsValid();
	}

	inline JsonifierResult<ObjectIterator>::JsonifierResult(ErrorCode error) noexcept
		: ImplementationJsonifierResultBase<ObjectIterator>({ this->first.iterator }, error){};

	inline JsonifierResult<Field> JsonifierResult<ObjectIterator>::operator*() noexcept {
		if (auto errorNew = error()) {
			return errorNew;
		}
		return *this->first;
	}

	inline bool JsonifierResult<ObjectIterator>::operator==(const JsonifierResult<ObjectIterator>& other) const noexcept {
		if (!this->first.iterator.isValid()) {
			return !error();
		}
		return this->first == other.first;
	}

	inline bool JsonifierResult<ObjectIterator>::operator!=(const JsonifierResult<ObjectIterator>& other) const noexcept {
		if (!this->first.iterator.isValid()) {
			return error();
		}
		return this->first != other.first;
	}

	inline JsonifierResult<ObjectIterator>& JsonifierResult<ObjectIterator>::operator++() noexcept {
		if (error()) {
			this->second = Success;
			return *this;
		}
		++this->first;
		return *this;
	}

	inline ValueIterator::ValueIterator(JsonIterator* jsonIter, size_t depth, uint32_t* startPosition) noexcept
		: jsonIterator{ jsonIter }, currentDepth{ depth }, rootStructural{ startPosition } {
	}

	inline JsonifierResult<bool> ValueIterator::startObject() noexcept {
		if (auto error = this->startContainer('{', "Not an object", "object")) {
			return error;
		}
		return this->startedObject();
	}

	inline JsonifierResult<bool> ValueIterator::startRootObject() noexcept {
		if (auto error = this->startContainer('{', "Not an object", "object")) {
			return error;
		}
		return this->startedRootObject();
	}

	inline JsonifierResult<bool> ValueIterator::startedObject() noexcept {
		this->assertAtContainerStart();
		if (*this->jsonIterator->peek() == '}') {
			this->jsonIterator->returnCurrentAndAdvance();
			if (auto error = this->endContainer()) {
				return error;
			}
			return false;
		}
		return true;
	}

	inline JsonifierResult<bool> ValueIterator::startedRootObject() noexcept {
		return this->startedObject();
	}

	inline ErrorCode ValueIterator::endContainer() noexcept {
		this->jsonIterator->ascendTo(static_cast<size_t>(depth()) - 1);
		return Success;
	}

	inline JsonifierResult<bool> ValueIterator::hasNextField() noexcept {
		this->assertAtNext();

		switch (*this->jsonIterator->returnCurrentAndAdvance()) {
			case '}':
				if (auto error = endContainer()) {
					return error;
				}
				return false;
			case ',':
				return true;
			default:
				return this->reportError(Tape_Error, "Missing comma between object fields");
		}
	}

	inline JsonifierResult<bool> ValueIterator::findFieldRaw(const std::string_view key) noexcept {
		ErrorCode error;
		bool hasValue;
		if (atFirstField()) {
			hasValue = true;
		} else if (!isOpen()) {
			return false;
		} else {
			if ((error = this->skipChild())) {
				this->abandon();
				return error;
			}
			if ((error = hasNextField().get(hasValue))) {
				abandon();
				return error;
			}
		}
		while (hasValue) {
			RawJsonString actualKey;
			if ((error = fieldKey().get(actualKey))) {
				abandon();
				return error;
			};
			if ((error = this->fieldValue())) {
				this->abandon();
				return error;
			}
			if (actualKey.unsafeIsEqual(key)) {
				return true;
			}

			if (auto error = skipChild()) {
				return error;
			}
			if ((error = hasNextField().get(hasValue))) {
				abandon();
				return error;
			}
		}
		return false;
	}

	inline JsonifierResult<bool> ValueIterator::findFieldUnorderedRaw(const std::string_view key) noexcept {
		ErrorCode error;
		bool hasValue;

		uint32_t* searchStart = jsonIterator->position();

		bool atFirst = atFirstField();
		if (atFirst) {
			hasValue = true;
		} else if (!isOpen()) {
			if (auto error = resetObject().get(hasValue)) {
				return error;
			}
			atFirst = true;
		} else {
			if ((error = this->skipChild())) {
				this->abandon();
				return error;
			}
			searchStart = jsonIterator->position();
			if ((error = hasNextField().get(hasValue))) {
				abandon();
				return error;
			}
		}

		while (hasValue) {
			assert(jsonIterator->currentDepth == currentDepth);
			RawJsonString actualKey;
			if ((error = fieldKey().get(actualKey))) {
				abandon();
				return error;
			};
			if ((error = this->fieldValue())) {
				this->abandon();
				return error;
			}
			if (actualKey.unsafeIsEqual(key)) {
				return true;
			}

			if (auto error = skipChild()) {
				return error;
			}

			if ((error = hasNextField().get(hasValue))) {
				abandon();
				return error;
			}
		}
		if (atFirst) {
			return false;
		}

		if (auto error = resetObject().get(hasValue)) {
			return error;
		}
		while (true) {
			assert(hasValue);
			assert(jsonIterator->currentDepth == currentDepth);
			RawJsonString actualKey;
			error = fieldKey().get(actualKey);
			assert(!error);
			error = this->fieldValue();
			assert(!error);
			if (actualKey.unsafeIsEqual(key)) {
				return true;
			}
			if (auto error = skipChild()) {
				return error;
			}
			if (jsonIterator->position() == searchStart) {
				return false;
			}
			error = hasNextField().get(hasValue);
			assert(!error);
		}
		return false;
	}

	inline JsonifierResult<RawJsonString> ValueIterator::fieldKey() noexcept {
		this->assertAtNext();
		const uint8_t* key = jsonIterator->returnCurrentAndAdvance();
		if (*(key++) != '"') {
			return this->reportError(Tape_Error, "Object key is not a string");
		}
		return RawJsonString(key);
	}

	inline ErrorCode ValueIterator::fieldValue() noexcept {
		this->assertAtNext();

		if (*this->jsonIterator->returnCurrentAndAdvance() != ':') {
			return this->reportError(Tape_Error, "Missing colon in object Field");
		}
		this->jsonIterator->descendTo(static_cast<size_t>(depth()) + 1);
		return Success;
	}

	inline JsonifierResult<bool> ValueIterator::startArray() noexcept {
		if (auto error = startContainer('[', "Not an array", "array")) {
			return error;
		}
		return startedArray();
	}

	inline JsonifierResult<bool> ValueIterator::startRootArray() noexcept {
		if (auto error = startContainer('[', "Not an array", "array")) {
			return error;
		}
		return startedRootArray();
	}

	inline std::string ValueIterator::toString() const noexcept {
		auto answer = std::string("ValueIterator [ depth : ") + std::to_string(this->currentDepth) + std::string(", ");
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
			if (auto error = endContainer()) {
				return error;
			}
			return false;
		}
		jsonIterator->descendTo(static_cast<size_t>(depth()) + 1);
		return true;
	}

	inline JsonifierResult<bool> ValueIterator::startedRootArray() noexcept {
		return startedArray();
	}

	inline JsonifierResult<bool> ValueIterator::hasNextElement() noexcept {
		assertAtNext();

		switch (*jsonIterator->returnCurrentAndAdvance()) {
			case ']':
				if (auto error = endContainer()) {
					return error;
				}
				return false;
			case ',':
				jsonIterator->descendTo(static_cast<size_t>(depth()) + 1);
				return true;
			default:
				return reportError(Tape_Error, "Missing comma between array elements");
		}
	}

	inline JsonifierResult<bool> ValueIterator::parseBool(const uint8_t* json) const noexcept {
		auto notTrue = StringParser::str4ncmp(json, "true");
		auto notFalse = StringParser::str4ncmp(json, "fals") | (json[4] ^ 'e');
		bool error = (notTrue && notFalse) || NumberParser::isNotStructuralOrWhitespace(json[notTrue ? 5 : 4]);
		if (error) {
			return incorrectTypeError("Not a boolean");
		}
		return JsonifierResult<bool>(!notTrue);
	}

	inline JsonifierResult<bool> ValueIterator::parseNull(const uint8_t* json) const noexcept {
		bool is_null_string = !StringParser::str4ncmp(json, "null") && NumberParser::isNotStructuralOrWhitespace(json[4]);
		if (!is_null_string && json[0] == 'n') {
			return incorrectTypeError("Not a null but starts with n");
		}
		return is_null_string;
	}

	inline JsonifierResult<std::string_view> ValueIterator::getString() noexcept {
		return getRawJsonString().unescape(jsonIter());
	}

	inline JsonifierResult<RawJsonString> ValueIterator::getRawJsonString() noexcept {
		auto json = peekScalar("string");
		if (*json != '"') {
			return incorrectTypeError("Not a string");
		}
		advanceScalar("string");
		return RawJsonString(json + 1);
	}

	inline JsonifierResult<uint64_t> ValueIterator::getUint64() noexcept {
		auto result = NumberParser::parseUnsigned(peekNonRootScalar("uint64"));
		advanceNonRootScalar("uint64");
		return result;
	}

	inline JsonifierResult<int64_t> ValueIterator::getInt64() noexcept {
		auto result = NumberParser::parseInteger(peekNonRootScalar("int64"));
		advanceNonRootScalar("int64");
		return result;
	}

	inline JsonifierResult<double> ValueIterator::getDouble() noexcept {
		auto result = NumberParser::parseDouble(peekNonRootScalar("double"));
		if (result.error() == Success) {
			advanceNonRootScalar("double");
		}
		return result;
	}

	inline JsonifierResult<bool> ValueIterator::getBool() noexcept {
		auto result = parseBool(peekNonRootScalar("bool"));
		if (result.error() == Success) {
			advanceNonRootScalar("bool");
		}
		return result;
	}

	inline JsonifierResult<bool> ValueIterator::isNull() noexcept {
		bool is_null_value;
		parseNull(peekNonRootScalar("null")).get(is_null_value);
		if (is_null_value) {
			advanceNonRootScalar("null");
		}
		return is_null_value;
	}

	inline JsonifierResult<std::string_view> ValueIterator::getRootString() noexcept {
		return getString();
	}

	inline JsonifierResult<RawJsonString> ValueIterator::getRootRawJsonString() noexcept {
		return getRawJsonString();
	}

	inline JsonifierResult<uint64_t> ValueIterator::getRootUint64() noexcept {
		auto max_len = peekStartLength();
		auto json = peekRootScalar("uint64");
		uint8_t tmpbuf[20 + 1];
		if (!jsonIterator->copyToBuffer(json, max_len, tmpbuf)) {
			return Number_Error;
		}
		auto result = NumberParser::parseUnsigned(tmpbuf);
		if (!jsonIterator->isSingleToken()) {
			return Trailing_Content;
		}
		advanceRootScalar("uint64");
		return result;
	}

	inline JsonifierResult<int64_t> ValueIterator::getRootInt64() noexcept {
		auto max_len = peekStartLength();
		auto json = peekRootScalar("int64");
		uint8_t tmpbuf[20 + 1];
		if (!jsonIterator->copyToBuffer(json, max_len, tmpbuf)) {
			return Number_Error;
		}

		auto result = NumberParser::parseInteger(tmpbuf);
		if (!jsonIterator->isSingleToken()) {
			return Trailing_Content;
		}
		advanceRootScalar("int64");
		return result;
	}

	inline JsonifierResult<double> ValueIterator::getRootDouble() noexcept {
		auto max_len = peekStartLength();
		auto json = peekRootScalar("double");
		uint8_t tmpbuf[1074 + 8 + 1];
		if (!jsonIterator->copyToBuffer(json, max_len, tmpbuf)) {
			return Number_Error;
		}
		auto result = NumberParser::parseDouble(tmpbuf);
		if (!jsonIterator->isSingleToken()) {
			return Trailing_Content;
		}
		advanceRootScalar("double");
		return result;
	}

	inline JsonifierResult<bool> ValueIterator::getRootBool() noexcept {
		auto max_len = peekStartLength();
		auto json = peekRootScalar("bool");
		uint8_t tmpbuf[5 + 1];
		if (!jsonIterator->copyToBuffer(json, max_len, tmpbuf)) {
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

	inline bool ValueIterator::isRootNull() noexcept {
		if (!jsonIterator->isSingleToken()) {
			return false;
		}
		auto max_len = peekStartLength();
		auto json = peekRootScalar("null");
		bool result =
			(max_len >= 4 && !StringParser::str4ncmp(json, "null") && (max_len == 4 || !NumberParser::isNotStructuralOrWhitespace(json[5])));
		if (result) {
			advanceRootScalar("null");
		}
		return result;
	}

	inline ErrorCode ValueIterator::skipChild() noexcept {
		assert(jsonIterator->token.currentPosition > rootStructural);
		assert(jsonIterator->currentDepth >= this->currentDepth);

		return jsonIterator->skipChild(static_cast<size_t>(depth()));
	}

	inline ValueIterator ValueIterator::child() const noexcept {
		assertAtChild();
		return { jsonIterator, static_cast<size_t>(depth()) + 1, jsonIterator->token.position() };
	}

	inline bool ValueIterator::isOpen() const noexcept {
		return jsonIterator->depth() >= depth();
	}

	inline bool ValueIterator::atEnd() const noexcept {
		return jsonIterator->atEnd();
	}

	inline bool ValueIterator::atStart() const noexcept {
		return jsonIterator->token.position() == startPosition();
	}

	inline bool ValueIterator::atFirstField() const noexcept {
		assert(jsonIterator->token.position() > rootStructural);
		return jsonIterator->token.position() == startPosition() + 1;
	}

	inline void ValueIterator::abandon() noexcept {
		jsonIterator->abandon();
	}

	inline int32_t ValueIterator::depth() const noexcept {
		return this->currentDepth;
	}

	inline ErrorCode ValueIterator::error() const noexcept {
		return jsonIterator->error;
	}

	inline uint8_t*& ValueIterator::stringBufLoc() noexcept {
		return jsonIterator->stringBufLoc();
	}

	inline const JsonIterator& ValueIterator::jsonIter() const noexcept {
		return *jsonIterator;
	}

	inline JsonIterator& ValueIterator::jsonIter() noexcept {
		return *jsonIterator;
	}

	inline const uint8_t* ValueIterator::peekStart() const noexcept {
		return jsonIterator->peek(startPosition());
	}

	inline uint32_t ValueIterator::peekStartLength() const noexcept {
		return jsonIterator->peekLength(startPosition());
	}

	inline const uint8_t* ValueIterator::peekScalar(const char* type) noexcept {
		if (!isAtStart()) {
			return peekStart();
		}
		assertAtStart();
		return jsonIterator->peek();
	}

	inline void ValueIterator::advanceScalar(const char* type) noexcept {
		if (!isAtStart()) {
			return;
		}
		assertAtStart();
		jsonIterator->returnCurrentAndAdvance();
		jsonIterator->ascendTo(static_cast<size_t>(depth()) - 1);
	}

	inline ErrorCode ValueIterator::startContainer(uint8_t start_char, const char* incorrect_type_message, const char* type) noexcept {
		const uint8_t* json;
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


		return Success;
	}

	inline const uint8_t* ValueIterator::peekRootScalar(const char* type) noexcept {
		if (!isAtStart()) {
			return peekStart();
		}

		assertAtRoot();
		return jsonIterator->peek();
	}

	inline const uint8_t* ValueIterator::peekNonRootScalar(const char* type) noexcept {
		if (!isAtStart()) {
			return peekStart();
		}

		assertAtNonRootStart();
		return jsonIterator->peek();
	}

	inline void ValueIterator::advanceRootScalar(const char* type) noexcept {
		if (!isAtStart()) {
			return;
		}

		assertAtRoot();
		jsonIterator->returnCurrentAndAdvance();
		jsonIterator->ascendTo(static_cast<size_t>(depth()) - 1);
	}

	inline void ValueIterator::advanceNonRootScalar(const char* type) noexcept {
		if (!isAtStart()) {
			return;
		}

		assertAtNonRootStart();
		jsonIterator->returnCurrentAndAdvance();
		jsonIterator->ascendTo(static_cast<size_t>(depth()) - 1);
	}

	inline ErrorCode ValueIterator::incorrectTypeError(const char* message) const noexcept {
		return Incorrect_Type;
	}

	inline bool ValueIterator::isAtStart() const noexcept {
		return position() == startPosition();
	}

	inline bool ValueIterator::isAtKey() const noexcept {
		return this->currentDepth == this->jsonIterator->currentDepth && *this->jsonIterator->peek() == '"';
	}

	inline bool ValueIterator::isAtIteratorStart() const noexcept {
		auto delta = position() - startPosition();
		return delta == 1 || delta == 2;
	}

	inline void ValueIterator::assertAtStart() const noexcept {
		assert(this->jsonIterator->token.currentPosition == this->rootStructural);
		assert(this->jsonIterator->currentDepth == this->currentDepth);
		assert(this->currentDepth > 0);
	}

	inline void ValueIterator::assertAtContainerStart() const noexcept {
		assert(this->jsonIterator->token.currentPosition == this->rootStructural + 1);
		assert(this->jsonIterator->currentDepth == this->currentDepth);
		assert(this->currentDepth > 0);
	}

	inline void ValueIterator::assertAtNext() const noexcept {
		assert(this->jsonIterator->token.currentPosition > this->rootStructural);
		assert(this->jsonIterator->currentDepth == this->currentDepth);
		assert(this->currentDepth > 0);
	}

	inline void ValueIterator::moveAtStart() noexcept {
		this->jsonIterator->currentDepth = this->currentDepth;
		this->jsonIterator->token.setPosition(rootStructural);
	}

	inline void ValueIterator::moveAtContainerStart() noexcept {
		this->jsonIterator->currentDepth = this->currentDepth;
		this->jsonIterator->token.setPosition(this->rootStructural + 1);
	}

	inline JsonifierResult<bool> ValueIterator::resetArray() noexcept {
		moveAtContainerStart();
		return startedArray();
	}

	inline JsonifierResult<bool> ValueIterator::resetObject() noexcept {
		moveAtContainerStart();
		return startedObject();
	}

	inline void ValueIterator::assertAtChild() const noexcept {
		assert(this->jsonIterator->token.currentPosition > this->rootStructural);
		assert(this->jsonIterator->currentDepth == this->currentDepth + 1);
		assert(this->currentDepth > 0);
	}

	inline void ValueIterator::assertAtRoot() const noexcept {
		assertAtStart();
		assert(this->currentDepth == 1);
	}

	inline void ValueIterator::assertAtNonRootStart() const noexcept {
		assertAtStart();
		assert(this->currentDepth > 1);
	}

	inline void ValueIterator::assertIsValid() const noexcept {
		assert(this->jsonIterator != nullptr);
	}

	inline bool ValueIterator::isValid() const noexcept {
		return this->jsonIterator != nullptr;
	}

	inline JsonifierResult<JsonType> ValueIterator::type() const noexcept {
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

	inline uint32_t* ValueIterator::startPosition() const noexcept {
		return this->rootStructural;
	}

	inline uint32_t* ValueIterator::position() const noexcept {
		return this->jsonIterator->position();
	}

	inline uint32_t* ValueIterator::endPosition() const noexcept {
		return this->jsonIterator->endPosition();
	}

	inline uint32_t* ValueIterator::lastPosition() const noexcept {
		return this->jsonIterator->lastPosition();
	}

	inline ErrorCode ValueIterator::reportError(ErrorCode error, const char* message) noexcept {
		return this->jsonIterator->reportError(error, message);
	}

	inline ObjectIterator::ObjectIterator(const ValueIterator& _iter) noexcept : iterator{ _iter } {
	}

	inline JsonifierResult<Field> ObjectIterator::operator*() noexcept {
		ErrorCode error = this->iterator.error();
		if (error) {
			this->iterator.abandon();
			return error;
		}
		auto result = Field::start(this->iterator);
		if (result.error()) {
			this->iterator.abandon();
		}
		return result;
	}

	inline bool ObjectIterator::operator==(const ObjectIterator& other) const noexcept {
		return !(*this != other);
	}

	inline bool ObjectIterator::operator!=(const ObjectIterator&) const noexcept {
		return this->iterator.isOpen();
	}

	inline ObjectIterator& ObjectIterator::operator++() noexcept {
		if (!this->iterator.isOpen()) {
			return *this;
		}

		ErrorCode error;
		if ((error = this->iterator.skipChild())) {
			return *this;
		}

		bool hasValue;
		if ((error = this->iterator.hasNextField().get(hasValue))) {
			return *this;
		};
		return *this;
	}

	inline TokenIterator::TokenIterator(const uint8_t* _buf, uint32_t* position) noexcept : stringView{ _buf }, currentPosition{ position } {
	}

	inline uint32_t TokenIterator::currentOffset() const noexcept {
		return *(currentPosition);
	}


	inline const uint8_t* TokenIterator::returnCurrentAndAdvance() noexcept {
		return &stringView[*(currentPosition++)];
	}

	inline const uint8_t* TokenIterator::peek(uint32_t* position) const noexcept {
		return &stringView[*position];
	}
	inline uint32_t TokenIterator::peekIndex(uint32_t* position) const noexcept {
		return *position;
	}
	inline uint32_t TokenIterator::peekLength(uint32_t* position) const noexcept {
		return *(position + 1) - *position;
	}

	inline const uint8_t* TokenIterator::peek(int32_t delta) const noexcept {
		return &stringView[*(currentPosition + delta)];
	}
	inline uint32_t TokenIterator::peekIndex(int32_t delta) const noexcept {
		return *(currentPosition + delta);
	}
	inline uint32_t TokenIterator::peekLength(int32_t delta) const noexcept {
		return *(currentPosition + delta + 1) - *(currentPosition + delta);
	}

	inline uint32_t* TokenIterator::position() const noexcept {
		return currentPosition;
	}
	inline void TokenIterator::setPosition(uint32_t* target_position) noexcept {
		currentPosition = target_position;
	}

	inline bool TokenIterator::operator==(const TokenIterator& other) const noexcept {
		return currentPosition == other.currentPosition;
	}
	inline bool TokenIterator::operator!=(const TokenIterator& other) const noexcept {
		return currentPosition != other.currentPosition;
	}
	inline bool TokenIterator::operator>(const TokenIterator& other) const noexcept {
		return currentPosition > other.currentPosition;
	}
	inline bool TokenIterator::operator>=(const TokenIterator& other) const noexcept {
		return currentPosition >= other.currentPosition;
	}
	inline bool TokenIterator::operator<(const TokenIterator& other) const noexcept {
		return currentPosition < other.currentPosition;
	}
	inline bool TokenIterator::operator<=(const TokenIterator& other) const noexcept {
		return currentPosition <= other.currentPosition;
	}

	inline JsonifierResult<RawJsonString>::JsonifierResult(RawJsonString&& value) noexcept
		: ImplementationJsonifierResultBase<RawJsonString>(std::forward<RawJsonString>(value)){};

	inline JsonifierResult<RawJsonString>::JsonifierResult(ErrorCode error) noexcept : ImplementationJsonifierResultBase<RawJsonString>(error) {
	}

	inline JsonifierResult<const char*> JsonifierResult<RawJsonString>::raw() const noexcept {
		if (auto errorNew = error()) {
			return errorNew;
		}
		return this->first.raw();
	}

	inline JsonifierResult<std::string_view> JsonifierResult<RawJsonString>::unescape(JsonIterator& iter) const noexcept {
		if (auto errorNew = error()) {
			return errorNew;
		}
		return this->first.unescape(iter);
	}

	inline RawJsonString::RawJsonString(const uint8_t* _buf) noexcept : stringView{ _buf } {
	}

	inline const char* RawJsonString::raw() const noexcept {
		return reinterpret_cast<const char*>(this->stringView);
	}

	inline bool RawJsonString::isFreeFromUnescapedQuote(std::string_view target) noexcept {
		size_t pos{ 0 };
		for (; pos < target.size() && target[pos] != '\\'; pos++) {
		}
		bool escaping{ false };
		for (; pos < target.size(); pos++) {
			if ((target[pos] == '"') && !escaping) {
				return false;
			} else if (target[pos] == '\\') {
				escaping = !escaping;
			} else {
				escaping = false;
			}
		}
		return true;
	}

	inline bool RawJsonString::isFreeFromUnescapedQuote(const char* target) noexcept {
		size_t pos{ 0 };
		for (; target[pos] && target[pos] != '\\'; pos++) {
		}
		bool escaping{ false };
		for (; target[pos]; pos++) {
			if ((target[pos] == '"') && !escaping) {
				return false;
			} else if (target[pos] == '\\') {
				escaping = !escaping;
			} else {
				escaping = false;
			}
		}
		return true;
	}

	inline bool RawJsonString::unsafeIsEqual(size_t length, std::string_view target) const noexcept {
		return (length >= target.size()) && (raw()[target.size()] == '"') && !memcmp(raw(), target.data(), target.size());
	}

	inline bool RawJsonString::unsafeIsEqual(std::string_view target) const noexcept {
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

	inline bool RawJsonString::isEqual(std::string_view target) const noexcept {
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

	inline bool RawJsonString::unsafeIsEqual(const char* target) const noexcept {
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

	inline bool RawJsonString::isEqual(const char* target) const noexcept {
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

	inline Value::Value(const ValueIterator& _iter) noexcept : iterator{ _iter } {
	}

	inline Value Value::start(const ValueIterator& iterator) noexcept {
		return iterator;
	}

	inline Value Value::resume(const ValueIterator& iterator) noexcept {
		return iterator;
	}

	inline JsonifierResult<Array> Value::getArray() noexcept {
		return Array::start(this->iterator);
	}

	inline JsonifierResult<Object> Value::getObject() noexcept {
		return Object::start(this->iterator);
	}

	inline JsonifierResult<Object> Value::startOrResumeObject() noexcept {
		if (this->iterator.atStart()) {
			return getObject();
		} else {
			return Object::resume(this->iterator);
		}
	}

	inline JsonifierResult<RawJsonString> Value::getRawJsonString() noexcept {
		return this->iterator.getRawJsonString();
	}

	inline JsonifierResult<std::string_view> Value::getString() noexcept {
		return this->iterator.getString();
	}

	inline JsonifierResult<double> Value::getDouble() noexcept {
		return this->iterator.getDouble();
	}

	inline JsonifierResult<uint64_t> Value::getUint64() noexcept {
		return this->iterator.getUint64();
	}

	inline JsonifierResult<int64_t> Value::getInt64() noexcept {
		return this->iterator.getInt64();
	}

	inline JsonifierResult<bool> Value::getBool() noexcept {
		return this->iterator.getBool();
	}

	inline JsonifierResult<bool> Value::isNull() noexcept {
		return this->iterator.isNull();
	}

	template<> inline JsonifierResult<Array> Value::get() noexcept {
		return getArray();
	}

	template<> inline JsonifierResult<Object> Value::get() noexcept {
		return getObject();
	}

	template<> inline JsonifierResult<RawJsonString> Value::get() noexcept {
		return getRawJsonString();
	}

	template<> inline JsonifierResult<std::string_view> Value::get() noexcept {
		return getString();
	}

	template<> inline JsonifierResult<double> Value::get() noexcept {
		return getDouble();
	}

	template<> inline JsonifierResult<uint64_t> Value::get() noexcept {
		return getUint64();
	}

	template<> inline JsonifierResult<int64_t> Value::get() noexcept {
		return getInt64();
	}

	template<> inline JsonifierResult<bool> Value::get() noexcept {
		return getBool();
	}

	template<typename T> inline ErrorCode Value::get(T& out) noexcept {
		return this->get<T>().get(out);
	}

	inline JsonifierResult<JsonType> Value::type() noexcept {
		return this->iterator.type();
	}

	inline JsonifierResult<bool> Value::isScalar() noexcept {
		JsonType this_type;
		auto error = type().get(this_type);
		if (error) {
			return error;
		}
		return !((this_type == JsonType::Array) || (this_type == JsonType::Object));
	}

	inline std::string_view Value::rawJsonToken() noexcept {
		return std::string_view(reinterpret_cast<const char*>(this->iterator.peekStart()), this->iterator.peekStartLength());
	}

	inline JsonifierResult<const char*> Value::currentLocation() noexcept {
		return this->iterator.jsonIter().currentLocation();
	}

	inline int32_t Value::currentDepth() const noexcept {
		return this->iterator.jsonIter().depth();
	}

	inline JsonifierResult<Value> Document::operator[](std::string_view key) & noexcept {
		return this->startOrResumeObject()[key];
	}

	inline JsonifierResult<Value> Document::operator[](const char* key) & noexcept {
		return this->startOrResumeObject()[key];
	}

	inline JsonifierResult<size_t> Value::countFields() & noexcept {
		JsonifierResult<size_t> answer;
		auto a = this->getObject();
		answer = a.countFields();
		this->iterator.moveAtStart();
		return std::move(answer);
	}

	inline JsonifierResult<size_t> Document::countFields() & noexcept {
		auto a = this->getObject();
		JsonifierResult<size_t> answer = a.countFields();
		if (answer.error() == Success) {
			this->rewind();
		}
		return answer;
	}

	inline ErrorCode Document::consume() noexcept {
		auto error = this->iterator.skipChild(0);
		if (error) {
			this->iterator.abandon();
		}
		return error;
	}

	inline JsonifierResult<std::string_view> Document::rawJson() noexcept {
		auto _iter = getRootValueIterator();
		const uint8_t* starting_point{ _iter.peekStart() };
		auto error = consume();
		if (error) {
			return error;
		}
		const uint8_t* final_point{ iterator.unsafePointer() };
		return std::string_view(reinterpret_cast<const char*>(starting_point), size_t(final_point - starting_point));
	}

	inline JsonifierResult<Value> Value::operator[](std::string_view key) noexcept {
		return startOrResumeObject()[key];
	}

	inline JsonifierResult<Value> Value::operator[](const char* key) noexcept {
		return startOrResumeObject()[key];
	}

	inline JsonifierResult<size_t> JsonifierResult<Value>::countElements() & noexcept {
		if (auto errorNew = error()) {
			return errorNew;
		}
		return first.countElements();
	}

	inline JsonifierResult<size_t> JsonifierResult<Value>::countFields() & noexcept {
		if (auto errorNew = error()) {
			return errorNew;
		}
		return first.countFields();
	}

	inline JsonifierResult<Value> JsonifierResult<Value>::at(size_t index) noexcept {
		if (auto errorNew = error()) {
			return errorNew;
		}
		return first.at(index);
	}

	inline JsonifierResult<ArrayIterator> JsonifierResult<Value>::begin() & noexcept {
		if (auto errorNew = error()) {
			return errorNew;
		}
		return first.begin();
	}

	inline JsonifierResult<ArrayIterator> JsonifierResult<Value>::end() & noexcept {
		if (auto errorNew = error()) {
			return errorNew;
		}
		return { this->first.iterator };
	}

	inline JsonifierResult<Value> JsonifierResult<Value>::findField(std::string_view key) noexcept {
		if (auto errorNew = error()) {
			return errorNew;
		}
		return first.findField(key);
	}

	inline JsonifierResult<Value> JsonifierResult<Value>::findField(const char* key) noexcept {
		if (auto errorNew = error()) {
			return errorNew;
		}
		return first.findField(key);
	}

	inline JsonifierResult<Value> JsonifierResult<Value>::findFieldUnordered(std::string_view key) noexcept {
		if (auto errorNew = error()) {
			return errorNew;
		}
		return first.findFieldUnordered(key);
	}

	inline JsonifierResult<Value> JsonifierResult<Value>::findFieldUnordered(const char* key) noexcept {
		if (auto errorNew = error()) {
			return errorNew;
		}
		return first.findFieldUnordered(key);
	}

	inline JsonifierResult<Value> JsonifierResult<Value>::operator[](std::string_view key) noexcept {
		if (auto errorNew = error()) {
			return errorNew;
		}
		return first[key];
	}

	inline JsonifierResult<Value> JsonifierResult<Value>::operator[](const char* key) noexcept {
		if (auto errorNew = error()) {
			return errorNew;
		}
		return first[key];
	}

	inline JsonifierResult<Array> JsonifierResult<Value>::getArray() noexcept {
		if (auto errorNew = error()) {
			return errorNew;
		}
		return first.getArray();
	}

	inline JsonifierResult<Object> JsonifierResult<Value>::getObject() noexcept {
		if (auto errorNew = error()) {
			return errorNew;
		}
		return first.getObject();
	}

	inline JsonifierResult<uint64_t> JsonifierResult<Value>::getUint64() noexcept {
		if (auto errorNew = error()) {
			return errorNew;
		}
		return first.getUint64();
	}

	inline JsonifierResult<int64_t> JsonifierResult<Value>::getInt64() noexcept {
		if (auto errorNew = error()) {
			return errorNew;
		}
		return first.getInt64();
	}

	inline JsonifierResult<double> JsonifierResult<Value>::getDouble() noexcept {
		if (auto errorNew = error()) {
			return errorNew;
		}
		return first.getDouble();
	}

	inline JsonifierResult<std::string_view> JsonifierResult<Value>::getString() noexcept {
		if (auto errorNew = error()) {
			return errorNew;
		}
		return first.getString();
	}

	inline JsonifierResult<RawJsonString> JsonifierResult<Value>::getRawJsonString() noexcept {
		if (auto errorNew = error()) {
			return errorNew;
		}
		return first.getRawJsonString();
	}

	inline JsonifierResult<bool> JsonifierResult<Value>::getBool() noexcept {
		if (auto errorNew = error()) {
			return errorNew;
		}
		return first.getBool();
	}

	inline JsonifierResult<bool> JsonifierResult<Value>::isNull() noexcept {
		if (auto errorNew = error()) {
			return errorNew;
		}
		return first.isNull();
	}

	template<typename T> inline JsonifierResult<T> JsonifierResult<Value>::get() noexcept {
		if (auto errorNew = error()) {
			return errorNew;
		}
		return first.get<T>();
	}

	template<typename T> inline ErrorCode JsonifierResult<Value>::get(T& out) noexcept {
		if (auto errorNew = error()) {
			return errorNew;
		}
		return first.get<T>(out);
	}

	template<> inline JsonifierResult<Value> JsonifierResult<Value>::get<Value>() noexcept {
		if (auto errorNew = error()) {
			return errorNew;
		}
		return std::move(first);
	}

	template<> inline ErrorCode JsonifierResult<Value>::get<Value>(Value& out) noexcept {
		if (auto errorNew = error()) {
			return errorNew;
		}
		out = first;
		return Success;
	}

	inline JsonifierResult<JsonType> JsonifierResult<Value>::type() noexcept {
		if (auto errorNew = error()) {
			return errorNew;
		}
		return first.type();
	}

	inline JsonifierResult<bool> JsonifierResult<Value>::isScalar() noexcept {
		if (auto errorNew = error()) {
			return errorNew;
		}
		return first.isScalar();
	}

	inline JsonifierResult<std::string_view> JsonifierResult<Value>::rawJsonToken() noexcept {
		if (auto errorNew = error()) {
			return errorNew;
		}
		return first.rawJsonToken();
	}

	inline JsonifierResult<const char*> JsonifierResult<Value>::currentLocation() noexcept {
		if (auto errorNew = error()) {
			return errorNew;
		}
		return first.currentLocation();
	}

	inline JsonifierResult<int32_t> JsonifierResult<Value>::currentDepth() const noexcept {
		if (auto errorNew = error()) {
			return errorNew;
		}
		return first.currentDepth();
	}

	inline JsonifierResult<Value> JsonifierResult<Value>::atPointer(std::string_view jsonPointer) noexcept {
		if (auto errorNew = error()) {
			return errorNew;
		}
		return first.atPointer(jsonPointer);
	}

	inline JsonifierResult<size_t> Value::countElements() & noexcept {
		JsonifierResult<size_t> answer;
		auto a = getArray();
		answer = a.countElements();
		iterator.moveAtStart();
		return answer;
	}

	inline JsonifierResult<Array>::JsonifierResult(Array&& value) noexcept : ImplementationJsonifierResultBase<Array>(std::forward<Array>(value)){}

	inline JsonifierResult<Array>::JsonifierResult(ErrorCode error) noexcept : ImplementationJsonifierResultBase<Array>(error){}

	inline JsonifierResult<ArrayIterator> JsonifierResult<Array>::begin() noexcept {
		if (auto errorNew = error()) {
			return errorNew;
		}
		return first.begin();
	}

	inline JsonifierResult<ArrayIterator> JsonifierResult<Array>::end() noexcept {
		if (auto errorNew = error()) {
			return errorNew;
		}
		return first.end();
	}

	inline JsonifierResult<size_t> JsonifierResult<Array>::countElements() & noexcept {
		if (auto errorNew = error()) {
			return errorNew;
		}
		return first.countElements();
	}

	inline JsonifierResult<bool> JsonifierResult<Array>::isEmpty() & noexcept {
		if (auto errorNew = error()) {
			return errorNew;
		}
		return first.isEmpty();
	}

	inline JsonifierResult<Value> JsonifierResult<Array>::at(size_t index) noexcept {
		if (auto errorNew = error()) {
			return errorNew;
		}
		return first.at(index);
	}

	inline JsonifierResult<Value> JsonifierResult<Array>::atPointer(std::string_view jsonPointer) noexcept {
		if (auto errorNew = error()) {
			return errorNew;
		}
		return first.atPointer(jsonPointer);
	}

	inline JsonifierResult<ArrayIterator>::JsonifierResult(ArrayIterator&& value) noexcept
		: ImplementationJsonifierResultBase<ArrayIterator>(std::forward<ArrayIterator>(value)) {
		first.iterator.assertIsValid();
	}

	inline JsonifierResult<ArrayIterator>::JsonifierResult(ErrorCode error) noexcept
		: ImplementationJsonifierResultBase<ArrayIterator>({ this->first.iterator }, error){}

	inline JsonifierResult<Value> JsonifierResult<ArrayIterator>::operator*() noexcept {
		if (auto errorNew = error()) {
			return errorNew;
		}
		return *first;
	}

	inline bool JsonifierResult<ArrayIterator>::operator==(const JsonifierResult<ArrayIterator>& other) const noexcept {
		if (!first.iterator.isValid()) {
			return !error();
		}
		return first == other.first;
	}

	inline bool JsonifierResult<ArrayIterator>::operator!=(const JsonifierResult<ArrayIterator>& other) const noexcept {
		if (!first.iterator.isValid()) {
			return error();
		}
		return first != other.first;
	}

	inline JsonifierResult<ArrayIterator>& JsonifierResult<ArrayIterator>::operator++() noexcept {
		if (error()) {
			second = Success;
			return *this;
		}
		++(first);
		return *this;
	}

	inline ArrayIterator::ArrayIterator(const ValueIterator& _iter) noexcept : iterator{ _iter } {
	}

	inline JsonifierResult<Value> ArrayIterator::operator*() noexcept {
		if (iterator.error()) {
			iterator.abandon();
			return iterator.error();
		}
		return Value(iterator.child());
	}

	inline bool ArrayIterator::operator==(const ArrayIterator& other) const noexcept {
		return !(*this != other);
	}

	inline bool ArrayIterator::operator!=(const ArrayIterator&) const noexcept {
		return iterator.isOpen();
	}

	inline ArrayIterator& ArrayIterator::operator++() noexcept {
		ErrorCode error{};
		if ((error = iterator.error())) {
			return *this;
		}
		if ((error = iterator.skipChild())) {
			return *this;
		}
		if ((error = iterator.hasNextElement().error())) {
			return *this;
		}
		return *this;
	}

	inline JsonifierResult<Value> Value::atPointer(std::string_view json_pointer) noexcept {
		JsonType t;
		if (auto error = type().get(t)) {
			return error;
		}
		switch (t) {
			case JsonType::Array:
				return (*this).getArray().atPointer(json_pointer);
			case JsonType::Object:
				return (*this).getObject().atPointer(json_pointer);
			default:
				return Invalid_Json_Pointer;
		}
	}

	inline JsonifierResult<Value> Object::atPointer(std::string_view json_pointer) noexcept {
		if (json_pointer[0] != '/') {
			return Invalid_Json_Pointer;
		}
		json_pointer = json_pointer.substr(1);
		size_t slash = json_pointer.find('/');
		std::string_view key = json_pointer.substr(0, slash);
		JsonifierResult<Value> child{ this->iterator };
		size_t escape = key.find('~');
		if (escape != std::string_view::npos) {
			std::string unescaped(key);
			do {
				switch (unescaped[escape + 1]) {
					case '0':
						unescaped.replace(escape, 2, "~");
						break;
					case '1':
						unescaped.replace(escape, 2, "/");
						break;
					default:
						return Invalid_Json_Pointer;
				}
				escape = unescaped.find('~', escape + 1);
			} while (escape != std::string::npos);
			child = findField(unescaped);
		} else {
			child = findField(key);
		}
		if (child.error()) {
			return child;
		}
		if (slash != std::string_view::npos) {
			child = child.atPointer(json_pointer.substr(slash));
		}
		return child;
	}

	inline JsonifierResult<Value> Object::findField(const std::string_view key) & noexcept {
		bool hasValue;
		if (auto error = iterator.findFieldRaw(key).get(hasValue)) {
			return error;
		}
		if (!hasValue) {
			return No_Such_Field;
		}
		return Value(iterator.child());
	}

	inline JsonifierResult<std::string_view> RawJsonString::unescape(JsonIterator& iter) const noexcept {
		return iter.unescape(*this);
	}

	inline JsonifierResult<ArrayIterator> Value::begin() & noexcept {
		return getArray().begin();
	}

	inline JsonifierResult<ArrayIterator> Value::end() & noexcept {
		return {};
	}

	inline JsonifierResult<JsonType> Document::type() noexcept {
		return getRootValueIterator().type();
	}

	inline JsonifierResult<Value> Array::at(size_t index) noexcept {
		size_t i = 0;
		for (auto value: *this) {
			if (i == index) {
				return value;
			}
			i++;
		}
		return Out_Of_Bounds;
	}

	inline JsonifierResult<Value> Value::at(size_t index) noexcept {
		auto a = getArray();
		return a.at(index);
	}

	inline JsonifierResult<Field>::JsonifierResult(Field&& value) noexcept : ImplementationJsonifierResultBase<Field>(std::forward<Field>(value)){}

	inline JsonifierResult<Field>::JsonifierResult(ErrorCode error) noexcept : ImplementationJsonifierResultBase<Field>(error){}

	inline JsonifierResult<RawJsonString> JsonifierResult<Field>::key() noexcept {
		if (auto errorNew = error()) {
			return errorNew;
		}
		return first.key();
	}

	inline JsonifierResult<std::string_view> JsonifierResult<Field>::unescaped_key() noexcept {
		if (auto errorNew = error()) {
			return errorNew;
		}
		return first.unescapedKey();
	}

	inline JsonifierResult<Value> JsonifierResult<Field>::value() noexcept {
		if (auto errorNew = error()) {
			return errorNew;
		}
		return std::move(first.value());
	}

	template<typename T> inline void ImplementationJsonifierResultBase<T>::tie(T& value, ErrorCode& error) && noexcept {
		error = this->second;
		if (!error) {
			value = std::forward<ImplementationJsonifierResultBase<T>>(*this).first;
		}
	}

	template<typename T> inline ErrorCode ImplementationJsonifierResultBase<T>::get(T& value) && noexcept {
		ErrorCode error;
		std::forward<ImplementationJsonifierResultBase<T>>(*this).tie(value, error);
		return error;
	}

	template<typename T> inline ErrorCode ImplementationJsonifierResultBase<T>::error() const noexcept {
		return this->second;
	}

	template<typename T> inline const T& ImplementationJsonifierResultBase<T>::valueUnsafe() const& noexcept {
		return this->first;
	}

	template<typename T> inline T& ImplementationJsonifierResultBase<T>::valueUnsafe() & noexcept {
		return this->first;
	}

	template<typename T> inline T&& ImplementationJsonifierResultBase<T>::valueUnsafe() && noexcept {
		return std::forward<T>(this->first);
	}

	template<typename T>
	inline ImplementationJsonifierResultBase<T>::ImplementationJsonifierResultBase(T&& value, ErrorCode error) noexcept
		: first{ std::forward<T>(value) }, second{ error } {}

	template<typename T>
	inline ImplementationJsonifierResultBase<T>::ImplementationJsonifierResultBase(ErrorCode error) noexcept
		: ImplementationJsonifierResultBase(T{}, error){};

	template<typename T>
	inline ImplementationJsonifierResultBase<T>::ImplementationJsonifierResultBase(T&& value) noexcept
		: ImplementationJsonifierResultBase(std::forward<T>(value), Success){}

	template<typename T> inline void JsonifierResultBase<T>::tie(T& value, ErrorCode& error) && noexcept {
		error = this->second;
		if (!error) {
			value = std::forward<JsonifierResultBase<T>>(*this).first;
		}
	}

	template<typename T> inline ErrorCode JsonifierResultBase<T>::get(T& value) && noexcept {
		ErrorCode error;
		std::forward<JsonifierResultBase<T>>(*this).tie(value, error);
		return error;
	}

	template<typename T> inline ErrorCode JsonifierResultBase<T>::error() const noexcept {
		return this->second;
	}

	template<typename T> inline const T& JsonifierResultBase<T>::valueUnsafe() const& noexcept {
		return this->first;
	}

	template<typename T> inline T&& JsonifierResultBase<T>::valueUnsafe() && noexcept {
		return std::forward<T>(this->first);
	}

	template<typename T>
	inline JsonifierResultBase<T>::JsonifierResultBase(T&& value, ErrorCode error) noexcept
		: std::pair<T, ErrorCode>(std::forward<T>(value), error){};

	template<typename T> inline JsonifierResultBase<T>::JsonifierResultBase(ErrorCode error) noexcept : JsonifierResultBase(T{}, error){};

	template<typename T>
	inline JsonifierResultBase<T>::JsonifierResultBase(T&& value) noexcept : JsonifierResultBase(std::forward<T>(value), Success){};

	template<typename T> inline JsonifierResultBase<T>::JsonifierResultBase() noexcept : JsonifierResultBase(T{}, Uninitialized){};


	template<typename T> inline void JsonifierResult<T>::tie(T& value, ErrorCode& error) && noexcept {
		std::forward<JsonifierResultBase<T>>(*this).tie(value, error);
	}

	template<typename T> inline ErrorCode JsonifierResult<T>::get(T& value) && noexcept {
		return std::forward<JsonifierResultBase<T>>(*this).get(value);
	}

	template<typename T> inline ErrorCode JsonifierResult<T>::error() const noexcept {
		return JsonifierResultBase<T>::error();
	}

	template<typename T> inline const T& JsonifierResult<T>::valueUnsafe() const& noexcept {
		return JsonifierResultBase<T>::valueUnsafe();
	}

	template<typename T> inline T&& JsonifierResult<T>::valueUnsafe() && noexcept {
		return std::forward<JsonifierResultBase<T>>(*this).valueUnsafe();
	}

	template<typename T>
	inline JsonifierResult<T>::JsonifierResult(T&& value, ErrorCode error) noexcept : JsonifierResultBase<T>(std::forward<T>(value), error){};

	template<typename T> inline JsonifierResult<T>::JsonifierResult(ErrorCode error) noexcept : JsonifierResultBase<T>(error){};

	template<typename T> inline JsonifierResult<T>::JsonifierResult(T&& value) noexcept : JsonifierResultBase<T>(std::forward<T>(value)){};

	template<typename T> inline JsonifierResult<T>::JsonifierResult() noexcept : JsonifierResultBase<T>(){};

	inline JsonifierResult<TokenIterator>::JsonifierResult(TokenIterator&& value) noexcept
		: ImplementationJsonifierResultBase<TokenIterator>(std::forward<TokenIterator>(value)) {
	}

	inline JsonifierResult<TokenIterator>::JsonifierResult(ErrorCode error) noexcept : ImplementationJsonifierResultBase<TokenIterator>(error) {
	}

	inline JsonifierResult<JsonType>::JsonifierResult(JsonType&& value) noexcept
		: ImplementationJsonifierResultBase<JsonType>(std::forward<JsonType>(value)) {
	}

	inline JsonifierResult<JsonType>::JsonifierResult(ErrorCode error) noexcept : ImplementationJsonifierResultBase<JsonType>(error) {
	}
	
	inline JsonifierResult<JsonIterator>::JsonifierResult(JsonIterator&& value) noexcept
		: ImplementationJsonifierResultBase<JsonIterator>(std::forward<JsonIterator>(value)){}
	inline JsonifierResult<JsonIterator>::JsonifierResult(ErrorCode error) noexcept : ImplementationJsonifierResultBase<JsonIterator>(error){}

	inline JsonifierResult<Value>::JsonifierResult(Value&& value) noexcept : ImplementationJsonifierResultBase<Value>(std::forward<Value>(value)){};

	inline JsonifierResult<Value>::JsonifierResult(ErrorCode error) noexcept : ImplementationJsonifierResultBase<Value>(error){};

	inline JsonifierResult<Document>::JsonifierResult(Document&& value) noexcept
		: ImplementationJsonifierResultBase<Document>(std::forward<Document>(value)){};

	inline JsonifierResult<Document>::JsonifierResult(ErrorCode error) noexcept : ImplementationJsonifierResultBase<Document>(error){};

	inline JsonifierResult<size_t> JsonifierResult<Document>::countElements() & noexcept {
		if (auto errorNew = error()) {
			return errorNew;
		}
		return first.countElements();
	}

	inline JsonifierResult<size_t> JsonifierResult<Document>::countFields() & noexcept {
		if (auto errorNew = error()) {
			return errorNew;
		}
		return first.countFields();
	}

	inline JsonifierResult<Value> JsonifierResult<Document>::at(size_t index) & noexcept {
		if (auto errorNew = error()) {
			return errorNew;
		}
		return first.at(index);
	}

	inline ErrorCode JsonifierResult<Document>::rewind() noexcept {
		if (auto errorNew = error()) {
			return errorNew;
		}
		first.rewind();
		return Success;
	}

	inline JsonifierResult<ArrayIterator> JsonifierResult<Document>::begin() & noexcept {
		if (auto errorNew = error()) {
			return errorNew;
		}
		return first.begin();
	}

	inline JsonifierResult<ArrayIterator> JsonifierResult<Document>::end() & noexcept {
		return ArrayIterator{ ValueIterator{ &this->first.iterator, 0ull, this->first.iterator.rootPosition() } };
	}

	inline JsonifierResult<Value> JsonifierResult<Document>::findFieldUnordered(std::string_view key) & noexcept {
		if (auto errorNew = error()) {
			return errorNew;
		}
		return first.findFieldUnordered(key);
	}

	inline JsonifierResult<Value> JsonifierResult<Document>::findFieldUnordered(const char* key) & noexcept {
		if (auto errorNew = error()) {
			return errorNew;
		}
		return first.findFieldUnordered(key);
	}

	inline JsonifierResult<Value> JsonifierResult<Document>::operator[](std::string_view key) & noexcept {
		if (auto errorNew = error()) {
			return errorNew;
		}
		return first[key];
	}

	inline JsonifierResult<Value> JsonifierResult<Document>::operator[](const char* key) & noexcept {
		if (auto errorNew = error()) {
			return errorNew;
		}
		return first[key];
	}

	inline JsonifierResult<Value> JsonifierResult<Document>::findField(std::string_view key) & noexcept {
		if (auto errorNew = error()) {
			return errorNew;
		}
		return first.findField(key);
	}

	inline JsonifierResult<Value> JsonifierResult<Document>::findField(const char* key) & noexcept {
		if (auto errorNew = error()) {
			return errorNew;
		}
		return first.findField(key);
	}

	inline JsonifierResult<Array> JsonifierResult<Document>::getArray() & noexcept {
		if (auto errorNew = error()) {
			return errorNew;
		}
		return first.getArray();
	}

	inline JsonifierResult<Object> JsonifierResult<Document>::getObject() & noexcept {
		if (auto errorNew = error()) {
			return errorNew;
		}
		return first.getObject();
	}

	inline JsonifierResult<uint64_t> JsonifierResult<Document>::getUint64() noexcept {
		if (auto errorNew = error()) {
			return errorNew;
		}
		return first.getUint64();
	}

	inline JsonifierResult<int64_t> JsonifierResult<Document>::getInt64() noexcept {
		if (auto errorNew = error()) {
			return errorNew;
		}
		return first.getInt64();
	}

	inline JsonifierResult<double> JsonifierResult<Document>::getDouble() noexcept {
		if (auto errorNew = error()) {
			return errorNew;
		}
		return first.getDouble();
	}

	inline JsonifierResult<std::string_view> JsonifierResult<Document>::getString() noexcept {
		if (auto errorNew = error()) {
			return errorNew;
		}
		return first.getString();
	}

	inline JsonifierResult<RawJsonString> JsonifierResult<Document>::getRawJsonString() noexcept {
		if (auto errorNew = error()) {
			return errorNew;
		}
		return first.getRawJsonString();
	}

	inline JsonifierResult<bool> JsonifierResult<Document>::getBool() noexcept {
		if (auto errorNew = error()) {
			return errorNew;
		}
		return first.getBool();
	}

	inline JsonifierResult<Value> JsonifierResult<Document>::getValue() noexcept {
		if (auto errorNew = error()) {
			return errorNew;
		}
		return first.getValue();
	}

	inline JsonifierResult<bool> JsonifierResult<Document>::isNull() noexcept {
		if (auto errorNew = error()) {
			return errorNew;
		}
		return first.isNull();
	}

	template<typename T> inline JsonifierResult<T> JsonifierResult<Document>::get() & noexcept {
		if (auto errorNew = error()) {
			return errorNew;
		}
		return first.get<T>();
	}

	template<typename T> inline JsonifierResult<T> JsonifierResult<Document>::get() && noexcept {
		if (auto errorNew = error()) {
			return errorNew;
		}
		return std::forward<Document>(first).get<T>();
	}

	template<typename T> inline ErrorCode JsonifierResult<Document>::get(T& out) & noexcept {
		if (auto errorNew = error()) {
			return errorNew;
		}
		return first.get<T>(out);
	}

	template<typename T> inline ErrorCode JsonifierResult<Document>::get(T& out) && noexcept {
		if (auto errorNew = error()) {
			return errorNew;
		}
		return std::forward<Document>(first).get<T>(out);
	}

	template<> inline JsonifierResult<Document> JsonifierResult<Document>::get<Document>() & noexcept = delete;
	template<> inline ErrorCode JsonifierResult<Document>::get<Document>(Document& out) & noexcept = delete;

	template<> inline JsonifierResult<Document> JsonifierResult<Document>::get<Document>() && noexcept {
		if (auto errorNew = error()) {
			return errorNew;
		}
		return std::forward<Document>(first);
	}

	template<> inline ErrorCode JsonifierResult<Document>::get<Document>(Document& out) && noexcept {
		if (auto errorNew = error()) {
			return errorNew;
		}
		out = std::forward<Document>(first);
		return Success;
	};

	inline JsonifierResult<JsonType> JsonifierResult<Document>::type() noexcept {
		if (auto errorNew = error()) {
			return errorNew;
		}
		return first.type();
	}

	inline JsonifierResult<bool> JsonifierResult<Document>::isScalar() noexcept {
		if (auto errorNew = error()) {
			return errorNew;
		}
		return first.isScalar();
	}

	inline JsonifierResult<const char*> JsonifierResult<Document>::currentLocation() noexcept {
		if (auto errorNew = error()) {
			return errorNew;
		}
		return first.currentLocation();
	}

	inline int32_t JsonifierResult<Document>::currentDepth() const noexcept {
		if (auto errorNew = error()) {
			return errorNew;
		}
		return first.currentDepth();
	}

	inline JsonifierResult<std::string_view> JsonifierResult<Document>::rawJsonToken() noexcept {
		if (auto errorNew = error()) {
			return errorNew;
		}
		return first.rawJsonToken();
	}

	inline JsonifierResult<Value> JsonifierResult<Document>::atPointer(std::string_view json_pointer) noexcept {
		if (auto errorNew = error()) {
			return errorNew;
		}
		return first.atPointer(json_pointer);
	}

	inline JsonifierResult<Object>::JsonifierResult(Object&& value) noexcept
		: ImplementationJsonifierResultBase<Object>(std::forward<Object>(value)) {
	}

	inline JsonifierResult<Object>::JsonifierResult(ErrorCode error) noexcept : ImplementationJsonifierResultBase<Object>(error) {
	}

	inline JsonifierResult<ObjectIterator> JsonifierResult<Object>::begin() noexcept {
		if (auto errorNew = error()) {
			return errorNew;
		}
		return first.begin();
	}

	inline JsonifierResult<ObjectIterator> JsonifierResult<Object>::end() noexcept {
		if (auto errorNew = error()) {
			return errorNew;
		}
		return first.end();
	}

	inline JsonifierResult<Value> JsonifierResult<Object>::findFieldUnordered(std::string_view key) & noexcept {
		if (auto errorNew = error()) {
			return errorNew;
		}
		return first.findFieldUnordered(key);
	}

	inline JsonifierResult<Value> JsonifierResult<Object>::findFieldUnordered(std::string_view key) && noexcept {
		if (auto errorNew = error()) {
			return errorNew;
		}
		return std::forward<Object>(first).findFieldUnordered(key);
	}

	inline JsonifierResult<Value> JsonifierResult<Object>::operator[](std::string_view key) & noexcept {
		if (auto errorNew = error()) {
			return errorNew;
		}
		return first[key];
	}

	inline JsonifierResult<Value> JsonifierResult<Object>::operator[](std::string_view key) && noexcept {
		if (auto errorNew = error()) {
			return errorNew;
		}
		return std::forward<Object>(first)[key];
	}

	inline JsonifierResult<Value> JsonifierResult<Object>::findField(std::string_view key) & noexcept {
		if (auto errorNew = error()) {
			return errorNew;
		}
		return first.findField(key);
	}

	inline JsonifierResult<Value> JsonifierResult<Object>::findField(std::string_view key) && noexcept {
		if (auto errorNew = error()) {
			return errorNew;
		}
		return std::forward<Object>(first).findField(key);
	}

	inline JsonifierResult<Value> JsonifierResult<Object>::atPointer(std::string_view json_pointer) noexcept {
		if (auto errorNew = error()) {
			return errorNew;
		}
		return first.atPointer(json_pointer);
	}

	inline JsonifierResult<bool> JsonifierResult<Object>::reset() noexcept {
		if (auto errorNew = error()) {
			return errorNew;
		}
		return first.reset();
	}

	inline JsonifierResult<bool> JsonifierResult<Object>::isEmpty() noexcept {
		if (auto errorNew = error()) {
			return errorNew;
		}
		return first.isEmpty();
	}

	inline JsonifierResult<size_t> JsonifierResult<Object>::countFields() & noexcept {
		if (auto errorNew = error()) {
			return errorNew;
		}
		return first.countFields();
	}

	template<> inline JsonifierResult<Array> Document::get() & noexcept {
		return getArray();
	}

	template<> inline JsonifierResult<Object> Document::get() & noexcept {
		return getObject();
	}

	template<> inline JsonifierResult<RawJsonString> Document::get() & noexcept {
		return getRawJsonString();
	}

	template<> inline JsonifierResult<std::string_view> Document::get() & noexcept {
		return getString();
	}

	template<> inline JsonifierResult<double> Document::get() & noexcept {
		return getDouble();
	}

	template<> inline JsonifierResult<uint64_t> Document::get() & noexcept {
		return getUint64();
	}

	template<> inline JsonifierResult<int64_t> Document::get() & noexcept {
		return getInt64();
	}

	template<> inline JsonifierResult<bool> Document::get() & noexcept {
		return getBool();
	}

	template<> inline JsonifierResult<Value> Document::get() & noexcept {
		return getValue();
	}

	template<> inline JsonifierResult<RawJsonString> Document::get() && noexcept {
		return std::forward<Document>(*this).getRawJsonString();
	}

	template<> inline JsonifierResult<std::string_view> Document::get() && noexcept {
		return std::forward<Document>(*this).getString();
	}

	template<> inline JsonifierResult<double> Document::get() && noexcept {
		return std::forward<Document>(*this).getDouble();
	}

	template<> inline JsonifierResult<uint64_t> Document::get() && noexcept {
		return std::forward<Document>(*this).getUint64();
	}

	template<> inline JsonifierResult<int64_t> Document::get() && noexcept {
		return std::forward<Document>(*this).getInt64();
	}

	template<> inline JsonifierResult<bool> Document::get() && noexcept {
		return std::forward<Document>(*this).getBool();
	}

	template<> inline JsonifierResult<Value> Document::get() && noexcept {
		return std::forward<Document>(*this).getValue();
	}

	template<typename T> inline ErrorCode Document::get(T& out) & noexcept {
		return get<T>().get(out);
	}

	template<typename T> inline ErrorCode Document::get(T& out) && noexcept {
		return std::forward<Document>(*this).get<T>().get(out);
	}

	inline JsonifierResult<Value>::operator Array() noexcept(false) {
		if (error()) {
			throw error();
		}
		return first;
	}

	inline JsonifierResult<Value>::operator Object() noexcept(false) {
		if (error()) {
			throw error();
		}
		return first;
	}

	inline JsonifierResult<Value>::operator uint64_t() noexcept(false) {
		if (error()) {
			throw error();
		}
		return first;
	}

	inline JsonifierResult<Value>::operator int64_t() noexcept(false) {
		if (error()) {
			throw error();
		}
		return first;
	}

	inline JsonifierResult<Value>::operator double() noexcept(false) {
		if (error()) {
			throw error();
		}
		return first;
	}

	inline JsonifierResult<Value>::operator std::string_view() noexcept(false) {
		if (error()) {
			throw error();
		}
		return first;
	}

	inline JsonifierResult<Value>::operator RawJsonString() noexcept(false) {
		if (error()) {
			throw error();
		}
		return first;
	}

	inline JsonifierResult<Value>::operator bool() noexcept(false) {
		if (error()) {
			throw error();
		}
		return first;
	}

	inline Value::operator Array() noexcept(false) {
		return getArray().valueUnsafe();
	}

	inline Value::operator Object() noexcept(false) {
		return getObject().valueUnsafe();
	}

	inline Value::operator uint64_t() noexcept(false) {
		return getUint64().valueUnsafe();
	}

	inline Value::operator int64_t() noexcept(false) {
		return getInt64().valueUnsafe();
	}

	inline Value::operator double() noexcept(false) {
		return getDouble().valueUnsafe();
	}

	inline Value::operator std::string_view() noexcept(false) {
		return getString().valueUnsafe();
	}

	inline Value::operator RawJsonString() noexcept(false) {
		return getRawJsonString().valueUnsafe();
	}

	inline Value::operator bool() noexcept(false) {
		return getBool().valueUnsafe();
	}

	inline JsonifierResult<ArrayIterator> Document::begin() & noexcept {
		return getArray().begin();
	}

	inline JsonifierResult<ArrayIterator> Document::end() & noexcept {
		return { ValueIterator{ &this->iterator, 0, this->iterator.rootPosition() } };
	}

	inline Array::Array(const ValueIterator& _iter) noexcept : iterator{ _iter } {
	}

	inline JsonifierResult<Array> Array::start(ValueIterator& iter) noexcept {
		bool hasValue;
		if (auto error = iter.startArray().get(hasValue)) {
			return error;
		}
		return Array(iter);
	}

	inline JsonifierResult<Array> Array::startRoot(ValueIterator& iter) noexcept {
		bool hasValue;
		if (auto error = iter.startedRootArray().get(hasValue)) {
			return error;
		}
		return Array(iter);
	}

	inline JsonifierResult<Array> Array::started(ValueIterator& iter) noexcept {
		bool hasValue;
		if (auto error = iter.startedArray().get(hasValue)) {
			return error;
		}
		return Array(iter);
	}

	inline JsonifierResult<ArrayIterator> Array::begin() noexcept {
		return ArrayIterator(iterator);
	}

	inline JsonifierResult<ArrayIterator> Array::end() noexcept {
		return ArrayIterator(iterator);
	}

	inline ErrorCode Array::consume() noexcept {
		auto error = iterator.jsonIter().skipChild(iterator.currentDepth - 1);
		if (error) {
			iterator.abandon();
		}
		return error;
	}

	inline JsonifierResult<std::string_view> Array::rawJson() noexcept {
		const uint8_t* starting_point{ iterator.peekStart() };
		auto error = consume();
		if (error) {
			return error;
		}
		const uint8_t* final_point{ iterator.jsonIterator->unsafePointer() };
		return std::string_view(reinterpret_cast<const char*>(starting_point), size_t(final_point - starting_point));
	}

	inline JsonifierResult<size_t> Array::countElements() & noexcept {
		size_t count{ 0 };
		for (auto v: *this) {
			count++;
		}
		if (iterator.error()) {
			return iterator.error();
		}
		iterator.resetArray();
		return count;
	}

	inline JsonifierResult<bool> Array::isEmpty() & noexcept {
		bool is_not_empty;
		auto error = iterator.resetArray().get(is_not_empty);
		if (error) {
			return error;
		}
		return !is_not_empty;
	}

	inline JsonifierResult<bool> Array::reset() & noexcept {
		return iterator.resetArray();
	}

	inline JsonifierResult<Value> Array::atPointer(std::string_view json_pointer) noexcept {
		if (json_pointer[0] != '/') {
			return Invalid_Json_Pointer;
		}
		json_pointer = json_pointer.substr(1);
		if (json_pointer == "-") {
			return Invalid_Json_Pointer;
		}

		size_t array_index = 0;
		size_t i;
		for (i = 0; i < json_pointer.length() && json_pointer[i] != '/'; i++) {
			uint8_t digit = uint8_t(json_pointer[i] - '0');
			if (digit > 9) {
				return Incorrect_Type;
			}
			array_index = array_index * 10 + digit;
		}

		if (i > 1 && json_pointer[0] == '0') {
			return Invalid_Json_Pointer;
		}

		if (i == 0) {
			return Invalid_Json_Pointer;
		}
		auto child = at(array_index);
		if (child.error()) {
			return child;
		}

		if (i < json_pointer.length()) {
			child = child.atPointer(json_pointer.substr(i));
		}
		return child;
	}
}
