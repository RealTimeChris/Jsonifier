#pragma once

#include <jsonifier/Simd.hpp>
#include <jsonifier/StringParsingUtils.hpp>
#include <jsonifier/NumberParsingUtils.hpp>

namespace Jsonifier {

	class Array;
	class Object;
	class Value;
	class RawJsonString;
	class Document;

	class Jsonifier_Dll Parser {
	  public:
		friend class JsonIterator;
		inline Parser& operator=(Parser&&) = default;
		inline Parser(Parser&&) = default;
		inline Parser& operator=(const Parser&) = delete;
		inline Parser(const Parser&) = delete;
		inline Parser(const std::string&) noexcept;
		inline Parser() noexcept = default;

		inline Document parseJson(const char* string, size_t stringLength);
		inline Document parseJson(const std::string& string);
		inline Document parseJson(std::string_view string);

		operator Document() noexcept;

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

	template<> inline BackslashAndQuote<SimdBase256> BackslashAndQuote<SimdBase256>::copyAndFind(const uint8_t* src, uint8_t* dst) {
		static_assert(256 >= (BYTES_PROCESSED - 1), "backslash and quote finder must process fewer than 256 bytes");
		SimdBase256 v(src);
		v.store(dst);
		return {
			static_cast<uint32_t>((v == '\\').toBitMask()),
			static_cast<uint32_t>((v == '"').toBitMask()),
		};
	}
};
