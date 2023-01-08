#pragma once

#include <jsonifier/RawJsonString.hpp>
#include <jsonifier/JsonIterator.hpp>

namespace Jsonifier {

	inline RawJsonString::RawJsonString(const uint8_t* _buf) noexcept : stringView{ _buf } {
	}

	inline const char* RawJsonString::raw() const noexcept {
		return reinterpret_cast<const char*>(stringView);
	}


	inline bool RawJsonString::is_free_from_unescaped_quote(std::string_view target) noexcept {
		size_t pos{ 0 };
		// if the content has no escape character, just scan through it quickly!
		for (; pos < target.size() && target[pos] != '\\'; pos++) {
		}
		// slow path may begin.
		bool escaping{ false };
		for (; pos < target.size(); pos++) {
			if ((target[pos] == '"') && !escaping) {
				return false;
			} else if (target[pos] == '\\') {
				escaping = !escaping;
			} else {
				escaping = false;
			}
		}
		return true;
	}

	inline bool RawJsonString::is_free_from_unescaped_quote(const char* target) noexcept {
		size_t pos{ 0 };
		// if the content has no escape character, just scan through it quickly!
		for (; target[pos] && target[pos] != '\\'; pos++) {
		}
		// slow path may begin.
		bool escaping{ false };
		for (; target[pos]; pos++) {
			if ((target[pos] == '"') && !escaping) {
				return false;
			} else if (target[pos] == '\\') {
				escaping = !escaping;
			} else {
				escaping = false;
			}
		}
		return true;
	}


	inline bool RawJsonString::unsafe_is_equal(size_t length, std::string_view target) const noexcept {
		// If we are going to call memcmp, then we must know something about the length of the RawJsonString.
		return (length >= target.size()) && (raw()[target.size()] == '"') && !memcmp(raw(), target.data(), target.size());
	}

	inline bool RawJsonString::unsafe_is_equal(std::string_view target) const noexcept {
		// Assumptions: does not contain unescaped quote characters, and
		// the raw content is quote terminated within a valid JSON string.
		if (target.size() <= 256) {
			return (raw()[target.size()] == '"') && !memcmp(raw(), target.data(), target.size());
		}
		const char* r{ raw() };
		size_t pos{ 0 };
		for (; pos < target.size(); pos++) {
			if (r[pos] != target[pos]) {
				return false;
			}
		}
		if (r[pos] != '"') {
			return false;
		}
		return true;
	}

	inline bool RawJsonString::is_equal(std::string_view target) const noexcept {
		const char* r{ raw() };
		size_t pos{ 0 };
		bool escaping{ false };
		for (; pos < target.size(); pos++) {
			if (r[pos] != target[pos]) {
				return false;
			}
			// if target is a compile-time constant and it is free from
			// quotes, then the next part could get optimized away through
			// inlining.
			if ((target[pos] == '"') && !escaping) {
				// We have reached the end of the RawJsonString but
				// the target is not done.
				return false;
			} else if (target[pos] == '\\') {
				escaping = !escaping;
			} else {
				escaping = false;
			}
		}
		if (r[pos] != '"') {
			return false;
		}
		return true;
	}


	inline bool RawJsonString::unsafe_is_equal(const char* target) const noexcept {
		// Assumptions: 'target' does not contain unescaped quote characters, is null terminated and
		// the raw content is quote terminated within a valid JSON string.
		const char* r{ raw() };
		size_t pos{ 0 };
		for (; target[pos]; pos++) {
			if (r[pos] != target[pos]) {
				return false;
			}
		}
		if (r[pos] != '"') {
			return false;
		}
		return true;
	}

	inline bool RawJsonString::is_equal(const char* target) const noexcept {
		// Assumptions: does not contain unescaped quote characters, and
		// the raw content is quote terminated within a valid JSON string.
		const char* r{ raw() };
		size_t pos{ 0 };
		bool escaping{ false };
		for (; target[pos]; pos++) {
			if (r[pos] != target[pos]) {
				return false;
			}
			// if target is a compile-time constant and it is free from
			// quotes, then the next part could get optimized away through
			// inlining.
			if ((target[pos] == '"') && !escaping) {
				// We have reached the end of the RawJsonString but
				// the target is not done.
				return false;
			} else if (target[pos] == '\\') {
				escaping = !escaping;
			} else {
				escaping = false;
			}
		}
		if (r[pos] != '"') {
			return false;
		}
		return true;
	}

	inline bool operator==(const RawJsonString& a, std::string_view c) noexcept {
		return a.unsafe_is_equal(c);
	}

	inline bool operator==(std::string_view c, const RawJsonString& a) noexcept {
		return a == c;
	}

	inline bool operator!=(const RawJsonString& a, std::string_view c) noexcept {
		return !(a == c);
	}

	inline bool operator!=(std::string_view c, const RawJsonString& a) noexcept {
		return !(a == c);
	}


	inline JsonifierResult<std::string_view> RawJsonString::unescape(JsonIterator& iterator) const noexcept {
		return iterator.unescape(*this);
	}


	inline std::ostream& operator<<(std::ostream& out, const RawJsonString& str) noexcept {
		bool in_escape = false;
		const char* s = str.raw();
		while (true) {
			switch (*s) {
				case '\\':
					in_escape = !in_escape;
					break;
				case '"':
					if (in_escape) {
						in_escape = false;
					} else {
						return out;
					}
					break;
				default:
					if (in_escape) {
						in_escape = false;
					}
			}
			out << *s;
			s++;
		}
	}
	inline JsonifierResult<RawJsonString>::JsonifierResult(RawJsonString&& Value) noexcept
		: ImplementationJsonifierResultBase<RawJsonString>(std::forward<RawJsonString>(Value)) {
	}
	inline JsonifierResult<RawJsonString>::JsonifierResult(ErrorCode error) noexcept : ImplementationJsonifierResultBase<RawJsonString>(error) {
	}

	inline JsonifierResult<const char*> JsonifierResult<RawJsonString>::raw() const noexcept {
		if (error()) {
			return error();
		}
		return first.raw();
	}
	inline JsonifierResult<std::string_view> JsonifierResult<RawJsonString>::unescape(JsonIterator& iterator) const noexcept {
		if (error()) {
			return error();
		}
		return first.unescape(iterator);
	}

}
