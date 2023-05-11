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
	License along with this library; if not, Serialize to the Free Software
	Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301
	USA
*/
/// https://github.com/RealTimeChris/Jsonifier
/// Feb 20, 2023
#pragma once

#include <iterator>

namespace JsonifierInternal {

	template<typename OTy, size_t N> class RawVector {
	  public:
		using value_type = OTy;
		using reference = value_type&;
		using const_reference = const value_type&;
		using pointer = value_type*;
		using const_pointer = const value_type*;
		using iterator = pointer;
		using const_iterator = const_pointer;
		using size_type = size_t;
		using difference_type = std::ptrdiff_t;

		inline constexpr RawVector() noexcept = default;
		inline constexpr RawVector& operator=(RawVector&&) noexcept = default;
		inline constexpr RawVector(RawVector&&) noexcept = default;
		inline constexpr RawVector& operator=(const RawVector&) noexcept = default;
		inline constexpr RawVector(const RawVector&) noexcept = default;

		inline constexpr RawVector(size_type count, const auto& value) : dsize(count) {
			for (size_t x = 0; x < N; ++x)
				data[x] = value;
		}

		inline constexpr iterator begin() noexcept {
			return data;
		}

		inline constexpr iterator end() noexcept {
			return data + dsize;
		}

		inline constexpr size_type size() const noexcept {
			return dsize;
		}

		inline constexpr reference operator[](size_t index) {
			return data[index];
		}

		inline constexpr const_reference operator[](size_t index) const noexcept {
			return data[index];
		}

		inline constexpr reference back() {
			return data[dsize - 1];
		}

		inline constexpr const_reference back() const noexcept {
			return data[dsize - 1];
		}

		inline constexpr void push_back(const OTy& a) {
			data[dsize++] = a;
		}

		inline constexpr void push_back(OTy&& a) {
			data[dsize++] = std::move(a);
		}

		inline constexpr void pop_back() {
			--dsize;
		}

		inline constexpr void clear() {
			dsize = 0;
		}

	  protected:
		OTy data[N] = {};
		size_t dsize = 0;
	};

}
