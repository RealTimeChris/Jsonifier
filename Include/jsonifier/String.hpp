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

#include <jsonifier/Vector.hpp>
#include <jsonifier/Hash.hpp>

namespace jsonifier_internal {

	template<typename value_type01, typename value_type02> constexpr bool stringConstCompare(const value_type01& string01, const value_type02& string02) {
		if (string01.size() != string02.size()) [[unlikely]] {
			return false;
		}
		using char_t = typename value_type01::value_type;
		for (uint64_t x = 0; x < string01.size(); ++x) {
			if (string01[x] != static_cast<char_t>(string02[x])) [[unlikely]] {
				return false;
			}
		}
		return true;
	}

	template<typename value_type> class char_traits : public std::char_traits<jsonifier::concepts::unwrap_t<value_type>> {};

	template<jsonifier::concepts::uint8_type value_type_new> class char_traits<value_type_new> {
	  public:
		using value_type	= uint8_t;
		using pointer		= value_type*;
		using const_pointer = const value_type*;
		using size_type		= uint64_t;

		static constexpr void move(pointer firstNew, pointer first2, size_type count) {
			if (std::is_constant_evaluated()) {
				bool loopForward = true;

				for (pointer source = first2; source != first2 + count; ++source) {
					if (firstNew == source) {
						loopForward = false;
					}
				}

				if (loopForward) {
					for (uint64_t index = 0; index != count; ++index) {
						firstNew[index] = first2[index];
					}
				} else {
					for (uint64_t index = count; index != 0; --index) {
						firstNew[index - 1] = first2[index - 1];
					}
				}

				return;
			}
			std::memmove(firstNew, first2, count * sizeof(value_type_new));
		}

		static constexpr size_type length(const_pointer first) {
			const_pointer newPtr = first;
			size_type count		 = 0;
			while (newPtr && *newPtr != static_cast<uint8_t>(0x00u)) {
				++count;
				++newPtr;
			}

			return count;
		}
	};

}

namespace jsonifier {

	template<typename value_type> class string_view_base;

	template<typename value_type_new, uint64_t newerSize = 0> class string_base : protected jsonifier_internal::alloc_wrapper<value_type_new> {
	  public:
		using value_type			 = value_type_new;
		using pointer				 = value_type*;
		using const_pointer			 = const value_type*;
		using reference				 = value_type&;
		using const_reference		 = const value_type&;
		using difference_type		 = std::ptrdiff_t;
		using iterator				 = jsonifier_internal::iterator<value_type>;
		using const_iterator		 = jsonifier_internal::const_iterator<value_type>;
		using reverse_iterator		 = std::reverse_iterator<iterator>;
		using const_reverse_iterator = std::reverse_iterator<const_iterator>;
		using size_type				 = uint64_t;
		using allocator				 = jsonifier_internal::alloc_wrapper<value_type>;
		using traits_type			 = jsonifier_internal::char_traits<value_type>;

		JSONIFIER_INLINE string_base() : capacityVal{}, sizeVal{}, dataVal{} {
			if constexpr (newerSize > 0) {
				resize(newerSize);
			}
		};

		static constexpr size_type bufSize = 16 / sizeof(value_type) < 1 ? 1 : 16 / sizeof(value_type);
		static constexpr size_type npos{ std::numeric_limits<size_type>::max() };

		JSONIFIER_INLINE string_base& operator=(string_base&& other) noexcept {
			if (this != &other) [[likely]] {
				string_base newValue{ other };
				this->swap(newValue);
			}
			return *this;
		}

		JSONIFIER_INLINE explicit string_base(string_base&& other) noexcept : capacityVal{}, sizeVal{}, dataVal{} {
			swap(other);
		};

		JSONIFIER_INLINE string_base& operator=(const string_base& other) {
			if (this != &other) [[likely]] {
				string_base newValue{ other };
				this->swap(newValue);
			}
			return *this;
		}

		JSONIFIER_INLINE string_base(const string_base& other) : capacityVal{}, sizeVal{}, dataVal{} {
			size_type newSize = other.size();
			if (newSize > 0 && newSize < maxSize()) [[likely]] {
				reserve(newSize);
				sizeVal = newSize;
				std::uninitialized_copy(other.data(), other.data() + newSize, dataVal);
				allocator::construct(&(*this)[newSize], value_type{});
			}
		}

		template<jsonifier::concepts::string_t value_type_newer> JSONIFIER_INLINE string_base& operator=(value_type_newer&& other) {
			string_base newValue{ other };
			this->swap(newValue);
			return *this;
		}

		template<jsonifier::concepts::string_t value_type_newer> JSONIFIER_INLINE string_base(value_type_newer&& other) : capacityVal{}, sizeVal{}, dataVal{} {
			size_type newSize = other.size() * (sizeof(typename jsonifier::concepts::unwrap_t<value_type_newer>::value_type) / sizeof(value_type));
			if (newSize > 0 && newSize < maxSize()) [[likely]] {
				reserve(newSize);
				sizeVal = newSize;
				std::uninitialized_copy(other.data(), other.data() + newSize, dataVal);
				allocator::construct(&(*this)[newSize], value_type{});
			}
		}

		template<jsonifier::concepts::pointer_t value_type_newer> JSONIFIER_INLINE string_base& operator=(value_type_newer other) {
			string_base newValue{ std::forward<value_type_newer>(other) };
			this->swap(newValue);
			return *this;
		}

		template<jsonifier::concepts::pointer_t value_type_newer> JSONIFIER_INLINE string_base(value_type_newer other) : capacityVal{}, sizeVal{}, dataVal{} {
			auto newSize =
				jsonifier_internal::char_traits<std::remove_pointer_t<value_type_newer>>::length(other) * (sizeof(std::remove_pointer_t<value_type_newer>) / sizeof(value_type));
			if (newSize > 0 && newSize < maxSize()) [[likely]] {
				reserve(newSize);
				sizeVal = newSize;
				std::uninitialized_copy(other, other + newSize, dataVal);
				allocator::construct(&(*this)[newSize], value_type{});
			}
		}

		template<jsonifier::concepts::char_t value_type_newer> JSONIFIER_INLINE string_base& operator=(value_type_newer other) {
			emplace_back(static_cast<value_type>(other));
			return *this;
		}

		template<jsonifier::concepts::char_t value_type_newer> JSONIFIER_INLINE string_base(value_type_newer other) : capacityVal{}, sizeVal{}, dataVal{} {
			*this = other;
		}

		JSONIFIER_INLINE string_base(const_pointer other, uint64_t newSize) : capacityVal{}, sizeVal{}, dataVal{} {
			if (newSize > 0 && newSize < maxSize()) [[likely]] {
				reserve(newSize);
				sizeVal = newSize;
				std::uninitialized_copy(other, other + newSize, dataVal);
				allocator::construct(&(*this)[newSize], value_type{});
			}
		}

		JSONIFIER_INLINE string_base substr(size_type position, size_type count = std::numeric_limits<size_type>::max()) const {
			if (static_cast<int64_t>(position) >= static_cast<int64_t>(sizeVal)) [[unlikely]] {
				throw std::out_of_range("Substring position is out of range.");
			}

			count = std::min(count, sizeVal - position);

			string_base result{};
			if (count > 0) [[likely]] {
				result.resize(count);
				std::copy(dataVal + position, dataVal + position + count * sizeof(value_type), result.dataVal);
			}
			return result;
		}

		constexpr size_type maxSize() {
			const size_type allocMax   = allocator::maxSize();
			const size_type storageMax = jsonifier_internal::max(allocMax, static_cast<size_type>(bufSize));
			return std::min(static_cast<size_type>((std::numeric_limits<difference_type>::max)()), storageMax - 1);
		}

		constexpr iterator begin() noexcept {
			return iterator{ dataVal };
		}

		constexpr iterator end() noexcept {
			return iterator{ dataVal + sizeVal };
		}

		constexpr reverse_iterator rbegin() noexcept {
			return reverse_iterator{ end() };
		}

		constexpr reverse_iterator rend() noexcept {
			return reverse_iterator{ begin() };
		}

		constexpr const_iterator begin() const noexcept {
			return const_iterator{ dataVal };
		}

		constexpr const_iterator end() const noexcept {
			return const_iterator{ dataVal + sizeVal };
		}

		constexpr const_reverse_iterator rbegin() const noexcept {
			return const_reverse_iterator{ end() };
		}

		constexpr const_reverse_iterator rend() const noexcept {
			return const_reverse_iterator{ begin() };
		}

		template<typename... arg_types> JSONIFIER_INLINE size_type rfind(arg_types&&... args) const {
			return operator std::basic_string_view<value_type>().rfind(std::forward<arg_types>(args)...);
		}

		template<typename... arg_types> JSONIFIER_INLINE size_type find(arg_types&&... args) const {
			return operator std::basic_string_view<value_type>().find(std::forward<arg_types>(args)...);
		}

		template<typename... arg_types> JSONIFIER_INLINE size_type findFirstOf(arg_types&&... args) const {
			return operator std::basic_string_view<value_type>().find_first_of(std::forward<arg_types>(args)...);
		}

		template<typename... arg_types> JSONIFIER_INLINE size_type findLastOf(arg_types&&... args) const {
			return operator std::basic_string_view<value_type>().find_last_of(std::forward<arg_types>(args)...);
		}

		template<typename... arg_types> JSONIFIER_INLINE size_type findFirstNotOf(arg_types&&... args) const {
			return operator std::basic_string_view<value_type>().find_first_not_of(std::forward<arg_types>(args)...);
		}

		template<typename... arg_types> JSONIFIER_INLINE size_type findLastNotOf(arg_types&&... args) const {
			return operator std::basic_string_view<value_type>().find_last_not_of(std::forward<arg_types>(args)...);
		}

		JSONIFIER_INLINE void append(const string_base& newSize) {
			if (sizeVal + newSize.size() >= capacityVal) [[unlikely]] {
				reserve(sizeVal + newSize.size());
			}
			if (newSize.size() > 0) [[likely]] {
				std::copy(newSize.data(), newSize.data() + newSize.size(), dataVal + sizeVal);
				sizeVal += newSize.size();
				allocator::construct(&dataVal[sizeVal], value_type{});
			}
		}

		template<typename value_type_newer> JSONIFIER_INLINE void append(value_type_newer* values, uint64_t newSize) {
			if (sizeVal + newSize >= capacityVal) [[unlikely]] {
				reserve(sizeVal + newSize);
			}
			if (newSize > 0 && values) [[likely]] {
				std::copy(values, values + newSize, dataVal + sizeVal);
				sizeVal += newSize;
				allocator::construct(&dataVal[sizeVal], value_type{});
			}
		}

		template<typename Iterator01, typename Iterator02> JSONIFIER_INLINE void insert(Iterator01 where, Iterator02 start, Iterator02 end) {
			int64_t newSize = end - start;
			auto posNew		= where.operator->() - dataVal;

			if (newSize <= 0) [[unlikely]] {
				return;
			}

			if (sizeVal + newSize >= capacityVal) [[unlikely]] {
				reserve(sizeVal + newSize);
			}

			std::memmove(dataVal + posNew + newSize, dataVal + posNew, (sizeVal - posNew) * sizeof(value_type));
			std::copy(start.operator->(), start.operator->() + newSize * sizeof(value_type), dataVal + posNew);
			sizeVal += newSize;
			allocator::construct(&dataVal[sizeVal], value_type{});
		}

		JSONIFIER_INLINE void insert(iterator values, value_type toInsert) {
			auto positionNew = values - begin();
			if (sizeVal + 1 >= capacityVal) [[unlikely]] {
				reserve((sizeVal + 1) * 2);
			}
			auto newSize = sizeVal - positionNew;
			std::memmove(dataVal + positionNew + 1, dataVal + positionNew, newSize * sizeof(value_type));
			allocator::construct(&dataVal[positionNew], toInsert);
			++sizeVal;
		}

		JSONIFIER_INLINE void erase(size_type count) {
			if (count == 0) [[unlikely]] {
				return;
			} else if (count > sizeVal) [[likely]] {
				count = sizeVal;
			}
			traits_type::move(dataVal, dataVal + count, sizeVal - count);
			sizeVal -= count;
			allocator::construct(&dataVal[sizeVal], static_cast<value_type>(0x00u));
		}

		JSONIFIER_INLINE void erase(iterator count) {
			auto newSize = count.operator->() - dataVal;
			if (newSize == 0) [[unlikely]] {
				return;
			} else if (newSize > static_cast<int64_t>(sizeVal)) [[unlikely]] {
				newSize = static_cast<int64_t>(sizeVal);
			}
			traits_type::move(dataVal, dataVal + newSize, sizeVal - newSize);
			sizeVal -= newSize;
			allocator::construct(&dataVal[sizeVal], static_cast<value_type>(0x00u));
		}

		JSONIFIER_INLINE void emplace_back(value_type value) {
			if (sizeVal + 1 >= capacityVal) [[unlikely]] {
				reserve((sizeVal + 2) * 4);
			}
			allocator::construct(&dataVal[sizeVal++], value);
			allocator::construct(&dataVal[sizeVal], value_type{});
		}

		JSONIFIER_INLINE const_reference at(size_type index) const {
			if (index >= sizeVal) [[unlikely]] {
				throw std::runtime_error{ "Sorry, but that index is beyond the end of this string." };
			}
			return dataVal[index];
		}

		JSONIFIER_INLINE reference at(size_type index) {
			if (index >= sizeVal) [[unlikely]] {
				throw std::runtime_error{ "Sorry, but that index is beyond the end of this string." };
			}
			return dataVal[index];
		}

		JSONIFIER_INLINE const_reference operator[](size_type index) const {
			return dataVal[index];
		}

		JSONIFIER_INLINE reference operator[](size_type index) {
			return dataVal[index];
		}

		JSONIFIER_INLINE operator std::basic_string_view<value_type>() const {
			return { dataVal, sizeVal };
		}

		template<typename value_type_newer> JSONIFIER_INLINE explicit operator jsonifier::string_base<value_type_newer>() const {
			jsonifier::string_base<value_type_newer> returnValue{};
			if (sizeVal > 0) [[likely]] {
				returnValue.resize(sizeVal);
				std::memcpy(returnValue.data(), data(), returnValue.size());
			}
			return returnValue;
		}

		template<typename value_type_newer> JSONIFIER_INLINE explicit operator std::basic_string<value_type_newer>() const {
			std::basic_string<value_type_newer> returnValue{};
			if (sizeVal > 0) [[likely]] {
				returnValue.resize(sizeVal);
				std::memcpy(returnValue.data(), data(), returnValue.size());
			}
			return returnValue;
		}

		virtual JSONIFIER_INLINE void clear() {
			if (sizeVal > 0) [[likely]] {
				allocator::construct(dataVal, static_cast<value_type>(0x00u));
			}
			sizeVal = 0;
		}

		JSONIFIER_INLINE void resize(size_type newSize) {
			if (static_cast<int64_t>(newSize) > 0) [[likely]] {
				if (newSize > capacityVal) [[likely]] {
					pointer newPtr = allocator::allocate(newSize + 1);
					try {
						if (dataVal) [[likely]] {
							if (sizeVal > 0) [[likely]] {
								std::uninitialized_move(dataVal, dataVal + sizeVal, newPtr);
							}
							allocator::deallocate(dataVal, capacityVal + 1);
						}
					} catch (...) {
						allocator::deallocate(newPtr, newSize + 1);
						throw;
					}
					capacityVal = newSize;
					dataVal		= newPtr;
					std::uninitialized_value_construct(dataVal + sizeVal, dataVal + capacityVal);
					allocator::construct(newPtr + newSize, value_type{});
					sizeVal = newSize;
				} else if (newSize > sizeVal) [[likely]] {
					std::uninitialized_value_construct(dataVal + sizeVal, dataVal + capacityVal);
					allocator::construct(dataVal + newSize, value_type{});
					sizeVal = newSize;
				} else if (newSize < sizeVal) [[likely]] {
					std::destroy(dataVal + newSize, dataVal + sizeVal);
					allocator::construct(dataVal + newSize, value_type{});
					sizeVal = newSize;
				}
			} else {
				std::destroy(dataVal, dataVal + sizeVal);
				sizeVal = 0;
			}
		}

		JSONIFIER_INLINE void reserve(size_type capacityNew) {
			if (capacityNew > capacityVal) [[likely]] {
				pointer newPtr = allocator::allocate(capacityNew + 1);
				try {
					if (dataVal) [[likely]] {
						if (sizeVal > 0) [[likely]] {
							std::uninitialized_move(dataVal, dataVal + sizeVal, newPtr);
						}
						allocator::deallocate(dataVal, capacityVal + 1);
					}
				} catch (...) {
					allocator::deallocate(newPtr, capacityNew + 1);
					throw;
				}
				capacityVal = capacityNew;
				dataVal		= newPtr;
				allocator::construct(&dataVal[sizeVal], value_type{});
			}
		}

		constexpr size_type capacity() const {
			return capacityVal;
		}

		constexpr size_type size() const {
			return sizeVal;
		}

		constexpr bool empty() const {
			return sizeVal == 0;
		}

		JSONIFIER_INLINE pointer data() const {
			return dataVal;
		}

		JSONIFIER_INLINE pointer data() {
			return dataVal;
		}

		template<jsonifier::concepts::pointer_t value_type_newer>
		JSONIFIER_INLINE friend std::enable_if_t<!std::is_array_v<value_type_newer>, bool> operator==(const string_base& lhs, const value_type_newer& rhs) {
			auto rhsLength = traits_type::length(rhs);
			return rhsLength == lhs.size() && jsonifier_asm::compare(lhs.data(), rhs, rhsLength);
		}

		template<jsonifier::concepts::string_t value_type_newer> JSONIFIER_INLINE friend bool operator==(const string_base& lhs, const value_type_newer& rhs) {
			if (lhs.size() == rhs.size()) {
				if (lhs.size() > 0) {
					return jsonifier_asm::compare(lhs.data(), rhs.data(), rhs.size());
				}
				return true;
			} else {
				return false;
			}
		}

		template<jsonifier::concepts::string_t value_type_newer> JSONIFIER_INLINE friend bool operator<(const string_base& lhs, const value_type_newer& rhs) {
			return lhs.size() < rhs.size();
		}

		template<typename string_base_new> JSONIFIER_INLINE void swap(string_base_new&& other) {
			std::swap(capacityVal, other.capacityVal);
			std::swap(sizeVal, other.sizeVal);
			std::swap(dataVal, other.dataVal);
		}

		template<typename value_type_newer, size_type size> JSONIFIER_INLINE friend string_base operator+(const value_type_newer (&lhs)[size], const string_base& rhs) {
			string_base newLhs{ lhs };
			newLhs += rhs;
			return newLhs;
		}

		template<typename value_type_newer, size_type size> JSONIFIER_INLINE friend string_base operator+=(const value_type_newer (&lhs)[size], const string_base& rhs) {
			string_base newLhs{ lhs };
			newLhs += rhs;
			return newLhs;
		}

		template<jsonifier::concepts::pointer_t string_type_new> JSONIFIER_INLINE friend string_base operator+(string_type_new&& lhs, const string_base& rhs) {
			string_base newLhs{ lhs };
			newLhs += rhs;
			return newLhs;
		}

		template<jsonifier::concepts::pointer_t string_type_new> JSONIFIER_INLINE friend string_base operator+=(string_type_new&& lhs, const string_base& rhs) {
			string_base newLhs{ lhs };
			newLhs += rhs;
			return newLhs;
		}

		JSONIFIER_INLINE string_base operator+(const value_type& rhs) {
			string_base newLhs{ *this };
			newLhs.emplace_back(rhs);
			return newLhs;
		}

		JSONIFIER_INLINE string_base& operator+=(const value_type& rhs) {
			emplace_back(rhs);
			return *this;
		}

		template<jsonifier::concepts::string_t string_type_new> JSONIFIER_INLINE string_base operator+(const string_type_new& rhs) const {
			string_base newLhs{ *this };
			newLhs += rhs;
			return newLhs;
		}

		template<jsonifier::concepts::string_t string_type_new> JSONIFIER_INLINE string_base& operator+=(const string_type_new& rhs) {
			append(static_cast<string_base>(rhs));
			return *this;
		}

		template<jsonifier::concepts::pointer_t string_type_new> JSONIFIER_INLINE string_base operator+(string_type_new&& rhs) {
			string_base newLhs{ *this };
			newLhs += rhs;
			return newLhs;
		}

		template<jsonifier::concepts::pointer_t string_type_new> JSONIFIER_INLINE string_base& operator+=(string_type_new&& rhs) {
			string_base newRhs{ rhs };
			*this += newRhs;
			return *this;
		}

		template<typename value_type_newer, size_type size> JSONIFIER_INLINE string_base operator+(const value_type_newer (&rhs)[size]) const {
			string_base newLhs{ *this };
			newLhs += rhs;
			return newLhs;
		}

		template<typename value_type_newer, size_type size> JSONIFIER_INLINE string_base& operator+=(const value_type_newer (&rhs)[size]) {
			string_base newRhs{ rhs };
			*this += newRhs;
			return *this;
		}

		virtual JSONIFIER_INLINE ~string_base() {
			reset();
		}

	  protected:
		size_type capacityVal{};
		size_type sizeVal{};
		pointer dataVal{};

		JSONIFIER_INLINE void reset() {
			if (dataVal && capacityVal) [[likely]] {
				if (sizeVal) [[likely]] {
					std::destroy(dataVal, dataVal + sizeVal);
					sizeVal = 0;
				}
				allocator::deallocate(dataVal, capacityVal + 1);
				dataVal		= nullptr;
				capacityVal = 0;
			}
		}
	};

	using string = string_base<char>;

	template<typename value_type> JSONIFIER_INLINE std::ostream& operator<<(std::ostream& os, const string_base<value_type>& string) {
		os << string.operator typename std::string();
		return os;
	}
}// namespace jsonifier
