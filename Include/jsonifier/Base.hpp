/*
	Jsonifier - For parsing and serializing Json - very rapidly.
	Copyright (C) 2023 Chris M. (RealTimeChris)

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
	Lesser General Public License for more details.

	You should have received a copy of the GNU Lesser General Public
	License along with this library; if not, write to the Free Software
	Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301
	USA
*/
/// https://github.com/RealTimeChris/Jsonifier
/// Feb 3, 2023
#pragma once

#pragma warning(push)
#pragma warning(disable : 4244)
#pragma warning(disable : 4251)

#include <source_location>
#include <unordered_map>
#include <immintrin.h>
#include <string_view>
#include <concepts>
#include <iostream>
#include <vector>
#include <chrono>
#include <bitset>
#include <map>

#ifndef JsonifierTry
	#define JsonifierTry(EXPR) \
		{ \
			auto _err = (EXPR); \
			if (_err) { \
				setError(_err); \
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
	#define __popcnt64 _popcnt64
	#define __forceinline __attribute__((always_inline))
	#define Jsonifier_Dll
#endif

#ifdef max
	#undef max
#endif
#ifdef min
	#undef min
#endif

namespace Jsonifier {

	using InStringPtr = const uint8_t*;
	using StructuralIndex = uint32_t*;
	using OutStringPtr = uint8_t*;

	enum ErrorCode {
		Success = 0,
		Empty = 1,
		Parse_Error = 2,
		String_Error = 3,
		Mem_Alloc_Error = 4,
		Invalid_Number = 5,
		Incorrect_Type = 6,
		Uninitialized = 7,
		Out_Of_Bounds = 8,
		No_Such_Field = 9,
		Number_Error = 10,
		No_String_Error = 11,
		Type_Error = 12,
		No_Such_Entity = 13
	};

	__forceinline std::ostream& operator<<(std::ostream& out, ErrorCode type) noexcept {
		switch (type) {
			case Success:
				out << "Success";
				break;
			case Empty:
				out << "Empty";
				break;
			case Parse_Error:
				out << "Parse Error";
				break;
			case String_Error:
				out << "String Error";
				break;
			case Mem_Alloc_Error:
				out << "Memory Allocation Error";
				break;
			case Invalid_Number:
				out << "Invalid Number Error";
				break;
			case Incorrect_Type:
				out << "Incorrect Type Error";
				break;
			case Uninitialized:
				out << "Uninitialized Error";
				break;
			case Out_Of_Bounds:
				out << "Out of Bounds Error";
				break;
			case No_Such_Field:
				out << "No Such Field Error";
				break;
			case Number_Error:
				out << "Number Error";
				break;
			case No_String_Error:
				out << "No String Error";
				break;
			case Type_Error:
				out << "Type Error";
				break;
		}
		return out;
	}

	enum class JsonType : uint16_t {
		Unset = 1 << 0,
		Object = 1 << 1,
		Array = 1 << 2,
		String = 1 << 3,
		Float = 1 << 4,
		Uint64 = 1 << 5,
		Int64 = 1 << 6,
		Bool = 1 << 7,
		Null = 1 << 8,
		Number = Float | Uint64 | Int64,
	};

	struct JsonifierException : public std::runtime_error, std::string {
		JsonifierException(const std::string&, std::source_location = std::source_location::current()) noexcept;
	};

	template<typename OTy> class ObjectBuffer {
	  public:
		using AllocatorType = std::allocator<OTy>;
		using AllocatorTraits = std::allocator_traits<AllocatorType>;

		__forceinline ObjectBuffer() noexcept = default;

		__forceinline ObjectBuffer& operator=(ObjectBuffer&& other) noexcept = delete;
		__forceinline ObjectBuffer(ObjectBuffer&& other) noexcept = delete;
		__forceinline ObjectBuffer& operator=(const ObjectBuffer&) = delete;
		__forceinline ObjectBuffer(const ObjectBuffer&) = delete;

		__forceinline OTy& operator[](size_t index) noexcept {
			return objects[index];
		}

		__forceinline operator OTy*() noexcept {
			return objects;
		}

		__forceinline void reset(size_t newSize) noexcept {
			if (currentSize < newSize) {
				deallocate();
				if (newSize != 0) {
					AllocatorType allocator{};
					objects = AllocatorTraits::allocate(allocator, newSize);
					currentSize = newSize;
				}
			}
		}

		__forceinline ~ObjectBuffer() noexcept {
			deallocate();
		}

	  protected:
		size_t currentSize{};
		OTy* objects{};

		__forceinline void deallocate() {
			if (currentSize > 0 && objects) {
				AllocatorType allocator{};
				AllocatorTraits::deallocate(allocator, objects, currentSize);
				currentSize = 0;
			}
		}
	};

	template<typename OTy> class JsonifierResultBase {
	  public:
		__forceinline ErrorCode get(OTy& value) noexcept {
			if (!first) {
				value = std::move(second);
			}
			return first;
		}

		__forceinline ErrorCode error() noexcept {
			return first;
		}

		__forceinline OTy&& value() noexcept(false) {
			if (first) {
				throw first;
			}
			return std::move(second);
		}

		__forceinline operator OTy&&() noexcept(false) {
			return value();
		}

		__forceinline JsonifierResultBase() noexcept = default;

		__forceinline JsonifierResultBase(OTy&& value, ErrorCode error) noexcept : second{ std::forward<OTy>(value) }, first{ error } {
		}

		__forceinline JsonifierResultBase(ErrorCode error) noexcept : second{ OTy{} }, first{ error } {
		}

		__forceinline JsonifierResultBase(OTy&& value) noexcept : second{ std::forward<OTy>(value) }, first{ Success } {
		}

	  protected:
		ErrorCode first{ Uninitialized };
		OTy second{};
	};

	template<typename OTy> class JsonifierResult : public JsonifierResultBase<OTy> {
	  public:
		__forceinline ErrorCode get(OTy& value) noexcept {
			return JsonifierResultBase<OTy>::get(value);
		}

		__forceinline ErrorCode error() noexcept {
			return JsonifierResultBase<OTy>::reportError();
		}

		__forceinline OTy&& value() noexcept(false) {
			return JsonifierResultBase<OTy>::value();
		}

		__forceinline operator OTy&&() noexcept(false) {
			return JsonifierResultBase<OTy>::operator OTy&&();
		}

		__forceinline JsonifierResult() noexcept = default;

		__forceinline JsonifierResult(OTy&& value, ErrorCode error) noexcept : JsonifierResultBase<OTy>(std::forward<OTy>(value), error) {
		}

		__forceinline JsonifierResult(ErrorCode error) noexcept : JsonifierResultBase<OTy>(error) {
		}

		__forceinline JsonifierResult(OTy&& value) noexcept : JsonifierResultBase<OTy>(std::forward<OTy>(value)) {
		}
	};

	template<typename TTy> class StopWatch {
	  public:
		using HRClock = std::chrono::high_resolution_clock;

		__forceinline StopWatch() = delete;

		__forceinline StopWatch<TTy>& operator=(StopWatch<TTy>&& data) {
			maxNumberOfMs.store(data.maxNumberOfMs.load());
			startTime.store(data.startTime.load());
			return *this;
		}

		__forceinline StopWatch(StopWatch<TTy>&& data) {
			*this = std::move(data);
		}

		__forceinline StopWatch(TTy maxNumberOfMsNew) {
			maxNumberOfMs.store(maxNumberOfMsNew);
			startTime.store(std::chrono::duration_cast<TTy>(HRClock::now().time_since_epoch()));
		}

		__forceinline TTy totalTimePassed() {
			return std::chrono::duration_cast<TTy>(HRClock::now().time_since_epoch()) - startTime.load();
		}

		__forceinline TTy getTotalWaitTime() {
			return maxNumberOfMs.load();
		}

		__forceinline bool hasTimePassed() {
			if (std::chrono::duration_cast<TTy>(HRClock::now().time_since_epoch()) - startTime.load() >= maxNumberOfMs.load()) {
				return true;
			} else {
				return false;
			}
		}

		__forceinline void resetTimer() {
			startTime.store(std::chrono::duration_cast<TTy>(HRClock::now().time_since_epoch()));
		}

	  protected:
		std::atomic<TTy> maxNumberOfMs{ TTy{ 0 } };
		std::atomic<TTy> startTime{ TTy{ 0 } };
	};

}