#pragma once

#include <jsonifier/Parser.hpp>

namespace Jsonifier {

	JsonifierResult<Document> Parser::parseJson(std::string_view string) {
		if (auto result = this->generateJsonIndices(reinterpret_cast<const uint8_t*>(string.data()), string.size())) {
			return result;
		}
		return std::forward<Document>(JsonIterator{ this });
	}

	JsonifierResult<Document> Parser::parseJson(const char* string, size_t stringLength) {
		if (auto result = this->generateJsonIndices(reinterpret_cast<const uint8_t*>(string), stringLength)) {
			return result;
		}
		return std::forward<Document>(JsonIterator{ this });
	}

	JsonifierResult<Document> Parser::parseJson(const std::string& string) {
		if (auto result = this->generateJsonIndices(reinterpret_cast<const uint8_t*>(string.data()), string.size())) {
			return result;
		}
		return std::forward<Document>(JsonIterator{ this });
	}

}
