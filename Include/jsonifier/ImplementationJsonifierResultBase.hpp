#pragma once

#include <jsonifier/Base.hpp>

namespace Jsonifier {
	
	template<typename T> struct ImplementationJsonifierResultBase {
		inline ImplementationJsonifierResultBase& operator=(const ImplementationJsonifierResultBase&) = delete;
		inline ImplementationJsonifierResultBase(const ImplementationJsonifierResultBase&) = delete;
		inline ImplementationJsonifierResultBase& operator=(ImplementationJsonifierResultBase&&) = default;
		inline ImplementationJsonifierResultBase(ImplementationJsonifierResultBase&&) = default;
		inline ImplementationJsonifierResultBase() noexcept = default;
		inline ImplementationJsonifierResultBase(ErrorCode error) noexcept;
		inline ImplementationJsonifierResultBase(T&& value) noexcept;
		inline ImplementationJsonifierResultBase(T&& value, ErrorCode error) noexcept;
		inline void tie(T& value, ErrorCode& error) && noexcept;
		inline ErrorCode get(T& value) && noexcept;
		inline ErrorCode error() const noexcept;
		inline T& valueUnsafe() & noexcept;
		inline const T& valueUnsafe() const& noexcept;
		inline T&& valueUnsafe() && noexcept;
		inline T& value() & noexcept(false);
		inline T&& value() && noexcept(false);
		inline T&& takeValue() && noexcept(false);
		inline operator T&&() && noexcept(false);

	  protected:
		T first;
		ErrorCode second{ Uninitialized };
	};



}