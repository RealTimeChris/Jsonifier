#pragma once

#include <jsonifier/RawJsonString.hpp>
#include <jsonifier/Document.hpp>
#include <jsonifier/Object.hpp>
#include <jsonifier/Array.hpp>
#include <jsonifier/ArrayIterator.hpp>

namespace Jsonifier {

	template<typename T> inline JsonifierResult<T> Document::get() & noexcept {
		static_assert(!sizeof(T), "The get method with given type is not implemented by the Jsonifier library.");
	}

	template<typename T> inline JsonifierResult<T> Document::get() && noexcept {
		static_assert(!sizeof(T), "The get method with given type is not implemented by the Jsonifier library.");
	}

	template<> inline JsonifierResult<Array> Document::get() & noexcept {
		return getArray();
	}

	template<> inline JsonifierResult<Object> Document::get() & noexcept {
		return getObject();
	}

	template<> inline JsonifierResult<RawJsonString> Document::get() & noexcept {
		return getRawJsonString();
	}

	template<> inline JsonifierResult<std::string_view> Document::get() & noexcept {
		return getString();
	}

	template<> inline JsonifierResult<double> Document::get() & noexcept {
		return getDouble();
	}

	template<> inline JsonifierResult<uint64_t> Document::get() & noexcept {
		return getUint64();
	}

	template<> inline JsonifierResult<int64_t> Document::get() & noexcept {
		return getInt64();
	}

	template<> inline JsonifierResult<bool> Document::get() & noexcept {
		return getBool();
	}

	template<> inline JsonifierResult<Value> Document::get() & noexcept {
		return getValue();
	}

	template<> inline JsonifierResult<RawJsonString> Document::get() && noexcept {
		return getRawJsonString();
	}

	template<> inline JsonifierResult<std::string_view> Document::get() && noexcept {
		return getString();
	}

	template<> inline JsonifierResult<double> Document::get() && noexcept {
		return std::forward<Document>(*this).getDouble();
	}

	template<> inline JsonifierResult<uint64_t> Document::get() && noexcept {
		return std::forward<Document>(*this).getUint64();
	}

	template<> inline JsonifierResult<int64_t> Document::get() && noexcept {
		return std::forward<Document>(*this).getInt64();
	}

	template<> inline JsonifierResult<bool> Document::get() && noexcept {
		return std::forward<Document>(*this).getBool();
	}

	template<> inline JsonifierResult<Value> Document::get() && noexcept {
		return getValue();
	}

	template<typename T> ErrorCode Document::get(T& out) & noexcept {
		return get<T>().get(out);
	}

	template<typename T> ErrorCode Document::get(T& out) && noexcept {
		return std::forward<Document>(*this).get<T>().get(out);
	}

	template<typename T> JsonifierResult<T> JsonifierResult<Document>::get() & noexcept {
		if (error()) {
			return error();
		}
		return first.get<T>();
	}

	template<typename T> JsonifierResult<T> JsonifierResult<Document>::get() && noexcept {
		if (error()) {
			return error();
		}
		return std::forward<Document>(first).get<T>();
	}

	template<typename T> ErrorCode JsonifierResult<Document>::get(T& out) & noexcept {
		if (error()) {
			return error();
		}
		return first.get<T>(out);
	}

	template<typename T> ErrorCode JsonifierResult<Document>::get(T& out) && noexcept {
		if (error()) {
			return error();
		}
		return std::forward<Document>(first).get<T>(out);
	}

	template<> inline JsonifierResult<Document> JsonifierResult<Document>::get<Document>() & noexcept = delete;

	template<> inline JsonifierResult<Document> JsonifierResult<Document>::get<Document>() && noexcept {
		if (error()) {
			return error();
		}
		return std::forward<Document>(first);
	}

	template<> inline ErrorCode JsonifierResult<Document>::get<Document>(Document& out) & noexcept = delete;

	template<> inline ErrorCode JsonifierResult<Document>::get<Document>(Document& out) && noexcept {
		if (error()) {
			return error();
		}
		out = std::forward<Document>(first);
		return Success;
	}

}
