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

		inline ObjectBuffer& operator=(ObjectBuffer&&) = delete;
		inline ObjectBuffer(ObjectBuffer&&) = delete;

		inline ObjectBuffer& operator=(const ObjectBuffer&) = delete;
		inline ObjectBuffer(const ObjectBuffer&) = delete;

		inline ObjectBuffer() noexcept = default;

		inline OTy& operator[](size_t index) noexcept {
			return this->objects[index];
		}

		inline operator OTy*() noexcept {
			return this->objects;
		}

		inline void reset(size_t newSize) noexcept {
			this->deallocate();
			if (newSize != 0) {
				AllocatorType allocator{};
				this->objects = AllocatorTraits::allocate(allocator, newSize);
				this->currentSize = newSize;
			}
		}

		inline ~ObjectBuffer() noexcept {
			this->deallocate();
		}

	  protected:
		size_t currentSize{};
		OTy* objects{};

		inline void deallocate() {
			if (this->currentSize > 0 && this->objects) {
				AllocatorType allocator{};
				AllocatorTraits::deallocate(allocator, this->objects, this->currentSize);
				this->currentSize = 0;
			}
		}
	};

	template<typename T> struct JsonifierResultBase  { 
		
		inline void tie(T& value, ErrorCode& error) && noexcept;
		inline ErrorCode get(T& value) && noexcept;
		
		inline ErrorCode error() const noexcept;

		inline T& value() & noexcept(false);
		
		inline T&& value() && noexcept(false);
		
		inline T&& takeValue() && noexcept(false);
		
		inline operator T&&() && noexcept(false);
		
		inline const T& valueUnsafe() const& noexcept;
		
		inline T&& valueUnsafe() && noexcept;
		
		inline JsonifierResultBase(T&& value, ErrorCode error) noexcept;

		inline JsonifierResultBase(ErrorCode error) noexcept;

		inline JsonifierResultBase(T&& value) noexcept;

		inline JsonifierResultBase() noexcept;

	  protected:
		T first{};
		ErrorCode second{};
	};

	template<typename T> struct JsonifierResult : public JsonifierResultBase<T> {
		inline void tie(T& value, ErrorCode& error) && noexcept;
		inline ErrorCode get(T& value) && noexcept;
		
		inline ErrorCode error() const noexcept;
		
		inline T& value() & noexcept(false);
		
		inline T&& value() && noexcept(false);
		
		inline T&& takeValue() && noexcept(false);
		
		inline operator T&&() && noexcept(false);
		
		inline const T& valueUnsafe() const& noexcept;
		
		inline T&& valueUnsafe() && noexcept;
		
		inline JsonifierResult(T&& value, ErrorCode error) noexcept;

		inline JsonifierResult(ErrorCode error) noexcept;

		inline JsonifierResult(T&& value) noexcept;

		inline JsonifierResult() noexcept;
	};

	inline uint32_t isStructuralOrWhitespace(uint8_t c);

	inline uint32_t isNotStructuralOrWhitespace(uint8_t c);

	template<typename TTy> class StopWatch {
	  public:
		using HRClock = std::chrono::high_resolution_clock;

		inline StopWatch() = delete;

		inline StopWatch<TTy>& operator=(const StopWatch<TTy>& data) {
			this->maxNumberOfMs.store(data.maxNumberOfMs.load());
			this->startTime.store(data.startTime.load());
			return *this;
		}

		inline StopWatch(const StopWatch<TTy>& data) {
			*this = data;
		}

		inline StopWatch(TTy maxNumberOfMsNew) {
			this->maxNumberOfMs.store(maxNumberOfMsNew);
			this->startTime.store(std::chrono::duration_cast<TTy>(HRClock::now().time_since_epoch()));
		}

		inline TTy totalTimePassed() {
			TTy currentTime = std::chrono::duration_cast<TTy>(HRClock::now().time_since_epoch());
			TTy elapsedTime = currentTime - this->startTime.load();
			return elapsedTime;
		}

		inline TTy getTotalWaitTime() {
			return this->maxNumberOfMs.load();
		}

		inline bool hasTimePassed() {
			TTy currentTime = std::chrono::duration_cast<TTy>(HRClock::now().time_since_epoch());
			TTy elapsedTime = currentTime - this->startTime.load();
			if (elapsedTime >= this->maxNumberOfMs.load()) {
				return true;
			} else {
				return false;
			}
		}

		inline void resetTimer() {
			this->startTime.store(std::chrono::duration_cast<TTy>(HRClock::now().time_since_epoch()));
		}

	  protected:
		std::atomic<TTy> maxNumberOfMs{ TTy{ 0 } };
		std::atomic<TTy> startTime{ TTy{ 0 } };
	};

}