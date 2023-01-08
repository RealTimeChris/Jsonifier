#pragma once

#include <jsonifier/Array.hpp>
#include <jsonifier/Object.hpp>
#include <jsonifier/Document.hpp>
#include <jsonifier/Field.hpp>
#include <jsonifier/Value.hpp>

namespace Jsonifier {

	template<typename T> inline void JsonifierResultBase<T>::tie(T& value, ErrorCode& error) && noexcept {
		error = this->second;
		if (!error) {
			value = std::forward<JsonifierResultBase<T>>(*this).first;
		}
	}

	template<typename T> inline ErrorCode JsonifierResultBase<T>::get(T& value) && noexcept {
		ErrorCode error{};
		std::forward<JsonifierResultBase<T>>(*this).tie(value, error);
		return error;
	}

	template<typename T> inline ErrorCode JsonifierResultBase<T>::error() const noexcept {
		return this->second;
	}

	template<typename T> inline T& JsonifierResultBase<T>::value() & noexcept(false) {
		if (error()) {
			throw simdjson_error(error());
		}
		return this->first;
	}

	template<typename T> inline T&& JsonifierResultBase<T>::value() && noexcept(false) {
		return std::forward<JsonifierResultBase<T>>(*this).takeValue();
	}

	template<typename T> inline T&& JsonifierResultBase<T>::takeValue() && noexcept(false) {
		if (error()) {
			throw error();
		}
		return std::forward<T>(this->first);
	}

	template<typename T> inline JsonifierResultBase<T>::operator T&&() && noexcept(false) {
		return std::forward<JsonifierResultBase<T>>(*this).takeValue();
	}

	template<typename T> inline const T& JsonifierResultBase<T>::valueUnsafe() const& noexcept {
		return this->first;
	}

	template<typename T> inline T&& JsonifierResultBase<T>::valueUnsafe() && noexcept {
		return std::forward<T>(this->first);
	}

	template<typename T>
	inline JsonifierResultBase<T>::JsonifierResultBase(T&& value, ErrorCode error) noexcept
		: std::pair<T, ErrorCode>(std::forward<T>(value), error){}

	template<typename T> inline JsonifierResultBase<T>::JsonifierResultBase(ErrorCode error) noexcept : JsonifierResultBase(T{}, error){}

	template<typename T>
	inline JsonifierResultBase<T>::JsonifierResultBase(T&& value) noexcept : JsonifierResultBase(std::forward<T>(value), Success){}

	template<typename T> inline JsonifierResultBase<T>::JsonifierResultBase() noexcept : JsonifierResultBase(T{}, Uninitialized){}

	template<typename T> inline void JsonifierResult<T>::tie(T& value, ErrorCode& error) && noexcept {
		std::forward<JsonifierResultBase<T>>(*this).tie(value, error);
	}

	template<typename T> inline ErrorCode JsonifierResult<T>::get(T& value) && noexcept {
		return std::forward<JsonifierResultBase<T>>(*this).get(value);
	}

	template<typename T> inline ErrorCode JsonifierResult<T>::error() const noexcept {
		return JsonifierResultBase<T>::error();
	}

	template<typename T> inline T& JsonifierResult<T>::value() & noexcept(false) {
		return JsonifierResultBase<T>::value();
	}

	template<typename T> inline T&& JsonifierResult<T>::value() && noexcept(false) {
		return std::forward<JsonifierResultBase<T>>(*this).value();
	}

	template<typename T> inline T&& JsonifierResult<T>::takeValue() && noexcept(false) {
		return std::forward<JsonifierResultBase<T>>(*this).takeValue();
	}

	template<typename T> inline JsonifierResult<T>::operator T&&() && noexcept(false) {
		return std::forward<JsonifierResultBase<T>>(*this).takeValue();
	}

	template<typename T> inline const T& JsonifierResult<T>::valueUnsafe() const& noexcept {
		return JsonifierResultBase<T>::valueUnsafe();
	}

	template<typename T> inline T&& JsonifierResult<T>::valueUnsafe() && noexcept {
		return std::forward<JsonifierResultBase<T>>(*this).valueUnsafe();
	}

	template<typename T>
	inline JsonifierResult<T>::JsonifierResult(T&& value, ErrorCode error) noexcept : JsonifierResultBase<T>(std::forward<T>(value), error){}

	template<typename T> inline JsonifierResult<T>::JsonifierResult(ErrorCode error) noexcept : JsonifierResultBase<T>(error){}

	template<typename T> inline JsonifierResult<T>::JsonifierResult(T&& value) noexcept : JsonifierResultBase<T>(std::forward<T>(value)){}

	template<typename T> inline JsonifierResult<T>::JsonifierResult() noexcept : JsonifierResultBase<T>(){}

	template<typename T> inline void ImplementationJsonifierResultBase<T>::tie(T& value, ErrorCode& error) && noexcept {
		error = this->second;
		if (!error) {
			value = std::forward<ImplementationJsonifierResultBase<T>>(*this).first;
		}
	}

	template<typename T> inline ErrorCode ImplementationJsonifierResultBase<T>::get(T& value) && noexcept {
		ErrorCode error{};
		std::forward<ImplementationJsonifierResultBase<T>>(*this).tie(value, error);
		return error;
	}

	template<typename T> inline ErrorCode ImplementationJsonifierResultBase<T>::error() const noexcept {
		return this->second;
	}

	template<typename T> inline T& ImplementationJsonifierResultBase<T>::value() & noexcept(false) {
		if (error()) {
			throw error();
		}
		return this->first;
	}

	template<typename T> inline T&& ImplementationJsonifierResultBase<T>::value() && noexcept(false) {
		return std::forward<ImplementationJsonifierResultBase<T>>(*this).takeValue();
	}

	template<typename T> inline T&& ImplementationJsonifierResultBase<T>::takeValue() && noexcept(false) {
		if (error()) {
			throw error();
		}
		return std::forward<T>(this->first);
	}

	template<typename T> inline ImplementationJsonifierResultBase<T>::operator T&&() && noexcept(false) {
		return std::forward<ImplementationJsonifierResultBase<T>>(*this).takeValue();
	}

	template<typename T> inline const T& ImplementationJsonifierResultBase<T>::valueUnsafe() const& noexcept {
		return this->first;
	}

	template<typename T> inline T& ImplementationJsonifierResultBase<T>::valueUnsafe() & noexcept {
		return this->first;
	}

	template<typename T> inline T&& ImplementationJsonifierResultBase<T>::valueUnsafe() && noexcept {
		return std::forward<T>(this->first);
	}

	template<typename T>
	inline ImplementationJsonifierResultBase<T>::ImplementationJsonifierResultBase(T&& value, ErrorCode error) noexcept
		: first{ std::forward<T>(value) }, second{ error } {
	}

	template<typename T>
	inline ImplementationJsonifierResultBase<T>::ImplementationJsonifierResultBase(ErrorCode error) noexcept
		: ImplementationJsonifierResultBase(T{ std::move(this->first) }, error) {
	}

	template<typename T>
	inline ImplementationJsonifierResultBase<T>::ImplementationJsonifierResultBase(T&& value) noexcept
		: ImplementationJsonifierResultBase(std::forward<T>(value), Success) {
	}

}
