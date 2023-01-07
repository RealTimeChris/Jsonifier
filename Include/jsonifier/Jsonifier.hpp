#pragma once

#pragma warning(push)
#pragma warning(disable : 4244)
#pragma warning(disable : 4251)

#ifdef _DEBUG
	#define JSONIFIER_DEVELOPMENT_CHECKS 0
#endif

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

#include <jsonifier/ImplementationJsonifierResultBase.hpp>
#include <jsonifier/JsonifierResult.hpp>

#include <jsonifier/FoundationEntities.hpp>
#include <jsonifier/NumberParsingUtils.hpp>
#include <jsonifier/StringParsingUtils.hpp>
#include <jsonifier/Simd.hpp>
#include <jsonifier/Object.hpp>
#include <jsonifier/Value.hpp>
#include <jsonifier/Array.hpp>
#include <jsonifier/Document.hpp>
#include <jsonifier/Field.hpp>

#include <jsonifier/ImplementationJsonifierResultBase_Impl.hpp>
#include <jsonifier/Simd.hpp>
#include <jsonifier/Object_Impl.hpp>
#include <jsonifier/Array_Impl.hpp>
#include <jsonifier/Document_Impl.hpp>
#include <jsonifier/FoundationEntities_Impl.hpp>
#include <jsonifier/Field_Impl.hpp>
#include <jsonifier/Value_Impl.hpp>

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

		inline Serializer() noexcept = default;

		template<IsConvertibleToJsonifier OTy> inline Serializer& operator=(std::vector<OTy>&& data) noexcept {
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

		inline Serializer& operator=(Serializer&& data) noexcept;

		inline Serializer& operator=(const Serializer& data) noexcept;

		inline Serializer(const Serializer& data) noexcept;

		inline operator std::string&&() noexcept;

		inline operator std::string() noexcept;

		inline void refreshString(JsonifierSerializeType OpCode);

		inline Serializer& operator=(EnumConverter&& data) noexcept;
		inline Serializer(EnumConverter&& data) noexcept;

		inline Serializer& operator=(EnumConverter& data) noexcept;
		inline Serializer(EnumConverter& data) noexcept;

		inline Serializer& operator=(std::string&& data) noexcept;
		inline Serializer(std::string&& data) noexcept;

		inline Serializer& operator=(const std::string& data) noexcept;
		inline Serializer(const std::string& data) noexcept;

		inline Serializer& operator=(const char* data) noexcept;
		inline Serializer(const char* data) noexcept;

		inline Serializer& operator=(double data) noexcept;
		inline Serializer(double data) noexcept;

		inline Serializer& operator=(float data) noexcept;
		inline Serializer(float data) noexcept;

		inline Serializer& operator=(uint64_t data) noexcept;
		inline Serializer(uint64_t data) noexcept;

		inline Serializer& operator=(uint32_t data) noexcept;
		inline Serializer(uint32_t data) noexcept;

		inline Serializer& operator=(uint16_t data) noexcept;
		inline Serializer(uint16_t data) noexcept;

		inline Serializer& operator=(uint8_t data) noexcept;
		inline Serializer(uint8_t data) noexcept;

		inline Serializer& operator=(int64_t data) noexcept;
		inline Serializer(int64_t data) noexcept;

		inline Serializer& operator=(int32_t data) noexcept;
		inline Serializer(int32_t data) noexcept;

		inline Serializer& operator=(int16_t data) noexcept;
		inline Serializer(int16_t data) noexcept;

		inline Serializer& operator=(int8_t data) noexcept;
		inline Serializer(int8_t data) noexcept;

		inline Serializer& operator=(bool data) noexcept;
		inline Serializer(bool data) noexcept;

		inline Serializer& operator=(JsonType TypeNew) noexcept;
		inline Serializer(JsonType type) noexcept;

		inline Serializer& operator=(std::nullptr_t) noexcept;
		inline Serializer(std::nullptr_t data) noexcept;

		inline Serializer& operator[](typename ObjectType::key_type key);

		inline Serializer& operator[](uint64_t index);

		template<typename Ty> inline const Ty& getValue() const {
			return Ty{};
		}

		template<typename Ty> inline Ty& getValue() {
			return Ty{};
		}

		inline size_t size() noexcept;

		inline JsonType getType() noexcept;

		inline void emplaceBack(Serializer&& data) noexcept;
		inline void emplaceBack(const Serializer& data) noexcept;

		inline ~Serializer() noexcept;

	  protected:
		JsonType type{ JsonType::Null };
		JsonValue jsonValue{};
		std::string string{};

		inline void serializeJsonToEtfString(const Serializer* dataToParse);

		inline void serializeJsonToJsonString(const Serializer* dataToParse);

		inline void writeJsonObject(const ObjectType& ObjectNew);

		inline void writeJsonArray(const ArrayType& Array);

		inline void writeJsonString(const StringType& StringNew);

		inline void writeJsonFloat(const FloatType x);

		template<typename NumberType,
			std::enable_if_t<
				std::is_integral<NumberType>::value || std::is_same<NumberType, uint64_t>::value || std::is_same<NumberType, int64_t>::value, int> =
				0>
		inline void writeJsonInt(NumberType Int) {
			auto IntNew = std::to_string(Int);
			this->writeString(IntNew.data(), IntNew.size());
		}

		inline void writeJsonBool(const BoolType ValueNew);

		inline void writeJsonNull();

		inline void writeEtfObject(const ObjectType& jsonData);

		inline void writeEtfArray(const ArrayType& jsonData);

		inline void writeEtfString(const StringType& jsonData);

		inline void writeEtfUint(const UintType jsonData);

		inline void writeEtfInt(const IntType jsonData);

		inline void writeEtfFloat(const FloatType jsonData);

		inline void writeEtfBool(const BoolType jsonData);

		inline void writeEtfNull();

		inline void writeString(const char* data, size_t length);

		inline void writeCharacter(const char Char);

		inline void appendBinaryExt(const std::string& bytes, const uint32_t sizeNew);

		inline void appendNewFloatExt(const double FloatValue);

		inline void appendListHeader(const uint32_t sizeNew);

		inline void appendMapHeader(const uint32_t sizeNew);

		inline void appendUint64(const uint64_t value);

		inline void appendUint32(const uint32_t value);

		inline void appendUint8(const uint8_t value);

		inline void appendInt64(const int64_t value);

		inline void appendInt32(const int32_t value);

		inline void appendInt8(const int8_t value);

		inline void appendBool(bool data);

		inline void appendVersion();

		inline void appendNilExt();

		inline void appendNil();

		inline void setValue(JsonType TypeNew);

		inline void destroy() noexcept;

		inline friend bool operator==(const Serializer& lhs, const Serializer& rhs);
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
		Parser& operator=(Parser&&) = default;
		Parser(Parser&&) = default;
		Parser& operator=(const Parser&) = delete;
		Parser(const Parser&) = delete;
		Parser(const std::string&) noexcept;
		Parser() noexcept = default;

		Document parseJson(const char* string, size_t stringLength);
		Document parseJson(const std::string& string);
		Document parseJson(std::string_view string);

		operator Document() noexcept;

	  protected:
#if JSONIFIER_DEVELOPMENT_CHECKS
		ObjectBuffer<uint32_t*> startPositions{};
#endif
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

		inline size_t maxDepth() {
			return 512;
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
#if JSONIFIER_DEVELOPMENT_CHECKS
			this->startPositions.reset(512);
#endif
			if (!(this->structuralIndexes && this->stringBuffer)) {
				this->stringBuffer.reset(0);
				this->structuralIndexes.reset(0);
#if JSONIFIER_DEVELOPMENT_CHECKS
				this->startPositions.reset(0);
#endif
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
				this->tapeLength = 0;
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

}
