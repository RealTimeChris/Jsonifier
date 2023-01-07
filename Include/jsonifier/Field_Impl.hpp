#pragma once

#include <jsonifier/ImplementationJsonifierResultBase_Impl.hpp>

namespace Jsonifier {

	inline JsonifierResult<Field>::JsonifierResult(Field&& Value) noexcept : ImplementationJsonifierResultBase<Field>(std::forward<Field>(Value)) {
	}

	inline JsonifierResult<Field>::JsonifierResult(ErrorCode error) noexcept : ImplementationJsonifierResultBase<Field>(error) {
	}

	inline JsonifierResult<RawJsonString> JsonifierResult<Field>::key() noexcept {
		if (error()) {
			return error();
		}
		return first.key();
	}

	inline JsonifierResult<std::string_view> JsonifierResult<Field>::unescaped_key() noexcept {
		if (error()) {
			return error();
		}
		return first.unescapedKey();
	}

	inline JsonifierResult<Value> JsonifierResult<Field>::value() noexcept {
		if (error()) {
			return error();
		}
		return std::move(first.value());
	}
}