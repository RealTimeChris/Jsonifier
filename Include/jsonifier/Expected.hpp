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
		jsonifier_constexpr unexpected(){};

		jsonifier_constexpr unexpected& operator=(unexpected_type&& other) {
			std::uninitialized_move(&other, (&other) + 1, &unex);
			return *this;
		}

		jsonifier_constexpr unexpected(unexpected_type&& other) {
			*this = std::move(other);
		}

		jsonifier_constexpr unexpected& operator=(const unexpected_type& other) {
			unex = other;
			return *this;
		}

		jsonifier_constexpr unexpected(const unexpected_type& other) {
			*this = other;
		}

		jsonifier_constexpr const unexpected_type&& error() const&& {
			return std::move(unex);
		}

		jsonifier_constexpr unexpected_type&& error() && {
			return std::move(unex);
		}

		jsonifier_constexpr const unexpected_type& error() const& {
			return unex;
		}

		jsonifier_constexpr unexpected_type& error() & {
			return unex;
		}

		jsonifier_constexpr void swap(unexpected& other) {
			std::swap(unex, other.unex);
		}

		friend jsonifier_constexpr bool operator==(const unexpected& lhs, const unexpected<unexpected_type>& rhs) {
			return lhs.unex == rhs.unex;
		}

		jsonifier_constexpr ~unexpected(){};

	  protected:
		unexpected_type unex{};
	};

	template<typename unexpected_type> unexpected(unexpected_type) -> unexpected<unexpected_type>;

	template<typename expected_type, typename unexpected_type_new> class expected {
	  public:
		using value_type				  = expected_type;
		using pointer					  = value_type*;
		using error_type				  = unexpected_type_new;
		using unexpected_type			  = unexpected_type_new;
		template<typename U> using rebind = expected<U, error_type>;

		jsonifier_constexpr expected(){};

		jsonifier_constexpr expected& operator=(expected&& other) {
			hasValue = other.hasValue;
			if (hasValue) {
				val = std::move(other.val);
			} else {
				unex = std::move(other.unex);
			}
			return *this;
		}

		jsonifier_constexpr expected(expected&& other) : unex{} {
			*this = std::move(other);
		}

		jsonifier_constexpr expected& operator=(const expected& other) {
			hasValue = other.hasValue;
			if (hasValue) {
				val = other.val;
			} else {
				unex = other.unex;
			}
			return *this;
		}

		jsonifier_constexpr expected(const expected& other) : unex{} {
			*this = other;
		}

		template<typename G> jsonifier_constexpr expected& operator=(unexpected<G>&& other) {
			unex	 = std::move(other.error());
			hasValue = false;
			return *this;
		}

		template<typename G> jsonifier_constexpr expected(unexpected<G>&& other) {
			*this = std::move(other);
		}

		template<typename G> jsonifier_constexpr expected& operator=(const unexpected<G>& other) {
			hasValue = false;
			return *this;
		}

		template<typename G> jsonifier_constexpr expected(const unexpected<G>& other) {
			*this = other;
		}

		jsonifier_constexpr expected& operator=(error_type&& other) {
			unex	 = std::move(other);
			hasValue = false;
			return *this;
		}

		jsonifier_constexpr expected(error_type&& other) {
			*this = std::move(other);
		}

		jsonifier_constexpr expected& operator=(const error_type& other) {
			unex	 = other;
			hasValue = false;
			return *this;
		}

		jsonifier_constexpr expected(const error_type& other) {
			*this = other;
		}

		jsonifier_constexpr expected& operator=(value_type&& other) {
			val		 = std::move(other);
			hasValue = true;
			return *this;
		}

		jsonifier_constexpr expected(value_type&& v) {
			*this = std::move(v);
		}

		jsonifier_constexpr expected& operator=(const value_type& other) {
			val		 = other;
			hasValue = true;
			return *this;
		}

		jsonifier_constexpr expected(const value_type& v) {
			*this = v;
		}

		jsonifier_constexpr void swap(expected& other) {
			if (hasValue) {
				std::swap(val, other.val);
			} else {
				std::swap(unex, other.unex);
			}
			std::swap(hasValue, other.hasValue);
		}

		jsonifier_constexpr pointer operator->() const {
			return &val;
		}

		jsonifier_constexpr pointer operator->() {
			return &val;
		}

		jsonifier_constexpr const value_type&& operator*() const&& {
			return std::move(val);
		}

		jsonifier_constexpr value_type&& operator*() && {
			return std::move(val);
		}

		jsonifier_constexpr const value_type& operator*() const& {
			return val;
		}

		jsonifier_constexpr value_type& operator*() & {
			return val;
		}

		jsonifier_constexpr explicit operator bool() const {
			return hasValue;
		}

		jsonifier_constexpr bool has_value() const {
			return hasValue;
		}

		jsonifier_constexpr const value_type&& value() const&& {
			return std::move(val);
		}

		jsonifier_constexpr value_type&& value() && {
			return std::move(val);
		}

		jsonifier_constexpr const value_type& value() const& {
			return val;
		}

		jsonifier_constexpr value_type& value() & {
			return val;
		}

		jsonifier_constexpr const unexpected_type&& error() const&& {
			return std::move(unex);
		}

		jsonifier_constexpr unexpected_type&& error() && {
			return std::move(unex);
		}

		jsonifier_constexpr const unexpected_type& error() const& {
			return unex;
		}

		jsonifier_constexpr unexpected_type& error() & {
			return unex;
		}

		friend jsonifier_constexpr bool operator==(const expected& x, const expected<value_type, unexpected_type>& y) {
			return x.hasValue == y.hasValue && x.unex == y.unex && x.val == y.val;
		}

		jsonifier_constexpr ~expected(){};

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
		using unexpected_type = unexpected_type_new;

		template<typename U> using rebind = expected<U, error_type>;

		jsonifier_constexpr expected(){};

		template<typename G> jsonifier_constexpr expected& operator=(unexpected<G>&& other) {
			return *this;
		}

		template<typename G> jsonifier_constexpr expected(unexpected<G>&& other) {
			*this = std::move(other);
		}

		template<typename G> jsonifier_constexpr expected& operator=(const unexpected<G>& other) {
			return *this;
		}

		template<typename G> jsonifier_constexpr expected(const unexpected<G>& other) {
			*this = other;
		}

		jsonifier_constexpr void swap(expected& other) {
			std::swap(unex, other.unex);
		}

		jsonifier_constexpr explicit operator bool() const {
			return false;
		}

		jsonifier_constexpr bool has_value() const {
			return false;
		}

		jsonifier_constexpr const unexpected_type&& error() const&& {
			return std::move(unex);
		}

		jsonifier_constexpr unexpected_type&& error() && {
			return std::move(unex);
		}

		jsonifier_constexpr const unexpected_type& error() const& {
			return unex;
		}

		jsonifier_constexpr unexpected_type& error() & {
			return unex;
		}

		template<typename T2, typename E2> friend jsonifier_constexpr bool operator==(const expected& x, const expected<T2, E2>& y) {
			return x.unex == y.unex;
		}

		jsonifier_constexpr ~expected(){};

	  protected:
		union {
			unexpected_type unex{};
		};
	};

}
