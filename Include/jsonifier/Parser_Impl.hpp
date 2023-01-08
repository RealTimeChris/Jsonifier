#pragma once

#include <jsonifier/Parser.hpp>

namespace Jsonifier {

	Document Parser::parseJson(std::string_view string) {
		//iterationCount++;
		//St//opWatch stopWatch{ std::chrono::nanoseconds{ 1 } };
		this->generateJsonIndices(reinterpret_cast<const uint8_t*>(string.data()), string.size());
		//for (size_t x = 0; x < this->getTapeLength(); ++x) {
		//std::cout << "CURRENT INDEX: " << this->getStructuralIndices()[x]
		//<< ", THAT INDEX'S VALUE: " << this->stringView[this->getStructuralIndices()[x]] << std::endl;
		//}
		//totalTimePassed += stopWatch.totalTimePassed().count();
		//std::cout << "TIME FOR STAGE1: " << totalTimePassed / iterationCount << std::endl;
		return std::forward<Document>(JsonIterator{ this });
	}

	Document Parser::parseJson(const char* string, size_t stringLength) {
		//iterationCount++;
		//StopWatch stopWatch{ std::chrono::nanoseconds{ 1 } };
		this->generateJsonIndices(reinterpret_cast<const uint8_t*>(string), stringLength);
		//for (size_t x = 0; x < this->getTapeLength(); ++x) {
		//std::cout << "CURRENT INDEX: " << this->getStructuralIndices()[x]
		//<< ", THAT INDEX'S VALUE: " << this->stringView[this->getStructuralIndices()[x]] << std::endl;
		//}
		//totalTimePassed += stopWatch.totalTimePassed().count();
		//std::cout << "TIME FOR STAGE1: " << totalTimePassed / iterationCount << std::endl;
		return std::forward<Document>(JsonIterator{ this });
	}

	Document Parser::parseJson(const std::string& string) {
		//iterationCount++;
		//StopWatch stopWatch{ std::chrono::nanoseconds{ 1 } };
		this->generateJsonIndices(reinterpret_cast<const uint8_t*>(string.data()), string.size());
		//totalTimePassed += stopWatch.totalTimePassed().count();
		//for (size_t x = 0; x < this->getTapeLength(); ++x) {
		//			std::cout << "CURRENT INDEX: " << this->getStructuralIndices()[x]
		//<< ", THAT INDEX'S VALUE: " << this->stringView[this->getStructuralIndices()[x]] << std::endl;
		//}
		//std::cout << "TIME FOR STAGE1: " << totalTimePassed / iterationCount << std::endl;
		return std::forward<Document>(JsonIterator{ this });
	}

}
