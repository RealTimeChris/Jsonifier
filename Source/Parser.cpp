#include <jsonifier/Parser.hpp>
#include <jsonifier/Document_Impl.hpp>
#include <jsonifier/Jsonifier_Impl.hpp>

namespace Jsonifier {

	JsonifierResult<Document> Parser::parseJson(std::string_view string) {
		//StopWatch stopWatch{ std::chrono::nanoseconds{ 1 } };
		//iterationCount++;
		// Run stage 1.
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
		this->generateJsonIndices<2>(reinterpret_cast<const uint8_t*>(string.data()), string.size());
		//totalTimePassed += stopWatch.totalTimePassed().count();
		//std::cout << "TIME FOR STAGE1: " << totalTimePassed / iterationCount << std::endl;
		return std::forward<Document>(JsonIterator{ this });
	}
	inline int64_t totalTimePassed{};
	inline int64_t iterationCount{};
	JsonifierResult<Document> Parser::parseJson(const char* string, size_t stringLength) {
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
		this->generateJsonIndices<2>(reinterpret_cast<const uint8_t*>(string), stringLength);
		totalTimePassed += stopWatch.totalTimePassed().count();
		std::cout << "TIME FOR STAGE1: " << totalTimePassed / iterationCount << std::endl;
		return std::forward<Document>(JsonIterator{ this });
	}

	JsonifierResult<Document> Parser::parseJson(const std::string& string) {
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
		//StopWatch stopWatch{ std::chrono::nanoseconds{ 1 } };
		//iterationCount++;
		// Run stage 1.
		this->generateJsonIndices<2>(reinterpret_cast<const uint8_t*>(string.data()), string.size());
		//totalTimePassed += stopWatch.totalTimePassed().count();
		//std::cout << "TIME FOR STAGE1: " << totalTimePassed / iterationCount << std::endl;
		return std::forward<Document>(JsonIterator{ this });
	}

}
