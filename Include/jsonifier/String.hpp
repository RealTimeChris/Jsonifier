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
	License along with this library; if not, write to the Free Software
	Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301
	USA
*/
/// https://github.com/RealTimeChris/Jsonifier
/// Feb 20, 2023
#pragma once

#ifdef _WIN32
	#define _aligned_alloc(y, x) _aligned_malloc(x, y)
	#define _realloc(x, y, z) _aligned_realloc(x, y, z)
	#define _aligned_free(x) _aligned_free(x)
#else
	#define _aligned_alloc(y, x) std::aligned_alloc(x, y)
	#define _realloc(x, y, z) std::realloc(x, y)
	#define _aligned_free(x) std::free(x)
#endif

#include <jsonifier/Base.hpp>
#include <jsonifier/IsaDetection.hpp>

namespace Jsonifier {

	template<typename OTy> class StringAllocator {
	  public:
		using value_type = OTy;
		using pointer = value_type*;
		using const_pointer = const value_type*;
		using reference = value_type&;
		using const_reference = const value_type&;
		using size_type = size_t;
		using difference_type = std::ptrdiff_t;

	  public:
		template<typename U> struct rebind {
			using other = StringAllocator<U>;
		};

	  public:
		inline constexpr explicit StringAllocator() {
		}

		inline constexpr ~StringAllocator() {
		}

		inline constexpr explicit StringAllocator(StringAllocator const&) {
		}
		template<typename U> inline constexpr explicit StringAllocator(StringAllocator<U> const&) {
		}

		inline constexpr pointer address(reference r) {
			return &r;
		}

		inline constexpr const_pointer address(const_reference r) {
			return &r;
		}

		inline constexpr pointer allocate(size_type cnt) {
			return static_cast<OTy*>(_aligned_alloc(cnt * sizeof(OTy), 32));
		}

		inline constexpr void deallocate(pointer p, size_type cnt) {
			_aligned_free(p);
		}

		inline constexpr size_type max_size() const {
			return std::numeric_limits<size_type>::max() / sizeof(OTy);
		}

		inline constexpr void construct(pointer p, const OTy& t) {
			new (p) OTy(t);
		}

		inline constexpr void destroy(pointer p) {
			p->~OTy();
		}
	};

	class String : public std::basic_string<char, std::char_traits<char>, StringAllocator<char>> {
	  public:
		inline constexpr String() noexcept = default;
		inline constexpr String(const std::string& string) : std::basic_string<char, std::char_traits<char>, StringAllocator<char>>{ string } {};
		inline constexpr String(std::string_view string) : std::basic_string<char, std::char_traits<char>, StringAllocator<char>>{ string } {};
		template<size_t N> constexpr String(const char (&string)[N])
			: std::basic_string<char, std::char_traits<char>, StringAllocator<char>>{ string } {};
		inline operator std::string_view() const {
			return { this->data(), this->size() };
		}

		inline friend bool operator==(const String& lhs, const String& rhs) {
			if (lhs.size() != rhs.size()) {
				return false;
			} else if (lhs.size() == 0) {
				return true;
			} else {
				return Jsonifier::JsonifierCoreInternal::compare(lhs.data(), rhs.data(), lhs.size());
			}
		}

		inline friend bool operator!=(const String& lhs, const String& rhs) {
			return !(lhs == rhs);
		}
		inline constexpr ~String() noexcept = default;
	};

	inline std::ostream& operator<<(std::ostream& os, const String& string) {
		os << string.operator std::basic_string_view<char, std::char_traits<char>>();
		return os;
	}

}
