#pragma once

#include <jsonifier/TokenIterator.hpp>

namespace Jsonifier {

	TokenIterator::TokenIterator(const uint8_t* _buf, uint32_t* position) noexcept : stringView{ _buf }, currentPosition{ position } {
	}

	uint32_t TokenIterator::currentOffset() const noexcept {
		return *(currentPosition);
	}

	const uint8_t* TokenIterator::returnCurrentAndAdvance() noexcept {
		return &stringView[*(currentPosition++)];
	}

	const uint8_t* TokenIterator::peek(uint32_t* position) const noexcept {
		return &stringView[*position];
	}

	uint32_t TokenIterator::peekIndex(uint32_t* position) const noexcept {
		return *position;
	}

	uint32_t TokenIterator::peekLength(uint32_t* position) const noexcept {
		return *(position + 1) - *position;
	}

	const uint8_t* TokenIterator::peek(int32_t delta) const noexcept {
		return &stringView[*(currentPosition + delta)];
	}

	uint32_t TokenIterator::peekIndex(int32_t delta) const noexcept {
		return *(currentPosition + delta);
	}

	uint32_t TokenIterator::peekLength(int32_t delta) const noexcept {
		return *(currentPosition + delta + 1) - *(currentPosition + delta);
	}

	uint32_t* TokenIterator::position() const noexcept {
		return currentPosition;
	}

	void TokenIterator::setPosition(uint32_t* target_position) noexcept {
		currentPosition = target_position;
	}

	bool TokenIterator::operator==(const TokenIterator& other) const noexcept {
		return currentPosition == other.currentPosition;
	}

	bool TokenIterator::operator!=(const TokenIterator& other) const noexcept {
		return currentPosition != other.currentPosition;
	}

	bool TokenIterator::operator>(const TokenIterator& other) const noexcept {
		return currentPosition > other.currentPosition;
	}

	bool TokenIterator::operator>=(const TokenIterator& other) const noexcept {
		return currentPosition >= other.currentPosition;
	}

	bool TokenIterator::operator<(const TokenIterator& other) const noexcept {
		return currentPosition < other.currentPosition;
	}

	bool TokenIterator::operator<=(const TokenIterator& other) const noexcept {
		return currentPosition <= other.currentPosition;
	}

	JsonifierResult<TokenIterator>::JsonifierResult(TokenIterator&& Value) noexcept
		: JsonifierResultBase<TokenIterator>(std::forward<TokenIterator>(Value)) {
	}

	JsonifierResult<TokenIterator>::JsonifierResult(ErrorCode error) noexcept : JsonifierResultBase<TokenIterator>(error) {
	}

}
