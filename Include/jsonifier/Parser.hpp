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

	  protected:
		ObjectBuffer<uint32_t> structuralIndexes{};
		ObjectBuffer<uint8_t> stringBuffer{};
		SimdStringSection section01{};
		SimdStringSection section02{};
		SimdStringSection section03{};
		SimdStringSection section04{};
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
		inline static int64_t totalTimeCollecting{};
		inline static int64_t totalTimeGenerating{};
		inline static int64_t totalTimePacking{};
		inline static int64_t iterations{};
		__forceinline ErrorCode generateJsonIndices(const uint8_t* stringNew, size_t stringLength) {
			StopWatch stopWatch{ std::chrono::nanoseconds{ 1 } };
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
				StringBlockReader<512> stringReader{};
				stringReader.addNewString(this->stringView, this->stringLengthRaw);
				section01.reset();
				section02.reset();
				//section03.reset();
				//section04.reset();
				this->tapeLength = 0;
				size_t tapeCurrentIndex{};
				size_t currentStringIndex{};
				while (stringReader.hasFULLBlock()) {
					//iterations++;
					//stopWatch.resetTimer();
					section01.submitDataForProcessing(stringReader.fULLBlock(), this->structuralIndexes, currentStringIndex);
					currentStringIndex += 256;
					section02.submitDataForProcessing(stringReader.fULLBlock() + 256, this->structuralIndexes, currentStringIndex);
					currentStringIndex += 256;
					//section03.submitDataForProcessing(stringReader.fULLBlock() + 256, this->structuralIndexes, currentStringIndex);
					//currentStringIndex += 128;
					//section04.submitDataForProcessing(stringReader.fULLBlock() + 384, this->structuralIndexes, currentStringIndex);
					//currentStringIndex += 128;
					//totalTimePacking += stopWatch.totalTimePassed().count();
					//std::cout << "TOTAL TIME FOR PACKING THE VALUES: " << totalTimePacking / iterations << std::endl;
					//stopWatch.resetTimer();
					//stopWatch.resetTimer();
					section01.generateStructurals();
					section02.generateStructurals();
					//section03.generateStructurals();
					//section04.generateStructurals();
					//totalTimeGenerating += stopWatch.totalTimePassed().count();
					//std::cout << "TOTAL TIME FOR GENERATING THE VALUES: " << totalTimeGenerating / iterations << std::endl;
					
					section01.getStructuralIndices(tapeCurrentIndex, this->stringLengthRaw);
					section02.getStructuralIndices(tapeCurrentIndex, this->stringLengthRaw);
					//section03.getStructuralIndices(tapeCurrentIndex, this->stringLengthRaw);
					//section04.getStructuralIndices(tapeCurrentIndex, this->stringLengthRaw);
					//totalTimeCollecting += stopWatch.totalTimePassed().count();
					//std::cout << "TOTAL TIME FOR COLLLECTING THE VALUES: " << totalTimeCollecting / iterations << std::endl;
					stringReader.advance();
				}
				uint8_t block[512];
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
