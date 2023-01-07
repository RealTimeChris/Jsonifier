#include <jsonifier/FoundationEntities.hpp>

namespace Jsonifier {

	inline JsonifierResult<RawJsonString>::JsonifierResult(RawJsonString&& value) noexcept
		: ImplementationJsonifierResultBase<RawJsonString>(std::forward<RawJsonString>(value)) {
	}

	inline JsonifierResult<RawJsonString>::JsonifierResult(ErrorCode error) noexcept : ImplementationJsonifierResultBase<RawJsonString>(error) {
	}

	inline JsonifierResult<const char*> JsonifierResult<RawJsonString>::raw() const noexcept {
		if (error()) {
			return error();
		}
		return first.raw();
	}

	inline JsonifierResult<std::string_view> JsonifierResult<RawJsonString>::unescape(JsonIterator& iter) const noexcept {
		if (error()) {
			return error();
		}
		return first.unescape(iter);
	}

	inline JsonifierResult<TokenIterator>::JsonifierResult(TokenIterator&& value) noexcept
		: ImplementationJsonifierResultBase<TokenIterator>(std::forward<TokenIterator>(value)) {
	}

	inline JsonifierResult<TokenIterator>::JsonifierResult(ErrorCode error) noexcept : ImplementationJsonifierResultBase<TokenIterator>(error) {
	}

	inline JsonifierResult<JsonIterator>::JsonifierResult(JsonIterator&& value) noexcept
		: ImplementationJsonifierResultBase<JsonIterator>(std::forward<JsonIterator>(value)) {
	}

	inline JsonifierResult<JsonIterator>::JsonifierResult(ErrorCode error) noexcept : ImplementationJsonifierResultBase<JsonIterator>(error) {
	}

	inline JsonifierResult<JsonType>::JsonifierResult(JsonType&& value) noexcept
		: ImplementationJsonifierResultBase<JsonType>(std::forward<JsonType>(value)){}
	inline JsonifierResult<JsonType>::JsonifierResult(ErrorCode error) noexcept : ImplementationJsonifierResultBase<JsonType>(error){};

}