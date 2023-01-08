#pragma once

#include <jsonifier/TokenIterator.hpp>

namespace Jsonifier {

	inline TokenIterator::TokenIterator(const uint8_t* _buf, uint32_t* position) noexcept : stringView{ _buf }, currentPosition{ position } {}

	inline uint32_t TokenIterator::currentOffset() const noexcept {
		return *(currentPosition);
	}

	inline const uint8_t* TokenIterator::returnCurrentAndAdvance() noexcept {
		return &stringView[*(currentPosition++)];
	}

	inline const uint8_t* TokenIterator::peek(uint32_t* position) const noexcept {
		return &stringView[*position];
	}

	inline uint32_t TokenIterator::peekIndex(uint32_t* position) const noexcept {
		return *position;
	}

	inline uint32_t TokenIterator::peekLength(uint32_t* position) const noexcept {
		return *(position + 1) - *position;
	}

	inline const uint8_t* TokenIterator::peek(int32_t delta) const noexcept {
		return &stringView[*(currentPosition + delta)];
	}

	inline uint32_t TokenIterator::peekIndex(int32_t delta) const noexcept {
		return *(currentPosition + delta);
	}

	inline uint32_t TokenIterator::peekLength(int32_t delta) const noexcept {
		return *(currentPosition + delta + 1) - *(currentPosition + delta);
	}

	inline uint32_t* TokenIterator::position() const noexcept {
		return currentPosition;
	}

	inline void TokenIterator::setPosition(uint32_t* target_position) noexcept {
		currentPosition = target_position;
	}

	inline bool TokenIterator::operator==(const TokenIterator& other) const noexcept {
		return currentPosition == other.currentPosition;
	}

	inline bool TokenIterator::operator!=(const TokenIterator& other) const noexcept {
		return currentPosition != other.currentPosition;
	}

	inline bool TokenIterator::operator>(const TokenIterator& other) const noexcept {
		return currentPosition > other.currentPosition;
	}

	inline bool TokenIterator::operator>=(const TokenIterator& other) const noexcept {
		return currentPosition >= other.currentPosition;
	}

	inline bool TokenIterator::operator<(const TokenIterator& other) const noexcept {
		return currentPosition < other.currentPosition;
	}

	inline bool TokenIterator::operator<=(const TokenIterator& other) const noexcept {
		return currentPosition <= other.currentPosition;
	}

	inline JsonifierResult<TokenIterator>::JsonifierResult(TokenIterator&& Value) noexcept
		: ImplementationJsonifierResultBase<TokenIterator>(std::forward<TokenIterator>(Value)){}

	inline JsonifierResult<TokenIterator>::JsonifierResult(ErrorCode error) noexcept : ImplementationJsonifierResultBase<TokenIterator>(error){}

}
