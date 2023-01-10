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
		__forceinline JsonifierResult<Document> parseJson(const char* string, size_t stringLength);
		__forceinline JsonifierResult<Document> parseJson(const std::string& string);
		__forceinline JsonifierResult<Document> parseJson(std::string_view string);

		operator Document() noexcept;

	  protected:
		ObjectBuffer<uint8_t> stringBuffer{};
		SimdStringSection section{};
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
			return this->section.getTapePtrs();
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
			this->allocatedSpace = round(5 * this->stringLengthRaw / 3 + 256, 256);
			if (!this->stringBuffer) {
				this->stringBuffer.reset(0);
				return ErrorCode::Mem_Alloc_Error;
			}

			return ErrorCode::Success;
		}

		template<size_t BlockCountPerIteration>
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
				StringBlockReader<BlockCountPerIteration * 256> stringReader{};
				section = SimdStringSection{ this->stringLengthRaw, round(this->stringLengthRaw + 3, 256) };
				stringReader.addNewString(this->stringView, this->stringLengthRaw);
				this->tapeLength = 0;
				while (stringReader.hasFullBlock()) {
					section.submitDataForProcessing<2>(stringReader.fullBlock());
					section.generateStructurals<2>();
					stringReader.advance();
				}
				uint8_t block[BlockCountPerIteration * 256];
				stringReader.getRemainder(block);
				section.submitDataForProcessing<2>(block);
				this->getTapeLength() = section.generateStructurals<2>();
			}
			//for (size_t x = 0; x < this->tapeLength; ++x) {
				//std::cout << "CURRENT INDEX: " << this->getStructuralIndices()[x]
							 //<< ", THE INDEXES'S VALUE: " << this->stringView[this->getStructuralIndices()[x]] << std::endl;
			//}
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
