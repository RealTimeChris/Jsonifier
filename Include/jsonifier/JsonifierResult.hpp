#pragma once

#include "StringParsingUtils.hpp"

namespace Jsonifier {

	template<typename T> struct JsonifierResultBase : protected std::pair<T, ErrorCode> {
		inline JsonifierResultBase() noexcept;
		inline JsonifierResultBase(ErrorCode error) noexcept;
		inline JsonifierResultBase(T&& value) noexcept;
		inline JsonifierResultBase(T&& value, ErrorCode error) noexcept;
		inline void tie(T& value, ErrorCode& error) && noexcept;
		inline ErrorCode get(T& value) && noexcept;
		inline ErrorCode error() const noexcept;
		inline const T& valueUnsafe() const& noexcept;
		inline T&& valueUnsafe() && noexcept;

	};

	template<typename T> struct JsonifierResult : public JsonifierResultBase<T> {
		inline JsonifierResult() noexcept;
		inline JsonifierResult(T&& value) noexcept;
		inline JsonifierResult(ErrorCode ErrorCode) noexcept;
		inline JsonifierResult(T&& value, ErrorCode error) noexcept;
		inline void tie(T& value, ErrorCode& error) && noexcept;
		inline ErrorCode get(T& value) && noexcept;
		inline ErrorCode error() const noexcept;
		inline const T& valueUnsafe() const& noexcept;
		inline T&& valueUnsafe() && noexcept;
	};
}