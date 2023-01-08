#pragma once

#include <jsonifier/Parser.hpp>

namespace Jsonifier {

	Document Parser::parseJson(std::string_view string) {
		this->generateJsonIndices(reinterpret_cast<const uint8_t*>(string.data()), string.size());
		return std::forward<Document>(JsonIterator{ this });
	}

	Document Parser::parseJson(const char* string, size_t stringLength) {
		this->generateJsonIndices(reinterpret_cast<const uint8_t*>(string), stringLength);
		return std::forward<Document>(JsonIterator{ this });
	}

	Document Parser::parseJson(const std::string& string) {
		this->generateJsonIndices(reinterpret_cast<const uint8_t*>(string.data()), string.size());
		return std::forward<Document>(JsonIterator{ this });
	}

}
