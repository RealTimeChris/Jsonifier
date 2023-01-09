#pragma once

#include <jsonifier/Base.hpp>

namespace Jsonifier {

	class Jsonifier_Dll TokenIterator {
	  public:
		TokenIterator() noexcept = default;
		TokenIterator(TokenIterator&& other) noexcept = default;
		TokenIterator& operator=(TokenIterator&& other) noexcept = default;
		TokenIterator(const TokenIterator& other) noexcept = default;
		TokenIterator& operator=(const TokenIterator& other) noexcept = default;
		const uint8_t* returnCurrentAndAdvance() noexcept;
		uint32_t currentOffset() const noexcept;
		const uint8_t* peek(int32_t delta = 0) const noexcept;
		uint32_t peekLength(int32_t delta = 0) const noexcept;
		const uint8_t* peek(uint32_t* position) const noexcept;
		uint32_t peekLength(uint32_t* position) const noexcept;
		uint32_t* position() const noexcept;
		void setPosition(uint32_t* target_position) noexcept;
		bool operator==(const TokenIterator& other) const noexcept;
		bool operator!=(const TokenIterator& other) const noexcept;
		bool operator>(const TokenIterator& other) const noexcept;
		bool operator>=(const TokenIterator& other) const noexcept;
		bool operator<(const TokenIterator& other) const noexcept;
		bool operator<=(const TokenIterator& other) const noexcept;

	  protected:
		TokenIterator(const uint8_t* buf, uint32_t* position) noexcept;
		uint32_t peekIndex(int32_t delta = 0) const noexcept;
		uint32_t peekIndex(uint32_t* position) const noexcept;

		const uint8_t* stringView{};
		uint32_t* currentPosition{};

		friend class JsonIterator;
		friend class ValueIterator;
		friend class Object;
	};

	template<> struct JsonifierResult<TokenIterator> : public JsonifierResultBase<TokenIterator> {
	  public:
		JsonifierResult(TokenIterator&& Value) noexcept;
		JsonifierResult(ErrorCode error) noexcept;
		JsonifierResult() noexcept = default;
		~JsonifierResult() noexcept = default;
	};

}
