#pragma once

#include <jsonifier/Simd.hpp>
#include <jsonifier/StringParsingUtils.hpp>
#include <jsonifier/NumberParsingUtils.hpp>
#include <jsonifier/Document.hpp>

namespace Jsonifier {

	class Jsonifier_Dll Parser {
	  public:
		friend class JsonIterator;
		Parser& operator=(Parser&&) = delete;
		Parser(Parser&&) = delete;
		Parser& operator=(const Parser&) = delete;
		Parser(const Parser&) = delete;
		Parser(const std::string&) noexcept;
		Parser() noexcept = default;

		JsonifierResult<Document> parseJson(const char* string, size_t stringLength) noexcept;
		JsonifierResult<Document> parseJson(const std::string& string) noexcept;
		JsonifierResult<Document> parseJson(std::string_view string) noexcept;

	  protected:
		ObjectBuffer<uint32_t> structuralIndices{};
		ObjectBuffer<uint8_t> stringBuffer{};
		size_t stringLengthRaw{};
		size_t allocatedSpace{};
		uint8_t* stringView{};
		size_t tapeLength{};

		uint8_t* getStringView() noexcept;

		uint8_t* getStringBuffer() noexcept;

		uint32_t* getStructuralIndices() noexcept;

		size_t maxDepth() noexcept;

		size_t getTapeLength() noexcept;

		uint64_t round(int64_t a, int64_t n) noexcept;

		ErrorCode allocate() noexcept;

		template<size_t BlockCountPerIteration> ErrorCode generateJsonIndices(const uint8_t* stringNew, size_t stringLength) noexcept;
	};
};
