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
/// https://github.com/RealTimeChris/Jsonifier
/// Feb 20, 2023
#pragma once

#include <utility>

namespace JsonifierInternal {

	template<typename ETy> class Unexpected;

	struct UnexpectT {
		explicit constexpr UnexpectT() noexcept = default;
	};

	template<typename ValueType, typename ETy> class Expected;

	template<typename ValueType, typename ETy>
		requires std::is_void_v<ValueType>
	class Expected<ValueType, ETy>;

	template<typename ETy> class Unexpected {
	  public:
		constexpr Unexpected& operator=(Unexpected&&) noexcept	   = default;
		constexpr Unexpected(Unexpected&& other) noexcept		   = default;
		constexpr Unexpected& operator=(const Unexpected&) noexcept = default;
		constexpr Unexpected(const Unexpected& other) noexcept	   = default;

		constexpr Unexpected& operator=(ETy&& other) {
			unex = std::move(other);
			return *this;
		}

		constexpr Unexpected(ETy&& other) {
			*this = std::move(other);
		}

		constexpr Unexpected& operator=(const ETy& other) {
			unex = other;
			return *this;
		}

		constexpr Unexpected(const ETy& other) {
			*this = other;
		}

		constexpr const ETy&& error() const&& noexcept {
			return std::move(unex);
		}

		constexpr ETy&& error() && noexcept {
			return std::move(unex);
		}

		constexpr const ETy& error() const& noexcept {
			return unex;
		}

		constexpr ETy& error() & noexcept {
			return unex;
		}

		constexpr void swap(Unexpected& other) noexcept {
			std::swap(unex, other.unex);
		}

		friend constexpr bool operator==(const Unexpected& lhs, const Unexpected<ETy>& rhs) {
			return lhs.unex == rhs.unex;
		}

	  protected:
		ETy unex{};
	};

	template<typename ETy> Unexpected(ETy) -> Unexpected<ETy>;

	template<typename ValueType, typename ETy> class Expected {
	  public:
		using value_type				  = ValueType;
		using error_type				  = ETy;
		using unexpected_type			  = Unexpected<ETy>;
		template<typename U> using rebind = Expected<U, error_type>;

		constexpr Expected() noexcept = default;

		constexpr Expected& operator=(Expected&&) noexcept	   = default;
		constexpr explicit Expected(Expected&&) noexcept		   = default;
		constexpr Expected& operator=(const Expected&) noexcept = default;
		constexpr Expected(const Expected&) noexcept			   = default;

		template<typename G> constexpr Expected& operator=(Unexpected<G>&& other) {
			hasValue = false;
			return *this;
		}

		template<typename G> constexpr Expected(Unexpected<G>&& other) {
			*this = std::move(other);
		}

		template<typename G> constexpr Expected& operator=(const Unexpected<G>& other) {
			hasValue = false;
			return *this;
		}

		template<typename G> constexpr Expected(const Unexpected<G>& other) {
			*this = other;
		}

		constexpr Expected& operator=(ValueType&& other) {
			val		 = std::move(other);
			hasValue = true;
			return *this;
		}

		constexpr Expected(ValueType&& v) {
			*this = std::move(v);
		}

		constexpr void swap(Expected& other) noexcept {
			if (hasValue) {
				std::swap(val, other.val);
			} else {
				std::swap(unex, other.unex);
			}
			std::swap(hasValue, other.hasValue);
		}

		constexpr const ValueType* operator->() const noexcept {
			return &val;
		}

		constexpr ValueType* operator->() noexcept {
			return &val;
		}

		constexpr const ValueType&& operator*() const&& noexcept {
			return std::move(val);
		}

		constexpr ValueType&& operator*() && noexcept {
			return std::move(val);
		}

		constexpr const ValueType& operator*() const& noexcept {
			return val;
		}

		constexpr ValueType& operator*() & noexcept {
			return val;
		}

		constexpr explicit operator bool() const noexcept {
			return hasValue;
		}

		constexpr bool has_value() const noexcept {
			return hasValue;
		}

		constexpr const ValueType&& value() const&& {
			return std::move(val);
		}

		constexpr ValueType&& value() && {
			return std::move(val);
		}

		constexpr const ValueType& value() const& {
			return val;
		}

		constexpr ValueType& value() & {
			return val;
		}

		constexpr const ETy&& error() const&& {
			return std::move(unex);
		}

		constexpr ETy&& error() && {
			return std::move(unex);
		}

		constexpr const ETy& error() const& {
			return unex;
		}

		constexpr ETy& error() & {
			return unex;
		}

		friend constexpr bool operator==(const Expected& x, const Expected<ValueType, ETy>& y) {
			return x.hasValue == y.hasValue && x.unex == y.unex && x.val == y.val;
		}

		constexpr ~Expected() noexcept {};

	  protected:
		bool hasValue{};
		union {
			ValueType val{};
			ETy unex;
		};
	};

	template<typename ValueType, typename ETy>
		requires std::is_void_v<ValueType>
	class Expected<ValueType, ETy> {
	  public:
		using value_type	  = ValueType;
		using error_type	  = ETy;
		using unexpected_type = Unexpected<ETy>;

		template<typename U> using rebind = Expected<U, error_type>;

		constexpr Expected() noexcept = default;

		constexpr Expected& operator=(Expected&&) noexcept	   = default;
		constexpr explicit Expected(Expected&&) noexcept		   = default;
		constexpr Expected& operator=(const Expected&) noexcept = default;
		constexpr explicit Expected(const Expected&) noexcept   = default;

		template<typename G> constexpr Expected& operator=(Unexpected<G>&& other) {
			return *this;
		}

		template<typename G> constexpr Expected(Unexpected<G>&& other) {
			*this = std::move(other);
		}

		template<typename G> constexpr Expected& operator=(const Unexpected<G>& other) {
			return *this;
		}

		template<typename G> constexpr Expected(const Unexpected<G>& other) {
			*this = other;
		}

		constexpr void swap(Expected& other) noexcept {
			std::swap(unex, other.unex);
		}

		constexpr explicit operator bool() const noexcept {
			return false;
		}

		constexpr bool has_value() const noexcept {
			return false;
		}

		constexpr const ETy&& error() const&& {
			return std::move(unex);
		}

		constexpr ETy&& error() && {
			return std::move(unex);
		}

		constexpr const ETy& error() const& {
			return unex;
		}

		constexpr ETy& error() & {
			return unex;
		}

		template<typename T2, typename E2> friend constexpr bool operator==(const Expected& x, const Expected<T2, E2>& y) {
			return x.unex == y.unex;
		}

		constexpr ~Expected() noexcept {};

	  protected:
		union {
			ETy unex{};
		};
	};

}
