#pragma once

#include <jsonifier/Base.hpp>

namespace Jsonifier {

	class Jsonifier_Dll TokenIterator {
	  public:
		__forceinline TokenIterator() noexcept = default;
		__forceinline TokenIterator(TokenIterator&& other) noexcept = default;
		__forceinline TokenIterator& operator=(TokenIterator&& other) noexcept = default;
		__forceinline TokenIterator(const TokenIterator& other) noexcept = default;
		__forceinline TokenIterator& operator=(const TokenIterator& other) noexcept = default;
		__forceinline const uint8_t* returnCurrentAndAdvance() noexcept;
		__forceinline uint32_t currentOffset() const noexcept;
		__forceinline const uint8_t* peek(int32_t delta = 0) const noexcept;
		__forceinline uint32_t peekLength(int32_t delta = 0) const noexcept;
		__forceinline const uint8_t* peek(uint32_t* position) const noexcept;
		__forceinline uint32_t peekLength(uint32_t* position) const noexcept;
		__forceinline uint32_t* position() const noexcept;
		__forceinline void setPosition(uint32_t* target_position) noexcept;
		__forceinline bool operator==(const TokenIterator& other) const noexcept;
		__forceinline bool operator!=(const TokenIterator& other) const noexcept;
		__forceinline bool operator>(const TokenIterator& other) const noexcept;
		__forceinline bool operator>=(const TokenIterator& other) const noexcept;
		__forceinline bool operator<(const TokenIterator& other) const noexcept;
		__forceinline bool operator<=(const TokenIterator& other) const noexcept;

	  protected:
		__forceinline TokenIterator(const uint8_t* buf, uint32_t* position) noexcept;
		__forceinline uint32_t peekIndex(int32_t delta = 0) const noexcept;
		__forceinline uint32_t peekIndex(uint32_t* position) const noexcept;

		const uint8_t* stringView;
		uint32_t* currentPosition;

		friend class JsonIterator;
		friend class ValueIterator;
		friend class Object;
	};

	template<> struct JsonifierResult<TokenIterator> : public JsonifierResultBase<TokenIterator> {
	  public:
		__forceinline JsonifierResult(TokenIterator&& Value) noexcept;
		__forceinline JsonifierResult(ErrorCode error) noexcept;
		__forceinline JsonifierResult() noexcept = default;
		__forceinline ~JsonifierResult() noexcept = default;
	};

}
