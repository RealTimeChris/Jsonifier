/*
	MIT License

	Copyright (c) 2023 RealTimeChris

	Permission is hereby granted, free of charge, to any person obtaining a copy of this
	software and associated documentation files (the "Software"), to deal in the Software
	without restriction, including without limitation the rights to use, copy, modify, merge,
	publish, distribute, sublicense, and/or sell copies of the Software, and to permit
	persons to whom the Software is furnished to do so, subject to the following conditions:

	The above copyright notice and this permission notice shall be included in all copies or
	substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
	INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
	PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE
	FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
	OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
	DEALINGS IN THE SOFTWARE.
*/
/// https://github.com/RealTimeChris/jsonifier
/// Feb 3, 2023
#pragma once

#include <jsonifier/TypeEntities.hpp>
#include <jsonifier/StringView.hpp>
#include <jsonifier/Tuple.hpp>
#include <functional>
#include <optional>
#include <atomic>

namespace jsonifier_internal {

	template<typename = void, size_t... indices> constexpr auto indexer(std::index_sequence<indices...>&&) {
		return [](auto f) -> decltype(auto) {
			return decltype(f)(f)(std::integral_constant<size_t, indices>{}...);
		};
	}

	template<size_t n, typename funtion_type> constexpr auto forEach(funtion_type&& f) {
		return indexer(std::make_index_sequence<n>{})([&](auto... x) {
			(std::forward<jsonifier::concepts::unwrap<funtion_type>>(f)(x), ...);
		});
	}

	template<typename value_type> constexpr decltype(auto) convSv(value_type&& value) noexcept {
		if constexpr (std::is_convertible_v<value_type, jsonifier::string_view>) {
			return jsonifier::string_view{ value };
		} else {
			return std::forward<value_type>(value);
		}
	}

	JSONIFIER_INLINE decltype(auto) getMember(auto&& value, auto& member_ptr) {
		using value_type = jsonifier::concepts::unwrap<decltype(member_ptr)>;
		if constexpr (std::is_member_object_pointer_v<value_type>) {
			return value.*member_ptr;
		} else if constexpr (std::is_pointer_v<value_type>) {
			return *member_ptr;
		} else {
			return member_ptr;
		}
	}

	template<typename value_type, typename mptr_t> using member_t = decltype(getMember(std::declval<value_type>(), std::declval<jsonifier::concepts::unwrap<mptr_t>&>()));

	template<jsonifier::concepts::time_type value_type> class stop_watch {
	  public:
		using hr_clock = std::chrono::high_resolution_clock;

		JSONIFIER_INLINE stop_watch(uint64_t newTime) {
			totalNumberOfTimeUnits.store(value_type{ newTime }, std::memory_order_release);
		}

		JSONIFIER_INLINE stop_watch(value_type newTime) {
			totalNumberOfTimeUnits.store(newTime, std::memory_order_release);
		}

		JSONIFIER_INLINE stop_watch& operator=(stop_watch&& other) {
			totalNumberOfTimeUnits.store(other.totalNumberOfTimeUnits.load(std::memory_order_acquire), std::memory_order_release);
			startTimeInTimeUnits.store(other.startTimeInTimeUnits.load(std::memory_order_acquire), std::memory_order_release);
			return *this;
		}

		JSONIFIER_INLINE stop_watch(stop_watch&& other) {
			*this = std::move(other);
		}

		JSONIFIER_INLINE stop_watch& operator=(const stop_watch& other) {
			totalNumberOfTimeUnits.store(other.totalNumberOfTimeUnits.load(std::memory_order_acquire), std::memory_order_release);
			startTimeInTimeUnits.store(other.startTimeInTimeUnits.load(std::memory_order_acquire), std::memory_order_release);
			return *this;
		}

		JSONIFIER_INLINE stop_watch(const stop_watch& other) {
			*this = other;
		}

		JSONIFIER_INLINE bool hasTimeElapsed() {
			if (std::chrono::duration_cast<value_type>(hr_clock::now().time_since_epoch()) - startTimeInTimeUnits.load(std::memory_order_acquire) >=
				totalNumberOfTimeUnits.load(std::memory_order_acquire)) {
				return true;
			} else {
				return false;
			}
		}

		JSONIFIER_INLINE void reset(value_type newTimeValue = value_type{}) {
			if (newTimeValue != value_type{}) {
				totalNumberOfTimeUnits.store(newTimeValue, std::memory_order_release);
				startTimeInTimeUnits.store(std::chrono::duration_cast<value_type>(hr_clock::now().time_since_epoch()), std::memory_order_release);
			} else {
				startTimeInTimeUnits.store(std::chrono::duration_cast<value_type>(hr_clock::now().time_since_epoch()), std::memory_order_release);
			}
		}

		JSONIFIER_INLINE value_type getTotalWaitTime() const {
			return totalNumberOfTimeUnits.load(std::memory_order_acquire);
		}

		JSONIFIER_INLINE value_type totalTimeElapsed() {
			return std::chrono::duration_cast<value_type>(hr_clock::now().time_since_epoch()) - startTimeInTimeUnits.load(std::memory_order_acquire);
		}

	  protected:
		std::atomic<value_type> totalNumberOfTimeUnits{};
		std::atomic<value_type> startTimeInTimeUnits{};
	};

	template<jsonifier::concepts::time_type value_type> stop_watch(value_type) -> stop_watch<value_type>;
}// namespace jsonifier_internal

namespace jsonifier {

	template<typename... value_types> constexpr auto createArray(value_types&&... args) {
		constexpr auto newSize = sizeof...(value_types);
		if constexpr (newSize > 0) {
			return jsonifier_internal::array{ std::make_tuple(std::forward<value_types>(args)...) };
		} else {
			return jsonifier_internal::array{ jsonifier::concepts::empty_val{} };
		}
	}

	template<typename... value_types> constexpr auto createObject(value_types&&... args) {
		constexpr auto newSize = sizeof...(value_types);
		if constexpr (newSize > 0) {
			auto newTuple	 = std::make_tuple(jsonifier_internal::convSv(std::forward<value_types>(args))...);
			using tuple_type = jsonifier::concepts::unwrap<decltype(newTuple)>;
			return jsonifier_internal::object{ jsonifier_internal::group_builder<tuple_type>::op(std::move(newTuple)) };
		} else {
			return jsonifier_internal::object{ jsonifier::concepts::empty_val{} };
		}
	}

}// namespace jsonifier