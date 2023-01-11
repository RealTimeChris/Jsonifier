#pragma once

#include <jsonifier/RawJsonString.hpp>
#include <jsonifier/JsonIterator.hpp>

namespace Jsonifier {

	__forceinline RawJsonString::RawJsonString(const uint8_t* _buf) noexcept : stringView{ _buf } {
	}

	__forceinline const char* RawJsonString::raw() const noexcept {
		return reinterpret_cast<const char*>(stringView);
	}

	__forceinline bool RawJsonString::isFreeFromUnescapedQuote(std::string_view target) noexcept {
		size_t pos{ 0 };
		for (; pos < target.size() && target[pos] != '\\'; pos++) {
		}
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

	__forceinline bool RawJsonString::isFreeFromUnescapedQuote(const char* target) noexcept {
		size_t pos{ 0 };
		for (; target[pos] && target[pos] != '\\'; pos++) {
		}
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

	__forceinline bool RawJsonString::unsafeIsEqual(size_t length, std::string_view target) const noexcept {
		return (length >= target.size()) && (raw()[target.size()] == '"') && !std::memcmp(raw(), target.data(), target.size());
	}

	__forceinline bool RawJsonString::unsafeIsEqual(std::string_view target) const noexcept {
		if (target.size() <= 256) {
			return (raw()[target.size()] == '"') && !std::memcmp(raw(), target.data(), target.size());
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

	__forceinline bool RawJsonString::isEqual(std::string_view target) const noexcept {
		const char* r{ raw() };
		size_t pos{ 0 };
		bool escaping{ false };
		for (; pos < target.size(); pos++) {
			if (r[pos] != target[pos]) {
				return false;
			}
			if ((target[pos] == '"') && !escaping) {
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

	__forceinline bool RawJsonString::unsafeIsEqual(const char* target) const noexcept {
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

	__forceinline bool RawJsonString::isEqual(const char* target) const noexcept {
		const char* r{ raw() };
		size_t pos{ 0 };
		bool escaping{ false };
		for (; target[pos]; pos++) {
			if (r[pos] != target[pos]) {
				return false;
			}
			if ((target[pos] == '"') && !escaping) {
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

	__forceinline bool operator==(const RawJsonString& a, std::string_view c) noexcept {
		return a.unsafeIsEqual(c);
	}

	__forceinline bool operator==(std::string_view c, const RawJsonString& a) noexcept {
		return a == c;
	}

	__forceinline bool operator!=(const RawJsonString& a, std::string_view c) noexcept {
		return !(a == c);
	}

	__forceinline bool operator!=(std::string_view c, const RawJsonString& a) noexcept {
		return !(a == c);
	}

	__forceinline JsonifierResult<std::string_view> RawJsonString::unescape(JsonIterator& iterator) const noexcept {
		return iterator.unescape(*this);
	}

	__forceinline std::ostream& operator<<(std::ostream& out, const RawJsonString& str) noexcept {
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

	__forceinline JsonifierResult<const char*> JsonifierResult<RawJsonString>::raw() noexcept {
		if (error()) {
			return error();
		}
		return first.raw();
	}

	__forceinline JsonifierResult<std::string_view> JsonifierResult<RawJsonString>::unescape(JsonIterator& iterator) noexcept {
		if (error()) {
			return error();
		}
		return first.unescape(iterator);
	}

}
