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
#include <Array>
#include <deque>
#include <map>

#pragma warning(push)
#pragma warning(disable : 4244)
#pragma warning(disable : 4251)

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

	class Field;

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
		Scalar_Document_As_Value = 19,
		Incomplete_Array_Or_Object = 20
	};

	enum class JsonType : uint8_t {
		Document = 0,
		Object = 1,
		Array = 2,
		String = 3,
		Float = 4,
		Number = 5,
		Uint64 = 6,
		Int64 = 7,
		Bool = 8,
		Null = 9
	};

	struct JsonifierException : public std::runtime_error, std::string {
		JsonifierException(const std::string&, std::source_location = std::source_location::current()) noexcept;
	};

	template<typename OTy> class ObjectBuffer {
	  public:
		using AllocatorType = std::allocator<OTy>;
		using AllocatorTraits = std::allocator_traits<AllocatorType>;

		__forceinline ObjectBuffer& operator=(ObjectBuffer&& other) noexcept {
			this->currentSize = other.currentSize;
			this->objects = other.objects;
			other.objects = nullptr;
			other.currentSize = 0;
			return *this;
		};
		
		__forceinline ObjectBuffer(ObjectBuffer&& other) noexcept {
			*this = std::move(other);
		};

		__forceinline ObjectBuffer& operator=(const ObjectBuffer&) = delete;
		__forceinline ObjectBuffer(const ObjectBuffer&) = delete;

		__forceinline ObjectBuffer() noexcept = default;

		__forceinline OTy& operator[](size_t index) noexcept {
			return this->objects[index];
		}

		__forceinline operator OTy*&() noexcept {
			return this->objects;
		}

		__forceinline void reset(size_t newSize) noexcept {
			if (this->currentSize < newSize) {
				this->deallocate();
				if (newSize != 0) {
					AllocatorType allocator{};
					this->objects = AllocatorTraits::allocate(allocator, newSize);
					this->currentSize = newSize;
				}
			}
		}

		__forceinline ~ObjectBuffer() noexcept {
			this->deallocate();
		}

	  protected:
		size_t currentSize{};
		OTy* objects{};

		__forceinline void deallocate() {
			if (this->currentSize > 0 && this->objects) {
				AllocatorType allocator{};
				AllocatorTraits::deallocate(allocator, this->objects, this->currentSize);
				this->currentSize = 0;
			}
		}
	};

	template<typename T> struct JsonifierResultBase {
		__forceinline void tie(T& value, ErrorCode& error) && noexcept {
			error = this->second;
			if (!error) {
				value = std::forward<JsonifierResultBase<T>>(*this).first;
			}
		}

		__forceinline ErrorCode get(T& value) && noexcept {
			ErrorCode error{};
			std::forward<JsonifierResultBase<T>>(*this).tie(value, error);
			return error;
		}

		__forceinline ErrorCode error() const noexcept {
			return this->second;
		}

		__forceinline T& value() & noexcept(false) {
			if (error()) {
				throw error();
			}
			return this->first;
		}

		__forceinline T&& value() && noexcept(false) {
			return std::forward<JsonifierResultBase<T>>(*this).takeValue();
		}

		__forceinline T&& takeValue() && noexcept(false) {
			if (error()) {
				throw error();
			}
			return std::forward<T>(this->first);
		}

		operator T&&() && noexcept(false) {
			return std::forward<JsonifierResultBase<T>>(*this).takeValue();
		}

		const T& valueUnsafe() const& noexcept {
			return this->first;
		}

		__forceinline T&& valueUnsafe() && noexcept {
			return std::forward<T>(this->first);
		}

		template<typename T>
		__forceinline JsonifierResultBase(T&& value, ErrorCode error) noexcept : first{ std::forward<T>(value) }, second{ error } {
		}

		__forceinline JsonifierResultBase(ErrorCode error) noexcept : JsonifierResultBase(T{}, error) {
		}

		template<typename T> __forceinline JsonifierResultBase(T&& value) noexcept : JsonifierResultBase(std::forward<T>(value), Success) {
		}

		__forceinline JsonifierResultBase() noexcept : JsonifierResultBase(T{}, Uninitialized) {
		}

	  protected:
		T first{};
		ErrorCode second{};
	};

	template<typename T> struct JsonifierResult : public JsonifierResultBase<T> {
		__forceinline void tie(T& value, ErrorCode& error) && noexcept {
			std::forward<JsonifierResultBase<T>>(*this).tie(value, error);
		}

		__forceinline ErrorCode get(T& value) && noexcept {
			return std::forward<JsonifierResultBase<T>>(*this).get(value);
		}

		__forceinline ErrorCode error() const noexcept {
			return JsonifierResultBase<T>::error();
		}

		__forceinline T& value() & noexcept(false) {
			return JsonifierResultBase<T>::value();
		}

		__forceinline T&& value() && noexcept(false) {
			return std::forward<JsonifierResultBase<T>>(*this).value();
		}

		__forceinline T&& takeValue() && noexcept(false) {
			return std::forward<JsonifierResultBase<T>>(*this).takeValue();
		}

		__forceinline operator T&&() && noexcept(false) {
			return std::forward<JsonifierResultBase<T>>(*this).takeValue();
		}

		const T& valueUnsafe() const& noexcept {
			return JsonifierResultBase<T>::valueUnsafe();
		}

		__forceinline T&& valueUnsafe() && noexcept {
			return std::forward<JsonifierResultBase<T>>(*this).valueUnsafe();
		}

		template<typename T>
		__forceinline JsonifierResult(T&& value, ErrorCode error) noexcept : JsonifierResultBase<T>(std::forward<T>(value), error) {
		}

		__forceinline JsonifierResult(ErrorCode error) noexcept : JsonifierResultBase<T>(error) {
		}

		__forceinline JsonifierResult(T&& value) noexcept : JsonifierResultBase<T>(std::forward<T>(value)) {
		}

		__forceinline JsonifierResult() noexcept : JsonifierResultBase<T>() {
		}
	};

	__forceinline uint32_t isStructuralOrWhitespace(uint8_t c);

	__forceinline uint32_t isNotStructuralOrWhitespace(uint8_t c);

	template<typename TTy> class StopWatch {
	  public:
		using HRClock = std::chrono::high_resolution_clock;

		__forceinline StopWatch() = delete;

		__forceinline StopWatch<TTy>& operator=(const StopWatch<TTy>& data) {
			this->maxNumberOfMs.store(data.maxNumberOfMs.load());
			this->startTime.store(data.startTime.load());
			return *this;
		}

		__forceinline StopWatch(const StopWatch<TTy>& data) {
			*this = data;
		}

		__forceinline StopWatch(TTy maxNumberOfMsNew) {
			this->maxNumberOfMs.store(maxNumberOfMsNew);
			this->startTime.store(std::chrono::duration_cast<TTy>(HRClock::now().time_since_epoch()));
		}

		__forceinline TTy totalTimePassed() {
			TTy currentTime = std::chrono::duration_cast<TTy>(HRClock::now().time_since_epoch());
			TTy elapsedTime = currentTime - this->startTime.load();
			return elapsedTime;
		}

		__forceinline TTy getTotalWaitTime() {
			return this->maxNumberOfMs.load();
		}

		__forceinline bool hasTimePassed() {
			TTy currentTime = std::chrono::duration_cast<TTy>(HRClock::now().time_since_epoch());
			TTy elapsedTime = currentTime - this->startTime.load();
			if (elapsedTime >= this->maxNumberOfMs.load()) {
				return true;
			} else {
				return false;
			}
		}

		__forceinline void resetTimer() {
			this->startTime.store(std::chrono::duration_cast<TTy>(HRClock::now().time_since_epoch()));
		}

	  protected:
		std::atomic<TTy> maxNumberOfMs{ TTy{ 0 } };
		std::atomic<TTy> startTime{ TTy{ 0 } };
	};

}