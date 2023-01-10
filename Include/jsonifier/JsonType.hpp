#pragma once

#include <jsonifier/Base.hpp>

namespace Jsonifier {

	class ValueIterator;

	std::ostream& operator<<(std::ostream& out, JsonType type) noexcept;

	std::ostream& operator<<(std::ostream& out, JsonifierResult<JsonType>& type) noexcept(false);

	template<> struct JsonifierResult<JsonType> : public JsonifierResultBase<JsonType> {
	  public:
		__forceinline JsonifierResult(JsonType&& Value) noexcept;
		__forceinline JsonifierResult(ErrorCode error) noexcept;
		__forceinline JsonType value() noexcept;
		JsonifierResult() noexcept = default;
		~JsonifierResult() noexcept = default;
	};

}
