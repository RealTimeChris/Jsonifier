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

	template<typename OTy, size_t N> struct RawArray {
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

		inline constexpr RawArray() noexcept = default;
		inline constexpr RawArray& operator=(RawArray&&) noexcept = default;
		inline constexpr RawArray(RawArray&&) noexcept = default;
		inline constexpr RawArray& operator=(const RawArray&) noexcept = default;
		inline constexpr RawArray(const RawArray&) noexcept = default;

		template<size_t M> inline constexpr RawArray(OTy const (&init)[M]) : RawArray(init, std::make_index_sequence<N>()) {
			static_assert(M >= N);
		}

		inline constexpr RawArray(const std::initializer_list<OTy>& other) {
			for (size_t x = 0; x < other.size(); ++x) {
				operator[](x) = std::move(other.begin()[x]);
			}
		}

		inline constexpr iterator begin() noexcept {
			return dataVal;
		}

		inline constexpr const_iterator begin() const noexcept {
			return dataVal;
		}

		inline constexpr iterator end() noexcept {
			return dataVal + N;
		}

		inline constexpr const_iterator end() const noexcept {
			return dataVal + N;
		}

		inline constexpr size_type size() const noexcept {
			return N;
		}

		inline constexpr size_type maxSize() const noexcept {
			return N;
		}

		inline constexpr reference operator[](size_t index) {
			return dataVal[index];
		}

		inline constexpr const_reference operator[](size_t index) const noexcept {
			return dataVal[index];
		}

		inline constexpr reference at(size_t index) noexcept {
			if (index > N) {
				std::abort();
			}
			return dataVal[index];
		}

		inline constexpr const_reference at(size_t index) const noexcept {
			if (index > N) {
				std::abort();
			}
			return dataVal[index];
		}

		inline constexpr reference front() {
			return dataVal[0];
		}

		inline constexpr const_reference front() const noexcept {
			return dataVal[0];
		}

		inline constexpr reference back() {
			return dataVal[N - 1];
		}

		inline constexpr const_reference back() const noexcept {
			return dataVal[N - 1];
		}

		inline constexpr value_type* data() noexcept {
			return dataVal;
		}

		inline constexpr const value_type* data() const noexcept {
			return dataVal;
		}

		inline constexpr void fill(const value_type& val) {
			for (size_t x = 0; x < N; ++x) {
				dataVal[x] = val;
			}
		}

		OTy dataVal[N] = {};

		template<size_t M, size_t... I> inline constexpr RawArray(OTy const (&init)[M], std::index_sequence<I...>) : dataVal{ init[I]... } {};
	};

	template<typename OTy> class RawArray<OTy, 0> {
	  public:
		using value_type = OTy;
		using reference = value_type&;
		using const_reference = const value_type&;
		using pointer = value_type*;
		using const_pointer = const value_type*;
		using iterator = pointer;
		using const_iterator = const_pointer;
		using reverse_iterator = std::reverse_iterator<iterator>;
		using const_reverse_iterator = std::reverse_iterator<const_iterator>;
		using size_type = size_t;
		using difference_type = std::ptrdiff_t;

		inline constexpr RawArray() noexcept = default;
	};

}
