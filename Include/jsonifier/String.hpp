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

	class String : public JsonifierInternal::Relational<String> {
	  public:
		using value_type = char;
		using traits_type = std::char_traits<value_type>;
		using pointer = value_type*;
		using const_pointer = const value_type*;
		using reference = value_type&;
		using const_reference = const value_type&;
		using iterator = JsonifierInternal::Iterator<value_type>;
		using const_iterator = const iterator;
		using reverse_iterator = std::reverse_iterator<iterator>;
		using const_reverse_iterator = const reverse_iterator;
		using size_type = size_t;
		using difference_type = ptrdiff_t;
		using allocator = JsonifierInternal::AllocWrapper<value_type>;

		inline constexpr String() noexcept = default;

		inline static constexpr size_type npos{ std::numeric_limits<size_type>::max() };

		inline constexpr String& operator=(String&& other) noexcept {
			if (this != &other) {
				JsonifierInternal::swapF(capacityVal, other.capacityVal);
				JsonifierInternal::swapF(sizeVal, other.sizeVal);
				JsonifierInternal::swapF(values, other.values);
			}
			return *this;
		}

		inline explicit String(String&& other) noexcept {
			*this = std::move(other);
		}

		inline constexpr String& operator=(const String& other) noexcept {
			if (this != &other) {
				auto sizeNew = other.size();
				if (sizeNew) [[likely]] {
					reserve(sizeNew);
					std::memcpy(values, other.data(), sizeNew);
					sizeVal = sizeNew;
				}
			}
			return *this;
		}

		inline String(const String& other) noexcept {
			*this = other;
		}

		inline String(value_type other) noexcept {
			*this = other;
		}

		inline String& operator=(const std::string& other) noexcept {
			auto sizeNew = other.size();
			if (sizeNew) [[likely]] {
				reserve(sizeNew);
				std::memcpy(values, other.data(), sizeNew);
				sizeVal = sizeNew;
			}
			return *this;
		}

		inline String(const std::string& other) noexcept {
			*this = other;
		}

		inline constexpr String& operator=(const std::string_view& other) noexcept {
			auto sizeNew = other.size();
			if (sizeNew) [[likely]] {
				reserve(sizeNew);
				std::memcpy(values, other.data(), sizeNew);
				sizeVal = sizeNew;
			}
			return *this;
		}

		inline String(const std::string_view& other) noexcept {
			*this = other;
		}

		template<size_t strLength> inline constexpr String(const char (&other)[strLength]) {
			if (strLength) [[likely]] {
				reserve(strLength);
				std::memcpy(values, other, strLength);
				sizeVal = strLength;
			}
		}

		inline constexpr String& operator=(const_pointer other) noexcept {
			auto sizeNew = traits_type::length(other);
			if (sizeNew) [[likely]] {
				reserve(sizeNew);
				std::memcpy(values, other, sizeNew);
				sizeVal = sizeNew;
			}
			return *this;
		}

		inline String(const_pointer other) noexcept {
			*this = other;
		}

		inline String(const_pointer other, size_t sizeNew) noexcept {
			if (sizeNew) [[likely]] {
				reserve(sizeNew);
				std::memcpy(values, other, sizeNew);
				sizeVal = sizeNew;
			}
		}

		inline String substr(size_type pos, size_type count) {
			if (pos + count >= sizeVal) {
				return String{};
			}
			String result{};
			result.reserve(count);
			std::memcpy(result.values, values + pos, count * sizeof(value_type));
			result.sizeVal = count;
			allocator alloc{};
			alloc.construct(&result.values[count], '\0');
			return result;
		}

		inline constexpr const_iterator begin() const noexcept {
			return const_iterator(values, sizeVal, 0);
		}

		inline constexpr const_iterator end() const noexcept {
			return const_iterator(values, sizeVal, sizeVal);
		}

		inline constexpr iterator begin() noexcept {
			return iterator(values, sizeVal, 0);
		}

		inline constexpr iterator end() noexcept {
			return iterator(values, sizeVal, sizeVal);
		}

		inline void pushBack(value_type value) {
			if (sizeVal + 1 >= capacityVal) {
				reserve((sizeVal + 2) * 4);
			}
			allocator alloc{};
			alloc.construct(&values[sizeVal++], value);
			alloc.construct(&values[sizeVal], '\0');
		}

		inline constexpr value_type& operator[](size_type index) noexcept {
			return values[index];
		}

		inline explicit operator std::string() const noexcept {
			return std::string{ data(), size() };
		}

		inline constexpr void clear() noexcept {
			sizeVal = 0;
		}

		inline constexpr size_type maxSize() const {
			return std::numeric_limits<size_type>::max();
		}

		inline void resize(size_type sizeNew) {
			if (sizeNew > 0) [[likely]] {
				if (sizeNew > capacityVal) [[likely]] {
					allocator alloc{};
					pointer newPtr = alloc.allocate(sizeNew + 1);
					try {
						if (values) [[likely]] {
							std::uninitialized_move(values, values + sizeVal, newPtr);
							alloc.deallocate(values, capacityVal + 1);
						}
					} catch (...) {
						alloc.deallocate(newPtr, sizeNew + 1);
						throw;
					}
					capacityVal = sizeNew;
					values = newPtr;
					std::uninitialized_default_construct(values + sizeVal, values + sizeVal + (sizeNew - sizeVal));
				} else if (sizeNew > sizeVal) [[unlikely]] {
					std::uninitialized_default_construct(values + sizeVal, values + sizeVal + (sizeNew - sizeVal));
				}
				sizeVal = sizeNew;
				values[sizeVal] = '\0';
			} else {
				sizeVal = 0;
			}
		}

		inline void reserve(size_type capacityNew) {
			if (capacityNew > capacityVal) [[likely]] {
				allocator alloc{};
				pointer newPtr = alloc.allocate(capacityNew + 1);
				try {
					if (values) [[likely]] {
						std::uninitialized_move(values, values + sizeVal, newPtr);
						alloc.deallocate(values, capacityVal + 1);
					}
				} catch (...) {
					alloc.deallocate(newPtr, capacityNew + 1);
					throw;
				}
				capacityVal = capacityNew;
				values = newPtr;
			}
		}

		inline constexpr size_type capacity() const noexcept {
			return capacityVal;
		}

		inline constexpr size_type size() const noexcept {
			return sizeVal;
		}

		inline constexpr bool empty() const noexcept {
			return sizeVal == 0;
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

		inline String& operator+=(const String& rhs) noexcept {
			auto oldSize = size();
			resize(oldSize + rhs.size());
			std::copy(rhs.data(), rhs.data() + rhs.size(), data() + oldSize);
			return *this;
		}

		inline String& operator+=(const_pointer rhs) noexcept {
			auto oldSize = size();
			auto rhsSize = traits_type::length(rhs);
			resize(oldSize + rhsSize);
			std::copy(rhs, rhs + rhsSize, data() + oldSize);
			return *this;
		}

		inline String operator+(const String& rhs) noexcept {
			String newString(*this);
			newString += rhs;
			return newString;
		}

		inline String operator+(const_pointer rhs) noexcept {
			String newString(*this);
			newString += rhs;
			return newString;
		}

		inline ~String() {
			if (values && capacityVal > 0) {
				allocator alloc{};
				alloc.deallocate(values, capacityVal + 1);
				values = nullptr;
				capacityVal = 0;
			}
		};

	  protected:
		size_type capacityVal{};
		size_type sizeVal{};
		pointer values{};
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

	inline std::basic_ostream<String::value_type, String::traits_type>& operator<<(
		std::basic_ostream<String::value_type, String::traits_type>& oStream, const String& string) {
		return insertString<String::value_type, String::traits_type>(oStream, string.data(), string.size());
	}

	inline String operator+(const char* lhs, const String& rhs) noexcept {
		String newString(lhs);
		newString += rhs;
		return newString;
	}
}
