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

#include <jsonifier/Vector.hpp>

#include <algorithm>

namespace JsonifierInternal {

	using StringViewPtr = const uint8_t*;
	using StructuralIndex = uint32_t*;
	using StringBufferPtr = uint8_t*;

	template<typename OTy1, typename OTy2> inline constexpr bool stringConstCompare(const OTy1& S0, const OTy2& S1) noexcept {
		if (S0.size() != S1.size()) [[unlikely]] {
			return false;
		}
		for (size_t x = 0; x < S0.size(); ++x) {
			if (S0[x] != S1[x]) [[unlikely]] {
				return false;
			}
		}
		return true;
	}
}

namespace Jsonifier {

	class StringView;

	inline static constexpr size_t BufSize = 16 / sizeof(char) < 1 ? 1 : 16 / sizeof(char);

	inline static constexpr size_t AllocMask = sizeof(char) <= 1 ? 15 : sizeof(char) <= 2 ? 7 : sizeof(char) <= 4 ? 3 : sizeof(char) <= 8 ? 1 : 0;

	class String {
	  public:
		using traits_type = std::char_traits<char>;
		using value_type = char;
		using pointer = char*;
		using const_pointer = const char*;
		using reference = char&;
		using const_reference = const char&;
		using iterator = Iterator<char>;
		using const_iterator = const iterator;
		using reverse_iterator = std::reverse_iterator<iterator>;
		using const_reverse_iterator = const reverse_iterator;
		using size_type = size_t;
		using difference_type = ptrdiff_t;
		using allocator = std::pmr::polymorphic_allocator<char>;

		inline constexpr String() noexcept = default;

		inline constexpr static size_t npos{ std::numeric_limits<size_type>::max() };

		inline constexpr String& operator=(String&& other) noexcept {
			if (this != &other) {
				capacityVal = other.capacityVal;
				other.capacityVal = 0;
				sizeVal = other.sizeVal;
				other.sizeVal = 0;
				values = other.values;
				other.values = nullptr;
			}
			return *this;
		}

		inline explicit String(String&& other) noexcept {
			*this = std::move(other);
		}

		inline constexpr String& operator=(const String& other) noexcept {
			if (this != &other) {
				reserve(other.capacityVal);
				std::copy_n(other.data(), other.size(), this->values);
				this->sizeVal = other.sizeVal;
			}
			return *this;
		}

		inline String(const String& other) noexcept {
			*this = other;
		}

		inline constexpr String& operator=(value_type other) noexcept {
			if (other == '\0') {
				pushBack(' ');
			} else {
				pushBack(other);
			}
			return *this;
		}

		inline String(value_type other) noexcept {
			*this = other;
		}

		inline String(const std::string& other) noexcept {
			reserve(other.capacity());
			std::copy_n(other.data(), other.size(), this->values);
			this->sizeVal = other.size();
		}

		inline constexpr String& operator=(const std::string_view& other) noexcept {
			reserve(other.size());
			std::copy(other.data(), other.data() + other.size(), this->values);
			this->sizeVal = other.size();
			return *this;
		}

		inline String(const std::string_view& other) noexcept {
			*this = other;
		}

		inline constexpr String& operator=(const StringView& other);

		inline constexpr explicit String(const StringView& other);

		template<size_t strLength> inline constexpr String(const char (&str)[strLength]) {
			std::copy_n(str, strLength, values);
		}

		inline constexpr String& operator=(const_pointer other) noexcept {
			auto newSize = traits_type::length(other) + 1;
			reserve(newSize);
			if (capacityVal > 0) {
				std::copy_n(other, newSize, this->values);
			}
			sizeVal = newSize - 1;
			values[sizeVal] = '\0';
			return *this;
		}

		inline String(const_pointer other) noexcept {
			*this = other;
		}

		inline constexpr String(const_pointer other, size_t sizeNew) noexcept {
			reserve(sizeNew);
			if (capacityVal > 0) {
				std::copy_n(other, sizeNew, this->values);
			}
			sizeVal = sizeNew - 1;
			values[sizeVal] = '\0';
		}

		inline constexpr void insert(const char* newValues, size_t position) {
			auto amount = traits_type::length(newValues);
			if (position > 0 && amount > 0) {
				size_t newSize = sizeVal + amount;
				if (newSize + 1 >= capacityVal) {
					reserve(newSize + 1);
				}
				std::copy_n(values + position, sizeVal - position, values + amount + position);
				std::copy_n(newValues, amount, values + position);
				sizeVal = newSize;
				values[sizeVal] = '\0';
			}
		}

		inline constexpr const_iterator begin() const noexcept {
			return const_iterator(values, sizeVal, 0);
		}

		inline constexpr const_iterator end() const noexcept {
			return const_iterator(values, sizeVal, sizeVal);
		}

		inline constexpr value_type& operator[](size_t index) const noexcept {
			return values[index];
		}

		inline constexpr const_pointer c_str() const noexcept {
			return values;
		}

		inline String substr(size_type pos, size_type count) const {
			if (pos >= sizeVal) {
				return String();
			}

			count = (pos + count > sizeVal) ? (sizeVal - pos) : count;

			String result{};
			result.reserve(count + 1);

			std::copy_n(values + pos, count * sizeof(value_type), result.values);
			result.sizeVal = count;
			result.values[count] = '\0';
			return result;
		}

		inline explicit operator std::string() const noexcept {
			return std::string{ data(), size() };
		}

		inline constexpr void pushBack(value_type c) {
			if (sizeVal + 2 >= capacityVal) {
				reserve(capacityVal * 2 + 2);
			}
			values[sizeVal++] = c;
			values[sizeVal] = '\0';
		}

		inline constexpr size_t maxSize() {
			return std::numeric_limits<uint64_t>::max();
		}

		inline constexpr String& append(const size_type count, const value_type newChar) {
			const size_type oldSize = size();
			if (count <= capacity() - oldSize) {
				sizeVal = oldSize + count;
				value_type* const oldPtr = values;
				traits_type::assign(oldPtr + oldSize, count, newChar);
				traits_type::assign(oldPtr[oldSize + count], value_type{});
				return *this;
			}
			return reallocateGrowBy(
				count,
				[](value_type* const newPtr, const value_type* const oldPtr, const size_type oldSize, const size_type count,
					const value_type newChar) {
					traits_type::copy(newPtr, oldPtr, oldSize);
					traits_type::assign(newPtr + oldSize, count, newChar);
					traits_type::assign(newPtr[oldSize + count], value_type{});
				},
				count, newChar);
		}

		inline constexpr void resize(const size_type newSize) {
			const size_type oldSize = size();
			if (newSize <= oldSize) {
				eos(newSize);
			} else {
				append(newSize - oldSize, value_type{});
			}
		}

		inline constexpr void reserve(size_t newCapacity) {
			if (std::is_constant_evaluated()) {
				std::allocator<char> alloc{};
				char* newBuffer = alloc.allocate(newCapacity);
				std::copy(values, values + sizeVal, newBuffer);
				newBuffer[sizeVal] = '\0';
				alloc.deallocate(values, capacityVal);
				values = newBuffer;
				capacityVal = newCapacity;
			} else {
				char* newBuffer = allocator{}.allocate(newCapacity + 1);
				std::copy(values, values + sizeVal, newBuffer);
				newBuffer[sizeVal] = '\0';
				allocator{}.deallocate(values, capacityVal);
				values = newBuffer;
				capacityVal = newCapacity;
			}
		}

		inline constexpr size_type capacity() const noexcept {
			return capacityVal;
		}

		inline constexpr size_type size() const noexcept {
			return sizeVal;
		}

		inline constexpr pointer data() const noexcept {
			return values;
		}

		template<typename OTy> inline constexpr bool operator==(const OTy& rhs) const noexcept {
			if (rhs.size() != size()) {
				return false;
			}
			return JsonifierInternal::JsonifierCoreInternal::compare(rhs.data(), data(), rhs.size());
		}

		template<typename OTy> inline constexpr bool operator!=(const OTy& rhs) const noexcept {
			return !(rhs == *this);
		}

		inline constexpr String& operator+=(const String& rhs) noexcept {
			auto oldSize = size();
			resize(oldSize + size());
			std::copy_n(data(), size(), data() + oldSize);
			return *this;
		}

		inline constexpr String& operator+=(const value_type* rhs) noexcept {
			auto rhsSize = std::char_traits<value_type>::length(rhs);
			auto oldSize = size();
			resize(oldSize + rhsSize);
			std::copy_n(rhs, rhsSize, data() + oldSize);
			return *this;
		}

		inline constexpr ~String() {
			if (values && capacityVal) {
				allocator{}.deallocate(values, capacityVal);
			}
		};

	  protected:
		size_type capacityVal{};
		size_type sizeVal{};
		pointer values{};

		template<typename OTy, typename... Types>
		inline constexpr void constructInPlace(OTy& object, Types&&... args) noexcept(std::is_nothrow_constructible_v<OTy, Types...>) {
			if (std::is_constant_evaluated()) {
				std::construct_at(std::addressof(object), std::forward<Types>(args)...);
			} else {
				::new (static_cast<void*>(std::addressof(object))) OTy(std::forward<Types>(args)...);
			}
		}

		inline constexpr static size_type calculateGrowth(const size_type requested, const size_type old, const size_type max) noexcept {
			const size_type masked = requested | AllocMask;
			if (masked > max) {
				return max;
			}

			if (old > max - old / 2) {
				return max;
			}

			return (std::max)(masked, old + old / 2);
		}

		template<typename FTy, typename... ArgTypes>
		inline constexpr String& reallocateGrowBy(const size_type sizeIncrease, FTy function, ArgTypes... args) {
			auto& myData = values;
			const size_type oldSize = size();
			if (maxSize() - oldSize < sizeIncrease) {
				throw "String too long.";
			}

			const size_type newSize = oldSize + sizeIncrease;
			const size_type oldCapacity = capacityVal;
			const size_type newCapacity = calculateGrowth(newSize);
			auto alloc = allocator{};
			const pointer newPtr = allocator{}.allocate(newCapacity + 1);

			if (std::is_constant_evaluated()) {
				traits_type::assign(newPtr, newCapacity + 1, value_type{});
			}

			sizeVal = newSize;
			capacityVal = newCapacity;
			value_type* const rawNew = newPtr;
			if (BufSize <= oldCapacity) {
				const pointer oldPtr = values;
				function(rawNew, oldPtr, oldSize, args...);
				allocator{}.deallocate(oldPtr, oldCapacity + 1);
				values = newPtr;
			} else {
				function(rawNew, values, oldSize, args...);
				constructInPlace(values, newPtr);
			}
			return *this;
		}

		inline constexpr size_type calculateGrowth(const size_type requested) noexcept {
			return calculateGrowth(requested, capacityVal, maxSize());
		}

		inline constexpr void eos(const size_type newSize) {
			traits_type::assign(values[sizeVal = newSize], value_type{});
		}
	};

	template<typename OTy, typename Traits, typename SizeType>
	std::basic_ostream<OTy, Traits>& insertString(std::basic_ostream<OTy, Traits>& oStream, const OTy* const data, const SizeType size) {
		using OstreamType = std::basic_ostream<OTy, Traits>;
		typename OstreamType::iostate state = OstreamType::goodbit;

		SizeType pad;
		if (oStream.width() <= 0 || static_cast<SizeType>(oStream.width()) <= size) {
			pad = 0;
		} else {
			pad = static_cast<SizeType>(oStream.width()) - size;
		}

		const typename OstreamType::sentry ok(oStream);

		if (!ok) {
			state |= OstreamType::badbit;
		} else {
			try {
				if ((oStream.flags() & OstreamType::adjustfield) != OstreamType::left) {
					for (; 0 < pad; --pad) {
						if (Traits::eq_int_type(Traits::eof(), oStream.rdbuf()->sputc(oStream.fill()))) {
							state |= OstreamType::badbit;
							break;
						}
					}
				}

				if (state == OstreamType::goodbit &&
					oStream.rdbuf()->sputn(data, static_cast<std::streamsize>(size)) != static_cast<std::streamsize>(size)) {
					state |= OstreamType::badbit;
				} else {
					for (; 0 < pad; --pad) {
						if (Traits::eq_int_type(Traits::eof(), oStream.rdbuf()->sputc(oStream.fill()))) {
							state |= OstreamType::badbit;
							break;
						}
					}
				}
				oStream.width(0);
			} catch (...) {
				oStream.setstate(OstreamType::badbit);
			}
		}
		oStream.setstate(state);
		return oStream;
	}

	inline std::basic_ostream<Jsonifier::String::value_type, Jsonifier::String::traits_type>& operator<<(
		std::basic_ostream<Jsonifier::String::value_type, Jsonifier::String::traits_type>& oStream, const Jsonifier::String& string) {
		return insertString<Jsonifier::String::value_type, Jsonifier::String::traits_type>(oStream, string.data(), string.size());
	}

}

inline Jsonifier::String operator+(const Jsonifier::String& lhs, const Jsonifier::String& rhs) noexcept {
	Jsonifier::String newString(rhs);
	newString += rhs;
	return newString;
}

inline Jsonifier::String operator+(const Jsonifier::String& lhs, const char* rhs) noexcept {
	Jsonifier::String newString(rhs);
	newString += rhs;
	return newString;
}

inline Jsonifier::String operator+(const char* lhs, const Jsonifier::String& rhs) noexcept {
	Jsonifier::String newString(rhs);
	newString += rhs;
	return newString;
}
