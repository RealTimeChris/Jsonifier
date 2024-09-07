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

namespace jsonifier_internal {

	template<typename value_type>
	concept not_uint8_t = !std::same_as<unwrap_t<value_type>, uint8_t>;

	template<typename value_type> class char_traits;

	template<not_uint8_t value_type> class char_traits<value_type> : public std::char_traits<unwrap_t<value_type>> {};

	template<jsonifier::concepts::uint8_type value_type_new> class char_traits<value_type_new> {
	  public:
		using value_type	= uint8_t;
		using pointer		= value_type*;
		using const_pointer = const value_type*;
		using size_type		= uint64_t;

		static constexpr void move(pointer firstNew, pointer first2, size_type count) noexcept {
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

		static constexpr size_type length(const_pointer first) noexcept {
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
		using const_iterator		 = jsonifier_internal::iterator<const value_type>;
		using reverse_iterator		 = std::reverse_iterator<iterator>;
		using const_reverse_iterator = std::reverse_iterator<const_iterator>;
		using size_type				 = uint64_t;
		using allocator				 = jsonifier_internal::alloc_wrapper<value_type>;
		using traits_type			 = jsonifier_internal::char_traits<value_type>;

		JSONIFIER_ALWAYS_INLINE string_base() noexcept : capacityVal{}, sizeVal{}, dataVal{} {
			if constexpr (newerSize > 0) {
				resize(newerSize);
			}
		};

		static constexpr size_type bufSize = 16 / sizeof(value_type) < 1 ? 1 : 16 / sizeof(value_type);
		static constexpr size_type npos{ std::numeric_limits<size_type>::max() };

		JSONIFIER_ALWAYS_INLINE string_base& operator=(string_base&& other) noexcept {
			if (this != &other) [[likely]] {
				string_base newValue{ other };
				swap(newValue);
			}
			return *this;
		}

		JSONIFIER_ALWAYS_INLINE explicit string_base(string_base&& other) noexcept : capacityVal{}, sizeVal{}, dataVal{} {
			swap(other);
		};

		JSONIFIER_ALWAYS_INLINE string_base& operator=(const string_base& other) noexcept {
			if (this != &other) [[likely]] {
				string_base newValue{ other };
				swap(newValue);
			}
			return *this;
		}

		JSONIFIER_ALWAYS_INLINE string_base(const string_base& other) noexcept : capacityVal{}, sizeVal{}, dataVal{} {
			size_type newSize = other.size();
			if (newSize > 0 && newSize < maxSize()) [[likely]] {
				reserve(newSize);
				sizeVal = newSize;
				std::uninitialized_copy(other.data(), other.data() + newSize, dataVal);
				allocator::construct(&(*this)[newSize], value_type{});
			}
		}

		template<jsonifier::concepts::string_t value_type_newer> JSONIFIER_ALWAYS_INLINE string_base& operator=(value_type_newer&& other) noexcept {
			string_base newValue{ other };
			swap(newValue);
			return *this;
		}

		template<jsonifier::concepts::string_t value_type_newer> JSONIFIER_ALWAYS_INLINE string_base(value_type_newer&& other) noexcept : capacityVal{}, sizeVal{}, dataVal{} {
			size_type newSize = other.size() * (sizeof(typename jsonifier_internal::unwrap_t<value_type_newer>::value_type) / sizeof(value_type));
			if (newSize > 0 && newSize < maxSize()) [[likely]] {
				reserve(newSize);
				sizeVal = newSize;
				std::uninitialized_copy(other.data(), other.data() + newSize, dataVal);
				allocator::construct(&(*this)[newSize], value_type{});
			}
		}

		template<jsonifier::concepts::pointer_t value_type_newer> JSONIFIER_ALWAYS_INLINE string_base& operator=(value_type_newer other) noexcept {
			string_base newValue{ std::forward<value_type_newer>(other) };
			swap(newValue);
			return *this;
		}

		template<jsonifier::concepts::pointer_t value_type_newer> JSONIFIER_ALWAYS_INLINE string_base(value_type_newer other) noexcept : capacityVal{}, sizeVal{}, dataVal{} {
			if (other) {
				auto newSize = jsonifier_internal::char_traits<std::remove_pointer_t<value_type_newer>>::length(other) *
					(sizeof(std::remove_pointer_t<value_type_newer>) / sizeof(value_type));
				if (newSize > 0 && newSize < maxSize()) [[likely]] {
					reserve(newSize);
					sizeVal = newSize;
					std::uninitialized_copy(other, other + newSize, dataVal);
					allocator::construct(&(*this)[newSize], value_type{});
				}
			}
		}

		template<jsonifier::concepts::char_t value_type_newer> JSONIFIER_ALWAYS_INLINE string_base& operator=(value_type_newer other) noexcept {
			emplace_back(static_cast<value_type>(other));
			return *this;
		}

		template<jsonifier::concepts::char_t value_type_newer> JSONIFIER_ALWAYS_INLINE string_base(value_type_newer other) noexcept : capacityVal{}, sizeVal{}, dataVal{} {
			*this = other;
		}

		JSONIFIER_ALWAYS_INLINE string_base(const_pointer other, uint64_t newSize) noexcept : capacityVal{}, sizeVal{}, dataVal{} {
			if (newSize > 0 && newSize < maxSize()) [[likely]] {
				reserve(newSize);
				sizeVal = newSize;
				std::uninitialized_copy(other, other + newSize, dataVal);
				allocator::construct(&(*this)[newSize], value_type{});
			}
		}

		JSONIFIER_ALWAYS_INLINE string_base(const_iterator other, uint64_t newSize) noexcept : capacityVal{}, sizeVal{}, dataVal{} {
			if (newSize > 0 && newSize < maxSize()) [[likely]] {
				reserve(newSize);
				sizeVal = newSize;
				std::uninitialized_copy(other.operator->(), other.operator->() + newSize, dataVal);
				allocator::construct(&(*this)[newSize], value_type{});
			}
		}

		JSONIFIER_ALWAYS_INLINE string_base substr(size_type position, size_type count = std::numeric_limits<size_type>::max()) const noexcept {
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

		JSONIFIER_ALWAYS_INLINE constexpr size_type maxSize() noexcept {
			const size_type allocMax   = allocator::maxSize();
			const size_type storageMax = jsonifier_internal::max(allocMax, static_cast<size_type>(bufSize));
			return std::min(static_cast<size_type>((std::numeric_limits<difference_type>::max)()), storageMax - 1);
		}

		JSONIFIER_ALWAYS_INLINE constexpr iterator begin() noexcept {
			return iterator{ dataVal };
		}

		JSONIFIER_ALWAYS_INLINE constexpr iterator end() noexcept {
			return iterator{ dataVal + sizeVal };
		}

		JSONIFIER_ALWAYS_INLINE constexpr reverse_iterator rbegin() noexcept {
			return reverse_iterator{ end() };
		}

		JSONIFIER_ALWAYS_INLINE constexpr reverse_iterator rend() noexcept {
			return reverse_iterator{ begin() };
		}

		JSONIFIER_ALWAYS_INLINE constexpr const_iterator begin() const noexcept {
			return const_iterator{ dataVal };
		}

		JSONIFIER_ALWAYS_INLINE constexpr const_iterator end() const noexcept {
			return const_iterator{ dataVal + sizeVal };
		}

		JSONIFIER_ALWAYS_INLINE constexpr const_reverse_iterator rbegin() const noexcept {
			return const_reverse_iterator{ end() };
		}

		JSONIFIER_ALWAYS_INLINE constexpr const_reverse_iterator rend() const noexcept {
			return const_reverse_iterator{ begin() };
		}

		template<typename... arg_types> JSONIFIER_ALWAYS_INLINE size_type rfind(arg_types&&... args) const noexcept {
			return operator std::basic_string_view<value_type>().rfind(std::forward<arg_types>(args)...);
		}

		template<typename... arg_types> JSONIFIER_ALWAYS_INLINE size_type find(arg_types&&... args) const noexcept {
			return operator std::basic_string_view<value_type>().find(std::forward<arg_types>(args)...);
		}

		template<typename... arg_types> JSONIFIER_ALWAYS_INLINE size_type findFirstOf(arg_types&&... args) const noexcept {
			return operator std::basic_string_view<value_type>().find_first_of(std::forward<arg_types>(args)...);
		}

		template<typename... arg_types> JSONIFIER_ALWAYS_INLINE size_type findLastOf(arg_types&&... args) const noexcept {
			return operator std::basic_string_view<value_type>().find_last_of(std::forward<arg_types>(args)...);
		}

		template<typename... arg_types> JSONIFIER_ALWAYS_INLINE size_type findFirstNotOf(arg_types&&... args) const noexcept {
			return operator std::basic_string_view<value_type>().find_first_not_of(std::forward<arg_types>(args)...);
		}

		template<typename... arg_types> JSONIFIER_ALWAYS_INLINE size_type findLastNotOf(arg_types&&... args) const noexcept {
			return operator std::basic_string_view<value_type>().find_last_not_of(std::forward<arg_types>(args)...);
		}

		JSONIFIER_ALWAYS_INLINE void append(const string_base& newSize) noexcept {
			if (sizeVal + newSize.size() >= capacityVal) [[unlikely]] {
				reserve(sizeVal + newSize.size());
			}
			if (newSize.size() > 0) [[likely]] {
				std::copy(newSize.data(), newSize.data() + newSize.size(), dataVal + sizeVal);
				sizeVal += newSize.size();
				allocator::construct(&dataVal[sizeVal], value_type{});
			}
		}

		template<typename value_type_newer> JSONIFIER_ALWAYS_INLINE void append(value_type_newer* values, uint64_t newSize) noexcept {
			if (sizeVal + newSize >= capacityVal) [[unlikely]] {
				reserve(sizeVal + newSize);
			}
			if (newSize > 0 && values) [[likely]] {
				std::copy(values, values + newSize, dataVal + sizeVal);
				sizeVal += newSize;
				allocator::construct(&dataVal[sizeVal], value_type{});
			}
		}

		template<typename Iterator01, typename Iterator02> JSONIFIER_ALWAYS_INLINE void insert(Iterator01 where, Iterator02 start, Iterator02 end) noexcept {
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

		JSONIFIER_ALWAYS_INLINE void insert(iterator values, value_type toInsert) noexcept {
			auto positionNew = values - begin();
			if (sizeVal + 1 >= capacityVal) [[unlikely]] {
				reserve((sizeVal + 1) * 2);
			}
			auto newSize = sizeVal - positionNew;
			std::memmove(dataVal + positionNew + 1, dataVal + positionNew, newSize * sizeof(value_type));
			allocator::construct(&dataVal[positionNew], toInsert);
			++sizeVal;
		}

		JSONIFIER_ALWAYS_INLINE void erase(size_type count) noexcept {
			if (count == 0) [[unlikely]] {
				return;
			} else if (count > sizeVal) [[likely]] {
				count = sizeVal;
			}
			traits_type::move(dataVal, dataVal + count, sizeVal - count);
			sizeVal -= count;
			allocator::construct(&dataVal[sizeVal], static_cast<value_type>(0x00u));
		}

		JSONIFIER_ALWAYS_INLINE void erase(iterator count) noexcept {
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

		JSONIFIER_ALWAYS_INLINE void emplace_back(value_type value) noexcept {
			if (sizeVal + 1 >= capacityVal) [[unlikely]] {
				reserve((sizeVal + 2) * 4);
			}
			allocator::construct(&dataVal[sizeVal++], value);
			allocator::construct(&dataVal[sizeVal], value_type{});
		}

		JSONIFIER_ALWAYS_INLINE const_reference at(size_type index) const noexcept {
			if (index >= sizeVal) [[unlikely]] {
				throw std::runtime_error{ "Sorry, but that index is beyond the end of this string." };
			}
			return dataVal[index];
		}

		JSONIFIER_ALWAYS_INLINE reference at(size_type index) noexcept {
			if (index >= sizeVal) [[unlikely]] {
				throw std::runtime_error{ "Sorry, but that index is beyond the end of this string." };
			}
			return dataVal[index];
		}

		JSONIFIER_ALWAYS_INLINE const_reference operator[](size_type index) const noexcept {
			return dataVal[index];
		}

		JSONIFIER_ALWAYS_INLINE reference operator[](size_type index) noexcept {
			return dataVal[index];
		}

		JSONIFIER_ALWAYS_INLINE operator std::basic_string_view<value_type>() const noexcept {
			return { dataVal, sizeVal };
		}

		template<typename value_type_newer> JSONIFIER_ALWAYS_INLINE explicit operator jsonifier::string_base<value_type_newer>() const noexcept {
			jsonifier::string_base<value_type_newer> returnValue{};
			if (sizeVal > 0) [[likely]] {
				returnValue.resize(sizeVal);
				std::memcpy(returnValue.data(), data(), returnValue.size());
			}
			return returnValue;
		}

		template<typename value_type_newer> JSONIFIER_ALWAYS_INLINE explicit operator std::basic_string<value_type_newer>() const noexcept {
			std::basic_string<value_type_newer> returnValue{};
			if (sizeVal > 0) [[likely]] {
				returnValue.resize(sizeVal);
				std::memcpy(returnValue.data(), data(), returnValue.size());
			}
			return returnValue;
		}

		JSONIFIER_ALWAYS_INLINE virtual void clear() noexcept {
			if (sizeVal > 0) [[likely]] {
				allocator::construct(dataVal, static_cast<value_type>(0x00u));
			}
			sizeVal = 0;
		}

		JSONIFIER_ALWAYS_INLINE void resize(size_type newSize) {
			if (static_cast<int64_t>(newSize) > 0) [[likely]] {
				if (newSize > capacityVal) [[likely]] {
					pointer newPtr = allocator::allocate(newSize + 1);
					try {
						if (dataVal) [[likely]] {
							if (sizeVal > 0) [[likely]] {
								std::uninitialized_move(dataVal, dataVal + sizeVal, newPtr);
							}
							allocator::deallocate(dataVal);
						}
					} catch (...) {
						allocator::deallocate(newPtr);
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

		JSONIFIER_ALWAYS_INLINE void reserve(size_type capacityNew) {
			if (capacityNew > capacityVal) [[likely]] {
				pointer newPtr = allocator::allocate(capacityNew + 1);
				try {
					if (dataVal) [[likely]] {
						if (sizeVal > 0) [[likely]] {
							std::uninitialized_move(dataVal, dataVal + sizeVal, newPtr);
						}
						allocator::deallocate(dataVal);
					}
				} catch (...) {
					allocator::deallocate(newPtr);
					throw;
				}
				capacityVal = capacityNew;
				dataVal		= newPtr;
				allocator::construct(&dataVal[sizeVal], value_type{});
			}
		}

		JSONIFIER_ALWAYS_INLINE constexpr size_type capacity() const noexcept {
			return capacityVal;
		}

		JSONIFIER_ALWAYS_INLINE constexpr size_type size() const noexcept {
			return sizeVal;
		}

		JSONIFIER_ALWAYS_INLINE constexpr bool empty() const noexcept {
			return sizeVal == 0;
		}

		JSONIFIER_ALWAYS_INLINE pointer data() const noexcept {
			return dataVal;
		}

		JSONIFIER_ALWAYS_INLINE pointer data() noexcept {
			return dataVal;
		}

		template<jsonifier::concepts::pointer_t value_type_newer>
		JSONIFIER_ALWAYS_INLINE friend std::enable_if_t<!std::is_array_v<value_type_newer>, bool> operator==(const string_base& lhs, const value_type_newer& rhs) noexcept {
			auto rhsLength = traits_type::length(rhs);
			return rhsLength == lhs.size() && jsonifier_internal::compare(lhs.data(), rhs, rhsLength);
		}

		template<jsonifier::concepts::string_t value_type_newer> JSONIFIER_ALWAYS_INLINE friend bool operator==(const string_base& lhs, const value_type_newer& rhs) noexcept {
			if (lhs.size() == rhs.size()) {
				if (lhs.size() > 0) {
					return jsonifier_internal::compare(lhs.data(), rhs.data(), rhs.size());
				}
				return true;
			} else {
				return false;
			}
		}

		template<jsonifier::concepts::string_t value_type_newer> JSONIFIER_ALWAYS_INLINE friend bool operator<(const string_base& lhs, const value_type_newer& rhs) noexcept {
			return lhs.size() < rhs.size();
		}

		template<typename string_base_new> JSONIFIER_ALWAYS_INLINE void swap(string_base_new&& other) noexcept {
			std::swap(capacityVal, other.capacityVal);
			std::swap(sizeVal, other.sizeVal);
			std::swap(dataVal, other.dataVal);
		}

		template<typename value_type_newer, size_type size>
		JSONIFIER_ALWAYS_INLINE friend string_base operator+(const value_type_newer (&lhs)[size], const string_base& rhs) noexcept {
			string_base newLhs{ lhs };
			newLhs += rhs;
			return newLhs;
		}

		template<typename value_type_newer, size_type size>
		JSONIFIER_ALWAYS_INLINE friend string_base operator+=(const value_type_newer (&lhs)[size], const string_base& rhs) noexcept {
			string_base newLhs{ lhs };
			newLhs += rhs;
			return newLhs;
		}

		template<jsonifier::concepts::pointer_t string_type_new> JSONIFIER_ALWAYS_INLINE friend string_base operator+(string_type_new&& lhs, const string_base& rhs) noexcept {
			string_base newLhs{ lhs };
			newLhs += rhs;
			return newLhs;
		}

		template<jsonifier::concepts::pointer_t string_type_new> JSONIFIER_ALWAYS_INLINE friend string_base operator+=(string_type_new&& lhs, const string_base& rhs) noexcept {
			string_base newLhs{ lhs };
			newLhs += rhs;
			return newLhs;
		}

		JSONIFIER_ALWAYS_INLINE string_base operator+(const value_type& rhs) noexcept {
			string_base newLhs{ *this };
			newLhs.emplace_back(rhs);
			return newLhs;
		}

		JSONIFIER_ALWAYS_INLINE string_base& operator+=(const value_type& rhs) noexcept {
			emplace_back(rhs);
			return *this;
		}

		template<jsonifier::concepts::string_t string_type_new> JSONIFIER_ALWAYS_INLINE string_base operator+(const string_type_new& rhs) const noexcept {
			string_base newLhs{ *this };
			newLhs += rhs;
			return newLhs;
		}

		template<jsonifier::concepts::string_t string_type_new> JSONIFIER_ALWAYS_INLINE string_base& operator+=(const string_type_new& rhs) noexcept {
			append(static_cast<string_base>(rhs));
			return *this;
		}

		template<jsonifier::concepts::pointer_t string_type_new> JSONIFIER_ALWAYS_INLINE string_base operator+(string_type_new&& rhs) noexcept {
			string_base newLhs{ *this };
			newLhs += rhs;
			return newLhs;
		}

		template<jsonifier::concepts::pointer_t string_type_new> JSONIFIER_ALWAYS_INLINE string_base& operator+=(string_type_new&& rhs) noexcept {
			string_base newRhs{ rhs };
			*this += newRhs;
			return *this;
		}

		template<typename value_type_newer, size_type size> JSONIFIER_ALWAYS_INLINE string_base operator+(const value_type_newer (&rhs)[size]) const noexcept {
			string_base newLhs{ *this };
			newLhs += rhs;
			return newLhs;
		}

		template<typename value_type_newer, size_type size> JSONIFIER_ALWAYS_INLINE string_base& operator+=(const value_type_newer (&rhs)[size]) noexcept {
			string_base newRhs{ rhs };
			*this += newRhs;
			return *this;
		}

		JSONIFIER_ALWAYS_INLINE virtual ~string_base() noexcept {
			reset();
		}

	  protected:
		size_type capacityVal{};
		size_type sizeVal{};
		pointer dataVal{};

		JSONIFIER_ALWAYS_INLINE void reset() noexcept {
			if (dataVal && capacityVal) [[likely]] {
				if (sizeVal) [[likely]] {
					std::destroy(dataVal, dataVal + sizeVal);
					sizeVal = 0;
				}
				allocator::deallocate(dataVal);
				dataVal		= nullptr;
				capacityVal = 0;
			}
		}
	};

	using string = string_base<char>;

	template<typename value_type> JSONIFIER_ALWAYS_INLINE std::ostream& operator<<(std::ostream& os, const string_base<value_type>& string) noexcept {
		os << string.operator typename std::string();
		return os;
	}
}// namespace jsonifier

namespace jsonifier_internal {

	static thread_local jsonifier::string_base<char, 1024 * 1024 * 4> stringBuffer{};

}