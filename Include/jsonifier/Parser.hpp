#pragma once

#include <jsonifier/Simd.hpp>
#include <jsonifier/StringParsingUtils.hpp>
#include <jsonifier/NumberParsingUtils.hpp>
#include <jsonifier/Document.hpp>

namespace Jsonifier {

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
		ObjectBuffer<uint32_t> structuralIndices{};
		ObjectBuffer<uint8_t> stringBuffer{};
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

		__forceinline uint32_t*& getStructuralIndices() {
			return this->structuralIndices.operator uint32_t*&();
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
			this->structuralIndices.reset(round(this->stringLengthRaw + 3, 256));
			this->allocatedSpace = round(5 * this->stringLengthRaw / 3 + 256, 256);
			if (!(this->stringBuffer && this->structuralIndices)) {
				this->stringBuffer.reset(0);
				return ErrorCode::Mem_Alloc_Error;
			}

			return ErrorCode::Success;
		}

		template<size_t BlockCountPerIteration>
		__forceinline ErrorCode generateJsonIndices(const uint8_t* stringNew, size_t stringLength) {
			StopWatch stopWatch{ std::chrono::nanoseconds{ 1 } };
			if (stringNew) {
				
				StringBlockReader<BlockCountPerIteration * 256> stringReader{};
				SimdStringSection<BlockCountPerIteration> section{ this->stringLengthRaw, this->getStructuralIndices() };
				stringReader.addNewString(this->stringView, this->stringLengthRaw);
				this->tapeLength = 0;
				while (stringReader.hasFullBlock()) {
					section.submitDataForProcessing<BlockCountPerIteration>(stringReader.fullBlock());
					section.generateStructurals<BlockCountPerIteration>();
					stringReader.advance();
				}
				uint8_t block[BlockCountPerIteration * 256];
				stringReader.getRemainder(block);
				section.submitDataForProcessing<BlockCountPerIteration>(block);
				this->getTapeLength() = section.generateStructurals<BlockCountPerIteration>();
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
