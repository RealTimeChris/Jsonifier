#pragma once

#include <jsonifier/Base.hpp>

namespace Jsonifier {

	class Jsonifier_Dll TokenIterator {
	  public:
		inline TokenIterator() noexcept = default;
		inline TokenIterator(TokenIterator&& other) noexcept = default;
		inline TokenIterator& operator=(TokenIterator&& other) noexcept = default;
		inline TokenIterator(const TokenIterator& other) noexcept = default;
		inline TokenIterator& operator=(const TokenIterator& other) noexcept = default;
		inline const uint8_t* returnCurrentAndAdvance() noexcept;
		inline uint32_t currentOffset() const noexcept;
		inline const uint8_t* peek(int32_t delta = 0) const noexcept;
		inline uint32_t peekLength(int32_t delta = 0) const noexcept;
		inline const uint8_t* peek(uint32_t* position) const noexcept;
		inline uint32_t peekLength(uint32_t* position) const noexcept;
		inline uint32_t* position() const noexcept;
		inline void setPosition(uint32_t* target_position) noexcept;
		inline bool operator==(const TokenIterator& other) const noexcept;
		inline bool operator!=(const TokenIterator& other) const noexcept;
		inline bool operator>(const TokenIterator& other) const noexcept;
		inline bool operator>=(const TokenIterator& other) const noexcept;
		inline bool operator<(const TokenIterator& other) const noexcept;
		inline bool operator<=(const TokenIterator& other) const noexcept;

	  protected:
		inline TokenIterator(const uint8_t* buf, uint32_t* position) noexcept;
		inline uint32_t peekIndex(int32_t delta = 0) const noexcept;
		inline uint32_t peekIndex(uint32_t* position) const noexcept;

		const uint8_t* stringView{};
		uint32_t* currentPosition{};

		friend class JsonIterator;
		friend class ValueIterator;
		friend class Object;
	};

	template<> struct JsonifierResult<TokenIterator> : public JsonifierResultBase<TokenIterator> {
	  public:
		inline JsonifierResult(TokenIterator&& Value) noexcept;
		inline JsonifierResult(ErrorCode error) noexcept;
		inline JsonifierResult() noexcept = default;
		inline ~JsonifierResult() noexcept = default;
	};

}
