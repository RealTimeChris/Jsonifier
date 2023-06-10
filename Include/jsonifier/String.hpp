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

	template<typename ValueType> class AllocatorWrapper {
	  public:
		using allocator = std::pmr::polymorphic_allocator<ValueType>;
		using size_type = size_t;
		using value_type = ValueType;
		using pointer = value_type*;

		inline constexpr auto allocate(size_type count) {
			return alloc.allocate(count);
		}

		inline constexpr auto deallocate(pointer ptr, size_type count) {
			return alloc.deallocate(ptr, count);
		}

	  protected:
		NO_UNIQUE_ADDRESS allocator alloc{};
	};

	inline static constexpr size_t BufSize = 16 / sizeof(char) < 1 ? 1 : 16 / sizeof(char);

	inline static constexpr size_t AllocMask = sizeof(char) <= 1 ? 15 : sizeof(char) <= 2 ? 7 : sizeof(char) <= 4 ? 3 : sizeof(char) <= 8 ? 1 : 0;

	class String {
	  public:
		using value_type = char;
		using traits_type = std::char_traits<value_type>;
		using pointer = value_type*;
		using const_pointer = const value_type*;
		using reference = value_type&;
		using const_reference = const value_type&;
		using iterator = Iterator<value_type>;
		using const_iterator = const iterator;
		using reverse_iterator = std::reverse_iterator<iterator>;
		using const_reverse_iterator = const reverse_iterator;
		using size_type = size_t;
		using difference_type = ptrdiff_t;
		using allocator = AllocatorWrapper<value_type>;

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
				if (other.size() > 0) {
					reserve(other.capacityVal);
					std::memcpy(values, other.data(), other.size());
					sizeVal = other.sizeVal;
				}
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
			if (other.size() > 0) {
				reserve(other.capacity());
				std::memcpy(values, other.data(), other.size());
				sizeVal = other.size();
			}
		}

		inline constexpr String& operator=(const std::string_view& other) noexcept {
			if (other.size() > 0) {
				reserve(other.size());
				std::memcpy(values, other.data(), other.size());
				sizeVal = other.size();
			}
			return *this;
		}

		inline String(const std::string_view& other) noexcept {
			*this = other;
		}

		template<size_t strLength> inline constexpr String(const char (&str)[strLength]) {
			std::memcpy(values, str, strLength);
		}

		inline constexpr String& operator=(const_pointer other) noexcept {
			auto newSize = traits_type::length(other) + 1;
			if (newSize > 0) {
				reserve(newSize);
				if (capacityVal > 0) {
					std::memcpy(values, other, newSize);
				}
				sizeVal = newSize - 1;
				values[sizeVal] = '\0';
			}
			return *this;
		}

		inline String(const_pointer other) noexcept {
			*this = other;
		}

		inline String(const_pointer other, size_t sizeNew) noexcept {
			if (sizeNew > 0) {
				reserve(sizeNew);
				if (capacityVal > 0) {
					std::memcpy(values, other, sizeNew);
				}
				sizeVal = sizeNew - 1;
				values[sizeVal] = '\0';
			}
		}

		inline constexpr void insert(const char* newValues, size_t position) {
			auto amount = traits_type::length(newValues);
			if (position > 0 && amount > 0) {
				size_t newSize = sizeVal + amount;
				if (newSize + 1 >= capacityVal) {
					reserve(newSize + 1);
				}
				std::memcpy(values + amount + position, values + position, sizeVal - position);
				std::memcpy(values + position, newValues, amount);
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

			std::memcpy(result.values, values + pos, count * sizeof(value_type));
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

		inline constexpr void clear() noexcept {
			sizeVal = 0;
		}

		inline constexpr size_t maxSize() {
			return std::numeric_limits<size_t>::max();
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
			return reallocateGrowBy(count, count, newChar);
		}

		inline constexpr void resize(const size_type newSize) {
			const size_type oldSize = size();
			if (newSize <= oldSize) [[unlikely]] {
				eos(newSize);
			} else {
				append(newSize - oldSize, value_type{});
			}
		}

		inline constexpr void reserve(size_t newCapacity) {
			if (std::is_constant_evaluated()) [[unlikely]] {
				std::allocator<char> alloc{};
				char* newBuffer = alloc.allocate(newCapacity + 1);
				std::copy(values, values + sizeVal, newBuffer);
				newBuffer[sizeVal] = '\0';
				alloc.deallocate(values, capacityVal);
				values = newBuffer;
				capacityVal = newCapacity + 1;
			} else {
				char* newBuffer = alloc.allocate(newCapacity + 1);
				std::memcpy(newBuffer, values, sizeVal);
				newBuffer[sizeVal] = '\0';
				alloc.deallocate(values, capacityVal);
				values = newBuffer;
				capacityVal = newCapacity + 1;
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

		template<typename ValueType> inline constexpr bool operator==(const ValueType& rhs) const noexcept {
			if (rhs.size() != size()) {
				return false;
			}
			return JsonifierInternal::JsonifierCoreInternal::compare(rhs.data(), data(), rhs.size());
		}

		template<typename ValueType> inline constexpr bool operator!=(const ValueType& rhs) const noexcept {
			return !(rhs == *this);
		}

		inline constexpr String& operator+=(const String& rhs) noexcept {
			auto oldSize = size();
			resize(oldSize + rhs.size());
			std::copy(rhs.data(), rhs.data() + rhs.size(), data() + oldSize);
			return *this;
		}

		inline constexpr String& operator+=(const value_type* rhs) noexcept {
			auto rhsSize = std::char_traits<value_type>::length(rhs);
			auto oldSize = size();
			resize(oldSize + rhsSize);
			std::copy(rhs, rhs + rhsSize, data() + oldSize);
			return *this;
		}

		inline constexpr ~String() {
			if (values && capacityVal) {
				alloc.deallocate(values, capacityVal);
			}
		};

	  protected:
		NO_UNIQUE_ADDRESS allocator alloc{};
		size_type capacityVal{};
		size_type sizeVal{};
		pointer values{};

		template<typename ValueType, typename... Types>
		inline constexpr void constructInPlace(ValueType& object, Types&&... args) noexcept(std::is_nothrow_constructible_v<ValueType, Types...>) {
			if (std::is_constant_evaluated()) {
				std::construct_at(std::addressof(object), std::forward<Types>(args)...);
			} else {
				::new (static_cast<void*>(std::addressof(object))) ValueType(std::forward<Types>(args)...);
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

		inline constexpr String& reallocateGrowBy(const size_type sizeIncrease, size_type count, value_type newChar) {
			const size_type oldSize = size();
			if (maxSize() - oldSize < sizeIncrease) {
				throw "String too long.";
			}

			const size_type newSize = oldSize + sizeIncrease;
			const size_type oldCapacity = capacityVal;
			const size_type newCapacity = calculateGrowth(newSize);
			const pointer newPtr = alloc.allocate(newCapacity + 1);

			if (std::is_constant_evaluated()) {
				traits_type::assign(newPtr, newCapacity + 1, value_type{});
			}

			sizeVal = newSize;
			capacityVal = newCapacity;
			value_type* const rawNew = newPtr;
			if (BufSize <= oldCapacity) {
				const pointer oldPtr = values;
				traits_type::copy(rawNew, oldPtr, oldSize);
				traits_type::assign(rawNew + oldSize, count, newChar);
				traits_type::assign(rawNew[oldSize + count], value_type{});
				alloc.deallocate(oldPtr, oldCapacity + 1);
				values = newPtr;
			} else {
				traits_type::copy(rawNew, values, oldSize);
				traits_type::assign(rawNew + oldSize, count, newChar);
				traits_type::assign(rawNew[oldSize + count], value_type{});
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

	template<typename ValueType, typename Traits, typename SizeType> std::basic_ostream<ValueType, Traits>& insertString(
		std::basic_ostream<ValueType, Traits>& oStream, const ValueType* const data, const SizeType size) {
		using OstreamType = std::basic_ostream<ValueType, Traits>;
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
