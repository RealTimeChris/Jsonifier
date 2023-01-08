#pragma once

#include <jsonifier/Base.hpp>

namespace Jsonifier {

	class ValueIterator;


	/**
 * Write the JSON type to the output stream
 *
 * @param out The output stream.
 * @param type The JsonType.
 */
	inline std::ostream& operator<<(std::ostream& out, JsonType type) noexcept;
	/**
 * Send JSON type to an output stream.
 *
 * @param out The output stream.
 * @param type The JsonType.
 * @throw simdjson_error if the result being printed has an error. If there is an error with the
 *        underlying output stream, that error will be propagated (simdjson_error will not be
 *        thrown).
 */
	inline std::ostream& operator<<(std::ostream& out, JsonifierResult<JsonType>& type) noexcept(false);
	template<> struct JsonifierResult<JsonType> : public ImplementationJsonifierResultBase<JsonType> {
	  public:
		inline JsonifierResult(JsonType&& Value) noexcept;///< @private
		inline JsonifierResult(ErrorCode error) noexcept;///< @private
		inline JsonifierResult() noexcept = default;
		inline ~JsonifierResult() noexcept = default;///< @private
	};

}
