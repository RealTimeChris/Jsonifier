#pragma once

#include <jsonifier/Base.hpp>

namespace Jsonifier {

	class Object;
	class parser;
	class JsonIterator;

	class Jsonifier_Dll RawJsonString {
	  public:
		__forceinline RawJsonString() noexcept = default;
		__forceinline RawJsonString(const uint8_t* _buf) noexcept;
		__forceinline const char* raw() const noexcept;
		__forceinline bool unsafeIsEqual(size_t length, std::string_view target) const noexcept;
		__forceinline bool unsafeIsEqual(std::string_view target) const noexcept;
		__forceinline bool unsafeIsEqual(const char* target) const noexcept;
		__forceinline bool isEqual(std::string_view target) const noexcept;
		__forceinline bool isEqual(const char* target) const noexcept;
		__forceinline static bool isFreeFromUnescapedQuote(std::string_view target) noexcept;
		__forceinline static bool isFreeFromUnescapedQuote(const char* target) noexcept;

	  protected:
		__forceinline void consume() noexcept {
			stringView = nullptr;
		}
		__forceinline bool alive() const noexcept {
			return stringView != nullptr;
		}
		__forceinline JsonifierResult<std::string_view> unescape(JsonIterator& iterator) const noexcept;

		const uint8_t* stringView{};
		friend class Object;
		friend class Field;
		friend class Parser;
		friend struct JsonifierResult<RawJsonString>;
		friend class JsonIterator;
	};

	__forceinline std::ostream& operator<<(std::ostream&, const RawJsonString&) noexcept;

	__forceinline bool operator==(const RawJsonString& a, std::string_view c) noexcept;
	__forceinline bool operator==(std::string_view c, const RawJsonString& a) noexcept;
	__forceinline bool operator!=(const RawJsonString& a, std::string_view c) noexcept;
	__forceinline bool operator!=(std::string_view c, const RawJsonString& a) noexcept;

	template<> struct JsonifierResult<RawJsonString> : public JsonifierResultBase<RawJsonString> {
	  public:
		__forceinline JsonifierResult(RawJsonString&& Value) noexcept : JsonifierResultBase<RawJsonString>(std::forward<RawJsonString>(Value)) {
		}

		__forceinline JsonifierResult(ErrorCode error) noexcept : JsonifierResultBase<RawJsonString>(error) {
		}
		__forceinline JsonifierResult() noexcept = default;
		__forceinline ~JsonifierResult() noexcept = default;

		__forceinline JsonifierResult<const char*> raw() noexcept;
		__forceinline JsonifierResult<std::string_view> unescape(JsonIterator& iterator) noexcept;
	};

}