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

		Document parseJson(const char* string, size_t stringLength);
		Document parseJson(const std::string& string);
		Document parseJson(std::string_view string);

		operator Document() noexcept;

		int64_t totalTimePassed{};
		int64_t iterationCount{};

	  protected:
		ObjectBuffer<uint32_t> structuralIndexes{};
		ObjectBuffer<uint8_t> stringBuffer{};
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

		inline ErrorCode generateJsonIndices(const uint8_t* stringNew, size_t stringLength) {
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
				StringBlockReader<256> stringReader{};
				SimdStringSection section{ this->structuralIndexes };
				stringReader.addNewString(this->stringView, this->stringLengthRaw);
				section.reset();
				this->tapeLength = 0;
				size_t tapeCurrentIndex{ 0 };
				while (stringReader.hasFullBlock()) {
					section.submitDataForProcessing(stringReader.fullBlock());
					section.getStructuralIndices(tapeCurrentIndex, this->stringLengthRaw);
					stringReader.advance();
				}
				uint8_t block[256];
				stringReader.getRemainder(block);
				section.submitDataForProcessing(block);
				section.getStructuralIndices(tapeCurrentIndex, this->stringLengthRaw);
				this->getTapeLength() = tapeCurrentIndex;
				//for (size_t x = 0; x < this->tapeLength; ++x) {
					//std::cout << "CURRENT INDEX: " << this->structuralIndexes[x] << ", THE INDICES'S VALUE"
							  //<< this->stringView[this->structuralIndexes[x]] << std::endl;
				//}
			}
			return Success;
		}
	};

	template<> BackslashAndQuote<SimdBase256> inline BackslashAndQuote<SimdBase256>::copyAndFind(const uint8_t* src, uint8_t* dst) {
		static_assert(256 >= (BYTES_PROCESSED - 1), "backslash and quote finder must process fewer than 256 bytes");
		SimdBase256 v(reinterpret_cast<const char*>(src));
		v.store(dst);
		return {
			static_cast<uint32_t>((v == '\\').toBitMask()),
			static_cast<uint32_t>((v == '"').toBitMask()),
		};
	}
};
