#pragma once

#include <jsonifier/Base.hpp>

namespace Jsonifier {

	class ValueIterator;

	std::ostream& operator<<(std::ostream& out, JsonType type) noexcept;

	std::ostream& operator<<(std::ostream& out, JsonifierResult<JsonType>& type) noexcept(false);

	template<> struct JsonifierResult<JsonType> : public JsonifierResultBase<JsonType> {
	  public:
		JsonifierResult(JsonType&& Value) noexcept;
		JsonifierResult(ErrorCode error) noexcept;
		JsonType value() noexcept;
		JsonifierResult() noexcept = default;
		~JsonifierResult() noexcept = default;
	};

}
