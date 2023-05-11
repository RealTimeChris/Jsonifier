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
        License along with this library; if not, Write to the Free Software
        Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301
        USA
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

	template<typename OTy, typename ETy> class Expected;

	template<typename OTy, typename ETy>
		requires std::is_void_v<OTy>
	class Expected<OTy, ETy>;

	template<typename ETy> class Unexpected {
	  public:
		inline constexpr Unexpected& operator=(Unexpected&&) noexcept = default;
		inline constexpr Unexpected(Unexpected&& other) noexcept = default;
		inline constexpr Unexpected& operator=(const Unexpected&) noexcept = default;
		inline constexpr Unexpected(const Unexpected& other) noexcept = default;

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
			swapF(unex, other.unex);
		}

		friend inline constexpr bool operator==(const Unexpected& lhs, const Unexpected<ETy>& rhs) {
			return lhs.unex == rhs.unex;
		}

	  protected:
		ETy unex{};
	};

	template<typename ETy> Unexpected(ETy) -> Unexpected<ETy>;

	template<typename OTy, typename ETy> class Expected {
	  public:
		using value_type = OTy;
		using error_type = ETy;
		using unexpected_type = Unexpected<ETy>;
		template<typename U> using rebind = Expected<U, error_type>;

		inline constexpr Expected() noexcept = default;

		inline constexpr Expected& operator=(Expected&&) noexcept = default;
		inline constexpr explicit Expected(Expected&&) noexcept = default;
		inline constexpr Expected& operator=(const Expected&) noexcept = default;
		inline constexpr Expected(const Expected&) noexcept = default;

		template<typename G> inline constexpr Expected& operator=(Unexpected<G>&& other) {
			hasValue = false;
			return *this;
		};

		template<typename G> inline constexpr Expected(Unexpected<G>&& other) {
			*this = std::move(other);
		};

		template<typename G> inline constexpr Expected& operator=(const Unexpected<G>& other) {
			hasValue = false;
			return *this;
		};

		template<typename G> inline constexpr Expected(const Unexpected<G>& other) {
			*this = other;
		};

		inline constexpr Expected& operator=(OTy&& other) {
			val = std::move(other);
			hasValue = true;
			return *this;
		};

		inline constexpr Expected(OTy&& v) {
			*this = std::move(v);
		}

		inline constexpr void swap(Expected& other) noexcept {
			if (hasValue) {
				swapF(val, other.val);
			} else {
				swapF(unex, other.unex);
			}
			swapF(hasValue, other.hasValue);
		}

		inline constexpr const OTy* operator->() const noexcept {
			return &val;
		}

		inline constexpr OTy* operator->() noexcept {
			return &val;
		}

		inline constexpr const OTy&& operator*() const&& noexcept {
			return std::move(val);
		}

		inline constexpr OTy&& operator*() && noexcept {
			return std::move(val);
		}

		inline constexpr const OTy& operator*() const& noexcept {
			return val;
		}

		inline constexpr OTy& operator*() & noexcept {
			return val;
		}

		inline constexpr explicit operator bool() const noexcept {
			return hasValue;
		}

		inline constexpr bool has_value() const noexcept {
			return hasValue;
		}

		inline constexpr const OTy&& value() const&& {
			return std::move(val);
		}

		inline constexpr OTy&& value() && {
			return std::move(val);
		}

		inline constexpr const OTy& value() const& {
			return val;
		}

		inline constexpr OTy& value() & {
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

		friend inline constexpr bool operator==(const Expected& x, const Expected<OTy, ETy>& y) {
			return x.hasValue == y.hasValue && x.unex == y.unex && x.val == y.val;
		}

		inline constexpr ~Expected() noexcept {};

	  protected:
		bool hasValue{};
		union {
			OTy val{};
			ETy unex;
		};
	};

	template<typename OTy, typename ETy>
		requires std::is_void_v<OTy>
	class Expected<OTy, ETy> {
	  public:
		using value_type = OTy;
		using error_type = ETy;
		using unexpected_type = Unexpected<ETy>;

		template<typename U> using rebind = Expected<U, error_type>;

		inline constexpr Expected() noexcept = default;

		inline constexpr Expected& operator=(Expected&&) noexcept = default;
		inline constexpr explicit Expected(Expected&&) noexcept = default;
		inline constexpr Expected& operator=(const Expected&) noexcept = default;
		inline constexpr explicit Expected(const Expected&) noexcept = default;

		template<typename G> inline constexpr Expected& operator=(Unexpected<G>&& other) {
			return *this;
		};

		template<typename G> inline constexpr Expected(Unexpected<G>&& other) {
			*this = std::move(other);
		};

		template<typename G> inline constexpr Expected& operator=(const Unexpected<G>& other) {
			return *this;
		};

		template<typename G> inline constexpr Expected(const Unexpected<G>& other) {
			*this = other;
		};

		inline constexpr void swap(Expected& other) noexcept {
			swapF(unex, other.unex);
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
