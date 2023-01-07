#pragma once
#include <jsonifier/FoundationEntities.hpp>
#include <jsonifier/Jsonifier.hpp>

namespace Jsonifier {
#if JSONIFIER_DEVELOPMENT_CHECKS
	inline uint32_t* JsonIterator::startPosition(size_t depth) const noexcept {
		assert(size_t(depth) < parser->maxDepth());
		return size_t(depth) < parser->maxDepth() ? parser->startPositions[depth] : 0;
	}

	inline void JsonIterator::setStartPosition(size_t depth, uint32_t* position) noexcept {
		assert(size_t(depth) < parser->maxDepth());
		if (size_t(depth) < parser->maxDepth()) {
			parser->startPositions[depth] = position;
		}
	}
#endif
}
