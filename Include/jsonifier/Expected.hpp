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

	template<typename unexpected_type> class unexpected {
	  public:
		constexpr unexpected(){};

		constexpr unexpected& operator=(unexpected_type&& other) {
			std::uninitialized_move(&other, (&other) + 1, &unex);
			return *this;
		}

		constexpr unexpected(unexpected_type&& other) {
			*this = std::move(other);
		}

		constexpr unexpected& operator=(const unexpected_type& other) {
			unex = other;
			return *this;
		}

		constexpr unexpected(const unexpected_type& other) {
			*this = other;
		}

		constexpr const unexpected_type&& error() const&& {
			return std::move(unex);
		}

		constexpr unexpected_type&& error() && {
			return std::move(unex);
		}

		constexpr const unexpected_type& error() const& {
			return unex;
		}

		constexpr unexpected_type& error() & {
			return unex;
		}

		constexpr void swap(unexpected& other) {
			std::swap(unex, other.unex);
		}

		friend constexpr bool operator==(const unexpected& lhs, const unexpected<unexpected_type>& rhs) {
			return lhs.unex == rhs.unex;
		}

		constexpr ~unexpected(){};

	  protected:
		unexpected_type unex{};
	};

	template<typename unexpected_type> unexpected(unexpected_type) -> unexpected<unexpected_type>;

	template<typename expected_type, typename unexpected_type_new> class expected {
	  public:
		using value_type				  = expected_type;
		using pointer					  = value_type*;
		using error_type				  = unexpected_type_new;
		using unexpected_type			  = unexpected<unexpected_type_new>;
		template<typename U> using rebind = expected<U, error_type>;

		constexpr expected(){};

		constexpr expected& operator=(expected&& other) {
			hasValue = other.hasValue;
			if (hasValue) {
				val = std::move(other.val);
			} else {
				unex = std::move(other.unex);
			}
			return *this;
		}

		constexpr expected(expected&& other) : unex{} {
			*this = std::move(other);
		}

		constexpr expected& operator=(const expected& other) {
			hasValue = other.hasValue;
			if (hasValue) {
				val = other.val;
			} else {
				unex = other.unex;
			}
			return *this;
		}

		constexpr expected(const expected& other) : unex{} {
			*this = other;
		}

		template<typename G> constexpr expected& operator=(unexpected<G>&& other) {
			unex	 = std::move(other.error());
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

		constexpr expected& operator=(error_type&& other) {
			unex	 = std::move(other);
			hasValue = false;
			return *this;
		}

		constexpr expected(error_type&& other) {
			*this = std::move(other);
		}

		constexpr expected& operator=(const error_type& other) {
			unex	 = other;
			hasValue = false;
			return *this;
		}

		constexpr expected(const error_type& other) {
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

		constexpr expected& operator=(const value_type& other) {
			val		 = other;
			hasValue = true;
			return *this;
		}

		constexpr expected(const value_type& v) {
			*this = v;
		}

		constexpr void swap(expected& other) {
			if (hasValue) {
				std::swap(val, other.val);
			} else {
				std::swap(unex, other.unex);
			}
			std::swap(hasValue, other.hasValue);
		}

		constexpr const pointer operator->() const {
			return &val;
		}

		constexpr pointer operator->() {
			return &val;
		}

		constexpr const value_type&& operator*() const&& {
			return std::move(val);
		}

		constexpr value_type&& operator*() && {
			return std::move(val);
		}

		constexpr const value_type& operator*() const& {
			return val;
		}

		constexpr value_type& operator*() & {
			return val;
		}

		constexpr explicit operator bool() const {
			return hasValue;
		}

		constexpr bool has_value() const {
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

		constexpr const unexpected_type&& error() const&& {
			return std::move(unex);
		}

		constexpr unexpected_type&& error() && {
			return std::move(unex);
		}

		constexpr const unexpected_type& error() const& {
			return unex;
		}

		constexpr unexpected_type& error() & {
			return unex;
		}

		friend constexpr bool operator==(const expected& x, const expected<value_type, unexpected_type>& y) {
			return x.hasValue == y.hasValue && x.unex == y.unex && x.val == y.val;
		}

		constexpr ~expected(){};

	  protected:
		bool hasValue{};
		union {
			value_type val{};
			unexpected_type unex;
		};
	};

	template<typename value_type_new, typename unexpected_type_new>
		requires std::is_void_v<value_type_new>
	class expected<value_type_new, unexpected_type_new> {
	  public:
		using value_type	  = value_type_new;
		using error_type	  = unexpected_type_new;
		using unexpected_type = unexpected<unexpected_type_new>;

		template<typename U> using rebind = expected<U, error_type>;

		constexpr expected(){};

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

		constexpr void swap(expected& other) {
			std::swap(unex, other.unex);
		}

		constexpr explicit operator bool() const {
			return false;
		}

		constexpr bool has_value() const {
			return false;
		}

		constexpr const unexpected_type&& error() const&& {
			return std::move(unex);
		}

		constexpr unexpected_type&& error() && {
			return std::move(unex);
		}

		constexpr const unexpected_type& error() const& {
			return unex;
		}

		constexpr unexpected_type& error() & {
			return unex;
		}

		template<typename T2, typename E2> friend constexpr bool operator==(const expected& x, const expected<T2, E2>& y) {
			return x.unex == y.unex;
		}

		constexpr ~expected(){};

	  protected:
		union {
			unexpected_type unex{};
		};
	};

}
