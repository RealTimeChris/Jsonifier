#pragma once

#include <jsonifier/TokenIterator.hpp>

namespace Jsonifier {

	__forceinline TokenIterator::TokenIterator(const uint8_t* _buf, uint32_t* position) noexcept : stringView{ _buf }, currentPosition{ position } {
	}

	__forceinline uint32_t TokenIterator::currentOffset() const noexcept {
		return *(currentPosition);
	}

	__forceinline const uint8_t* TokenIterator::returnCurrentAndAdvance() noexcept {
		return &stringView[*(currentPosition++)];
	}

	__forceinline const uint8_t* TokenIterator::peek(uint32_t* position) const noexcept {
		return &stringView[*position];
	}

	__forceinline uint32_t TokenIterator::peekIndex(uint32_t* position) const noexcept {
		return *position;
	}

	__forceinline uint32_t TokenIterator::peekLength(uint32_t* position) const noexcept {
		return *(position + 1) - *position;
	}

	__forceinline const uint8_t* TokenIterator::peek(int32_t delta) const noexcept {
		return &stringView[*(currentPosition + delta)];
	}

	__forceinline uint32_t TokenIterator::peekIndex(int32_t delta) const noexcept {
		return *(currentPosition + delta);
	}

	__forceinline uint32_t TokenIterator::peekLength(int32_t delta) const noexcept {
		return *(currentPosition + delta + 1) - *(currentPosition + delta);
	}

	__forceinline uint32_t* TokenIterator::position() const noexcept {
		return currentPosition;
	}

	__forceinline void TokenIterator::setPosition(uint32_t* target_position) noexcept {
		currentPosition = target_position;
	}

	__forceinline bool TokenIterator::operator==(const TokenIterator& other) const noexcept {
		return currentPosition == other.currentPosition;
	}

	__forceinline bool TokenIterator::operator!=(const TokenIterator& other) const noexcept {
		return currentPosition != other.currentPosition;
	}

	__forceinline bool TokenIterator::operator>(const TokenIterator& other) const noexcept {
		return currentPosition > other.currentPosition;
	}

	__forceinline bool TokenIterator::operator>=(const TokenIterator& other) const noexcept {
		return currentPosition >= other.currentPosition;
	}

	__forceinline bool TokenIterator::operator<(const TokenIterator& other) const noexcept {
		return currentPosition < other.currentPosition;
	}

	__forceinline bool TokenIterator::operator<=(const TokenIterator& other) const noexcept {
		return currentPosition <= other.currentPosition;
	}

	__forceinline JsonifierResult<TokenIterator>::JsonifierResult(TokenIterator&& Value) noexcept {};

	__forceinline JsonifierResult<TokenIterator>::JsonifierResult(ErrorCode error) noexcept {};

}
