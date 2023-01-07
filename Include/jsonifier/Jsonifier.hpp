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

#define JsonifierTry(EXPR) { auto _err = (EXPR); if (_err) { return _err; } }

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

		inline ErrorCode allocate() noexcept {
			if (this->stringLengthRaw == 0) {
				return ErrorCode::Success;
			}
			this->stringBuffer.reset(round(5 * this->stringLengthRaw / 3 + 256, 256));
			this->structuralIndexes.reset(round(this->stringLengthRaw + 3, 256));
			this->allocatedSpace = round(5 * this->stringLengthRaw / 3 + 256, 256);
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
				this->stringView = ( uint8_t* )stringNew;
				this->stringLengthRaw = stringLength;
				if (this->allocatedSpace < round(5 * this->stringLengthRaw / 3 + 256, 256)) {
					if (this->allocate() != ErrorCode::Success) {
						throw JsonifierException{ "Failed to allocate properly!" };
					}
				}
				this->section.reset();
				StringBlockReader<256> stringReader{ this->stringView, this->stringLengthRaw };
				size_t tapeCurrentIndex{ 0 };
				while (stringReader.hasFullBlock()) {
					section.submitDataForProcessing(stringReader.fullBlock());
					section.getStructuralIndices(this->structuralIndexes, tapeCurrentIndex, this->stringLengthRaw);
					stringReader.advance();
				}
				uint8_t block[256];
				stringReader.getRemainder(block);
				section.submitDataForProcessing(block);
				section.getStructuralIndices(this->structuralIndexes, tapeCurrentIndex, this->stringLengthRaw);
				this->getTapeLength() = tapeCurrentIndex;
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

	inline RawJsonString::RawJsonString(const uint8_t* _buf) noexcept :stringView{ _buf } {
	}

	inline const char* RawJsonString::raw() const noexcept {
		return reinterpret_cast<const char*>(stringView);
	}

	template<typename T> inline void JsonifierResultBase<T>::tie(T& value, ErrorCode& error) && noexcept {
		error = this->second;
		if (!error) {
			value = std::forward<JsonifierResultBase<T>>(*this).first;
		}
	}

	template<typename T>  inline ErrorCode JsonifierResultBase<T>::get(T& value) && noexcept {
		ErrorCode error;
		std::forward<JsonifierResultBase<T>>(*this).tie(value, error);
		return error;
	}

	template<typename T> inline ErrorCode JsonifierResultBase<T>::error() const noexcept {
		return this->second;
	}

	template<typename T> inline T& JsonifierResultBase<T>::value() & noexcept(false) {
		if (error()) {
			throw error();
		}
		return this->first;
	}

	template<typename T> inline T&& JsonifierResultBase<T>::value() && noexcept(false) {
		return std::forward<JsonifierResultBase<T>>(*this).takeValue();
	}

	template<typename T> inline T&& JsonifierResultBase<T>::takeValue() && noexcept(false) {
		if (error()) {
			throw error();
		}
		return std::forward<T>(this->first);
	}

	template<typename T> inline JsonifierResultBase<T>::operator T&&() && noexcept(false) {
		return std::forward<JsonifierResultBase<T>>(*this).takeValue();
	}


	template<typename T> inline const T& JsonifierResultBase<T>::valueUnsafe() const& noexcept {
		return this->first;
	}

	template<typename T> inline T&& JsonifierResultBase<T>::valueUnsafe() && noexcept {
		return std::forward<T>(this->first);
	}

	template<typename T>
	inline JsonifierResultBase<T>::JsonifierResultBase(T&& value, ErrorCode error) noexcept
		: std::pair<T, ErrorCode>(std::forward<T>(value), error) {
	}
	template<typename T> inline JsonifierResultBase<T>::JsonifierResultBase(ErrorCode error) noexcept : JsonifierResultBase(T{}, error) {
	}
	template<typename T>
	inline JsonifierResultBase<T>::JsonifierResultBase(T&& value) noexcept : JsonifierResultBase(std::forward<T>(value), Success) {
	}
	template<typename T> inline JsonifierResultBase<T>::JsonifierResultBase() noexcept : JsonifierResultBase(T{}, Uninitialized) {
	}

	template<typename T> inline void JsonifierResult<T>::tie(T& value, ErrorCode& error) && noexcept {
		std::forward<JsonifierResultBase<T>>(*this).tie(value, error);
	}

	template<typename T>  inline ErrorCode JsonifierResult<T>::get(T& value) && noexcept {
		return std::forward<JsonifierResultBase<T>>(*this).get(value);
	}

	template<typename T> inline ErrorCode JsonifierResult<T>::error() const noexcept {
		return JsonifierResultBase<T>::error();
	}

	template<typename T> inline T& JsonifierResult<T>::value() & noexcept(false) {
		return JsonifierResultBase<T>::value();
	}

	template<typename T> inline T&& JsonifierResult<T>::value() && noexcept(false) {
		return std::forward<JsonifierResultBase<T>>(*this).value();
	}

	template<typename T> inline T&& JsonifierResult<T>::takeValue() && noexcept(false) {
		return std::forward<JsonifierResultBase<T>>(*this).takeValue();
	}

	template<typename T> inline JsonifierResult<T>::operator T&&() && noexcept(false) {
		return std::forward<JsonifierResultBase<T>>(*this).takeValue();
	}

	template<typename T> inline const T& JsonifierResult<T>::valueUnsafe() const& noexcept {
		return JsonifierResultBase<T>::valueUnsafe();
	}

	template<typename T> inline T&& JsonifierResult<T>::valueUnsafe() && noexcept {
		return std::forward<JsonifierResultBase<T>>(*this).valueUnsafe();
	}

	template<typename T>
	inline JsonifierResult<T>::JsonifierResult(T&& value, ErrorCode error) noexcept
		: JsonifierResultBase<T>(std::forward<T>(value), error) {
	}
	template<typename T> inline JsonifierResult<T>::JsonifierResult(ErrorCode error) noexcept : JsonifierResultBase<T>(error) {
	}
	template<typename T>
	inline JsonifierResult<T>::JsonifierResult(T&& value) noexcept : JsonifierResultBase<T>(std::forward<T>(value)) {
	}
	template<typename T> inline JsonifierResult<T>::JsonifierResult() noexcept : JsonifierResultBase<T>() {
	}

	inline bool RawJsonString::unsafeIsEqual(size_t length, std::string_view target) const noexcept {
		// If we are going to call memcmp, then we must know something about the length of the RawJsonString.
		return (length >= target.size()) && (raw()[target.size()] == '"') && !memcmp(raw(), target.data(), target.size());
	}

	inline bool RawJsonString::unsafeIsEqual(std::string_view target) const noexcept {
		// Assumptions: does not contain unescaped quote characters, and
		// the raw content is quote terminated within a valid JSON string.
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
			// if target is a compile-time constant and it is free from
			// quotes, then the next part could get optimized away through
			// inlining.
			if ((target[pos] == '"') && !escaping) {
				// We have reached the end of the RawJsonString but
				// the target is not done.
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
		// Assumptions: 'target' does not contain unescaped quote characters, is null terminated and
		// the raw content is quote terminated within a valid JSON string.
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
		// Assumptions: does not contain unescaped quote characters, and
		// the raw content is quote terminated within a valid JSON string.
		const char* r{ raw() };
		size_t pos{ 0 };
		bool escaping{ false };
		for (; target[pos]; pos++) {
			if (r[pos] != target[pos]) {
				return false;
			}
			// if target is a compile-time constant and it is free from
			// quotes, then the next part could get optimized away through
			// inlining.
			if ((target[pos] == '"') && !escaping) {
				// We have reached the end of the RawJsonString but
				// the target is not done.
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

	inline bool operator==(const RawJsonString& a, std::string_view c) noexcept {
		return a.unsafeIsEqual(c);
	}

	inline bool operator==(std::string_view c, const RawJsonString& a) noexcept {
		return a == c;
	}

	inline bool operator!=(const RawJsonString& a, std::string_view c) noexcept {
		return !(a == c);
	}

	inline bool operator!=(std::string_view c, const RawJsonString& a) noexcept {
		return !(a == c);
	}


	inline JsonifierResult<std::string_view> RawJsonString::unescape(JsonIterator& iter) const noexcept {
		return iter.unescape(*this);
	}


	inline std::ostream& operator<<(std::ostream& out, const RawJsonString& str) noexcept {
		bool in_escape = false;
		const char* s = str.raw();
		while (true) {
			switch (*s) {
				case '\\':
					in_escape = !in_escape;
					break;
				case '"':
					if (in_escape) {
						in_escape = false;
					} else {
						return out;
					}
					break;
				default:
					if (in_escape) {
						in_escape = false;
					}
			}
			out << *s;
			s++;
		}
	}

	inline JsonifierResult<RawJsonString>::JsonifierResult(
		RawJsonString&& value) noexcept
		: ImplementationJsonifierResultBase<RawJsonString>(
			  std::forward<RawJsonString>(value)) {
	}
	inline JsonifierResult<RawJsonString>::JsonifierResult(ErrorCode error) noexcept
		: ImplementationJsonifierResultBase<RawJsonString>(error) {
	}

	inline JsonifierResult<const char*> JsonifierResult<RawJsonString>::raw() const noexcept {
		if (error()) {
			return error();
		}
		return first.raw();
	}
	inline JsonifierResult<std::string_view>
	JsonifierResult<RawJsonString>::unescape(
		JsonIterator& iter) const noexcept {
		if (error()) {
			return error();
		}
		return first.unescape(iter);
	}


	inline TokenIterator::TokenIterator(const uint8_t* _buf, uint32_t* position) noexcept
		: stringView{ _buf }, currentPosition{ position } {
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
		return *(currentPosition + 1) - *position;
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

	inline JsonifierResult<TokenIterator>::JsonifierResult(
		TokenIterator&& value) noexcept
		: ImplementationJsonifierResultBase<TokenIterator>(
			  std::forward<TokenIterator>(value)) {
	}
	inline JsonifierResult<TokenIterator>::JsonifierResult(ErrorCode error) noexcept
		: ImplementationJsonifierResultBase<TokenIterator>(error) {
	}

	inline JsonIterator::JsonIterator(JsonIterator&& other) noexcept
		: token(std::forward<TokenIterator>(other.token)), parser{ other.parser }, stringBuffer{ other.stringBuffer }, error{ other.error },
		  currentDepth{ other.currentDepth }, rootStructural{ other.rootStructural } {
	}

	inline JsonIterator& JsonIterator::operator=(JsonIterator&& other) noexcept {
		this->rootStructural = other.rootStructural;
		this->currentDepth = other.currentDepth;
		this->stringBuffer = other.stringBuffer;
		this->parser = other.parser;
		this->error = other.error;
		this->token = other.token;
		return *this;
	}

	inline JsonIterator::JsonIterator(Parser* _parser) noexcept
		: token(_parser->getStringView(), _parser->getStructuralIndices()), parser{ _parser }, stringBuffer{ _parser->getStringBuffer() },
		  currentDepth{ 1 }, rootStructural{ _parser->getStructuralIndices() } {};

	inline void JsonIterator::rewind() noexcept {
		token.setPosition(rootPosition());
		stringBuffer = parser->getStringBuffer();
		currentDepth = 1;
	}

	inline bool JsonIterator::balanced() const noexcept {
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
			return ErrorCode::Success;
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
					return ErrorCode::Success;
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
					return ErrorCode::Success;
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
						return ErrorCode::Success;
					}
					break;
				default:
					break;
			}
		}

		return reportError(Tape_Error, "not enough close braces");
	}

	inline bool JsonIterator::atRoot() const noexcept {
		return position() == rootPosition();
	}

	inline bool JsonIterator::isSingleToken() const noexcept {
		return parser->tapeLength == 1;
	}

	inline uint32_t* JsonIterator::rootPosition() const noexcept {
		return rootStructural;
	}

	inline void JsonIterator::assertAtDocumentDepth() const noexcept {
		assert(currentDepth == 1);
	}

	inline void JsonIterator::assertAtRoot() const noexcept {
		assert(currentDepth == 1);
	}

	inline bool JsonIterator::atEnd() const noexcept {
		return position() == endPosition();
	}
	inline uint32_t* JsonIterator::endPosition() const noexcept {
		size_t n_structural_indexes{ parser->getTapeLength() };
		return &parser->getStructuralIndices()[n_structural_indexes];
	}

	inline std::string JsonIterator::toString() const noexcept {
		if (!isAlive()) {
			return "dead JsonIterator instance";
		}
		const char* current_structural = reinterpret_cast<const char*>(token.peek());
		return std::string("JsonIterator [ depth : ") + std::to_string(currentDepth) + std::string(", structural : '") +
			std::string(current_structural, 1) + std::string("', offset : ") + std::to_string(token.currentOffset()) + std::string("', error : ") +
			std::to_string(error) + std::string(" ]");
	}

	inline JsonifierResult<const char*> JsonIterator::currentLocation() noexcept {
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

	inline bool JsonIterator::isAlive() const noexcept {
		return parser;
	}

	inline void JsonIterator::abandon() noexcept {
		parser = nullptr;
		currentDepth = 0;
	}

	inline const uint8_t* JsonIterator::returnCurrentAndAdvance() noexcept {
		return token.returnCurrentAndAdvance();
	}

	inline const uint8_t* JsonIterator::unsafePointer() const noexcept {
		return token.peek(0);
	}

	inline const uint8_t* JsonIterator::peek(int32_t delta) const noexcept {
		return token.peek(delta);
	}

	inline uint32_t JsonIterator::peekLength(int32_t delta) const noexcept {
		return token.peekLength(delta);
	}

	inline const uint8_t* JsonIterator::peek(uint32_t* position) const noexcept {
		return token.peek(position);
	}

	inline uint32_t JsonIterator::peekLength(uint32_t* position) const noexcept {
		return token.peekLength(position);
	}

	inline uint32_t* JsonIterator::lastPosition() const noexcept {
		// The following line fails under some compilers...
		// assert(parser->implementation->n_structural_indexes > 0);
		// since it has side-effects.
		size_t n_structural_indexes{ parser->getTapeLength() };
		assert(n_structural_indexes > 0);
		return &parser->getStructuralIndices()[n_structural_indexes - 1];
	}
	inline const uint8_t* JsonIterator::peekLast() const noexcept {
		return token.peek(lastPosition());
	}

	inline void JsonIterator::ascendTo(size_t parent_depth) noexcept {
		assert(parent_depth >= 0 && parent_depth < INT32_MAX - 1);
		assert(currentDepth == parent_depth + 1);
		currentDepth = parent_depth;
	}

	inline void JsonIterator::descendTo(size_t child_depth) noexcept {
		assert(child_depth >= 1 && child_depth < INT32_MAX);
		assert(currentDepth == child_depth - 1);
		currentDepth = child_depth;
	}

	inline size_t JsonIterator::depth() const noexcept {
		return currentDepth;
	}

	inline uint8_t*& JsonIterator::stringBufLoc() noexcept {
		return stringBuffer;
	}

	inline ErrorCode JsonIterator::reportError(ErrorCode _error, const char* message) noexcept {
		assert(_error != Success && _error != Uninitialized&& _error != INCORRECT_TYPE && _error != No_Such_Field);
		error = _error;
		return error;
	}

	inline uint32_t* JsonIterator::position() const noexcept {
		return token.position();
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

	inline void JsonIterator::reenterChild(uint32_t* position, size_t child_depth) noexcept {
		assert(child_depth >= 1 && child_depth < INT32_MAX);
		assert(currentDepth == child_depth - 1);
		token.setPosition(position);
		currentDepth = child_depth;
	}

	inline ErrorCode JsonIterator::optionalError(ErrorCode _error, const char* message) noexcept {
		assert(_error == Incorrect_Type || _error == No_Such_Field);
		return _error;
	}

	template<int N>
	inline bool JsonIterator::copyToBuffer(const uint8_t* json, uint32_t max_len,
		uint8_t (&tmpbuf)[N]) noexcept {
		// Let us guard against silly cases:
		if ((N < max_len) || (N == 0)) {
			return false;
		}
		// Truncate whitespace to fit the buffer.
		if (max_len > N - 1) {
			// if (jsoncharutils::isNot_structural_or_whitespace(json[N-1])) { return false; }
			max_len = N - 1;
		}

		// Copy to the buffer.
		std::memcpy(tmpbuf, json, max_len);
		tmpbuf[max_len] = ' ';
		return true;
	}

	inline JsonifierResult<JsonIterator>::JsonifierResult(
	JsonIterator&& value) noexcept
	: ImplementationJsonifierResultBase<JsonIterator>(
			std::forward<JsonIterator>(value)) {
	}
	inline JsonifierResult<JsonIterator>::JsonifierResult(ErrorCode error) noexcept
	: ImplementationJsonifierResultBase<JsonIterator>(error) {
	}

	inline ValueIterator::ValueIterator(JsonIterator* jsonIter, size_t depth, uint32_t* startPosition) noexcept
		: jsonIterator{ jsonIter }, currentDepth{ depth }, rootStructural{ startPosition } {
	}

	inline JsonifierResult<bool> ValueIterator::startObject() noexcept {
		JsonifierTry(startContainer('{', "Not an object", "object"));
		return startedObject();
	}

	inline JsonifierResult<bool> ValueIterator::startRootObject() noexcept {
		JsonifierTry(startContainer('{', "Not an object", "object"));
		return startedRootObject();
	}

	inline JsonifierResult<bool> ValueIterator::startedObject() noexcept {
		assertAtContainerStart();
		if (*jsonIterator->peek() == '}') {
			jsonIterator->returnCurrentAndAdvance();
			endContainer();
			return false;
		}
		return true;
	}

	inline JsonifierResult<bool> ValueIterator::startedRootObject() noexcept {
		return startedObject();
	}

	inline ErrorCode ValueIterator::endContainer() noexcept {
		jsonIterator->ascendTo(depth() - 1);
		return ErrorCode::Success;
	}

	inline JsonifierResult<bool> ValueIterator::hasNextField() noexcept {
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
		ErrorCode error;
		bool has_value;
		if (atFirstField()) {
			has_value = true;
		} else if (!isOpen()) {
			return false;
		} else {
			if ((error = skipChild())) {
				abandon();
				return error;
			}
			if ((error = hasNextField().get(has_value))) {
				abandon();
				return error;
			}
		}
		while (has_value) {
			RawJsonString actual_key;
			if ((error = fieldKey().get(actual_key))) {
				abandon();
				return error;
			};
			if ((error = fieldValue())) {
				abandon();
				return error;
			}
			if (actual_key.unsafeIsEqual(key)) {
				return true;
			}
			JsonifierTry(skipChild());
			if ((error = hasNextField().get(has_value))) {
				abandon();
				return error;
			}
		}

		return false;
	}

	inline JsonifierResult<bool> ValueIterator::findFieldUnorderedRaw(const std::string_view key) noexcept {
		ErrorCode error;
		bool has_value;
		uint32_t* search_start = jsonIterator->position();
		bool at_first = atFirstField();
		if (at_first) {
			has_value = true;
		} else if (!isOpen()) {
			JsonifierTry(resetObject().get(has_value));
			at_first = true;
		} else {
			if ((error = skipChild())) {
				abandon();
				return error;
			}
			search_start = jsonIterator->position();
			if ((error = hasNextField().get(has_value))) {
				abandon();
				return error;
			}
		}
		while (has_value) {
			assert(jsonIterator->currentDepth == currentDepth);
			RawJsonString actual_key;
			if ((error = fieldKey().get(actual_key))) {
				abandon();
				return error;
			};
			if ((error = fieldValue())) {
				abandon();
				return error;
			}
			if (actual_key.unsafeIsEqual(key)) {
				return true;
			}
			JsonifierTry(skipChild());
			if ((error = hasNextField().get(has_value))) {
				abandon();
				return error;
			}
		}
		if (at_first) {
			return false;
		}
		JsonifierTry(resetObject().get(has_value));
		while (true) {
			assert(has_value);
			assert(jsonIterator->currentDepth == currentDepth);
			RawJsonString actual_key;
			error = fieldKey().get(actual_key);
			assert(!error);
			error = fieldValue();
			assert(!error);
			if (actual_key.unsafeIsEqual(key)) {
				return true;
			}
			JsonifierTry(skipChild());
			if (jsonIterator->position() == search_start) {
				return false;
			}
			error = hasNextField().get(has_value);
			assert(!error);
		}
		return false;
	}

	inline JsonifierResult<RawJsonString> ValueIterator::fieldKey() noexcept {
		assertAtNext();

		const uint8_t* key = jsonIterator->returnCurrentAndAdvance();
		if (*(key++) != '"') {
			return reportError(Tape_Error, "Object key is not a string");
		}
		return RawJsonString(key);
	}

	inline ErrorCode ValueIterator::fieldValue() noexcept {
		assertAtNext();

		if (*jsonIterator->returnCurrentAndAdvance() != ':') {
			return reportError(Tape_Error, "Missing colon in object field");
		}
		jsonIterator->descendTo(depth() + 1);
		return ErrorCode::Success;
	}

	inline JsonifierResult<bool> ValueIterator::startArray() noexcept {
		JsonifierTry(startContainer('[', "Not an array", "array"));
		return startedArray();
	}

	inline JsonifierResult<bool> ValueIterator::starRootArray() noexcept {
		JsonifierTry(startContainer('[', "Not an array", "array"));
		return startedRootArray();
	}

	inline std::string ValueIterator::toString() const noexcept {
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
		return true;
	}

	inline JsonifierResult<bool> ValueIterator::startedRootArray() noexcept {
		return startedArray();
	}

	inline JsonifierResult<bool> ValueIterator::hasNextElement() noexcept {
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

	inline JsonifierResult<bool> ValueIterator::parseBool(const uint8_t* json) const noexcept {
		auto not_true = str4ncmp(json, "true");
		auto not_false = str4ncmp(json, "fals") | (json[4] ^ 'e');
		bool error = (not_true && not_false) || isNotStructuralOrWhitespace(json[not_true ? 5 : 4]);
		if (error) {
			return incorrectTypeError("Not a boolean");
		}
		return JsonifierResult<bool>(!not_true);
	}
	inline JsonifierResult<bool> ValueIterator::parseNull(const uint8_t* json) const noexcept {
		bool isNull_string = !str4ncmp(json, "null") && isNotStructuralOrWhitespace(json[4]);
		// if we start with 'n', we must be a null
		if (!isNull_string && json[0] == 'n') {
			return incorrectTypeError("Not a null but starts with n");
		}
		return isNull_string;
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
		auto result = parseUnsigned(peekNonRootScalar("uint64"));
		if (result.error() == Success) {
			advanceNonRootScalar("uint64");
		}
		return result;
	}
	inline JsonifierResult<int64_t> ValueIterator::getInt64() noexcept {
		auto result = parseInteger(peekNonRootScalar("int64"));
		if (result.error() == Success) {
			advanceNonRootScalar("int64");
		}
		return result;
	}

	inline int64_t totalTimeNew{};
	inline int64_t iterationsNew{};

	inline JsonifierResult<double> ValueIterator::getDouble() noexcept {
		auto result = parseDouble(peekNonRootScalar("double"));
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
		bool isNull_value;
		JsonifierTry(parseNull(peekNonRootScalar("null")).get(isNull_value));
		if (isNull_value) {
			advanceNonRootScalar("null");
		}
		return isNull_value;
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
		uint8_t tmpbuf[20 + 1];// <20 digits> is the longest possible unsigned integer
		if (!jsonIterator->copyToBuffer(json, max_len, tmpbuf)) {
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

	inline JsonifierResult<int64_t> ValueIterator::getRootInt64() noexcept {
		auto max_len = peekStartLength();
		auto json = peekRootScalar("int64");
		uint8_t tmpbuf[20 + 1];// -<19 digits> is the longest possible integer
		if (!jsonIterator->copyToBuffer(json, max_len, tmpbuf)) {
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

	inline JsonifierResult<double> ValueIterator::getRootDouble() noexcept {
		auto max_len = peekStartLength();
		auto json = peekRootScalar("double");
		// Per https://www.exploringbinary.com/maximum-number-of-decimal-digits-in-binary-floating-point-numbers/,
		// 1074 is the maximum number of significant fractional digits. Add 8 more digits for the biggest
		// number: -0.<fraction>e-308.
		uint8_t tmpbuf[1074 + 8 + 1];
		if (!jsonIterator->copyToBuffer(json, max_len, tmpbuf)) {
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
		// If there is trailing content, then the Document is not null.
		if (!jsonIterator->isSingleToken()) {
			return false;
		}
		auto max_len = peekStartLength();
		auto json = peekRootScalar("null");
		bool result =
			(max_len >= 4 && !str4ncmp(json, "null") && (max_len == 4 || isNotStructuralOrWhitespace(json[5])));
		if (result) {
			advanceRootScalar("null");
		}
		return result;
	}

	inline ErrorCode ValueIterator::skipChild() noexcept {
		assert(jsonIterator->token.currentPosition > rootStructural);
		assert(jsonIterator->currentDepth >= currentDepth);

		return jsonIterator->skipChild(depth());
	}

	inline ValueIterator ValueIterator::child() const noexcept {
		assertAtChild();
		return { jsonIterator, depth() + 1, jsonIterator->token.position() };
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
		assert(jsonIterator->token.currentPosition > rootStructural);
		return jsonIterator->token.position() == startPosition() + 1;
	}

	inline void ValueIterator::abandon() noexcept {
		jsonIterator->abandon();
	}

	inline size_t ValueIterator::depth() const noexcept {
		return currentDepth;
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
		jsonIterator->ascendTo(depth() - 1);
	}

	inline ErrorCode ValueIterator::startContainer(uint8_t start_char, const char* incorrect_type_message,
		const char* type) noexcept {
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
		return ErrorCode::Success;
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
		jsonIterator->ascendTo(depth() - 1);
	}

	inline void ValueIterator::advanceNonRootScalar(const char* type) noexcept {
		if (!isAtStart()) {
			return;
		}

		assertAtNonRootStart();
		jsonIterator->returnCurrentAndAdvance();
		jsonIterator->ascendTo(depth() - 1);
	}

	inline ErrorCode ValueIterator::incorrectTypeError(const char* message) const noexcept {
		return Incorrect_Type;
	}

	inline bool ValueIterator::isAtStart() const noexcept {
		return position() == startPosition();
	}

	inline bool ValueIterator::isAtKey() const noexcept {
		return currentDepth == jsonIterator->currentDepth && *jsonIterator->peek() == '"';
	}

	inline bool ValueIterator::isAtIteratorStart() const noexcept {
		auto delta = position() - startPosition();
		return delta == 1 || delta == 2;
	}

	inline void ValueIterator::assertAtStart() const noexcept {
		assert(jsonIterator->token.currentPosition == rootStructural);
		assert(jsonIterator->currentDepth == currentDepth);
		assert(currentDepth > 0);
	}

	inline void ValueIterator::assertAtContainerStart() const noexcept {
		assert(jsonIterator->token.currentPosition == rootStructural + 1);
		assert(jsonIterator->currentDepth == currentDepth);
		assert(currentDepth > 0);
	}

	inline void ValueIterator::assertAtNext() const noexcept {
		assert(jsonIterator->token.currentPosition > rootStructural);
		assert(jsonIterator->currentDepth == currentDepth);
		assert(currentDepth > 0);
	}

	inline void ValueIterator::moveAtStart() noexcept {
		jsonIterator->currentDepth = currentDepth;
		jsonIterator->token.setPosition(rootStructural);
	}

	inline void ValueIterator::moveAtContainerStart() noexcept {
		jsonIterator->currentDepth = currentDepth;
		jsonIterator->token.setPosition(rootStructural + 1);
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
		assert(jsonIterator->token.currentPosition > rootStructural);
		assert(jsonIterator->currentDepth == currentDepth + 1);
		assert(currentDepth > 0);
	}

	inline void ValueIterator::assertAtRoot() const noexcept {
		assertAtStart();
		assert(currentDepth == 1);
	}

	inline void ValueIterator::assertAtNonRootStart() const noexcept {
		assertAtStart();
		assert(currentDepth > 1);
	}

	inline void ValueIterator::assertIsValid() const noexcept {
		assert(jsonIterator != nullptr);
	}

	inline bool ValueIterator::isValid() const noexcept {
		return jsonIterator != nullptr;
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
		return rootStructural;
	}

	inline uint32_t* ValueIterator::position() const noexcept {
		return jsonIterator->position();
	}

	inline uint32_t* ValueIterator::endPosition() const noexcept {
		return jsonIterator->endPosition();
	}

	inline uint32_t* ValueIterator::lastPosition() const noexcept {
		return jsonIterator->lastPosition();
	}

	inline ErrorCode ValueIterator::reportError(ErrorCode error, const char* message) noexcept {
		return jsonIterator->reportError(error, message);
	}

	inline JsonifierResult<ValueIterator>::JsonifierResult(
	ValueIterator&& value) noexcept
	: ImplementationJsonifierResultBase<ValueIterator>(
			std::forward<ValueIterator>(value)) {
	}
	inline JsonifierResult<ValueIterator>::JsonifierResult(ErrorCode error) noexcept
	: ImplementationJsonifierResultBase<ValueIterator>(error) {
	}

	inline ArrayIterator::ArrayIterator(const ValueIterator& _iterator) noexcept : iterator{ _iterator } {
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
		ErrorCode error;
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

	inline JsonifierResult<ArrayIterator>::JsonifierResult(
	ArrayIterator&& Value) noexcept
	: ImplementationJsonifierResultBase<ArrayIterator>(
			std::forward<ArrayIterator>(Value)) {
		first.iterator.assertIsValid();
	}
	inline JsonifierResult<ArrayIterator>::JsonifierResult(ErrorCode error) noexcept
	: ImplementationJsonifierResultBase<ArrayIterator>({}, error) {
	}

	inline JsonifierResult<Value>
	JsonifierResult<ArrayIterator>::operator*() noexcept {
	if (error()) {
		return error();
	}
	return *first;
	}
	inline bool JsonifierResult<ArrayIterator>::operator==(
	const JsonifierResult<ArrayIterator>& other) const noexcept {
	if (!first.iterator.isValid()) {
		return !error();
	}
	return first == other.first;
	}
	inline bool JsonifierResult<ArrayIterator>::operator!=(
	const JsonifierResult<ArrayIterator>& other) const noexcept {
		if (!first.iterator.isValid()) {
			return error();
		}
		return first != other.first;
	}
	inline JsonifierResult<ArrayIterator>& JsonifierResult<ArrayIterator>::operator++() noexcept {

		if (error()) {
			second = ErrorCode::Success;
			return *this;
		}
		++(first);
		return *this;
	}

	inline ObjectIterator::ObjectIterator(const ValueIterator& _iterator) noexcept : iterator{ _iterator } {
	}

	inline JsonifierResult<Field> ObjectIterator::operator*() noexcept {
		ErrorCode error = iterator.error();
		if (error) {
			iterator.abandon();
			return error;
		}
		auto result = Field::start(iterator);
		if (result.error()) {
			iterator.abandon();
		}
		return result;
	}
	inline bool ObjectIterator::operator==(const ObjectIterator& other) const noexcept {
		return !(*this != other);
	}
	inline bool ObjectIterator::operator!=(const ObjectIterator&) const noexcept {
		return iterator.isOpen();
	}


	inline ObjectIterator& ObjectIterator::operator++() noexcept {
		if (!iterator.isOpen()) {
			return *this;
		}

		ErrorCode error;
		if ((error = iterator.skipChild())) {
			return *this;
		}

		bool has_Value;
		if ((error = iterator.hasNextField().get(has_Value))) {
			return *this;
		};
		return *this;
	}

	inline JsonifierResult<ObjectIterator>::JsonifierResult(
	ObjectIterator&& Value) noexcept
	: ImplementationJsonifierResultBase<ObjectIterator>(
			std::forward<ObjectIterator>(Value)) {
	first.iterator.assertIsValid();
	}
	inline JsonifierResult<ObjectIterator>::JsonifierResult(ErrorCode error) noexcept
	: ImplementationJsonifierResultBase<ObjectIterator>({}, error) {
	}

	inline JsonifierResult<Field>
	JsonifierResult<ObjectIterator>::operator*() noexcept {
	if (error()) {
		return error();
	}
	return *first;
	}
	inline bool JsonifierResult<ObjectIterator>::operator==(
	const JsonifierResult<ObjectIterator>& other) const noexcept {
	if (!first.iterator.isValid()) {
		return !error();
	}
	return first == other.first;
	}
	inline bool JsonifierResult<ObjectIterator>::operator!=(
	const JsonifierResult<ObjectIterator>& other) const noexcept {
	if (!first.iterator.isValid()) {
		return error();
	}
	return first != other.first;
	}
	inline JsonifierResult<ObjectIterator>&
	JsonifierResult<ObjectIterator>::operator++() noexcept {
	if (error()) {
		second = ErrorCode::Success;
		return *this;
	}
	++first;
	return *this;
	}
	inline Array::Array(const ValueIterator& _iterator) noexcept : iterator{ _iterator } {
	}

	inline JsonifierResult<Array> Array::start(ValueIterator& iterator) noexcept {
		bool has_Value;
		JsonifierTry(iterator.startArray().get(has_Value));
		return Array(iterator);
	}
	inline JsonifierResult<Array> Array::startRoot(ValueIterator& iterator) noexcept {
		bool has_Value;
		JsonifierTry(iterator.starRootArray().get(has_Value));
		return Array(iterator);
	}
	inline JsonifierResult<Array> Array::started(ValueIterator& iterator) noexcept {
		bool has_Value;
		JsonifierTry(iterator.startedArray().get(has_Value));
		return Array(iterator);
	}

	inline JsonifierResult<ArrayIterator> Array::begin() noexcept {
		return ArrayIterator(iterator);
	}
	inline JsonifierResult<ArrayIterator> Array::end() noexcept {
		return ArrayIterator(iterator);
	}
	inline ErrorCode Array::consume() noexcept {
		auto error = iterator.jsonIter().skipChild(iterator.depth() - 1);
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
		bool isNot_empty;
		auto error = iterator.resetArray().get(isNot_empty);
		if (error) {
			return error;
		}
		return !isNot_empty;
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
			return Out_Of_Bounds;
		}

		size_t Array_index = 0;
		size_t i;
		for (i = 0; i < json_pointer.length() && json_pointer[i] != '/'; i++) {
			uint8_t digit = uint8_t(json_pointer[i] - '0');
			if (digit > 9) {
				return Incorrect_Type;
			}
			Array_index = Array_index * 10 + digit;
		}

		if (i > 1 && json_pointer[0] == '0') {
			return Invalid_Json_Pointer;
		}

		if (i == 0) {
			return Invalid_Json_Pointer;
		}
		auto child = at(Array_index);
		if (child.error()) {
			return child;
		}

		if (i < json_pointer.length()) {
			child = child.atPointer(json_pointer.substr(i));
		}
		return child;
	}

	inline JsonifierResult<Value> Array::at(size_t index) noexcept {
		size_t i = 0;
		for (auto Value: *this) {
			if (i == index) {
				return Value;
			}
			i++;
		}
		return Out_Of_Bounds;
	}

	inline JsonifierResult<Array>::JsonifierResult(
	Array&& Value) noexcept
	: ImplementationJsonifierResultBase<Array>(
			std::forward<Array>(Value)) {
	}
	inline JsonifierResult<Array>::JsonifierResult(ErrorCode error) noexcept
	: ImplementationJsonifierResultBase<Array>(error) {
	}

	inline JsonifierResult<ArrayIterator>
	JsonifierResult<Array>::begin() noexcept {
	if (error()) {
		return error();
	}
	return first.begin();
	}
	inline JsonifierResult<ArrayIterator>
	JsonifierResult<Array>::end() noexcept {
	if (error()) {
		return error();
	}
	return first.end();
	}
	inline JsonifierResult<size_t> JsonifierResult<Array>::countElements() & noexcept {
	if (error()) {
		return error();
	}
	return first.countElements();
	}
	inline JsonifierResult<bool> JsonifierResult<Array>::isEmpty() & noexcept {
	if (error()) {
		return error();
	}
	return first.isEmpty();
	}
	inline JsonifierResult<Value>
	JsonifierResult<Array>::at(size_t index) noexcept {
	if (error()) {
		return error();
	}
	return first.at(index);
	}
	inline JsonifierResult<Value>
	JsonifierResult<Array>::atPointer(std::string_view json_pointer) noexcept {
	if (error()) {
		return error();
	}
	return first.atPointer(json_pointer);
	}

	inline Document::Document(JsonIterator&& _iterator) noexcept : iterator{ std::forward<JsonIterator>(_iterator) } {
	}

	inline void Document::rewind() noexcept {
		iterator.rewind();
	}

	inline JsonifierResult<const char*> Document::currentLocation() noexcept {
		return iterator.currentLocation();
	}

	inline int32_t Document::currentDepth() const noexcept {
		return iterator.depth();
	}

	inline bool Document::isAlive() noexcept {
		return iterator.isAlive();
	}
	inline ValueIterator Document::resumeValueIterator() noexcept {
		return ValueIterator(&iterator, 1ull, iterator.rootPosition());
	}
	inline ValueIterator Document::getRootValueIterator() noexcept {
		return resumeValueIterator();
	}
	inline JsonifierResult<Object> Document::startOrResumeObject() noexcept {
		if (iterator.atRoot()) {
			return getObject();
		} else {
			return Object::resume(resumeValueIterator());
		}
	}
	inline JsonifierResult<Value> Document::getValue() noexcept {
		iterator.assertAtDocumentDepth();
		switch (*iterator.peek()) {
			case '[':
			case '{':
				return Value(getRootValueIterator());
			default:
				return Scalar_Document_As_Value;
		}
	}
	inline JsonifierResult<Array> Document::getArray() & noexcept {
		auto Value = getRootValueIterator();
		return Array::startRoot(Value);
	}
	inline JsonifierResult<Object> Document::getObject() & noexcept {
		auto Value = getRootValueIterator();
		return Object::startRoot(Value);
	}
	inline JsonifierResult<uint64_t> Document::getUint64() noexcept {
		return getRootValueIterator().getRootUint64();
	}
	inline JsonifierResult<int64_t> Document::getInt64() noexcept {
		return getRootValueIterator().getRootInt64();
	}
	inline JsonifierResult<double> Document::getDouble() noexcept {
		return getRootValueIterator().getRootDouble();
	}
	inline JsonifierResult<std::string_view> Document::getString() noexcept {
		return getRootValueIterator().getRootString();
	}
	inline JsonifierResult<RawJsonString> Document::getRawJsonString() noexcept {
		return getRootValueIterator().getRootRawJsonString();
	}
	inline JsonifierResult<bool> Document::getBool() noexcept {
		return getRootValueIterator().getRootBool();
	}
	inline JsonifierResult<bool> Document::isNull() noexcept {
		return getRootValueIterator().isRootNull();
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
		return getRawJsonString();
	}
	template<> inline JsonifierResult<std::string_view> Document::get() && noexcept {
		return getString();
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
		return getValue();
	}

	template<typename T> inline ErrorCode Document::get(T& out) & noexcept {
		return get<T>().get(out);
	}
	template<typename T> inline ErrorCode Document::get(T& out) && noexcept {
		return std::forward<Document>(*this).get<T>().get(out);
	}


	inline Document::operator Array() & noexcept(false) {
		return getArray();
	}
	inline Document::operator Object() & noexcept(false) {
		return getObject();
	}
	inline Document::operator uint64_t() noexcept(false) {
		return getUint64();
	}
	inline Document::operator int64_t() noexcept(false) {
		return getInt64();
	}
	inline Document::operator double() noexcept(false) {
		return getDouble();
	}
	inline Document::operator std::string_view() noexcept(false) {
		return getString();
	}
	inline Document::operator RawJsonString() noexcept(false) {
		return getRawJsonString();
	}
	inline Document::operator bool() noexcept(false) {
		return getBool();
	}
	inline Document::operator Value() noexcept(false) {
		return getValue();
	}


	inline JsonifierResult<size_t> Document::countElements() & noexcept {
		auto a = getArray();
		JsonifierResult<size_t> answer = a.countElements();
		if (answer.error() == Success) {
			rewind();
		}
		return answer;
	}

	inline JsonifierResult<size_t> Document::countFields() & noexcept {
		auto a = getObject();
		JsonifierResult<size_t> answer = a.countFields();
		if (answer.error() == Success) {
			rewind();
		}
		return answer;
	}
	inline JsonifierResult<Value> Document::at(size_t index) & noexcept {
		auto a = getArray();
		return a.at(index);
	}
	inline JsonifierResult<ArrayIterator> Document::begin() & noexcept {
		return getArray().begin();
	}
	inline JsonifierResult<ArrayIterator> Document::end() & noexcept {
		return {};
	}

	inline JsonifierResult<Value> Document::findField(std::string_view key) & noexcept {
		return startOrResumeObject().findField(key);
	}
	inline JsonifierResult<Value> Document::findField(const char* key) & noexcept {
		return startOrResumeObject().findField(key);
	}
	inline JsonifierResult<Value> Document::findFieldUnordered(std::string_view key) & noexcept {
		return startOrResumeObject().findFieldUnordered(key);
	}
	inline JsonifierResult<Value> Document::findFieldUnordered(const char* key) & noexcept {
		return startOrResumeObject().findFieldUnordered(key);
	}
	inline JsonifierResult<Value> Document::operator[](std::string_view key) & noexcept {
		return startOrResumeObject()[key];
	}
	inline JsonifierResult<Value> Document::operator[](const char* key) & noexcept {
		return startOrResumeObject()[key];
	}

	inline ErrorCode Document::consume() noexcept {
		auto error = iterator.skipChild(0);
		if (error) {
			iterator.abandon();
		}
		return error;
	}

	inline JsonifierResult<std::string_view> Document::rawJson() noexcept {
		auto _iterator = getRootValueIterator();
		const uint8_t* starting_point{ _iterator.peekStart() };
		auto error = consume();
		if (error) {
			return error;
		}
		const uint8_t* final_point{ iterator.unsafePointer() };
		return std::string_view(reinterpret_cast<const char*>(starting_point), size_t(final_point - starting_point));
	}

	inline JsonifierResult<JsonType> Document::type() noexcept {
		return getRootValueIterator().type();
	}

	inline JsonifierResult<bool> Document::isScalar() noexcept {
		JsonType this_type;
		auto error = type().get(this_type);
		if (error) {
			return error;
		}
		return !((this_type == JsonType::Array) || (this_type == JsonType::Object));
	}

	inline JsonifierResult<std::string_view> Document::rawJsonToken() noexcept {
		auto _iterator = getRootValueIterator();
		return std::string_view(reinterpret_cast<const char*>(_iterator.peekStart()), _iterator.peekStartLength());
	}

	inline JsonifierResult<Value> Document::atPointer(std::string_view json_pointer) noexcept {
		rewind();// Rewind the Document each time atPointer is called
		if (json_pointer.empty()) {
			return this->getValue();
		}
		JsonType t;
		JsonifierTry(type().get(t));
		switch (t) {
			case JsonType::Array:
				return (*this).getArray().atPointer(json_pointer);
			case JsonType::Object:
				return (*this).getObject().atPointer(json_pointer);
			default:
				return Invalid_Json_Pointer;
		}
	}

	inline JsonifierResult<Document>::JsonifierResult(
	Document&& Value) noexcept
	: ImplementationJsonifierResultBase<Document>(
			std::forward<Document>(Value)) {
	}
	inline JsonifierResult<Document>::JsonifierResult(ErrorCode error) noexcept
	: ImplementationJsonifierResultBase<Document>(error) {
	}
	inline JsonifierResult<size_t> JsonifierResult<Document>::countElements() & noexcept {
	if (error()) {
		return error();
	}
	return first.countElements();
	}
	inline JsonifierResult<size_t> JsonifierResult<Document>::countFields() & noexcept {
	if (error()) {
		return error();
	}
	return first.countFields();
	}
	inline JsonifierResult<Value>
	JsonifierResult<Document>::at(size_t index) & noexcept {
	if (error()) {
		return error();
	}
	return first.at(index);
	}
	inline ErrorCode JsonifierResult<Document>::rewind() noexcept {
	if (error()) {
		return error();
	}
	first.rewind();
	return ErrorCode::Success;
	}
	inline JsonifierResult<ArrayIterator>
	JsonifierResult<Document>::begin() & noexcept {
	if (error()) {
		return error();
	}
	return first.begin();
	}
	inline JsonifierResult<ArrayIterator>
	JsonifierResult<Document>::end() & noexcept {
	return {};
	}
	inline JsonifierResult<Value>
	JsonifierResult<Document>::findFieldUnordered(std::string_view key) & noexcept {
	if (error()) {
		return error();
	}
	return first.findFieldUnordered(key);
	}
	inline JsonifierResult<Value>
	JsonifierResult<Document>::findFieldUnordered(const char* key) & noexcept {
	if (error()) {
		return error();
	}
	return first.findFieldUnordered(key);
	}
	inline JsonifierResult<Value>
	JsonifierResult<Document>::operator[](std::string_view key) & noexcept {
	if (error()) {
		return error();
	}
	return first[key];
	}
	inline JsonifierResult<Value>
	JsonifierResult<Document>::operator[](const char* key) & noexcept {
	if (error()) {
		return error();
	}
	return first[key];
	}
	inline JsonifierResult<Value>
	JsonifierResult<Document>::findField(std::string_view key) & noexcept {
	if (error()) {
		return error();
	}
	return first.findField(key);
	}
	inline JsonifierResult<Value>
	JsonifierResult<Document>::findField(const char* key) & noexcept {
	if (error()) {
		return error();
	}
	return first.findField(key);
	}
	inline JsonifierResult<Array>
	JsonifierResult<Document>::getArray() & noexcept {
	if (error()) {
		return error();
	}
	return first.getArray();
	}
	inline JsonifierResult<Object>
	JsonifierResult<Document>::getObject() & noexcept {
	if (error()) {
		return error();
	}
	return first.getObject();
	}
	inline JsonifierResult<uint64_t> JsonifierResult<Document>::getUint64() noexcept {
	if (error()) {
		return error();
	}
	return first.getUint64();
	}
	inline JsonifierResult<int64_t> JsonifierResult<Document>::getInt64() noexcept {
	if (error()) {
		return error();
	}
	return first.getInt64();
	}
	inline JsonifierResult<double> JsonifierResult<Document>::getDouble() noexcept {
	if (error()) {
		return error();
	}
	return first.getDouble();
	}
	inline JsonifierResult<std::string_view> JsonifierResult<Document>::getString() noexcept {
	if (error()) {
		return error();
	}
	return first.getString();
	}
	inline JsonifierResult<RawJsonString>
	JsonifierResult<Document>::getRawJsonString() noexcept {
	if (error()) {
		return error();
	}
	return first.getRawJsonString();
	}
	inline JsonifierResult<bool> JsonifierResult<Document>::getBool() noexcept {
	if (error()) {
		return error();
	}
	return first.getBool();
	}
	inline JsonifierResult<Value>
	JsonifierResult<Document>::getValue() noexcept {
	if (error()) {
		return error();
	}
	return first.getValue();
	}
	inline JsonifierResult<bool> JsonifierResult<Document>::isNull() noexcept {
	if (error()) {
		return error();
	}
	return first.isNull();
	}

	template<typename T> inline JsonifierResult<T> JsonifierResult<Document>::get() & noexcept {
	if (error()) {
		return error();
	}
	return first.get<T>();
	}
	template<typename T> inline JsonifierResult<T> JsonifierResult<Document>::get() && noexcept {
	if (error()) {
		return error();
	}
	return std::forward<Document>(first).get<T>();
	}
	template<typename T> inline ErrorCode JsonifierResult<Document>::get(T& out) & noexcept {
	if (error()) {
		return error();
	}
	return first.get<T>(out);
	}
	template<typename T> inline ErrorCode JsonifierResult<Document>::get(T& out) && noexcept {
	if (error()) {
		return error();
	}
	return std::forward<Document>(first).get<T>(out);
	}

	template<>
	inline JsonifierResult<Document>
	JsonifierResult<Document>::get<Document>() & noexcept =
	delete;
	template<>
	inline JsonifierResult<Document>
	JsonifierResult<Document>::get<Document>() && noexcept {
	if (error()) {
		return error();
	}
	return std::forward<Document>(first);
	}
	template<>
	inline ErrorCode
	JsonifierResult<Document>::get<Document>(
	Document& out) & noexcept = delete;
	template<>
	inline ErrorCode
	JsonifierResult<Document>::get<Document>(
	Document& out) && noexcept {
	if (error()) {
		return error();
	}
	out = std::forward<Document>(first);
	return ErrorCode::Success;
	}

	inline JsonifierResult<JsonType>
	JsonifierResult<Document>::type() noexcept {
	if (error()) {
		return error();
	}
	return first.type();
	}

	inline JsonifierResult<bool> JsonifierResult<Document>::isScalar() noexcept {
	if (error()) {
		return error();
	}
	return first.isScalar();
	}

	inline
	JsonifierResult<Document>::operator Array() & noexcept(
		false) {
	if (error()) {
		throw error();
	}
	return first;
	}
	inline
	JsonifierResult<Document>::operator Object() & noexcept(
		false) {
	if (error()) {
		throw error();
	}
	return first;
	}
	inline JsonifierResult<Document>::operator uint64_t() noexcept(false) {
	if (error()) {
		throw error();
	}
	return first;
	}
	inline JsonifierResult<Document>::operator int64_t() noexcept(false) {
	if (error()) {
		throw error();
	}
	return first;
	}
	inline JsonifierResult<Document>::operator double() noexcept(false) {
	if (error()) {
		throw error();
	}
	return first;
	}
	inline JsonifierResult<Document>::operator std::string_view() noexcept(false) {
	if (error()) {
		throw error();
	}
	return first;
	}
	inline JsonifierResult<
	Document>::operator RawJsonString() noexcept(false) {
	if (error()) {
		throw error();
	}
	return first;
	}
	inline JsonifierResult<Document>::operator bool() noexcept(false) {
	if (error()) {
		throw error();
	}
	return first;
	}
	inline
	JsonifierResult<Document>::operator Value() noexcept(
		false) {
	if (error()) {
		throw error();
	}
	return first;
	}

	inline JsonifierResult<const char*> JsonifierResult<Document>::currentLocation() noexcept {
	if (error()) {
		return error();
	}
	return first.currentLocation();
	}

	inline int32_t JsonifierResult<Document>::currentDepth() const noexcept {
	if (error()) {
		return error();
	}
	return first.currentDepth();
	}

	inline JsonifierResult<std::string_view>
	JsonifierResult<Document>::rawJsonToken() noexcept {
	if (error()) {
		return error();
	}
	return first.rawJsonToken();
	}

	inline JsonifierResult<Value>
	JsonifierResult<Document>::atPointer(std::string_view json_pointer) noexcept {
	if (error()) {
		return error();
	}
	return first.atPointer(json_pointer);
	}

	inline Value::Value(const ValueIterator& _iterator) noexcept : iterator{ _iterator } {
	}
	inline Value Value::start(const ValueIterator& iterator) noexcept {
		return iterator;
	}
	inline Value Value::resume(const ValueIterator& iterator) noexcept {
		return iterator;
	}

	inline JsonifierResult<Array> Value::getArray() noexcept {
		return Array::start(iterator);
	}
	inline JsonifierResult<Object> Value::getObject() noexcept {
		return Object::start(iterator);
	}
	inline JsonifierResult<Object> Value::startOrResumeObject() noexcept {
		if (iterator.atStart()) {
			return getObject();
		} else {
			return Object::resume(iterator);
		}
	}

	inline JsonifierResult<RawJsonString> Value::getRawJsonString() noexcept {
		return iterator.getRawJsonString();
	}
	inline JsonifierResult<std::string_view> Value::getString() noexcept {
		return iterator.getString();
	}
	inline JsonifierResult<double> Value::getDouble() noexcept {
		return iterator.getDouble();
	}
	inline JsonifierResult<uint64_t> Value::getUint64() noexcept {
		return iterator.getUint64();
	}
	inline JsonifierResult<int64_t> Value::getInt64() noexcept {
		return iterator.getInt64();
	}
	inline JsonifierResult<bool> Value::getBool() noexcept {
		return iterator.getBool();
	}
	inline JsonifierResult<bool> Value::isNull() noexcept {
		return iterator.isNull();
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
		return get<T>().get(out);
	}


	inline Value::operator Array() noexcept(false) {
		return getArray();
	}
	inline Value::operator Object() noexcept(false) {
		return getObject();
	}
	inline Value::operator uint64_t() noexcept(false) {
		return getUint64();
	}
	inline Value::operator int64_t() noexcept(false) {
		return getInt64();
	}
	inline Value::operator double() noexcept(false) {
		return getDouble();
	}
	inline Value::operator std::string_view() noexcept(false) {
		return getString();
	}
	inline Value::operator RawJsonString() noexcept(false) {
		return getRawJsonString();
	}
	inline Value::operator bool() noexcept(false) {
		return getBool();
	}

	inline JsonifierResult<ArrayIterator> Value::begin() & noexcept {
		return getArray().begin();
	}
	inline JsonifierResult<ArrayIterator> Value::end() & noexcept {
		return {};
	}
	inline JsonifierResult<size_t> Value::countElements() & noexcept {
		JsonifierResult<size_t> answer;
		auto a = getArray();
		answer = a.countElements();
		iterator.moveAtStart();
		return answer;
	}
	inline JsonifierResult<size_t> Value::countFields() & noexcept {
		JsonifierResult<size_t> answer;
		auto a = getObject();
		answer = a.countFields();
		iterator.moveAtStart();
		return answer;
	}
	inline JsonifierResult<Value> Value::at(size_t index) noexcept {
		auto a = getArray();
		return a.at(index);
	}

	inline JsonifierResult<Value> Value::findField(std::string_view key) noexcept {
		return startOrResumeObject().findField(key);
	}
	inline JsonifierResult<Value> Value::findField(const char* key) noexcept {
		return startOrResumeObject().findField(key);
	}

	inline JsonifierResult<Value> Value::findFieldUnordered(std::string_view key) noexcept {
		return startOrResumeObject().findFieldUnordered(key);
	}
	inline JsonifierResult<Value> Value::findFieldUnordered(const char* key) noexcept {
		return startOrResumeObject().findFieldUnordered(key);
	}

	inline JsonifierResult<Value> Value::operator[](std::string_view key) noexcept {
		return startOrResumeObject()[key];
	}

	inline int64_t totalTime{};
	inline int64_t iteratorations{};

	inline JsonifierResult<Value> Value::operator[](const char* key) noexcept {
		auto result = startOrResumeObject()[key];
		return result;
	}

	inline JsonifierResult<JsonType> Value::type() noexcept {
		return iterator.type();
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
		return std::string_view(reinterpret_cast<const char*>(iterator.peekStart()), iterator.peekStartLength());
	}

	inline JsonifierResult<const char*> Value::currentLocation() noexcept {
		return iterator.jsonIter().currentLocation();
	}

	inline int32_t Value::currentDepth() const noexcept {
		return iterator.jsonIter().depth();
	}

	inline JsonifierResult<Value> Value::atPointer(std::string_view json_pointer) noexcept {
		JsonType t;
		JsonifierTry(type().get(t));
		switch (t) {
			case JsonType::Array:
				return (*this).getArray().atPointer(json_pointer);
			case JsonType::Object:
				return (*this).getObject().atPointer(json_pointer);
			default:
				return Invalid_Json_Pointer;
		}
	}

	inline JsonifierResult<Value>::JsonifierResult(Value&& value) noexcept : ImplementationJsonifierResultBase<Value>(std::forward<Value>(value)){}
	inline JsonifierResult<Value>::JsonifierResult(ErrorCode error) noexcept : ImplementationJsonifierResultBase<Value>(error){}

	inline JsonifierResult<size_t> JsonifierResult<Value>::countElements() & noexcept {
		if (error()) {
			return error();
		}
		return first.countElements();
	}
	inline JsonifierResult<size_t> JsonifierResult<Value>::countFields() & noexcept {
		if (error()) {
			return error();
		}
		return first.countFields();
	}
	inline JsonifierResult<Value>
	JsonifierResult<Value>::at(size_t index) noexcept {
		if (error()) {
			return error();
		}
		return first.at(index);
	}
	inline JsonifierResult<ArrayIterator>
	JsonifierResult<Value>::begin() & noexcept {
		if (error()) {
			return error();
		}
		return first.begin();
	}
	inline JsonifierResult<ArrayIterator>
	JsonifierResult<Value>::end() & noexcept {
		if (error()) {
			return error();
		}
		return {};
	}

	inline JsonifierResult<Value>
	JsonifierResult<Value>::findField(std::string_view key) noexcept {
		if (error()) {
			return error();
		}
		return first.findField(key);
	}
	inline JsonifierResult<Value>
	JsonifierResult<Value>::findField(const char* key) noexcept {
		if (error()) {
			return error();
		}
		return first.findField(key);
	}

	inline JsonifierResult<Value>
	JsonifierResult<Value>::findFieldUnordered(std::string_view key) noexcept {
		if (error()) {
			return error();
		}
		return first.findFieldUnordered(key);
	}
	inline JsonifierResult<Value>
	JsonifierResult<Value>::findFieldUnordered(const char* key) noexcept {
		if (error()) {
			return error();
		}
		return first.findFieldUnordered(key);
	}

	inline JsonifierResult<Value>
	JsonifierResult<Value>::operator[](std::string_view key) noexcept {
		if (error()) {
			return error();
		}
		return first[key];
	}
	inline JsonifierResult<Value>
	JsonifierResult<Value>::operator[](const char* key) noexcept {
		if (error()) {
			return error();
		}
		return first[key];
	}

	inline JsonifierResult<Array>
	JsonifierResult<Value>::getArray() noexcept {
		if (error()) {
			return error();
		}
		return first.getArray();
	}
	inline JsonifierResult<Object>
	JsonifierResult<Value>::getObject() noexcept {
		if (error()) {
			return error();
		}
		return first.getObject();
	}
	inline JsonifierResult<uint64_t> JsonifierResult<Value>::getUint64() noexcept {
		if (error()) {
			return error();
		}
		return first.getUint64();
	}

	inline JsonifierResult<int64_t> JsonifierResult<Value>::getInt64() noexcept {
		if (error()) {
			return error();
		}
		return first.getInt64();
	}

	inline JsonifierResult<double> JsonifierResult<Value>::getDouble() noexcept {
		if (error()) {
			return error();
		}
		auto result = first.getDouble();
		return result;
	}
	
	inline JsonifierResult<std::string_view> JsonifierResult<Value>::getString() noexcept {
		if (error()) {
			return error();
		}
		return first.getString();
	}
	inline JsonifierResult<RawJsonString>
	JsonifierResult<Value>::getRawJsonString() noexcept {
		if (error()) {
			return error();
		}
		return first.getRawJsonString();
	}
	inline JsonifierResult<bool> JsonifierResult<Value>::getBool() noexcept {
		if (error()) {
			return error();
		}
		return first.getBool();
	}
	inline JsonifierResult<bool> JsonifierResult<Value>::isNull() noexcept {
		if (error()) {
			return error();
		}
		return first.isNull();
	}

	template<typename T> inline JsonifierResult<T> JsonifierResult<Value>::get() noexcept {
		if (error()) {
			return error();
		}
		return first.get<T>();
	}
	template<typename T> inline ErrorCode JsonifierResult<Value>::get(T& out) noexcept {
		if (error()) {
			return error();
		}
		return first.get<T>(out);
	}

	template<>
	inline JsonifierResult<Value>
	JsonifierResult<Value>::get<Value>() noexcept {
		if (error()) {
			return error();
		}
		return std::move(first);
	}
	template<>
	inline ErrorCode
	JsonifierResult<Value>::get<Value>(
		Value& out) noexcept {
		if (error()) {
			return error();
		}
		out = first;
		return ErrorCode::Success;
	}

	inline JsonifierResult<JsonType>
	JsonifierResult<Value>::type() noexcept {
		if (error()) {
			return error();
		}
		return first.type();
	}
	inline JsonifierResult<bool> JsonifierResult<Value>::isScalar() noexcept {
		if (error()) {
			return error();
		}
		return first.isScalar();
	}

	inline
		JsonifierResult<Value>::operator Array() noexcept(
			false) {
		if (error()) {
			throw error();
		}
		return first;
	}
	inline
		JsonifierResult<Value>::operator Object() noexcept(
			false) {
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
	inline JsonifierResult<
		Value>::operator RawJsonString() noexcept(false) {
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
	inline JsonifierResult<std::string_view> JsonifierResult<Value>::rawJsonToken() noexcept {
		if (error()) {
			return error();
		}
		return first.rawJsonToken();
	}

	inline JsonifierResult<const char*> JsonifierResult<Value>::currentLocation() noexcept {
		if (error()) {
			return error();
		}
		return first.currentLocation();
	}

	inline JsonifierResult<int32_t> JsonifierResult<Value>::currentDepth() const noexcept {
		if (error()) {
			return error();
		}
		return first.currentDepth();
	}

	inline JsonifierResult<Value>
	JsonifierResult<Value>::atPointer(std::string_view json_pointer) noexcept {
		if (error()) {
			return error();
		}
		return first.atPointer(json_pointer);
	}

	inline Field::Field() noexcept : std::pair<RawJsonString, Value>() {
	}

	inline Field::Field(RawJsonString key, Value&& value) noexcept
		: std::pair<RawJsonString, Value>(key, std::forward<Value>(value)) {
	}

	inline JsonifierResult<Field> Field::start(ValueIterator& parent_iterator) noexcept {
		RawJsonString key;
		JsonifierTry(parent_iterator.fieldKey().get(key));
		JsonifierTry(parent_iterator.fieldValue());
		return Field::start(parent_iterator, key);
	}

	inline JsonifierResult<Field> Field::start(const ValueIterator& parent_iterator, RawJsonString key) noexcept {
		return Field(key, parent_iterator.child());
	}

	inline JsonifierResult<std::string_view> Field::unescapedKey() noexcept {
		assert(first.stringView != nullptr);
		JsonifierResult<std::string_view> answer = first.unescape(second.iterator.jsonIter());
		first.consume();
		return answer;
	}

	inline RawJsonString Field::key() const noexcept {
		assert(first.stringView != nullptr);
		return first;
	}

	inline Value& Field::value() & noexcept {
		return second;
	}

	inline Value Field::value() && noexcept {
		return std::forward<Field>(*this).second;
	}

	inline JsonifierResult<Field>::JsonifierResult(
		Field&& Value) noexcept
		: ImplementationJsonifierResultBase<Field>(
			  std::forward<Field>(Value)) {
	}
	inline JsonifierResult<Field>::JsonifierResult(ErrorCode error) noexcept
		: ImplementationJsonifierResultBase<Field>(error) {
	}

	inline JsonifierResult<RawJsonString>
	JsonifierResult<Field>::key() noexcept {
		if (error()) {
			return error();
		}
		return first.key();
	}
	inline JsonifierResult<std::string_view> JsonifierResult<Field>::unescaped_key() noexcept {
		if (error()) {
			return error();
		}
		return first.unescapedKey();
	}

	inline JsonifierResult<Value> JsonifierResult<Field>::value() noexcept {
		if (error()) {
			return error();
		}
		return std::move(first.value());
	}

	inline JsonifierResult<Value> Object::findFieldUnordered(const std::string_view key) & noexcept {
		bool has_Value;
		JsonifierTry(iterator.findFieldUnorderedRaw(key).get(has_Value));
		if (!has_Value) {
			return No_Such_Field;
		}
		return Value(iterator.child());
	}
	inline JsonifierResult<Value> Object::findFieldUnordered(const std::string_view key) && noexcept {
		bool has_Value;
		JsonifierTry(iterator.findFieldUnorderedRaw(key).get(has_Value));
		if (!has_Value) {
			return No_Such_Field;
		}
		return Value(iterator.child());
	}
	inline JsonifierResult<Value> Object::operator[](const std::string_view key) & noexcept {
		return findFieldUnordered(key);
	}
	inline JsonifierResult<Value> Object::operator[](const std::string_view key) && noexcept {
		return std::forward<Object>(*this).findFieldUnordered(key);
	}
	inline JsonifierResult<Value> Object::findField(const std::string_view key) & noexcept {
		bool has_Value;
		JsonifierTry(iterator.findFieldRaw(key).get(has_Value));
		if (!has_Value) {
			return No_Such_Field;
		}
		return Value(iterator.child());
	}
	inline JsonifierResult<Value> Object::findField(const std::string_view key) && noexcept {
		bool has_Value;
		JsonifierTry(iterator.findFieldRaw(key).get(has_Value));
		if (!has_Value) {
			return No_Such_Field;
		}
		return Value(iterator.child());
	}

	inline JsonifierResult<Object> Object::start(ValueIterator& iterator) noexcept {
		JsonifierTry(iterator.startObject().error());
		return Object(iterator);
	}
	inline JsonifierResult<Object> Object::startRoot(ValueIterator& iterator) noexcept {
		JsonifierTry(iterator.startRootObject().error());
		return Object(iterator);
	}
	inline ErrorCode Object::consume() noexcept {
		if (iterator.isAtKey()) {
			RawJsonString actual_key;
			auto error = iterator.fieldKey().get(actual_key);
			if (error) {
				iterator.abandon();
				return error;
			};
			if ((error = iterator.fieldValue())) {
				iterator.abandon();
				return error;
			}
		}
		auto error_skip = iterator.jsonIter().skipChild(iterator.depth() - 1);
		if (error_skip) {
			iterator.abandon();
		}
		return error_skip;
	}

	inline JsonifierResult<std::string_view> Object::rawJson() noexcept {
		const uint8_t* starting_point{ iterator.peekStart() };
		auto error = consume();
		if (error) {
			return error;
		}
		const uint8_t* final_point{ iterator.jsonIterator->peek(0) };
		return std::string_view(reinterpret_cast<const char*>(starting_point), size_t(final_point - starting_point));
	}

	inline JsonifierResult<Object> Object::started(ValueIterator& iterator) noexcept {
		JsonifierTry(iterator.startedObject().error());
		return Object(iterator);
	}

	inline Object Object::resume(const ValueIterator& iterator) noexcept {
		return iterator;
	}

	inline Object::Object(const ValueIterator& _iterator) noexcept : iterator{ _iterator } {
	}

	inline JsonifierResult<ObjectIterator> Object::begin() noexcept {
		return ObjectIterator(iterator);
	}
	inline JsonifierResult<ObjectIterator> Object::end() noexcept {
		return ObjectIterator(iterator);
	}

	inline JsonifierResult<Value> Object::atPointer(std::string_view json_pointer) noexcept {
		if (json_pointer[0] != '/') {
			return Invalid_Json_Pointer;
		}
		json_pointer = json_pointer.substr(1);
		size_t slash = json_pointer.find('/');
		std::string_view key = json_pointer.substr(0, slash);
		JsonifierResult<Value> child;
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

	inline JsonifierResult<size_t> Object::countFields() & noexcept {
		size_t count{ 0 };
		for (auto v: *this) {
			count++;
		}
		if (iterator.error()) {
			return iterator.error();
		}
		iterator.resetObject();
		return count;
	}

	inline JsonifierResult<bool> Object::isEmpty() & noexcept {
		bool isNot_empty;
		auto error = iterator.resetObject().get(isNot_empty);
		if (error) {
			return error;
		}
		return !isNot_empty;
	}

	inline JsonifierResult<bool> Object::reset() & noexcept {
		return iterator.resetObject();
	}

	inline JsonifierResult<Object>::JsonifierResult(
		Object&& Value) noexcept
		: ImplementationJsonifierResultBase<Object>(
			  std::forward<Object>(Value)) {
	}
	inline JsonifierResult<Object>::JsonifierResult(ErrorCode error) noexcept
		: ImplementationJsonifierResultBase<Object>(error) {
	}

	inline JsonifierResult<ObjectIterator>
	JsonifierResult<Object>::begin() noexcept {
		if (error()) {
			return error();
		}
		return first.begin();
	}
	inline JsonifierResult<ObjectIterator>
	JsonifierResult<Object>::end() noexcept {
		if (error()) {
			return error();
		}
		return first.end();
	}
	inline JsonifierResult<Value>
	JsonifierResult<Object>::findFieldUnordered(std::string_view key) & noexcept {
		if (error()) {
			return error();
		}
		return first.findFieldUnordered(key);
	}
	inline JsonifierResult<Value>
	JsonifierResult<Object>::findFieldUnordered(std::string_view key) && noexcept {
		if (error()) {
			return error();
		}
		return std::forward<Object>(first).findFieldUnordered(key);
	}
	inline JsonifierResult<Value>
	JsonifierResult<Object>::operator[](std::string_view key) & noexcept {
		if (error()) {
			return error();
		}
		return first[key];
	}
	inline JsonifierResult<Value>
	JsonifierResult<Object>::operator[](std::string_view key) && noexcept {
		if (error()) {
			return error();
		}
		return std::forward<Object>(first)[key];
	}
	inline JsonifierResult<Value>
	JsonifierResult<Object>::findField(std::string_view key) & noexcept {
		if (error()) {
			return error();
		}
		return first.findField(key);
	}
	inline JsonifierResult<Value>
	JsonifierResult<Object>::findField(std::string_view key) && noexcept {
		if (error()) {
			return error();
		}
		return std::forward<Object>(first).findField(key);
	}

	inline JsonifierResult<Value>
	JsonifierResult<Object>::atPointer(std::string_view json_pointer) noexcept {
		if (error()) {
			return error();
		}
		return first.atPointer(json_pointer);
	}

	inline JsonifierResult<bool> JsonifierResult<Object>::reset() noexcept {
		if (error()) {
			return error();
		}
		return first.reset();
	}

	inline JsonifierResult<bool> JsonifierResult<Object>::isEmpty() noexcept {
		if (error()) {
			return error();
		}
		return first.isEmpty();
	}

	inline JsonifierResult<size_t> JsonifierResult<Object>::countFields() & noexcept {
		if (error()) {
			return error();
		}
		return first.countFields();
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

	template<typename T> inline T& ImplementationJsonifierResultBase<T>::value() & noexcept(false) {
		if (error()) {
			throw simdjson_error(error());
		}
		return this->first;
	}

	template<typename T> inline T&& ImplementationJsonifierResultBase<T>::value() && noexcept(false) {
		return std::forward<ImplementationJsonifierResultBase<T>>(*this).takeValue();
	}

	template<typename T> inline T&& ImplementationJsonifierResultBase<T>::takeValue() && noexcept(false) {
		if (error()) {
			throw error();
		}
		return std::forward<T>(this->first);
	}

	template<typename T> inline ImplementationJsonifierResultBase<T>::operator T&&() && noexcept(false) {
		return std::forward<ImplementationJsonifierResultBase<T>>(*this).takeValue();
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
		: first{ std::forward<T>(value) }, second{ error } {
	}
	template<typename T>
	inline ImplementationJsonifierResultBase<T>::ImplementationJsonifierResultBase(ErrorCode error) noexcept
		: ImplementationJsonifierResultBase(T{}, error) {
	}
	template<typename T>
	inline ImplementationJsonifierResultBase<T>::ImplementationJsonifierResultBase(T&& value) noexcept
		: ImplementationJsonifierResultBase(std::forward<T>(value), Success) {
	}

	inline JsonifierResult<JsonType>::JsonifierResult(JsonType&& value) noexcept
		: ImplementationJsonifierResultBase<JsonType>(std::forward<JsonType>(value)){};
	inline JsonifierResult<JsonType>::JsonifierResult(ErrorCode error) noexcept : ImplementationJsonifierResultBase<JsonType>(error){};
}
