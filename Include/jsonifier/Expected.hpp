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
		explicit inline constexpr UnexpectT() noexcept = default;
	};

	template<typename ValueType, typename ETy> class Expected;

	template<typename ValueType, typename ETy>
		requires std::is_void_v<ValueType>
	class Expected<ValueType, ETy>;

	template<typename ETy> class Unexpected {
	  public:
		inline constexpr Unexpected& operator=(Unexpected&&) noexcept	   = default;
		inline constexpr Unexpected(Unexpected&& other) noexcept		   = default;
		inline constexpr Unexpected& operator=(const Unexpected&) noexcept = default;
		inline constexpr Unexpected(const Unexpected& other) noexcept	   = default;

		inline constexpr Unexpected& operator=(ETy&& other) {
			unex = std::move(other);
			return *this;
		}

		inline constexpr Unexpected(ETy&& other) {
			*this = std::move(other);
		}

		inline constexpr Unexpected& operator=(const ETy& other) {
			unex = other;
			return *this;
		}

		inline constexpr Unexpected(const ETy& other) {
			*this = other;
		}

		inline constexpr const ETy&& error() const&& noexcept {
			return std::move(unex);
		}

		inline constexpr ETy&& error() && noexcept {
			return std::move(unex);
		}

		inline constexpr const ETy& error() const& noexcept {
			return unex;
		}

		inline constexpr ETy& error() & noexcept {
			return unex;
		}

		inline constexpr void swap(Unexpected& other) noexcept {
			std::swap(unex, other.unex);
		}

		friend inline constexpr bool operator==(const Unexpected& lhs, const Unexpected<ETy>& rhs) {
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

		inline constexpr Expected() noexcept = default;

		inline constexpr Expected& operator=(Expected&&) noexcept	   = default;
		inline constexpr explicit Expected(Expected&&) noexcept		   = default;
		inline constexpr Expected& operator=(const Expected&) noexcept = default;
		inline constexpr Expected(const Expected&) noexcept			   = default;

		template<typename G> inline constexpr Expected& operator=(Unexpected<G>&& other) {
			hasValue = false;
			return *this;
		}

		template<typename G> inline constexpr Expected(Unexpected<G>&& other) {
			*this = std::move(other);
		}

		template<typename G> inline constexpr Expected& operator=(const Unexpected<G>& other) {
			hasValue = false;
			return *this;
		}

		template<typename G> inline constexpr Expected(const Unexpected<G>& other) {
			*this = other;
		}

		inline constexpr Expected& operator=(ValueType&& other) {
			val		 = std::move(other);
			hasValue = true;
			return *this;
		}

		inline constexpr Expected(ValueType&& v) {
			*this = std::move(v);
		}

		inline constexpr void swap(Expected& other) noexcept {
			if (hasValue) {
				std::swap(val, other.val);
			} else {
				std::swap(unex, other.unex);
			}
			std::swap(hasValue, other.hasValue);
		}

		inline constexpr const ValueType* operator->() const noexcept {
			return &val;
		}

		inline constexpr ValueType* operator->() noexcept {
			return &val;
		}

		inline constexpr const ValueType&& operator*() const&& noexcept {
			return std::move(val);
		}

		inline constexpr ValueType&& operator*() && noexcept {
			return std::move(val);
		}

		inline constexpr const ValueType& operator*() const& noexcept {
			return val;
		}

		inline constexpr ValueType& operator*() & noexcept {
			return val;
		}

		inline constexpr explicit operator bool() const noexcept {
			return hasValue;
		}

		inline constexpr bool has_value() const noexcept {
			return hasValue;
		}

		inline constexpr const ValueType&& value() const&& {
			return std::move(val);
		}

		inline constexpr ValueType&& value() && {
			return std::move(val);
		}

		inline constexpr const ValueType& value() const& {
			return val;
		}

		inline constexpr ValueType& value() & {
			return val;
		}

		inline constexpr const ETy&& error() const&& {
			return std::move(unex);
		}

		inline constexpr ETy&& error() && {
			return std::move(unex);
		}

		inline constexpr const ETy& error() const& {
			return unex;
		}

		inline constexpr ETy& error() & {
			return unex;
		}

		friend inline constexpr bool operator==(const Expected& x, const Expected<ValueType, ETy>& y) {
			return x.hasValue == y.hasValue && x.unex == y.unex && x.val == y.val;
		}

		inline constexpr ~Expected() noexcept {};

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

		inline constexpr Expected() noexcept = default;

		inline constexpr Expected& operator=(Expected&&) noexcept	   = default;
		inline constexpr explicit Expected(Expected&&) noexcept		   = default;
		inline constexpr Expected& operator=(const Expected&) noexcept = default;
		inline constexpr explicit Expected(const Expected&) noexcept   = default;

		template<typename G> inline constexpr Expected& operator=(Unexpected<G>&& other) {
			return *this;
		}

		template<typename G> inline constexpr Expected(Unexpected<G>&& other) {
			*this = std::move(other);
		}

		template<typename G> inline constexpr Expected& operator=(const Unexpected<G>& other) {
			return *this;
		}

		template<typename G> inline constexpr Expected(const Unexpected<G>& other) {
			*this = other;
		}

		inline constexpr void swap(Expected& other) noexcept {
			std::swap(unex, other.unex);
		}

		inline constexpr explicit operator bool() const noexcept {
			return false;
		}

		inline constexpr bool has_value() const noexcept {
			return false;
		}

		inline constexpr const ETy&& error() const&& {
			return std::move(unex);
		}

		inline constexpr ETy&& error() && {
			return std::move(unex);
		}

		inline constexpr const ETy& error() const& {
			return unex;
		}

		inline constexpr ETy& error() & {
			return unex;
		}

		template<typename T2, typename E2> friend inline constexpr bool operator==(const Expected& x, const Expected<T2, E2>& y) {
			return x.unex == y.unex;
		}

		inline constexpr ~Expected() noexcept {};

	  protected:
		union {
			ETy unex{};
		};
	};

}
