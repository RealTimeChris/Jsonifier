#pragma once

#include <source_location>
#include <unordered_map>
#include <immintrin.h>
#include <string_view>
#include <functional>
#include <concepts>
#include <iostream>
#include <vector>
#include <atomic>
#include <memory>
#include <thread>
#include <chrono>
#include <bitset>
#include <array>
#include <deque>
#include <map>


namespace Jsonifier {

	enum ErrorCode : int8_t {
		Success = 0,
		Tape_Error = 1,
		Depth_Error = 2,
		Empty = 3,
		Parse_Error = 4,
		String_Error = 5,
		TAtom_Error = 6,
		FAtom_Error = 7,
		NAtom_Error = 8,
		Mem_Alloc_Error = 9,
		Invalid_Number = 10,
		Incorrect_Type = 11,
		Uninitialized = 12,
		Out_Of_Bounds = 13,
		Invalid_Json_Pointer = 14,
		No_Such_Field = 15,
		Number_Error = 16,
		Trailing_Content = 17,
		Out_Of_Order_Iteration = 18,
		Scalar_Document_As_Value = 19
	};

	template<typename T> struct ImplementationJsonifierResultBase {
		inline ImplementationJsonifierResultBase& operator=(const ImplementationJsonifierResultBase&) = delete;
		inline ImplementationJsonifierResultBase (const ImplementationJsonifierResultBase&) = delete;
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

	template<typename T> struct JsonifierResultBase : protected std::pair<T, ErrorCode> {
		inline JsonifierResultBase& operator=(const JsonifierResultBase&) = delete;
		inline JsonifierResultBase(const JsonifierResultBase&) = delete;
		inline JsonifierResultBase& operator=(JsonifierResultBase&&) = default;
		inline JsonifierResultBase(JsonifierResultBase&&) = default;
		inline JsonifierResultBase() noexcept;
		inline JsonifierResultBase(ErrorCode error) noexcept;
		inline JsonifierResultBase(T&& value) noexcept;
		inline JsonifierResultBase(T&& value, ErrorCode error) noexcept;
		inline void tie(T& value, ErrorCode& error) && noexcept;
		inline ErrorCode get(T& value) && noexcept;
		inline ErrorCode error() const noexcept;
		inline const T& valueUnsafe() const& noexcept;
		inline T&& valueUnsafe() && noexcept;
		inline T& value() & noexcept(false);
		inline T&& value() && noexcept(false);
		inline T&& takeValue() && noexcept(false);
		inline operator T&&() && noexcept(false);
	};

	template<typename T> struct JsonifierResult : public JsonifierResultBase<T> {
		inline JsonifierResult& operator=(const JsonifierResult&) = delete;
		inline JsonifierResult(const JsonifierResult&) = delete;
		inline JsonifierResult& operator=(JsonifierResult&&) = default;
		inline JsonifierResult(JsonifierResult&&) = default;
		inline JsonifierResult() noexcept;
		inline JsonifierResult(T&& value) noexcept;
		inline JsonifierResult(ErrorCode ErrorCode) noexcept;
		inline JsonifierResult(T&& value, ErrorCode error) noexcept;
		inline void tie(T& value, ErrorCode& error) && noexcept;
		inline ErrorCode get(T& value) && noexcept;
		inline ErrorCode error() const noexcept;
		inline const T& valueUnsafe() const& noexcept;
		inline T&& valueUnsafe() && noexcept;
		inline T& value() & noexcept(false);
		inline T&& value() && noexcept(false);
		inline T&& takeValue() && noexcept(false);
		inline operator T&&() && noexcept(false);
	};
}