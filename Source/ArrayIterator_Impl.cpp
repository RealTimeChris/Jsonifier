#pragma once

#include <jsonifier/ArrayIterator.hpp>
#include <jsonifier/Value_Impl.hpp>

namespace Jsonifier {

	JsonifierResult<ArrayIterator>::JsonifierResult(ArrayIterator&& value) noexcept
		: JsonifierResultBase<ArrayIterator>(std::forward<ArrayIterator>(value)) {
		first.assertIsValid();
	}

	JsonifierResult<ArrayIterator>::JsonifierResult(ErrorCode error) noexcept : JsonifierResultBase<ArrayIterator>(this->second){};

	

}
