#pragma once

#include <jsonifier/Base.hpp>

namespace Jsonifier {

	class ValueIterator;

	std::ostream& operator<<(std::ostream& out, JsonType type) noexcept;

	std::ostream& operator<<(std::ostream& out, JsonifierResult<JsonType>& type) noexcept(false);

	template<> struct JsonifierResult<JsonType> : public JsonifierResultBase<JsonType> {
	  public:
		inline JsonifierResult(JsonType&& Value) noexcept;
		inline JsonifierResult(ErrorCode error) noexcept;
		inline JsonType value() noexcept;
		JsonifierResult() noexcept = default;
		~JsonifierResult() noexcept = default;
	};

}
