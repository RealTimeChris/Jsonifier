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
		Parser& operator=(Parser&&) = default;
		Parser(Parser&&) = default;
		Parser& operator=(const Parser&) = delete;
		Parser(const Parser&) = delete;
		Parser(const std::string&) noexcept;
		Parser() noexcept = default;

		JsonifierResult<Document> parseJson(const char* string, size_t stringLength);
		JsonifierResult<Document> parseJson(const std::string& string);
		JsonifierResult<Document> parseJson(std::string_view string);

		operator Document() noexcept;

		int64_t totalTimePassed{};
		int64_t iterationCount{};

	  protected:
		ObjectBuffer<uint32_t> structuralIndexes{};
		ObjectBuffer<uint8_t> stringBuffer{};
		SimdStringSection section01{};
		size_t stringLengthRaw{};
		size_t allocatedSpace{};
		uint8_t* stringView{};
		size_t tapeLength{};

		__forceinline uint8_t* getStringView() {
			return this->stringView;
		}

		__forceinline uint8_t* getStringBuffer() {
			return this->stringBuffer;
		}

		__forceinline uint32_t* getStructuralIndices() {
			return this->structuralIndexes;
		}

		__forceinline size_t maxDepth() {
			return 512;
		}

		__forceinline size_t& getTapeLength() {
			return this->tapeLength;
		}

		__forceinline uint64_t round(int64_t a, int64_t n) {
			return (((a) + (( n )-1)) & ~(( n )-1));
		}

		__forceinline ErrorCode allocate() noexcept {
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

		__forceinline ErrorCode generateJsonIndices(const uint8_t* stringNew, size_t stringLength) {
			if (stringNew) {
				if (stringLength == 0) {
					return String_Error;
				}
				this->stringView = ( uint8_t* )stringNew;
				this->stringLengthRaw = stringLength;
				if (this->allocatedSpace < round(5 * this->stringLengthRaw / 3 + 256, 256)) {
					if (this->allocate() != ErrorCode::Success) {
						return Mem_Alloc_Error;
					}
				}
				StringBlockReader<128> stringReader{};
				stringReader.addNewString(this->stringView, this->stringLengthRaw);
				section01.reset();
				this->tapeLength = 0;
				size_t tapeCurrentIndex{};
				size_t currentStringIndex{};
				while (stringReader.hasFullBlock()) {
					section01.submitDataForProcessing(stringReader.fullBlock(), this->structuralIndexes, currentStringIndex);
					currentStringIndex += 128;
					//section03.submitDataForProcessing(stringReader.fullBlock() + 512, this->structuralIndexes, currentStringIndex);
					//currentStringIndex += 256;
					//section04.submitDataForProcessing(stringReader.fullBlock() + 768, this->structuralIndexes, currentStringIndex);
					//currentStringIndex += 256;
					section01.generateStructurals();
					//section03.generateStructurals();
					//section04.generateStructurals();
					section01.getStructuralIndices(tapeCurrentIndex, this->stringLengthRaw);
					//section02.getStructuralIndices(tapeCurrentIndex, this->stringLengthRaw);
					//section03.getStructuralIndices(tapeCurrentIndex, this->stringLengthRaw);
					//section04.getStructuralIndices(tapeCurrentIndex, this->stringLengthRaw);
					stringReader.advance();
				}
				uint8_t block[128];
				stringReader.getRemainder(block);
				section01.submitDataForProcessing(block, this->structuralIndexes, currentStringIndex);
				section01.getStructuralIndices(tapeCurrentIndex, this->stringLengthRaw);
				this->getTapeLength() = tapeCurrentIndex;
				//for (size_t x = 0; x < this->tapeLength; ++x) {
					//std::cout << "CURRENT INDEX: " << this->structuralIndexes[x] << ", THE INDICES'S VALUE"
							  //<< this->stringView[this->structuralIndexes[x]] << std::endl;
				//}
			}
			return Success;
		}
	};

	template<> BackslashAndQuote<SimdBase256> __forceinline BackslashAndQuote<SimdBase256>::copyAndFind(const uint8_t* src, uint8_t* dst) {
		static_assert(256 >= (BYTES_PROCESSED - 1), "backslash and quote finder must process fewer than 256 bytes");
		SimdBase256 v(reinterpret_cast<const char*>(src));
		v.store(dst);
		return {
			static_cast<uint32_t>((v == '\\').toBitMask()),
			static_cast<uint32_t>((v == '"').toBitMask()),
		};
	}
};
