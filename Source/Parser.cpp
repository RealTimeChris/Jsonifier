#include <jsonifier/Parser.hpp>
#include <jsonifier/Document_Impl.hpp>
#include <jsonifier/Jsonifier_Impl.hpp>

namespace Jsonifier {

	inline int64_t totalTimePassed{};
	inline int64_t iterationCount{};
	JsonifierResult<Document> Parser::parseJson(std::string_view string) noexcept {
		if (string.size() == 0) {
			return String_Error;
		}
		this->stringView = ( uint8_t* )string.data();
		this->stringLengthRaw = string.size();
		if (this->allocatedSpace < round(5 * this->stringLengthRaw / 3 + 256, 256)) {
			if (this->allocate() != ErrorCode::Success) {
				return Mem_Alloc_Error;
			}
		}
		iterationCount++;
		StopWatch stopWatch{ std::chrono::nanoseconds{ 1 } };
		this->generateJsonIndices(reinterpret_cast<const uint8_t*>(string.data()), string.size());
		totalTimePassed += stopWatch.totalTimePassed().count();
		std::cout << "TIME FOR STAGE1: " << totalTimePassed / iterationCount << std::endl;
		//for (size_t x = 0; x < this->tapeLength; ++x) {
		//			std::cout << "CURRENT INDEX: " << this->structuralIndices[x] << ", THE INDEX'S VALUE: " << this->stringView[this->structuralIndices[x]]
		//<< std::endl;
		//}
		return std::forward<Document>(JsonIterator{ this });
	}
	
	JsonifierResult<Document> Parser::parseJson(const char* string, size_t stringLength) noexcept {
		if (stringLength == 0) {
			return String_Error;
		}
		this->stringView = ( uint8_t* )string;
		this->stringLengthRaw = stringLength;
		if (this->allocatedSpace < round(5 * this->stringLengthRaw / 3 + 256, 256)) {
			if (this->allocate() != ErrorCode::Success) {
				return Mem_Alloc_Error;
			}
		}
		iterationCount++;
		StopWatch stopWatch{ std::chrono::nanoseconds{ 1 } };
		this->generateJsonIndices(reinterpret_cast<const uint8_t*>(string), stringLength);
		totalTimePassed += stopWatch.totalTimePassed().count();
		std::cout << "TIME FOR STAGE1: " << totalTimePassed / iterationCount << std::endl;
		//for (size_t x = 0; x < this->tapeLength; ++x) {
		//std::cout << "CURRENT INDEX: " << this->structuralIndices[x] << ", THE INDEX'S VALUE: " << this->stringView[this->structuralIndices[x]]
		//<< std::endl;
		//}
		return std::forward<Document>(JsonIterator{ this });
	}

	JsonifierResult<Document> Parser::parseJson(const std::string& string) noexcept {
		if (string.size() == 0) {
			return String_Error;
		}
		this->stringView = ( uint8_t* )string.data();
		this->stringLengthRaw = string.size();
		if (this->allocatedSpace < round(5 * this->stringLengthRaw / 3 + 256, 256)) {
			if (this->allocate() != ErrorCode::Success) {
				return Mem_Alloc_Error;
			}
		}
		iterationCount++;
		std::cout << "STRING LENGTH RAW: " << this->stringLengthRaw << std::endl;
		StopWatch stopWatch{ std::chrono::nanoseconds{ 1 } };
		this->generateJsonIndices(reinterpret_cast<const uint8_t*>(string.data()), string.size());
		totalTimePassed += stopWatch.totalTimePassed().count();
		std::cout << "TIME FOR STAGE1: " << totalTimePassed / iterationCount << std::endl;
		//for (size_t x = 0; x < this->tapeLength; ++x) {
		//			std::cout << "CURRENT INDEX: " << this->structuralIndices[x] << ", THE INDEX'S VALUE: " << this->stringView[this->structuralIndices[x]]
		//<< std::endl;
		//}
		return std::forward<Document>(JsonIterator{ this });
	}

	uint8_t* Parser::getStringView() noexcept {
		return this->stringView;
	}

	uint8_t* Parser::getStringBuffer() noexcept {
		return this->stringBuffer;
	}

	uint32_t* Parser::getStructuralIndices() noexcept {
		return this->structuralIndices.operator uint32_t*();
	}

	size_t Parser::maxDepth() noexcept {
		return 512;
	}

	size_t Parser::getTapeLength() noexcept {
		return this->tapeLength;
	}

	uint64_t Parser::round(int64_t a, int64_t n) noexcept {
		return (((a) + (( n )-1)) & ~(( n )-1));
	}

	ErrorCode Parser::allocate() noexcept {
		if (this->stringLengthRaw == 0) {
			return ErrorCode::Success;
		}
		this->stringBuffer.reset(round(5 * this->stringLengthRaw / 3 + 256, 256));
		this->structuralIndices.reset(round(this->stringLengthRaw + 3, 256));
		this->allocatedSpace = round(5 * this->stringLengthRaw / 3 + 256, 256);
		std::cout << "WERE ALLOCATING!" << std::endl;
		if (!(this->stringBuffer && this->structuralIndices)) {
			this->stringBuffer.reset(0);
			return ErrorCode::Mem_Alloc_Error;
		}

		return ErrorCode::Success;
	}

	ErrorCode Parser::generateJsonIndices(const uint8_t* stringNew, size_t stringLength) noexcept {
		StopWatch stopWatch{ std::chrono::nanoseconds{ 1 } };
		if (stringNew) {
			this->stringView = ( uint8_t* )stringNew;
			this->stringLengthRaw = stringLength;
			StringBlockReader<256> stringReader{};
			std::fill(this->structuralIndices.operator uint32_t*(), this->structuralIndices + round(this->stringLengthRaw + 3, 256), 0x00);
			SimdStringSection section{ this->stringLengthRaw, this->getStructuralIndices() };
			stringReader.addNewString(this->stringView, this->stringLengthRaw);
			this->tapeLength = 0;
			while (stringReader.hasFullBlock()) {
				section.submitDataForProcessing(stringReader.fullBlock());
				section.generateStructurals();
				stringReader.advance();
			}
			uint8_t block[256];
			stringReader.getRemainder(block);
			section.submitDataForProcessing(block);
			this->tapeLength = section.generateStructurals();
		}
		return Success;
	}

}
