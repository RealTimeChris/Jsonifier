#pragma once

#include <jsonifier/Base.hpp>

namespace Jsonifier {

	class ValueIterator;

	inline std::ostream& operator<<(std::ostream& out, JsonType type) noexcept;

	inline std::ostream& operator<<(std::ostream& out, JsonifierResult<JsonType>& type) noexcept(false);

	template<> struct JsonifierResult<JsonType> : public JsonifierResultBase<JsonType> {
	  public:
		inline JsonifierResult(JsonType&& Value) noexcept;
		inline JsonifierResult(ErrorCode error) noexcept;
		inline JsonType value() noexcept;
		inline JsonifierResult() noexcept = default;
		inline ~JsonifierResult() noexcept = default;
	};

}
