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
/// Feb 20, 2023
#pragma once

#include <utility>

namespace jsonifier_internal {

	template<typename ETy> class unexpected;

	template<typename value_type, typename ETy> class expected;

	template<typename value_type, typename ETy>
		requires std::is_void_v<value_type>
	class expected<value_type, ETy>;

	template<typename ETy> class unexpected {
	  public:
		constexpr unexpected& operator=(ETy&& other) {
			unex = std::move(other);
			return *this;
		}

		constexpr unexpected(ETy&& other) {
			*this = std::move(other);
		}

		constexpr unexpected& operator=(const ETy& other) {
			unex = other;
			return *this;
		}

		constexpr unexpected(const ETy& other) {
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

		constexpr void swap(unexpected& other) noexcept {
			std::swap(unex, other.unex);
		}

		friend constexpr bool operator==(const unexpected& lhs, const unexpected<ETy>& rhs) {
			return lhs.unex == rhs.unex;
		}

	  protected:
		ETy unex{};
	};

	template<typename ETy> unexpected(ETy) -> unexpected<ETy>;

	template<typename value_type_new, typename ETy> class expected {
	  public:
		using value_type				  = value_type_new;
		using error_type				  = ETy;
		using unexpected_type			  = unexpected<ETy>;
		template<typename U> using rebind = expected<U, error_type>;

		constexpr expected() noexcept = default;

		template<typename G> constexpr expected& operator=(unexpected<G>&& other) {
			hasValue = false;
			return *this;
		}

		template<typename G> constexpr expected(unexpected<G>&& other) {
			*this = std::move(other);
		}

		template<typename G> constexpr expected& operator=(const unexpected<G>& other) {
			hasValue = false;
			return *this;
		}

		template<typename G> constexpr expected(const unexpected<G>& other) {
			*this = other;
		}

		constexpr expected& operator=(value_type&& other) {
			val		 = std::move(other);
			hasValue = true;
			return *this;
		}

		constexpr expected(value_type&& v) {
			*this = std::move(v);
		}

		constexpr void swap(expected& other) noexcept {
			if (hasValue) {
				std::swap(val, other.val);
			} else {
				std::swap(unex, other.unex);
			}
			std::swap(hasValue, other.hasValue);
		}

		constexpr const value_type* operator->() const noexcept {
			return &val;
		}

		constexpr value_type* operator->() noexcept {
			return &val;
		}

		constexpr const value_type&& operator*() const&& noexcept {
			return std::move(val);
		}

		constexpr value_type&& operator*() && noexcept {
			return std::move(val);
		}

		constexpr const value_type& operator*() const& noexcept {
			return val;
		}

		constexpr value_type& operator*() & noexcept {
			return val;
		}

		constexpr explicit operator bool() const noexcept {
			return hasValue;
		}

		constexpr bool has_value() const noexcept {
			return hasValue;
		}

		constexpr const value_type&& value() const&& {
			return std::move(val);
		}

		constexpr value_type&& value() && {
			return std::move(val);
		}

		constexpr const value_type& value() const& {
			return val;
		}

		constexpr value_type& value() & {
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

		friend constexpr bool operator==(const expected& x, const expected<value_type, ETy>& y) {
			return x.hasValue == y.hasValue && x.unex == y.unex && x.val == y.val;
		}

		constexpr ~expected() noexcept {};

	  protected:
		bool hasValue{};
		union {
			value_type val{};
			ETy unex;
		};
	};

	template<typename value_type_new, typename ETy>
		requires std::is_void_v<value_type_new>
	class expected<value_type_new, ETy> {
	  public:
		using value_type	  = value_type_new;
		using error_type	  = ETy;
		using unexpected_type = unexpected<ETy>;

		template<typename U> using rebind = expected<U, error_type>;

		constexpr expected() noexcept = default;

		template<typename G> constexpr expected& operator=(unexpected<G>&& other) {
			return *this;
		}

		template<typename G> constexpr expected(unexpected<G>&& other) {
			*this = std::move(other);
		}

		template<typename G> constexpr expected& operator=(const unexpected<G>& other) {
			return *this;
		}

		template<typename G> constexpr expected(const unexpected<G>& other) {
			*this = other;
		}

		constexpr void swap(expected& other) noexcept {
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

		template<typename T2, typename E2> friend constexpr bool operator==(const expected& x, const expected<T2, E2>& y) {
			return x.unex == y.unex;
		}

		constexpr ~expected() noexcept {};

	  protected:
		union {
			ETy unex{};
		};
	};

}
