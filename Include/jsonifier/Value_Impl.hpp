#pragma once

#include <jsonifier/Field.hpp>
#include <jsonifier/Array.hpp>
#include <jsonifier/Object.hpp>
#include <jsonifier/ArrayIterator.hpp>
#include <jsonifier/ValueIterator.hpp>
#include <jsonifier/RawJsonString.hpp>

namespace Jsonifier {

	template<typename T> inline ErrorCode Value::get(T& out) noexcept {
		return get<T>().get(out);
	}

	template<> inline JsonifierResult<Array> Value::get() noexcept {
		return getArray();
	}

	template<> inline JsonifierResult<Object> Value::get() noexcept {
		return getObject();
	}

	template<> inline JsonifierResult<RawJsonString> Value::get() noexcept {
		return getRawJsonString();
	}

	template<> inline JsonifierResult<std::string_view> Value::get() noexcept {
		return getString();
	}

	template<> inline JsonifierResult<double> Value::get() noexcept {
		return getDouble();
	}

	template<> inline JsonifierResult<uint64_t> Value::get() noexcept {
		return getUint64();
	}

	template<> inline JsonifierResult<int64_t> Value::get() noexcept {
		return getInt64();
	}

	template<> inline JsonifierResult<bool> Value::get() noexcept {
		return getBool();
	}

	template<> inline JsonifierResult<Value> JsonifierResult<Value>::get<Value>() noexcept {
		if (error()) {
			return error();
		}
		return std::move(first);
	}

	template<> inline JsonifierResult<Array> JsonifierResult<Value>::get<Array>() noexcept {
		if (error()) {
			return error();
		}
		return std::move(first.getArray());
	}

	template<> inline ErrorCode JsonifierResult<Value>::get<Value>(Value& out) noexcept {
		if (error()) {
			return error();
		}
		out = first;
		return ErrorCode::Success;
	}

	template<> inline ErrorCode Value::get<Array>(Array& out) noexcept {
		if (error()) {
			return error();
		}
		out = getArray();
		return ErrorCode::Success;
	}

	template<> inline ErrorCode JsonifierResult<Value>::get<Array>(Array& out) noexcept {
		if (error()) {
			return error();
		}
		out = first;
		return ErrorCode::Success;
	}

	template<typename T> inline ErrorCode JsonifierResult<Value>::get(T& out) noexcept {
		if (error()) {
			return error();
		};
		auto result = first.get<T>(out);
		return result;
	}

	template<typename T> inline JsonifierResult<T> Value::get() noexcept {
		static_assert(!sizeof(T), "The get method with given type is not implemented by the Jsonifier library.");
	}
}