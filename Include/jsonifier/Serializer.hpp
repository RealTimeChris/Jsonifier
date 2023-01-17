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
#pragma once

#include <jsonifier/Base.hpp>

namespace Jsonifier {

	template<typename RTy> void reverseByteOrder(RTy& net) {
		if (std::endian::native == std::endian::little) {
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

	template<typename RTy> void storeBits(char* to, RTy num) {
		uint8_t byteSize{ 8 };
		reverseByteOrder<RTy>(num);
		for (uint32_t x = 0; x < sizeof(RTy); ++x) {
			to[x] = static_cast<uint8_t>(num >> (byteSize * x));
		}
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

	class Jsonifier_Dll EnumConverter {
	  public:
		template<IsEnum EnumType> EnumConverter& operator=(const std::vector<EnumType>& data) {
			for (auto& value: data) {
				vector.emplace_back(std::move(static_cast<uint64_t>(value)));
			}
			return *this;
		};

		template<IsEnum EnumType> EnumConverter(const std::vector<EnumType>& data) {
			*this = data;
		};

		template<IsEnum EnumType> EnumConverter& operator=(EnumType data) {
			integer = static_cast<uint64_t>(data);
			return *this;
		};

		template<IsEnum EnumType> EnumConverter(EnumType data) {
			*this = data;
		};

		operator std::vector<uint64_t>() const noexcept;

		operator uint64_t() const noexcept;

		bool isItAVector() const noexcept;

	  protected:
		std::vector<uint64_t> vector{};
		bool vectorType{};
		uint64_t integer{};
	};

	enum class JsonifierSerializeType { Etf = 0, Json = 1 };

	class Serializer;

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
			JsonValue() noexcept = default;
			JsonValue& operator=(JsonValue&&) noexcept = delete;
			JsonValue(JsonValue&&) noexcept = delete;
			JsonValue& operator=(const JsonValue&) noexcept = delete;
			JsonValue(const JsonValue&) noexcept = delete;
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
			setValue(JsonType::Array);
			for (auto& value: data) {
				jsonValue.array->push_back(std::move(value));
			}
			return *this;
		}

		template<IsConvertibleToJsonifier OTy> Serializer(std::vector<OTy>&& data) noexcept {
			*this = std::move(data);
		}

		template<IsConvertibleToJsonifier OTy> Serializer& operator=(std::vector<OTy>& data) noexcept {
			setValue(JsonType::Array);
			for (auto& value: data) {
				jsonValue.array->push_back(value);
			}
			return *this;
		}

		template<IsConvertibleToJsonifier OTy> Serializer(std::vector<OTy>& data) noexcept {
			*this = data;
		}

		template<IsConvertibleToJsonifier KTy, IsConvertibleToJsonifier OTy> Serializer& operator=(std::unordered_map<KTy, OTy>&& data) noexcept {
			setValue(JsonType::Object);
			for (auto& [key, value]: data) {
				(*jsonValue.object)[key] = std::move(value);
			}
			return *this;
		}

		template<IsConvertibleToJsonifier KTy, IsConvertibleToJsonifier OTy> Serializer(std::unordered_map<KTy, OTy>&& data) noexcept {
			*this = std::move(data);
		};

		template<IsConvertibleToJsonifier KTy, IsConvertibleToJsonifier OTy> Serializer& operator=(std::unordered_map<KTy, OTy>& data) noexcept {
			setValue(JsonType::Object);
			for (auto& [key, value]: data) {
				(*jsonValue.object)[key] = value;
			}
			return *this;
		}

		template<IsConvertibleToJsonifier KTy, IsConvertibleToJsonifier OTy> Serializer(std::unordered_map<KTy, OTy>& data) noexcept {
			*this = data;
		};

		template<IsConvertibleToJsonifier KTy, IsConvertibleToJsonifier OTy> Serializer& operator=(std::map<KTy, OTy>&& data) noexcept {
			setValue(JsonType::Object);
			for (auto& [key, value]: data) {
				(*jsonValue.object)[key] = std::move(value);
			}
			return *this;
		}

		template<IsConvertibleToJsonifier KTy, IsConvertibleToJsonifier OTy> Serializer(std::map<KTy, OTy>&& data) noexcept {
			*this = std::move(data);
		};

		template<IsConvertibleToJsonifier KTy, IsConvertibleToJsonifier OTy> Serializer& operator=(std::map<KTy, OTy>& data) noexcept {
			setValue(JsonType::Object);
			for (auto& [key, value]: data) {
				(*jsonValue.object)[key] = value;
			}
			return *this;
		}

		template<IsConvertibleToJsonifier KTy, IsConvertibleToJsonifier OTy> Serializer(std::map<KTy, OTy>& data) noexcept {
			*this = data;
		};

		template<IsEnum Ty> Serializer& operator=(Ty data) noexcept {
			jsonValue.numberUint = static_cast<uint64_t>(data);
			type = JsonType::Uint64;
			return *this;
		}

		template<IsEnum Ty> Serializer(Ty data) noexcept {
			*this = data;
		}

		Serializer& operator=(Serializer&& data) noexcept;

		Serializer(Serializer&& data) noexcept;

		Serializer& operator=(const Serializer& data) noexcept;

		Serializer(const Serializer& data) noexcept;

		operator std::string&&() noexcept;

		operator std::string() noexcept;

		void refreshString(JsonifierSerializeType OpCode);

		Serializer& operator=(EnumConverter&& data) noexcept;
		Serializer(EnumConverter&& data) noexcept;

		Serializer& operator=(const EnumConverter& data) noexcept;
		Serializer(const EnumConverter& data) noexcept;

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

		template<typename Ty> __forceinline Ty getValue() && {
			return Ty{};
		}

		template<typename Ty> __forceinline Ty& getValue() & {
			return Ty{};
		}

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
			std::enable_if_t<std::is_integral<NumberType>::value || std::is_same<NumberType, uint64_t>::value ||
					std::is_same<NumberType, int64_t>::value,
				int32_t> = 0>
		__forceinline void writeJsonInt(NumberType Int) {
			auto IntNew = std::to_string(Int);
			writeString(IntNew.data(), IntNew.size());
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

	template<> __forceinline Serializer::ObjectType Serializer::getValue() && {
		return *std::forward<Serializer>(*this).jsonValue.object;
	}

	template<> __forceinline Serializer::ArrayType Serializer::getValue() && {
		return *std::forward<Serializer>(*this).jsonValue.array;
	}

	template<> __forceinline Serializer::StringType Serializer::getValue() && {
		return *std::forward<Serializer>(*this).jsonValue.string;
	}

	template<> __forceinline Serializer::FloatType Serializer::getValue() && {
		return jsonValue.numberDouble;
	}

	template<> __forceinline Serializer::UintType Serializer::getValue() && {
		return jsonValue.numberUint;
	}

	template<> __forceinline Serializer::IntType Serializer::getValue() && {
		return jsonValue.numberInt;
	}

	template<> __forceinline Serializer::BoolType Serializer::getValue() && {
		return jsonValue.boolean;
	}

	template<> __forceinline Serializer::ObjectType& Serializer::getValue() & {
		return *jsonValue.object;
	}

	template<> __forceinline Serializer::ArrayType& Serializer::getValue() & {
		return *jsonValue.array;
	}

	template<> __forceinline Serializer::StringType& Serializer::getValue() & {
		return *jsonValue.string;
	}

	template<> __forceinline Serializer::FloatType& Serializer::getValue() & {
		return jsonValue.numberDouble;
	}

	template<> __forceinline Serializer::UintType& Serializer::getValue() & {
		return jsonValue.numberUint;
	}

	template<> __forceinline Serializer::IntType& Serializer::getValue() & {
		return jsonValue.numberInt;
	}

	template<> __forceinline Serializer::BoolType& Serializer::getValue() & {
		return jsonValue.boolean;
	}
}