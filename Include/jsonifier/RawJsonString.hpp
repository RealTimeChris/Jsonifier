
#pragma once

#include <jsonifier/Base.hpp>

namespace Jsonifier {

	class Object;
	class parser;
	class JsonIterator;

	class Jsonifier_Dll RawJsonString {
	  public:
		inline RawJsonString() noexcept = default;
		inline RawJsonString(const uint8_t* _buf) noexcept;
		inline const char* raw() const noexcept;
		inline bool unsafeIsEqual(size_t length, std::string_view target) const noexcept;
		inline bool unsafeIsEqual(std::string_view target) const noexcept;
		inline bool unsafeIsEqual(const char* target) const noexcept;
		inline bool isEqual(std::string_view target) const noexcept;
		inline bool isEqual(const char* target) const noexcept;
		static inline bool isFreeFromUnescapedQuote(std::string_view target) noexcept;
		static inline bool isFreeFromUnescapedQuote(const char* target) noexcept;

	  protected:
		inline void consume() noexcept {
			stringView = nullptr;
		}
		inline bool alive() const noexcept {
			return stringView != nullptr;
		}
		inline JsonifierResult<std::string_view> unescape(JsonIterator& iterator) const noexcept;

		const uint8_t* stringView{};
		friend class Object;
		friend class Field;
		friend class Parser;
		friend struct JsonifierResult<RawJsonString>;
		friend class JsonIterator;
	};

	inline std::ostream& operator<<(std::ostream&, const RawJsonString&) noexcept;

	inline bool operator==(const RawJsonString& a, std::string_view c) noexcept;
	inline bool operator==(std::string_view c, const RawJsonString& a) noexcept;
	inline bool operator!=(const RawJsonString& a, std::string_view c) noexcept;
	inline bool operator!=(std::string_view c, const RawJsonString& a) noexcept;

	template<> struct JsonifierResult<RawJsonString> : public ImplementationJsonifierResultBase<RawJsonString> {
	  public:
		inline JsonifierResult(RawJsonString&& Value) noexcept;
		inline JsonifierResult(ErrorCode error) noexcept;
		inline JsonifierResult() noexcept = default;
		inline ~JsonifierResult() noexcept = default;

		inline JsonifierResult<const char*> raw() const noexcept;
		inline JsonifierResult<std::string_view> unescape(JsonIterator& iterator) const noexcept;
	};

}
