#pragma once

#pragma warning(push)
#pragma warning(disable : 4244)
#pragma warning(disable : 4251)

#include <source_location>
#include <unordered_map>
#include <immintrin.h>
#include <string_view>
#include <functional>
#include <concepts>
#include <intrin.h>
#include <iostream>
#include <cassert>
#include <vector>
#include <atomic>
#include <memory>
#include <thread>
#include <chrono>
#include <bitset>
#include <array>
#include <deque>
#include <map>

#pragma warning(push)
#pragma warning(disable : 4244)
#pragma warning(disable : 4251)

#ifdef _DEBUG
	#ifdef JSONIFIER_DEV_CHECKS
		#define JSONIFIER_DEVELOPMENT_CHECKS 0
	#endif
#endif

#ifndef JsonifierTry
#define JsonifierTry(EXPR) \
	{ \
		auto _err = (EXPR); \
		if (_err) { \
			return _err; \
		} \
	}
#endif

#ifdef _WIN32
	#ifdef Jsonifier_EXPORTS
		#define Jsonifier_Dll __declspec(dllexport)
	#else
		#define Jsonifier_Dll __declspec(dllimport)
	#endif
#else
	#define Jsonifier_Dll
#endif

#ifdef max
	#undef max
#endif
#ifdef min
	#undef min
#endif

namespace Jsonifier {

	enum ErrorCode {
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

	template<typename T> struct JsonifierResultBase : protected std::pair<T, ErrorCode> {
		inline JsonifierResultBase() noexcept;
		inline JsonifierResultBase(ErrorCode error) noexcept;
		inline JsonifierResultBase(T&& value) noexcept;
		inline JsonifierResultBase(T&& value, ErrorCode error) noexcept;
		inline void tie(T& value, ErrorCode& error) && noexcept;
		inline ErrorCode get(T& value) && noexcept;
		inline ErrorCode error() const noexcept;
		inline T& value() & noexcept(false);
		inline T&& value() && noexcept(false);
		inline T&& takeValue() && noexcept(false);
		inline operator T&&() && noexcept(false);
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
		inline T& value() & noexcept(false);
		inline T&& value() && noexcept(false);
		inline T&& takeValue() && noexcept(false);
		inline operator T&&() && noexcept(false);
		inline const T& valueUnsafe() const& noexcept;
		inline T&& valueUnsafe() && noexcept;
	};

	inline uint32_t isStructuralOrWhitespace(uint8_t c);
	
	inline uint32_t isNotStructuralOrWhitespace(uint8_t c);

}