#pragma once

#include <jsonifier/Base.hpp>

namespace Jsonifier {

	class Object;
	class parser;
	class JsonIterator;

	class Jsonifier_Dll RawJsonString {
	  public:
		RawJsonString() noexcept = default;
		RawJsonString(const uint8_t* _buf) noexcept;
		const char* raw() const noexcept;
		bool unsafeIsEqual(size_t length, std::string_view target) const noexcept;
		bool unsafeIsEqual(std::string_view target) const noexcept;
		bool unsafeIsEqual(const char* target) const noexcept;
		bool isEqual(std::string_view target) const noexcept;
		bool isEqual(const char* target) const noexcept;
		static bool isFreeFromUnescapedQuote(std::string_view target) noexcept;
		static bool isFreeFromUnescapedQuote(const char* target) noexcept;

	  protected:
		void consume() noexcept {
			stringView = nullptr;
		}
		bool alive() const noexcept {
			return stringView != nullptr;
		}
		JsonifierResult<std::string_view> unescape(JsonIterator& iterator) const noexcept;

		const uint8_t* stringView{};
		friend class Object;
		friend class Field;
		friend class Parser;
		friend struct JsonifierResult<RawJsonString>;
		friend class JsonIterator;
	};

	std::ostream& operator<<(std::ostream&, const RawJsonString&) noexcept;

	bool operator==(const RawJsonString& a, std::string_view c) noexcept;
	bool operator==(std::string_view c, const RawJsonString& a) noexcept;
	bool operator!=(const RawJsonString& a, std::string_view c) noexcept;
	bool operator!=(std::string_view c, const RawJsonString& a) noexcept;

	template<> struct JsonifierResult<RawJsonString> : public JsonifierResultBase<RawJsonString> {
	  public:
		JsonifierResult(RawJsonString&& Value) noexcept : JsonifierResultBase<RawJsonString>(std::forward<RawJsonString>(Value)) {
		}

		JsonifierResult(ErrorCode error) noexcept : JsonifierResultBase<RawJsonString>(error) {
		}
		JsonifierResult() noexcept = default;
		~JsonifierResult() noexcept = default;

		JsonifierResult<const char*> raw() noexcept;
		JsonifierResult<std::string_view> unescape(JsonIterator& iterator) noexcept;
	};

}