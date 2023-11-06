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

#include <jsonifier/TypeEntities.hpp>
#include <utility>

namespace jsonifier_internal {

	template<typename unexpected_type_new> class unexpected {
	  public:
		using unexpected_type = unexpected_type_new;

		template<jsonifier::concepts::same_as<unexpected_type> unexpected_type_newer> constexpr unexpected& operator=(unexpected_type_newer&& other) {
			unex = std::forward<unexpected_type_newer>(other);
			return *this;
		}

		template<jsonifier::concepts::same_as<unexpected_type> unexpected_type_newer> constexpr unexpected(unexpected_type_newer&& other) {
			*this = std::forward<unexpected_type_newer>(other);
		}

		constexpr unexpected_type&& error() const&& {
			return std::move(unex);
		}

		constexpr unexpected_type& error() const& {
			return unex;
		}

	  protected:
		unexpected_type unex{};
	};

	template<typename expected_type, typename unexpected_type_new> class expected {
	  public:
		using value_type	  = expected_type;
		using pointer		  = value_type*;
		using unexpected_type = unexpected<unexpected_type_new>;

		constexpr expected& operator=(expected&& other) {
			std::swap(hasValue, other.hasValue);
			if (hasValue) {
				std::construct_at(&val, std::move(other.val));
			} else {
				std::construct_at(&unex, std::move(other.unex));
			}
			return *this;
		}

		constexpr expected(expected&& other) : unex{} {
			*this = std::move(other);
		}

		constexpr expected& operator=(const expected& other) {
			hasValue = other.hasValue;
			if (hasValue) {
				std::construct_at(&val, other.val);
			} else {
				std::construct_at(&unex, other.unex);
			}
			return *this;
		}

		constexpr expected(const expected& other) : unex{} {
			*this = other;
		}

		template<jsonifier::concepts::same_as<expected_type> expected_type_newer> constexpr expected& operator=(expected_type_newer&& other) {
			val		 = std::forward<expected_type_newer>(other);
			hasValue = true;
			return *this;
		}

		template<jsonifier::concepts::same_as<expected_type> expected_type_newer> constexpr expected(expected_type_newer&& other) {
			*this = std::forward<expected_type_newer>(other);
		}

		template<jsonifier::concepts::same_as<unexpected_type> unexpected_type_newer> constexpr expected& operator=(unexpected_type_newer&& other) {
			unex	 = std::forward<unexpected_type_newer>(other);
			hasValue = false;
			return *this;
		}

		template<jsonifier::concepts::same_as<unexpected_type> unexpected_type_newer> constexpr expected(unexpected_type_newer&& other) {
			*this = std::forward<unexpected_type_newer>(other);
		}

		constexpr void swap(expected& other) {
			std::swap(hasValue, other.hasValue);
			if (hasValue) {
				std::swap(val, other.val);
			} else {
				std::swap(unex, other.unex);
			}
		}

		constexpr pointer operator->() const {
			return &val;
		}

		constexpr value_type&& operator*() const&& {
			return std::move(val);
		}

		constexpr value_type& operator*() const& {
			return val;
		}

		constexpr explicit operator bool() const {
			return hasValue;
		}

		constexpr bool has_value() const {
			return hasValue;
		}

		constexpr value_type value() const&& {
			return std::move(val);
		}

		constexpr value_type value() const& {
			return val;
		}

		constexpr unexpected_type error() const&& {
			return std::move(unex);
		}

		constexpr unexpected_type error() const& {
			return unex;
		}

		friend constexpr bool operator==(const expected& x, const expected<value_type, unexpected_type>& y) {
			if (x.hasValue == y.hasValue) {
				if (x.hasValue) {
					return x.val == y.val;
				} else {
					return x.unex == y.unex;
				}
			}
			return false;
		}

		constexpr ~expected() {
			if (hasValue) {
				std::destroy_at(&val);
			} else {
				std::destroy_at(&unex);
			}
		};

	  protected:
		bool hasValue{};
		union {
			unexpected_type unex;
			value_type val;
		};
	};

}
