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
#include <jsonifier/Tables.hpp>

namespace jsonifier_internal {

	template<typename value_type01, typename value_type02> constexpr bool stringConstCompare(const value_type01& string01, const value_type02& string02) {
		if (string01.size() != string02.size()) [[unlikely]] {
			return false;
		}
		using char_t = value_type01::value_type;
		for (uint64_t x = 0; x < string01.size(); ++x) {
			if (string01[x] != static_cast<char_t>(string02[x])) [[unlikely]] {
				return false;
			}
		}
		return true;
	}

	template<typename value_type> class char_traits : public std::char_traits<value_type> {};

	template<> class char_traits<uint8_t> {
	  public:
		using value_type	= uint8_t;
		using pointer		= value_type*;
		using const_pointer = const pointer;
		using size_type		= uint64_t;

		static constexpr void move(pointer first1, pointer first2, size_type count) {
			if (std::is_constant_evaluated()) {
				bool loopForward = true;

				for (const value_type* source = first2; source != first2 + count; ++source) {
					if (first1 == source) {
						loopForward = false;
					}
				}

				if (loopForward) {
					for (uint64_t index = 0; index != count; ++index) {
						first1[index] = first2[index];
					}
				} else {
					for (uint64_t index = count; index != 0; --index) {
						first1[index - 1] = first2[index - 1];
					}
				}

				return;
			}
			std::memmove(first1, first2, count * sizeof(value_type));
		}

		static constexpr size_type length(const value_type* first) {
			size_type count = 0;
			while (*first != value_type{}) {
				++count;
				++first;
			}

			return count;
		}
	};

}// namespace jsonifier_internal

namespace jsonifier {

	template<typename value_type> class string_view_base;

	template<typename value_type_new> class string_base : protected jsonifier_internal::alloc_wrapper<value_type_new> {
	  public:
		using value_type			 = value_type_new;
		using pointer				 = value_type*;
		using const_pointer			 = const value_type*;
		using reference				 = value_type&;
		using const_reference		 = const value_type&;
		using iterator				 = jsonifier_internal::iterator<value_type>;
		using const_iterator		 = jsonifier_internal::iterator<const value_type>;
		using reverse_iterator		 = std::reverse_iterator<iterator>;
		using const_reverse_iterator = std::reverse_iterator<const_iterator>;
		using size_type				 = uint64_t;
		using allocator				 = jsonifier_internal::alloc_wrapper<value_type>;
		using traits_type			 = jsonifier_internal::char_traits<value_type>;

		inline string_base() : capacityVal{}, sizeVal{}, dataVal{} {};

		static constexpr size_type npos{ std::numeric_limits<size_type>::max() };

		inline string_base& operator=(string_base&& other) noexcept {
			if (this != &other) {
				reset();
				swap(other);
			}
			return *this;
		}

		inline explicit string_base(string_base&& other) noexcept : capacityVal{}, sizeVal{}, dataVal{} {
			*this = std::move(other);
		}

		inline string_base& operator=(const string_base& other) {
			if (this != &other) {
				size_type sizeNew = other.size();
				if (sizeNew > 0) {
					reset();
					string_base temp{};
					temp.resize(sizeNew);
					std::memcpy(temp.dataVal, other.data(), sizeNew);
					temp.getAlloc().construct(&temp[sizeNew], '\0');
					swap(temp);
				}
			}
			return *this;
		}

		inline string_base(const string_base& other) : capacityVal{}, sizeVal{}, dataVal{} {
			*this = other;
		}

		template<jsonifier_internal::string_t value_type_newer> inline string_base& operator=(value_type_newer&& other) {
			size_type sizeNew = other.size() * (sizeof(typename jsonifier_internal::ref_unwrap<value_type_newer>::value_type) / sizeof(value_type));
			if (sizeNew > 0) {
				reset();
				string_base temp{};
				temp.resize(sizeNew);
				std::memcpy(temp.dataVal, other.data(), sizeNew);
				temp.getAlloc().construct(&temp[sizeNew], '\0');
				swap(temp);
			}
			return *this;
		}

		template<jsonifier_internal::string_t value_type_newer> inline explicit string_base(value_type_newer&& other) : capacityVal{}, sizeVal{}, dataVal{} {
			*this = std::forward<value_type_newer>(other);
		}

		template<jsonifier_internal::char_array_t value_type_newer> inline string_base& operator=(const value_type_newer& other) {
			size_type sizeNew = std::size(other) - 1 * (sizeof(value_type_new) / sizeof(value_type));
			if (sizeNew > 0) {
				reset();
				string_base temp{};
				temp.resize(sizeNew);
				std::memcpy(temp.dataVal, other, sizeNew);
				temp.getAlloc().construct(&temp[sizeNew], '\0');
				swap(temp);
			}
			return *this;
		}

		template<jsonifier_internal::char_array_t value_type_newer> inline string_base(const value_type_newer& other) : capacityVal{}, sizeVal{}, dataVal{} {
			*this = other;
		}

		template<jsonifier_internal::pointer_t value_type_newer> inline string_base& operator=(value_type_newer&& other) {
			auto sizeNew =
				jsonifier_internal::char_traits<std::remove_pointer_t<value_type_newer>>::length(other) * (sizeof(std::remove_pointer_t<value_type_newer>) / sizeof(value_type));
			if (sizeNew) [[likely]] {
				reset();
				string_base temp{};
				temp.resize(sizeNew);
				std::memcpy(temp.dataVal, other, sizeNew);
				temp.getAlloc().construct(&temp[sizeNew], '\0');
				swap(temp);
			}
			return *this;
		}

		template<jsonifier_internal::pointer_t value_type_newer> inline string_base(value_type_newer&& other) : capacityVal{}, sizeVal{}, dataVal{} {
			*this = std::forward<value_type_newer>(other);
		}

		inline string_base(const_pointer other, uint64_t sizeNew) : capacityVal{}, sizeVal{}, dataVal{} {
			if (sizeNew) [[likely]] {
				reset();
				string_base temp{};
				temp.resize(sizeNew);
				std::memcpy(temp.dataVal, other, sizeNew);
				temp.getAlloc().construct(&temp[sizeNew], '\0');
				swap(temp);
			}
		}

		inline string_base substr(size_type position, size_type count = std::numeric_limits<size_type>::max()) const {
			if (position >= sizeVal) {
				throw std::out_of_range("Substring position is out of range.");
			}

			count = std::min(count, sizeVal - position);

			string_base result{};
			if (count > 0) {
				result.resize(count);
				std::memcpy(result.dataVal, dataVal + position, count * sizeof(value_type));
				result.getAlloc().construct(&result.dataVal[count], static_cast<value_type>('\0'));
			}
			return result;
		}

		inline iterator begin() {
			return iterator(dataVal);
		}

		inline iterator end() {
			return iterator(dataVal + sizeVal);
		}

		inline const_iterator begin() const {
			return const_iterator(dataVal);
		}

		inline const_iterator end() const {
			return const_iterator(dataVal + sizeVal);
		}

		inline reverse_iterator rbegin() {
			return reverse_iterator(end());
		}

		inline reverse_iterator rend() {
			return reverse_iterator(begin());
		}

		inline const_iterator cbegin() const {
			return const_iterator(begin());
		}

		inline const_iterator cend() const {
			return const_iterator(end());
		}

		inline const_reverse_iterator crbegin() const {
			return const_reverse_iterator(cend());
		}

		inline const_reverse_iterator crend() const {
			return const_reverse_iterator(cbegin());
		}

		inline size_type find(const_pointer args, size_type position = 0) const {
			auto newSize = traits_type::length(args);
			if (position + newSize > sizeVal) {
				return npos;
			}
			auto foundValue = jsonifier_internal::find(dataVal + position, sizeVal - position, args, newSize);
			return foundValue == npos ? npos : foundValue + position;
		}

		template<jsonifier_internal::char_t value_type_newer> inline size_type find(value_type_newer args, size_type position = 0) const {
			value_type newValue{ static_cast<value_type>(args) };
			if (position + 1 > sizeVal) {
				return npos;
			}
			auto foundValue = jsonifier_internal::find(dataVal + position, sizeVal - position, &newValue, 1);
			return foundValue == npos ? npos : foundValue + position;
		}

		template<jsonifier_internal::string_t value_type_newer> inline size_type find(const value_type_newer& args, size_type position = 0) const {
			if (position + args.size() > sizeVal) {
				return npos;
			}
			auto foundValue = jsonifier_internal::find(dataVal + position, sizeVal - position, args.data(), args.size());
			return foundValue == npos ? npos : foundValue + position;
		}

		template<typename... ArgTypes> inline size_type findFirstOf(ArgTypes&&... args) const {
			return this->operator std::basic_string_view<value_type>().find_first_of(std::forward<ArgTypes>(args)...);
		}

		template<typename... ArgTypes> inline size_type findLastOf(ArgTypes&&... args) const {
			return this->operator std::basic_string_view<value_type>().find_last_of(std::forward<ArgTypes>(args)...);
		}

		template<typename... ArgTypes> inline size_type findFirstNotOf(ArgTypes&&... args) const {
			return this->operator std::basic_string_view<value_type>().find_first_not_of(std::forward<ArgTypes>(args)...);
		}

		template<typename... ArgTypes> inline size_type findLastNotOf(ArgTypes&&... args) const {
			return this->operator std::basic_string_view<value_type>().find_last_not_of(std::forward<ArgTypes>(args)...);
		}

		inline void append(const string_base& sizeNew) {
			if (sizeVal + sizeNew.size() > capacityVal) {
				reserve(sizeVal + sizeNew.size());
			}
			std::memcpy(dataVal + sizeVal, sizeNew.data(), sizeNew.size());
			sizeVal += sizeNew.size();
			getAlloc().construct(&dataVal[sizeVal], static_cast<value_type>('\0'));
		}

		template<typename value_type_newer> inline void append(const value_type_newer* valuesNew, uint64_t sizeNew) {
			if (sizeVal + sizeNew > capacityVal) {
				reserve(sizeVal + sizeNew);
			}
			std::memcpy(dataVal + sizeVal, valuesNew, sizeNew);
			sizeVal += sizeNew;
			getAlloc().construct(&dataVal[sizeVal], static_cast<value_type>('\0'));
		}

		template<typename Iterator01, typename Iterator02> inline void insert(Iterator01 where, Iterator02 start, Iterator02 end) {
			auto sizeNew = end - start;
			auto posNew	 = where.operator->() - dataVal;

			if (static_cast<int64_t>(sizeNew) <= 0) {
				return;
			}

			if (sizeVal + sizeNew > capacityVal) {
				reserve(sizeVal + sizeNew);
			}

			std::memmove(dataVal + posNew + sizeNew, dataVal + posNew, (sizeVal - posNew) * sizeof(value_type));
			std::memcpy(dataVal + posNew, start.operator->(), sizeNew * sizeof(value_type));
			sizeVal += sizeNew;
			getAlloc().construct(&dataVal[sizeVal], static_cast<value_type>('\0'));
		}

		inline void insert(iterator valuesNew, value_type toInsert) {
			auto positionNew = valuesNew - begin();
			if (sizeVal + 1 > capacityVal) {
				reserve((sizeVal + 1) * 2);
			}
			auto sizeNew = sizeVal - positionNew;
			std::memmove(dataVal + positionNew + 1, dataVal + positionNew, sizeNew * sizeof(value_type));
			getAlloc().construct(&dataVal[positionNew], toInsert);
			++sizeVal;
		}

		inline void erase(size_type count) {
			if (count == 0) {
				return;
			} else if (count > sizeVal) {
				count = sizeVal;
			}
			traits_type::move(dataVal, dataVal + count, sizeVal - count);
			sizeVal -= count;
			getAlloc().construct(&dataVal[sizeVal], static_cast<value_type>('\0'));
		}

		inline void erase(iterator count) {
			auto sizeNew = count - dataVal;
			if (sizeNew == 0) {
				return;
			} else if (sizeNew > static_cast<int64_t>(sizeVal)) {
				sizeNew = static_cast<int64_t>(sizeVal);
			}
			traits_type::move(dataVal, dataVal + sizeNew, sizeVal - sizeNew);
			sizeVal -= sizeNew;
			getAlloc().construct(&dataVal[sizeVal], static_cast<value_type>('\0'));
		}

		inline void pushBack(value_type value) {
			if (sizeVal + 1 >= capacityVal) {
				reserve((sizeVal + 2) * 4);
			}
			getAlloc().construct(&dataVal[sizeVal++], value);
			getAlloc().construct(&dataVal[sizeVal], static_cast<value_type>('\0'));
		}

		inline const_reference at(size_type index) const {
			if (index >= sizeVal) {
				throw std::runtime_error{ "Sorry, but that index is beyond the end of this string." };
			}
			return dataVal[index];
		}

		inline reference at(size_type index) {
			if (index >= sizeVal) {
				throw std::runtime_error{ "Sorry, but that index is beyond the end of this string." };
			}
			return dataVal[index];
		}

		inline const_reference operator[](size_type index) const {
			return dataVal[index];
		}

		inline reference operator[](size_type index) {
			return dataVal[index];
		}

		inline std::string_view stringView(size_type offSet, size_type count) const {
			return std::string_view{ data() + offSet, count };
		}

		inline operator std::basic_string_view<value_type>() const {
			return { dataVal, sizeVal };
		}

		template<typename value_type_newer> inline explicit operator std::basic_string<value_type_newer>() const {
			std::basic_string<value_type_newer> returnValue{};
			returnValue.resize(sizeVal);
			std::memcpy(returnValue.data(), data(), returnValue.size());
			return returnValue;
		}

		inline void clear() {
			sizeVal = 0;
		}

		inline size_type maxSize() const {
			return std::numeric_limits<size_type>::max();
		}

		inline void resize(size_type sizeNew) {
			if (sizeNew > 0) [[likely]] {
				if (sizeNew > capacityVal) [[likely]] {
					const size_type sizeNewer = (sizeNew + JSONIFIER_ALIGNMENT - 1) / JSONIFIER_ALIGNMENT * JSONIFIER_ALIGNMENT;
					pointer newPtr			  = getAlloc().allocate(sizeNewer + 1);
					try {
						if (dataVal) [[likely]] {
							std::uninitialized_move(dataVal, dataVal + sizeVal, newPtr);
							getAlloc().deallocate(dataVal, capacityVal + 1);
						}
					} catch (...) {
						getAlloc().deallocate(newPtr, sizeNewer + 1);
						throw;
					}
					capacityVal = sizeNewer;
					dataVal		= newPtr;
					std::uninitialized_value_construct(dataVal + sizeVal, dataVal + capacityVal);
				} else if (sizeNew > sizeVal) [[unlikely]] {
					std::uninitialized_value_construct(dataVal + sizeVal, dataVal + capacityVal);
				}
				sizeVal = sizeNew;
				getAlloc().construct(&dataVal[sizeVal], static_cast<value_type>('\0'));
			} else {
				sizeVal = 0;
			}
		}


		inline void reserve(size_type sizeNew) {
			if (sizeNew > capacityVal) [[likely]] {
				const size_type sizeNewer = (sizeNew + JSONIFIER_ALIGNMENT - 1) / JSONIFIER_ALIGNMENT * JSONIFIER_ALIGNMENT;
				pointer newPtr			  = getAlloc().allocate(sizeNewer + 1);
				try {
					if (dataVal && sizeVal) [[likely]] {
						std::uninitialized_move(dataVal, dataVal + sizeVal, newPtr);
						getAlloc().deallocate(dataVal, capacityVal + 1);
					}
				} catch (...) {
					getAlloc().deallocate(newPtr, sizeNewer + 1);
					throw;
				}
				capacityVal = sizeNewer;
				dataVal		= newPtr;
			}
		}

		inline size_type capacity() const {
			return capacityVal;
		}

		inline size_type size() const {
			return sizeVal;
		}

		inline bool empty() const {
			return sizeVal == 0;
		}

		inline pointer data() const {
			return dataVal;
		}

		template<jsonifier_internal::pointer_t value_type_newer>
		inline friend std::enable_if_t<!std::is_array_v<value_type_newer>, bool> operator==(const string_base& lhs, const value_type_newer& rhs) {
			auto rhsLength = traits_type::length(rhs);
			if (lhs.size() != rhsLength) {
				return false;
			}
			return jsonifier_internal::jsonifier_core_internal::compare(rhs, lhs.data(), rhsLength);
		}

		template<jsonifier_internal::char_array_t value_type_newer> inline bool operator==(const value_type_newer& rhs) {
			auto rhsLength = std::size(rhs) - 1;
			if (size() != rhsLength) {
				return false;
			}
			return jsonifier_internal::jsonifier_core_internal::compare(rhs, data(), rhsLength);
		}

		template<jsonifier_internal::string_t value_type_newer> inline friend bool operator==(const string_base& lhs, const value_type_newer& rhs) {
			if (rhs.size() != lhs.size()) {
				return false;
			}
			return jsonifier_internal::jsonifier_core_internal::compare(rhs.data(), lhs.data(), rhs.size());
		}

		inline void swap(string_base& other) {
			std::swap(capacityVal, other.capacityVal);
			std::swap(sizeVal, other.sizeVal);
			std::swap(dataVal, other.dataVal);
		}

		template<typename value_type_newer, size_type N> inline friend string_base operator+(const value_type_newer (&lhs)[N], const string_base& rhs) {
			string_base newLhs{ lhs };
			newLhs += rhs;
			return newLhs;
		}

		template<typename value_type_newer, size_type N> inline friend string_base operator+=(const value_type_newer (&lhs)[N], const string_base& rhs) {
			string_base newLhs{ lhs };
			newLhs += rhs;
			return newLhs;
		}

		template<jsonifier_internal::pointer_t string_type_new> inline friend string_base operator+(string_type_new&& lhs, const string_base& rhs) {
			string_base newLhs{ lhs };
			newLhs += rhs;
			return newLhs;
		}

		template<jsonifier_internal::pointer_t string_type_new> inline friend string_base operator+=(string_type_new&& lhs, const string_base& rhs) {
			string_base newLhs{ lhs };
			newLhs += rhs;
			return newLhs;
		}

		inline string_base operator+(const value_type& rhs) {
			string_base newLhs{ *this };
			newLhs.pushBack(rhs);
			return newLhs;
		}

		inline string_base& operator+=(const value_type& rhs) {
			pushBack(rhs);
			return *this;
		}

		template<jsonifier_internal::string_t string_type_new> inline string_base operator+(const string_type_new& rhs) const {
			string_base newLhs{ *this };
			newLhs += rhs;
			return newLhs;
		}

		template<jsonifier_internal::string_t string_type_new> inline string_base& operator+=(const string_type_new& rhs) {
			append(rhs.data(), rhs.size());
			return *this;
		}

		template<typename value_type_newer, size_type N> inline string_base operator+(const value_type_newer (&rhs)[N]) const {
			string_base newLhs{ *this };
			newLhs += rhs;
			return newLhs;
		}

		template<typename value_type_newer, size_type N> inline string_base& operator+=(const value_type_newer (&rhs)[N]) {
			string_base newLhs{ rhs };
			*this += newLhs;
			return *this;
		}

		inline ~string_base() {
			reset();
		}

	  protected:
		size_type capacityVal{};
		size_type sizeVal{};
		pointer dataVal{};

		inline allocator& getAlloc() {
			return *this;
		}

		inline void reset() {
			if (dataVal && capacityVal) {
				if (sizeVal) {
					std::destroy(dataVal, dataVal + sizeVal + 1);
					sizeVal = 0;
				}
				getAlloc().deallocate(dataVal, capacityVal + 1);
				dataVal		= nullptr;
				capacityVal = 0;
			}
		}
	};

	using string = string_base<char>;

	template<typename value_type> inline std::ostream& operator<<(std::ostream& os, const string_base<value_type>& string) {
		os << string.operator typename std::string();
		return os;
	}
}// namespace jsonifier
