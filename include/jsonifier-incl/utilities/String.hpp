/*
	MIT License

	Copyright (c) 2024 RealTimeChris

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

#include <jsonifier-incl/containers/Allocator.hpp>
#include <jsonifier-incl/containers/Iterator.hpp>
#include <jsonifier-incl/utilities/Compare.hpp>

namespace jsonifier::internal {

	template<typename value_type>
	concept not_uint8_t = !std::is_same_v<value_type, uint8_t>;

	template<typename value_type> class char_traits;

	template<not_uint8_t value_type> class char_traits<value_type> : public std::char_traits<jsonifier::internal::remove_cvref_t<value_type>> {};

	template<concepts::uns8_t value_type_new> class char_traits<value_type_new> {
	  public:
		using value_type	= uint8_t;
		using pointer		= value_type*;
		using const_pointer = const value_type*;
		using size_type		= uint64_t;

		inline static constexpr void move(pointer firstNew, pointer first2, size_type count) noexcept {
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

		inline static constexpr size_type length(const_pointer first) noexcept {
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

	template<typename value_type_new, uint64_t newerSize = 0> class string_base : protected internal::alloc_wrapper<value_type_new> {
	  public:
		using value_type			 = value_type_new;
		using pointer				 = value_type*;
		using const_pointer			 = const value_type*;
		using reference				 = value_type&;
		using const_reference		 = const value_type&;
		using difference_type		 = std::ptrdiff_t;
		using iterator				 = internal::basic_iterator<value_type>;
		using const_iterator		 = internal::basic_iterator<const value_type>;
		using reverse_iterator		 = std::reverse_iterator<iterator>;
		using const_reverse_iterator = std::reverse_iterator<const_iterator>;
		using size_type				 = uint64_t;
		using allocator				 = internal::alloc_wrapper<value_type>;
		using traits_type			 = internal::char_traits<value_type>;

		inline string_base() noexcept : internal::alloc_wrapper<value_type_new>{}, capacityVal{}, sizeVal{}, dataVal{} {
			if constexpr (newerSize > 0) {
				resize(newerSize);
			}
		}

		inline static constexpr size_type bufSize = 16 / sizeof(value_type) < 1 ? 1 : 16 / sizeof(value_type);
		inline static constexpr size_type npos{ std::numeric_limits<size_type>::max() };

		inline string_base& operator=(string_base&& other) noexcept {
			if JSONIFIER_LIKELY (this != &other) {
				string_base newValue{ other };
				swap(newValue);
			}
			return *this;
		}

		inline explicit string_base(string_base&& other) noexcept : internal::alloc_wrapper<value_type_new>{}, capacityVal{}, sizeVal{}, dataVal{} {
			swap(other);
		}

		inline string_base& operator=(const string_base& other) noexcept {
			if JSONIFIER_LIKELY (this != &other) {
				string_base newValue{ other };
				swap(newValue);
			}
			return *this;
		}

		inline string_base(const string_base& other) noexcept : internal::alloc_wrapper<value_type_new>{}, capacityVal{}, sizeVal{}, dataVal{} {
			size_type newSize = other.size();
			if JSONIFIER_LIKELY (newSize > 0 && newSize < maxSize()) {
				reserve(newSize);
				sizeVal = newSize;
				std::uninitialized_copy(other.data(), other.data() + newSize, dataVal);
				allocator::construct(&(*this)[newSize], value_type{});
			}
		}

		template<concepts::string_t value_type_newer> inline string_base& operator=(value_type_newer&& other) noexcept {
			string_base newValue{ other };
			swap(newValue);
			return *this;
		}

		template<concepts::string_t value_type_newer> inline string_base(value_type_newer&& other) noexcept : capacityVal{}, sizeVal{}, dataVal{} {
			size_type newSize = other.size() * (sizeof(typename jsonifier::internal::remove_cvref_t<value_type_newer>::value_type) / sizeof(value_type));
			if JSONIFIER_LIKELY (newSize > 0 && newSize < maxSize()) {
				reserve(newSize);
				sizeVal = newSize;
				std::uninitialized_copy(other.data(), other.data() + newSize, dataVal);
				allocator::construct(&(*this)[newSize], value_type{});
			}
		}

		template<concepts::pointer_t value_type_newer> inline string_base& operator=(value_type_newer other) noexcept {
			string_base newValue{ std::forward<value_type_newer>(other) };
			swap(newValue);
			return *this;
		}

		template<concepts::pointer_t value_type_newer> inline string_base(value_type_newer other) noexcept : capacityVal{}, sizeVal{}, dataVal{} {
			if (other) {
				const auto newSize = internal::char_traits<jsonifier::internal::remove_pointer_t<value_type_newer>>::length(other) *
					(sizeof(jsonifier::internal::remove_pointer_t<value_type_newer>) / sizeof(value_type));
				if JSONIFIER_LIKELY (newSize > 0 && newSize < maxSize()) {
					reserve(newSize);
					sizeVal = newSize;
					std::uninitialized_copy(other, other + newSize, dataVal);
					allocator::construct(&(*this)[newSize], value_type{});
				}
			}
		}

		template<concepts::char_t value_type_newer> inline string_base& operator=(value_type_newer other) noexcept {
			emplace_back(static_cast<value_type>(other));
			return *this;
		}

		template<concepts::char_t value_type_newer> inline string_base(value_type_newer other) noexcept : capacityVal{}, sizeVal{}, dataVal{} {
			*this = other;
		}

		inline string_base(const_pointer other, uint64_t newSize) noexcept : capacityVal{}, sizeVal{}, dataVal{} {
			if JSONIFIER_LIKELY (newSize > 0 && newSize < maxSize()) {
				reserve(newSize);
				sizeVal = newSize;
				std::uninitialized_copy(other, other + newSize, dataVal);
				allocator::construct(&(*this)[newSize], value_type{});
			}
		}

		inline string_base(const_iterator other, uint64_t newSize) noexcept : capacityVal{}, sizeVal{}, dataVal{} {
			if JSONIFIER_LIKELY (newSize > 0 && newSize < maxSize()) {
				reserve(newSize);
				sizeVal = newSize;
				std::uninitialized_copy(other.operator->(), other.operator->() + newSize, dataVal);
				allocator::construct(&(*this)[newSize], value_type{});
			}
		}

		inline string_base substr(size_type position, size_type count = std::numeric_limits<size_type>::max()) const {
			if JSONIFIER_UNLIKELY (static_cast<int64_t>(position) >= static_cast<int64_t>(sizeVal)) {
				throw std::out_of_range("Substring position is out of range.");
			}

			count = internal::min(count, sizeVal - position);

			string_base result{};
			if JSONIFIER_LIKELY (count > 0) {
				result.resize(count);
				std::copy(dataVal + position, dataVal + position + count * sizeof(value_type), result.dataVal);
			}
			return result;
		}

		inline static constexpr size_type maxSize() noexcept {
			const size_type allocMax   = allocator::maxSize();
			const size_type storageMax = internal::max(allocMax, static_cast<size_type>(bufSize));
			return internal::min(static_cast<size_type>((std::numeric_limits<difference_type>::max)()), storageMax - 1);
		}

		inline constexpr iterator begin() noexcept {
			return iterator{ dataVal };
		}

		inline constexpr iterator end() noexcept {
			return iterator{ dataVal + sizeVal };
		}

		inline constexpr reverse_iterator rbegin() noexcept {
			return reverse_iterator{ end() };
		}

		inline constexpr reverse_iterator rend() noexcept {
			return reverse_iterator{ begin() };
		}

		inline constexpr const_iterator begin() const noexcept {
			return const_iterator{ dataVal };
		}

		inline constexpr const_iterator end() const noexcept {
			return const_iterator{ dataVal + sizeVal };
		}

		inline constexpr const_reverse_iterator rbegin() const noexcept {
			return const_reverse_iterator{ end() };
		}

		inline constexpr const_reverse_iterator rend() const noexcept {
			return const_reverse_iterator{ begin() };
		}

		template<typename... arg_types> inline size_type rfind(arg_types&&... args) const noexcept {
			return operator std::basic_string_view<value_type>().rfind(std::forward<arg_types>(args)...);
		}

		template<typename... arg_types> inline size_type find(arg_types&&... args) const noexcept {
			return operator std::basic_string_view<value_type>().find(std::forward<arg_types>(args)...);
		}

		template<typename... arg_types> inline size_type findFirstOf(arg_types&&... args) const noexcept {
			return operator std::basic_string_view<value_type>().find_first_of(std::forward<arg_types>(args)...);
		}

		template<typename... arg_types> inline size_type findLastOf(arg_types&&... args) const noexcept {
			return operator std::basic_string_view<value_type>().find_last_of(std::forward<arg_types>(args)...);
		}

		template<typename... arg_types> inline size_type findFirstNotOf(arg_types&&... args) const noexcept {
			return operator std::basic_string_view<value_type>().find_first_not_of(std::forward<arg_types>(args)...);
		}

		template<typename... arg_types> inline size_type findLastNotOf(arg_types&&... args) const noexcept {
			return operator std::basic_string_view<value_type>().find_last_not_of(std::forward<arg_types>(args)...);
		}

		inline void append(const string_base& newSize) noexcept {
			if JSONIFIER_UNLIKELY (sizeVal + newSize.size() >= capacityVal) {
				reserve(sizeVal + newSize.size());
			}
			if JSONIFIER_LIKELY (newSize.size() > 0) {
				std::copy(newSize.data(), newSize.data() + newSize.size(), dataVal + sizeVal);
				sizeVal += newSize.size();
				allocator::construct(&dataVal[sizeVal], value_type{});
			}
		}

		template<typename value_type_newer> inline void append(value_type_newer* values, uint64_t newSize) noexcept {
			if JSONIFIER_UNLIKELY (sizeVal + newSize >= capacityVal) {
				reserve(sizeVal + newSize);
			}
			if JSONIFIER_LIKELY (newSize > 0 && values) {
				std::copy(values, values + newSize, dataVal + sizeVal);
				sizeVal += newSize;
				allocator::construct(&dataVal[sizeVal], value_type{});
			}
		}

		template<typename Iterator01, typename Iterator02> inline void insert(Iterator01 where, Iterator02 start, Iterator02 end) noexcept {
			int64_t newSize = end - start;
			auto posNew		= where.operator->() - dataVal;

			if JSONIFIER_UNLIKELY (newSize <= 0) {
				return;
			}

			if JSONIFIER_UNLIKELY (sizeVal + newSize >= capacityVal) {
				reserve(sizeVal + newSize);
			}

			std::memmove(dataVal + posNew + newSize, dataVal + posNew, (sizeVal - posNew) * sizeof(value_type));
			std::copy(start.operator->(), start.operator->() + newSize * sizeof(value_type), dataVal + posNew);
			sizeVal += newSize;
			allocator::construct(&dataVal[sizeVal], value_type{});
		}

		inline void insert(iterator values, value_type toInsert) noexcept {
			auto positionNew = values - begin();
			if JSONIFIER_UNLIKELY (sizeVal + 1 >= capacityVal) {
				reserve((sizeVal + 1) * 2);
			}
			const auto newSize = sizeVal - positionNew;
			std::memmove(dataVal + positionNew + 1, dataVal + positionNew, newSize * sizeof(value_type));
			allocator::construct(&dataVal[positionNew], toInsert);
			++sizeVal;
		}

		inline void erase(size_type count) noexcept {
			if JSONIFIER_UNLIKELY (count == 0) {
				return;
			} else if JSONIFIER_LIKELY (count > sizeVal) {
				count = sizeVal;
			}
			traits_type::move(dataVal, dataVal + count, sizeVal - count);
			sizeVal -= count;
			allocator::construct(&dataVal[sizeVal], static_cast<value_type>(0x00u));
		}

		inline void erase(iterator count) noexcept {
			auto newSize = count.operator->() - dataVal;
			if JSONIFIER_UNLIKELY (newSize == 0) {
				return;
			} else if JSONIFIER_UNLIKELY (newSize > static_cast<int64_t>(sizeVal)) {
				newSize = static_cast<int64_t>(sizeVal);
			}
			traits_type::move(dataVal, dataVal + newSize, sizeVal - newSize);
			sizeVal -= newSize;
			allocator::construct(&dataVal[sizeVal], static_cast<value_type>(0x00u));
		}

		inline void emplace_back(value_type value) noexcept {
			if JSONIFIER_UNLIKELY (sizeVal + 1 >= capacityVal) {
				reserve((sizeVal + 2) * 4);
			}
			allocator::construct(&dataVal[sizeVal++], value);
			allocator::construct(&dataVal[sizeVal], value_type{});
		}

		inline const_reference at(size_type index) const noexcept {
			if JSONIFIER_UNLIKELY (index >= sizeVal) {
				throw std::runtime_error{ "Sorry, but that index is beyond the end of this string." };
			}
			return dataVal[index];
		}

		inline reference at(size_type index) noexcept {
			if JSONIFIER_UNLIKELY (index >= sizeVal) {
				throw std::runtime_error{ "Sorry, but that index is beyond the end of this string." };
			}
			return dataVal[index];
		}

		inline const_reference operator[](size_type index) const noexcept {
			return dataVal[index];
		}

		inline reference operator[](size_type index) noexcept {
			return dataVal[index];
		}

		inline operator std::basic_string_view<value_type>() const noexcept {
			return { dataVal, sizeVal };
		}

		template<typename value_type_newer> inline explicit operator string_base<value_type_newer>() const noexcept {
			string_base<value_type_newer> returnValue{};
			if JSONIFIER_LIKELY (sizeVal > 0) {
				returnValue.resize(sizeVal);
				std::memcpy(returnValue.data(), data(), returnValue.size());
			}
			return returnValue;
		}

		template<typename value_type_newer> inline explicit operator std::basic_string<value_type_newer>() const noexcept {
			std::basic_string<value_type_newer> returnValue{};
			if JSONIFIER_LIKELY (sizeVal > 0) {
				returnValue.resize(sizeVal);
				std::memcpy(returnValue.data(), data(), returnValue.size());
			}
			return returnValue;
		}

		inline virtual void clear() noexcept {
			if JSONIFIER_LIKELY (sizeVal > 0) {
				allocator::construct(dataVal, value_type{});
			}
			sizeVal = 0;
		}

		void resize(size_type newSize) {
			if JSONIFIER_LIKELY (static_cast<int64_t>(newSize) > 0) {
				if JSONIFIER_LIKELY (newSize > capacityVal) {
					pointer newPtr = allocator::allocate(newSize + 1);
					try {
						if JSONIFIER_LIKELY (dataVal) {
							if JSONIFIER_LIKELY (sizeVal > 0) {
								std::uninitialized_move(dataVal, dataVal + sizeVal, newPtr);
							}
							allocator::deallocate(dataVal, capacityVal);
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
				} else if JSONIFIER_LIKELY (newSize > sizeVal) {
					std::uninitialized_value_construct(dataVal + sizeVal, dataVal + capacityVal);
					allocator::construct(dataVal + newSize, value_type{});
					sizeVal = newSize;
				} else if JSONIFIER_LIKELY (newSize < sizeVal) {
					std::destroy(dataVal + newSize, dataVal + sizeVal);
					allocator::construct(dataVal + newSize, value_type{});
					sizeVal = newSize;
				}
			} else {
				std::destroy(dataVal, dataVal + sizeVal);
				sizeVal = 0;
			}
		}

		void reserve(size_type capacityNew) {
			if JSONIFIER_LIKELY (capacityNew > capacityVal) {
				pointer newPtr = allocator::allocate(capacityNew + 1);
				try {
					if JSONIFIER_LIKELY (dataVal) {
						if JSONIFIER_LIKELY (sizeVal > 0) {
							std::uninitialized_move(dataVal, dataVal + sizeVal, newPtr);
						}
						allocator::deallocate(dataVal, capacityVal);
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

		inline constexpr size_type capacity() const noexcept {
			return capacityVal;
		}

		inline constexpr size_type size() const noexcept {
			return sizeVal;
		}

		inline constexpr bool empty() const noexcept {
			return sizeVal == 0;
		}

		inline pointer data() const noexcept {
			return dataVal;
		}

		inline pointer data() noexcept {
			return dataVal;
		}

		template<uint64_t size> inline friend bool operator==(const string_base& lhs, const char (&rhs)[size]) noexcept {
			auto rhsLength = traits_type::length(rhs);
			return rhsLength == lhs.size() && internal::comparison::compare(lhs.data(), rhs, rhsLength);
		}

		template<concepts::string_t value_type_newer> inline friend bool operator==(const string_base& lhs, const value_type_newer& rhs) noexcept {
			if (lhs.size() == rhs.size()) {
				if (lhs.size() > 0) {
					return internal::comparison::compare(lhs.data(), rhs.data(), rhs.size());
				}
				return true;
			} else {
				return false;
			}
		}

		template<concepts::string_t value_type_newer> inline friend bool operator<(const string_base& lhs, const value_type_newer& rhs) noexcept {
			return lhs.size() < rhs.size();
		}

		template<typename string_base_new> inline void swap(string_base_new&& other) noexcept {
			std::swap(capacityVal, other.capacityVal);
			std::swap(sizeVal, other.sizeVal);
			std::swap(dataVal, other.dataVal);
		}

		template<typename value_type_newer, size_type size> inline friend string_base operator+(const value_type_newer (&lhs)[size], const string_base& rhs) noexcept {
			string_base newLhs{ lhs };
			newLhs += rhs;
			return newLhs;
		}

		template<typename value_type_newer, size_type size> inline friend string_base operator+=(const value_type_newer (&lhs)[size], const string_base& rhs) noexcept {
			string_base newLhs{ lhs };
			newLhs += rhs;
			return newLhs;
		}

		template<concepts::pointer_t string_type_new> inline friend string_base operator+(string_type_new&& lhs, const string_base& rhs) noexcept {
			string_base newLhs{ lhs };
			newLhs += rhs;
			return newLhs;
		}

		template<concepts::pointer_t string_type_new> inline friend string_base operator+=(string_type_new&& lhs, const string_base& rhs) noexcept {
			string_base newLhs{ lhs };
			newLhs += rhs;
			return newLhs;
		}

		inline string_base operator+(const value_type& rhs) noexcept {
			string_base newLhs{ *this };
			newLhs.emplace_back(rhs);
			return newLhs;
		}

		inline string_base& operator+=(const value_type& rhs) noexcept {
			emplace_back(rhs);
			return *this;
		}

		template<concepts::string_t string_type_new> inline string_base operator+(const string_type_new& rhs) const noexcept {
			string_base newLhs{ *this };
			newLhs += rhs;
			return newLhs;
		}

		template<concepts::string_t string_type_new> inline string_base& operator+=(const string_type_new& rhs) noexcept {
			append(static_cast<string_base>(rhs));
			return *this;
		}

		template<concepts::pointer_t string_type_new> inline string_base operator+(string_type_new&& rhs) noexcept {
			string_base newLhs{ *this };
			newLhs += rhs;
			return newLhs;
		}

		template<concepts::pointer_t string_type_new> inline string_base& operator+=(string_type_new&& rhs) noexcept {
			string_base newRhs{ rhs };
			*this += newRhs;
			return *this;
		}

		template<typename value_type_newer, size_type size> inline string_base operator+(const value_type_newer (&rhs)[size]) const noexcept {
			string_base newLhs{ *this };
			newLhs += rhs;
			return newLhs;
		}

		template<typename value_type_newer, size_type size> inline string_base& operator+=(const value_type_newer (&rhs)[size]) noexcept {
			string_base newRhs{ rhs };
			*this += newRhs;
			return *this;
		}

		inline virtual ~string_base() noexcept {
			reset();
		}

	  protected:
		size_type capacityVal{};
		size_type sizeVal{};
		pointer dataVal{};

		inline void reset() noexcept {
			if JSONIFIER_LIKELY (dataVal && capacityVal) {
				if JSONIFIER_LIKELY (sizeVal) {
					std::destroy(dataVal, dataVal + sizeVal);
					sizeVal = 0;
				}
				allocator::deallocate(dataVal, capacityVal);
				dataVal		= nullptr;
				capacityVal = 0;
			}
		}
	};

	using string = string_base<char>;

	template<typename value_type> std::ostream& operator<<(std::ostream& os, const string_base<value_type>& stringNew) noexcept {
		os << stringNew.data();
		return os;
	}
}// namespace jsonifier
