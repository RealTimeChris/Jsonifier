#pragma once

#include <jsonifier/RawJsonString.hpp>
#include <jsonifier/Document.hpp>
#include <jsonifier/Object.hpp>
#include <jsonifier/Array.hpp>
#include <jsonifier/ArrayIterator.hpp>

namespace Jsonifier {

	template<typename T> __forceinline JsonifierResult<T> Document::get() & noexcept {
		static_assert(!sizeof(T), "The get method with given type is not implemented by the Jsonifier library.");
	}

	template<typename T> __forceinline JsonifierResult<T> Document::get() && noexcept {
		static_assert(!sizeof(T), "The get method with given type is not implemented by the Jsonifier library.");
	}

	template<> __forceinline JsonifierResult<Array> Document::get() & noexcept {
		return getArray();
	}

	template<> __forceinline JsonifierResult<Object> Document::get() & noexcept {
		return getObject();
	}

	template<> __forceinline JsonifierResult<RawJsonString> Document::get() & noexcept {
		return getRawJsonString();
	}

	template<> __forceinline JsonifierResult<std::string_view> Document::get() & noexcept {
		return getString();
	}

	template<> __forceinline JsonifierResult<double> Document::get() & noexcept {
		return getDouble();
	}

	template<> __forceinline JsonifierResult<uint64_t> Document::get() & noexcept {
		return getUint64();
	}

	template<> __forceinline JsonifierResult<int64_t> Document::get() & noexcept {
		return getInt64();
	}

	template<> __forceinline JsonifierResult<bool> Document::get() & noexcept {
		return getBool();
	}

	template<> __forceinline JsonifierResult<Value> Document::get() & noexcept {
		return getValue();
	}

	template<> __forceinline JsonifierResult<RawJsonString> Document::get() && noexcept {
		return getRawJsonString();
	}

	template<> __forceinline JsonifierResult<std::string_view> Document::get() && noexcept {
		return getString();
	}

	template<> __forceinline JsonifierResult<double> Document::get() && noexcept {
		return std::forward<Document>(*this).getDouble();
	}

	template<> __forceinline JsonifierResult<uint64_t> Document::get() && noexcept {
		return std::forward<Document>(*this).getUint64();
	}

	template<> __forceinline JsonifierResult<int64_t> Document::get() && noexcept {
		return std::forward<Document>(*this).getInt64();
	}

	template<> __forceinline JsonifierResult<bool> Document::get() && noexcept {
		return std::forward<Document>(*this).getBool();
	}

	template<> __forceinline JsonifierResult<Value> Document::get() && noexcept {
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

	template<> __forceinline JsonifierResult<Document> JsonifierResult<Document>::get<Document>() & noexcept = delete;

	template<> __forceinline JsonifierResult<Document> JsonifierResult<Document>::get<Document>() && noexcept {
		if (error()) {
			return error();
		}
		return std::forward<Document>(first);
	}

	template<> __forceinline ErrorCode JsonifierResult<Document>::get<Document>(Document& out) & noexcept = delete;

	template<> __forceinline ErrorCode JsonifierResult<Document>::get<Document>(Document& out) && noexcept {
		if (error()) {
			return error();
		}
		out = std::forward<Document>(first);
		return Success;
	}

}
