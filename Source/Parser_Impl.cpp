#pragma once

#include <jsonifier/Parser.hpp>
#include <jsonifier/Document.hpp>

namespace Jsonifier {

	Document Parser::parseJson(std::string_view string) {
		//StopWatch stopWatch{ std::chrono::nanoseconds{ 1 } };
		//iterationCount++;
		// Run stage 1.
		if (string.size() == 0) {
			return Document{};
		}
		this->stringLengthRaw = string.size();
		if (this->allocatedSpace < round(5ull * this->stringLengthRaw / 3 + 256, 256)) {
			if (this->allocate() != ErrorCode::Success) {
				return Document{};
			}
		}
		this->generateJsonIndices(reinterpret_cast<const uint8_t*>(string.data()), string.size());
		//totalTimePassed += stopWatch.totalTimePassed().count();
		//std::cout << "TIME FOR STAGE1: " << totalTimePassed / iterationCount << std::endl;
		return std::forward<Document>(JsonIterator{ this });
	}

	Document Parser::parseJson(const char* string, size_t stringLength) {
		if (stringLength == 0) {
			return Document{};
		}
		this->stringLengthRaw = stringLength;
		if (this->allocatedSpace < round(5ull * this->stringLengthRaw / 3 + 256, 256)) {
			if (this->allocate() != ErrorCode::Success) {
				return Document{};
			}
		}
		iterationCount++;
		StopWatch stopWatch{ std::chrono::nanoseconds{ 1 } };
		this->generateJsonIndices(reinterpret_cast<const uint8_t*>(string), stringLength);
		totalTimePassed += stopWatch.totalTimePassed().count();
		std::cout << "TIME FOR STAGE1: " << totalTimePassed / iterationCount << std::endl;
		return std::forward<Document>(JsonIterator{ this });
	}

	Document Parser::parseJson(const std::string& string) {
		if (string.size() == 0) {
			return Document{};
		}
		this->stringLengthRaw = string.size();
		if (this->allocatedSpace < round(5ull * this->stringLengthRaw / 3 + 256, 256)) {
			if (this->allocate() != ErrorCode::Success) {
				return Document{};
			}
		}
		//StopWatch stopWatch{ std::chrono::nanoseconds{ 1 } };
		//iterationCount++;
		// Run stage 1.
		this->generateJsonIndices(reinterpret_cast<const uint8_t*>(string.data()), string.size());
		//totalTimePassed += stopWatch.totalTimePassed().count();
		//std::cout << "TIME FOR STAGE1: " << totalTimePassed / iterationCount << std::endl;
		return std::forward<Document>(JsonIterator{ this });
	}

}
