#pragma once

#include <jsonifier/Array.hpp>
#include <jsonifier/Object.hpp>
#include <jsonifier/Document.hpp>
#include <jsonifier/Field.hpp>
#include <jsonifier/Value.hpp>

namespace Jsonifier {

	template<typename T> inline void JsonifierResultBase<T>::tie(T& Value, ErrorCode& error) && noexcept {
		error = this->second;
		if (!error) {
			Value = std::forward<JsonifierResultBase<T>>(*this).first;
		}
	}

	template<typename T> inline ErrorCode JsonifierResultBase<T>::get(T& Value) && noexcept {
		ErrorCode error{};
		std::forward<JsonifierResultBase<T>>(*this).tie(Value, error);
		return error;
	}

	template<typename T> inline ErrorCode JsonifierResultBase<T>::error() const noexcept {
		return this->second;
	}

	template<typename T> inline T& JsonifierResultBase<T>::value() & noexcept(false) {
		if (error()) {
			throw error();
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
	inline JsonifierResultBase<T>::JsonifierResultBase(T&& Value, ErrorCode error) noexcept : std::pair<T, ErrorCode>(std::forward<T>(Value), error) {
	}

	template<typename T> inline JsonifierResultBase<T>::JsonifierResultBase(ErrorCode error) noexcept : JsonifierResultBase(T{}, error) {
	}

	template<typename T>
	inline JsonifierResultBase<T>::JsonifierResultBase(T&& Value) noexcept : JsonifierResultBase(std::forward<T>(Value), Success) {
	}

	template<typename T> inline JsonifierResultBase<T>::JsonifierResultBase() noexcept : JsonifierResultBase(T{}, Uninitialized) {
	}

	template<typename T> inline void JsonifierResult<T>::tie(T& Value, ErrorCode& error) && noexcept {
		std::forward<JsonifierResultBase<T>>(*this).tie(Value, error);
	}

	template<typename T> inline ErrorCode JsonifierResult<T>::get(T& Value) && noexcept {
		return std::forward<JsonifierResultBase<T>>(*this).get(Value);
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
	inline JsonifierResult<T>::JsonifierResult(T&& Value, ErrorCode error) noexcept : JsonifierResultBase<T>(std::forward<T>(Value), error) {
	}

	template<typename T> inline JsonifierResult<T>::JsonifierResult(ErrorCode error) noexcept : JsonifierResultBase<T>(error) {
	}

	template<typename T> inline JsonifierResult<T>::JsonifierResult(T&& Value) noexcept : JsonifierResultBase<T>(std::forward<T>(Value)) {
	}

	template<typename T> inline JsonifierResult<T>::JsonifierResult() noexcept : JsonifierResultBase<T>() {
	}

	template<typename T> inline void ImplementationJsonifierResultBase<T>::tie(T& Value, ErrorCode& error) && noexcept {
		error = this->second;
		if (!error) {
			Value = std::forward<ImplementationJsonifierResultBase<T>>(*this).first;
		}
	}

	template<typename T> inline ErrorCode ImplementationJsonifierResultBase<T>::get(T& Value) && noexcept {
		ErrorCode error{};
		std::forward<ImplementationJsonifierResultBase<T>>(*this).tie(Value, error);
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
	inline ImplementationJsonifierResultBase<T>::ImplementationJsonifierResultBase(T&& Value, ErrorCode error) noexcept
		: first{ std::forward<T>(Value) }, second{ error } {
	}

	template<typename T>
	inline ImplementationJsonifierResultBase<T>::ImplementationJsonifierResultBase(ErrorCode error) noexcept
		: ImplementationJsonifierResultBase(T{ std::move(this->first) }, error) {
	}

	template<typename T>
	inline ImplementationJsonifierResultBase<T>::ImplementationJsonifierResultBase(T&& Value) noexcept
		: ImplementationJsonifierResultBase(std::forward<T>(Value), Success) {
	}

}
