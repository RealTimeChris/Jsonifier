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
#include <jsonifier/Base01.hpp>

namespace jsonifier {

	template<typename value_type> class string_view_base;

	template<typename value_type_new> class string_base : protected jsonifier_internal::alloc_wrapper<value_type_new> {
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

		jsonifier_inline string_base() : capacityVal{}, sizeVal{}, dataVal{} {};

		static jsonifier_constexpr size_type bufSize = 16 / sizeof(value_type) < 1 ? 1 : 16 / sizeof(value_type);
		static jsonifier_constexpr size_type npos{ std::numeric_limits<size_type>::max() };

		jsonifier_inline string_base& operator=(string_base&& other) noexcept {
			if (this != &other) {
				reset();
				swap(other);
			}
			return *this;
		}

		jsonifier_inline explicit string_base(string_base&& other) noexcept : capacityVal{}, sizeVal{}, dataVal{} {
			*this = std::move(other);
		}

		jsonifier_inline string_base& operator=(const string_base& other) {
			if (this != &other) {
				size_type sizeNew = other.size();
				if (sizeNew > 0 && sizeNew < maxSize()) {
					reset();
					string_base temp{};
					temp.reserve(sizeNew);
					temp.sizeVal = sizeNew;
					std::uninitialized_copy(other.data(), other.data() + sizeNew, temp.dataVal);
					getAlloc().construct(&temp[sizeNew], static_cast<value_type>(0x00));
					swap(temp);
				}
			}
			return *this;
		}

		jsonifier_inline string_base(const string_base& other) : capacityVal{}, sizeVal{}, dataVal{} {
			*this = other;
		}

		template<jsonifier::concepts::string_t value_type_newer> jsonifier_inline string_base& operator=(value_type_newer&& other) {
			size_type sizeNew = other.size() * (sizeof(typename jsonifier::concepts::unwrap<value_type_newer>::value_type) / sizeof(value_type));
			if (sizeNew > 0) {
				reset();
				string_base temp{};
				temp.reserve(sizeNew);
				temp.sizeVal = sizeNew;
				std::uninitialized_copy(other.data(), other.data() + sizeNew, temp.dataVal);
				getAlloc().construct(&temp[sizeNew], static_cast<value_type>(0x00));
				swap(temp);
			}
			return *this;
		}

		template<jsonifier::concepts::string_t value_type_newer> jsonifier_inline explicit string_base(value_type_newer&& other) : capacityVal{}, sizeVal{}, dataVal{} {
			*this = std::forward<value_type_newer>(other);
		}

		template<jsonifier::concepts::pointer_t value_type_newer> jsonifier_inline string_base& operator=(value_type_newer other) {
			auto sizeNew =
				jsonifier_internal::char_traits<std::remove_pointer_t<value_type_newer>>::length(other) * (sizeof(std::remove_pointer_t<value_type_newer>) / sizeof(value_type));
			if (sizeNew) [[likely]] {
				reset();
				string_base temp{};
				temp.reserve(sizeNew);
				temp.sizeVal = sizeNew;
				std::uninitialized_copy(other, other + sizeNew, temp.dataVal);
				getAlloc().construct(&temp[sizeNew], static_cast<value_type>(0x00));
				swap(temp);
			}
			return *this;
		}

		template<jsonifier::concepts::pointer_t value_type_newer> jsonifier_inline string_base(value_type_newer other) : capacityVal{}, sizeVal{}, dataVal{} {
			*this = std::forward<value_type_newer>(other);
		}

		template<jsonifier::concepts::char_type value_type_newer> jsonifier_inline string_base& operator=(value_type_newer other) {
			push_back(static_cast<value_type>(other));
			return *this;
		}

		template<jsonifier::concepts::char_type value_type_newer> jsonifier_inline string_base(value_type_newer other) : capacityVal{}, sizeVal{}, dataVal{} {
			*this = other;
		}

		jsonifier_inline string_base(const_pointer other, uint64_t sizeNew) : capacityVal{}, sizeVal{}, dataVal{} {
			if (sizeNew && sizeNew < maxSize()) [[likely]] {
				reset();
				string_base temp{};
				temp.reserve(sizeNew);
				temp.sizeVal = sizeNew;
				std::uninitialized_copy(other, other + sizeNew, temp.dataVal);
				getAlloc().construct(&temp[sizeNew], static_cast<value_type>(0x00));
				swap(temp);
			}
		}

		jsonifier_inline string_base substr(size_type position, size_type count = std::numeric_limits<size_type>::max()) const {
			if (position >= sizeVal) {
				throw std::out_of_range("Substring position is out of range.");
			}

			count = std::min(count, sizeVal - position);

			string_base result{};
			if (count > 0) {
				result.resize(count);
				std::memcpy(result.dataVal, dataVal + position, count * sizeof(value_type));
			}
			return result;
		}

		jsonifier_constexpr static size_type maxSize() {
			const size_type allocMax   = allocator::maxSize();
			const size_type storageMax = std::max(allocMax, static_cast<size_type>(bufSize));
			return std::min(static_cast<size_type>((std::numeric_limits<difference_type>::max)()), storageMax - 1);
		}

		jsonifier_constexpr iterator begin() noexcept {
			return iterator{ dataVal };
		}

		jsonifier_constexpr iterator end() noexcept {
			return iterator{ dataVal + sizeVal };
		}

		jsonifier_constexpr reverse_iterator rbegin() noexcept {
			return reverse_iterator{ end() };
		}

		jsonifier_constexpr reverse_iterator rend() noexcept {
			return reverse_iterator{ begin() };
		}

		jsonifier_constexpr const_iterator begin() const noexcept {
			return const_iterator{ dataVal };
		}

		jsonifier_constexpr const_iterator end() const noexcept {
			return const_iterator{ dataVal + sizeVal };
		}

		jsonifier_constexpr const_reverse_iterator rbegin() const noexcept {
			return const_reverse_iterator{ end() };
		}

		jsonifier_constexpr const_reverse_iterator rend() const noexcept {
			return const_reverse_iterator{ begin() };
		}

		jsonifier_inline size_type find(const_pointer args, size_type position = 0) const {
			auto newSize = traits_type::length(args);
			if (position + newSize > sizeVal) {
				return npos;
			}
			auto foundValue = jsonifier_internal::find(dataVal + position, sizeVal - position, args, newSize);
			return foundValue == npos ? npos : foundValue + position;
		}

		template<jsonifier::concepts::char_type value_type_newer> jsonifier_inline size_type find(value_type_newer args, size_type position = 0) const {
			value_type newValue{ static_cast<value_type>(args) };
			if (position + 1 > sizeVal) {
				return npos;
			}
			auto foundValue = jsonifier_internal::find(dataVal + position, sizeVal - position, &newValue, 1);
			return foundValue == npos ? npos : foundValue + position;
		}

		template<jsonifier::concepts::string_t value_type_newer> jsonifier_inline size_type find(const value_type_newer& args, size_type position = 0) const {
			if (position + args.size() > sizeVal) {
				return npos;
			}
			auto foundValue = jsonifier_internal::find(dataVal + position, sizeVal - position, args.data(), args.size());
			return foundValue == npos ? npos : foundValue + position;
		}

		template<typename... ArgTypes> jsonifier_inline size_type findFirstOf(ArgTypes&&... args) const {
			return operator std::basic_string_view<value_type>().find_first_of(std::forward<ArgTypes>(args)...);
		}

		template<typename... ArgTypes> jsonifier_inline size_type findLastOf(ArgTypes&&... args) const {
			return operator std::basic_string_view<value_type>().find_last_of(std::forward<ArgTypes>(args)...);
		}

		template<typename... ArgTypes> jsonifier_inline size_type findFirstNotOf(ArgTypes&&... args) const {
			return operator std::basic_string_view<value_type>().find_first_not_of(std::forward<ArgTypes>(args)...);
		}

		template<typename... ArgTypes> jsonifier_inline size_type findLastNotOf(ArgTypes&&... args) const {
			return operator std::basic_string_view<value_type>().find_last_not_of(std::forward<ArgTypes>(args)...);
		}

		jsonifier_inline void append(const string_base& sizeNew) {
			if (sizeVal + sizeNew.size() >= capacityVal) {
				reserve(sizeVal + sizeNew.size());
			}
			std::memcpy(dataVal + sizeVal, sizeNew.data(), sizeNew.size());
			sizeVal += sizeNew.size();
			getAlloc().construct(&dataVal[sizeVal], static_cast<value_type>(0x00));
		}

		template<typename value_type_newer> jsonifier_inline void append(value_type_newer* valuesNew, uint64_t sizeNew) {
			if (sizeVal + sizeNew >= capacityVal) {
				resize(sizeVal + sizeNew);
			}
			if (sizeNew > 0 && valuesNew) {
				std::memcpy(dataVal + sizeVal, valuesNew, sizeNew);
				sizeVal += sizeNew;
				getAlloc().construct(&dataVal[sizeVal], static_cast<value_type>(0x00));
			}
		}

		template<typename Iterator01, typename Iterator02> jsonifier_inline void insert(Iterator01 where, Iterator02 start, Iterator02 end) {
			int64_t sizeNew = end - start;
			auto posNew		= where.operator->() - dataVal;

			if (sizeNew <= 0) {
				return;
			}

			if (sizeVal + sizeNew >= capacityVal) {
				reserve(sizeVal + sizeNew);
			}

			std::memmove(dataVal + posNew + sizeNew, dataVal + posNew, (sizeVal - posNew) * sizeof(value_type));
			std::memcpy(dataVal + posNew, start.operator->(), sizeNew * sizeof(value_type));
			sizeVal += sizeNew;
			getAlloc().construct(&dataVal[sizeVal], static_cast<value_type>(0x00));
		}

		jsonifier_inline void insert(iterator valuesNew, value_type toInsert) {
			auto positionNew = valuesNew - begin();
			if (sizeVal + 1 >= capacityVal) {
				reserve((sizeVal + 1) * 2);
			}
			auto sizeNew = sizeVal - positionNew;
			std::memmove(dataVal + positionNew + 1, dataVal + positionNew, sizeNew * sizeof(value_type));
			getAlloc().construct(&dataVal[positionNew], toInsert);
			++sizeVal;
		}

		jsonifier_inline void erase(size_type count) {
			if (count == 0) {
				return;
			} else if (count > sizeVal) {
				count = sizeVal;
			}
			traits_type::move(dataVal, dataVal + count, sizeVal - count);
			sizeVal -= count;
			getAlloc().construct(&dataVal[sizeVal], static_cast<value_type>(0x00));
		}

		jsonifier_inline void erase(iterator count) {
			auto sizeNew = count - dataVal;
			if (sizeNew == 0) {
				return;
			} else if (sizeNew > static_cast<int64_t>(sizeVal)) {
				sizeNew = static_cast<int64_t>(sizeVal);
			}
			traits_type::move(dataVal, dataVal + sizeNew, sizeVal - sizeNew);
			sizeVal -= sizeNew;
			getAlloc().construct(&dataVal[sizeVal], static_cast<value_type>(0x00));
		}

		jsonifier_inline void push_back(value_type value) {
			if (sizeVal + 1 >= capacityVal) {
				reserve((sizeVal + 2) * 4);
			}
			getAlloc().construct(&dataVal[sizeVal++], value);
			getAlloc().construct(&dataVal[sizeVal], static_cast<value_type>(0x00));
		}

		jsonifier_inline const_reference at(size_type index) const {
			if (index >= sizeVal) {
				throw std::runtime_error{ "Sorry, but that index is beyond the end of this string." };
			}
			return dataVal[index];
		}

		jsonifier_inline reference at(size_type index) {
			if (index >= sizeVal) {
				throw std::runtime_error{ "Sorry, but that index is beyond the end of this string." };
			}
			return dataVal[index];
		}

		jsonifier_inline const_reference operator[](size_type index) const {
			return dataVal[index];
		}

		jsonifier_inline reference operator[](size_type index) {
			return dataVal[index];
		}

		jsonifier_inline operator std::basic_string_view<value_type>() const {
			return { dataVal, sizeVal };
		}

		template<typename value_type_newer> jsonifier_inline explicit operator std::basic_string<value_type_newer>() const {
			std::basic_string<value_type_newer> returnValue{};
			if (sizeVal > 0) {
				returnValue.resize(sizeVal);
				std::memcpy(returnValue.data(), data(), returnValue.size());
			}
			return returnValue;
		}

		jsonifier_inline void clear() {
			sizeVal = 0;
		}

		jsonifier_inline void resize(size_type sizeNew) {
			if (sizeNew > 0) [[likely]] {
				sizeNew = jsonifier_internal::roundUpToMultiple<BytesPerStep>(sizeNew);
				if (sizeNew > capacityVal) [[likely]] {
					pointer newPtr = getAlloc().allocate(sizeNew + 1);
					try {
						if (dataVal) [[likely]] {
							if (sizeVal > 0) {
								std::uninitialized_move(dataVal, dataVal + sizeVal, newPtr);
							}
							getAlloc().deallocate(dataVal, capacityVal + 1);
						}
					} catch (...) {
						getAlloc().deallocate(newPtr, sizeNew + 1);
						throw;
					}
					capacityVal = sizeNew;
					dataVal		= newPtr;
					std::uninitialized_fill(dataVal + sizeVal, dataVal + capacityVal, value_type{});
				} else if (sizeNew > sizeVal) [[unlikely]] {
					std::uninitialized_fill(dataVal + sizeVal, dataVal + capacityVal, value_type{});
				}
				sizeVal = sizeNew;
				getAlloc().construct(&dataVal[sizeVal], static_cast<value_type>(0x00));
			} else {
				sizeVal = 0;
			}
		}

		jsonifier_inline void reserve(size_type capacityNew) {
			size_type capacityNewer = jsonifier_internal::roundUpToMultiple<BitsPerStep>(capacityNew);
			if (capacityNewer > capacityVal) [[likely]] {
				pointer newPtr = getAlloc().allocate(capacityNewer + 1);
				try {
					if (dataVal) [[likely]] {
						if (sizeVal > 0) {
							std::uninitialized_move(dataVal, dataVal + sizeVal, newPtr);
						}
						getAlloc().deallocate(dataVal, capacityVal + 1);
					}
				} catch (...) {
					getAlloc().deallocate(newPtr, capacityNewer + 1);
					throw;
				}
				capacityVal = capacityNewer;
				dataVal		= newPtr;
				getAlloc().construct(&dataVal[sizeVal], static_cast<value_type>(0x00));
			}
		}

		jsonifier_inline size_type capacity() const {
			return capacityVal;
		}

		jsonifier_inline size_type size() const {
			return sizeVal;
		}

		jsonifier_inline bool empty() const {
			return sizeVal == 0;
		}

		jsonifier_inline pointer data() const {
			return dataVal;
		}

		jsonifier_inline pointer data() {
			return dataVal;
		}

		template<jsonifier::concepts::pointer_t value_type_newer>
		jsonifier_inline friend std::enable_if_t<!std::is_array_v<value_type_newer>, bool> operator==(const string_base& lhs, const value_type_newer& rhs) {
			auto rhsLength = traits_type::length(rhs);
			if (lhs.size() != rhsLength) {
				return false;
			}
			return jsonifier_internal::jsonifier_core_internal::compare(rhs, lhs.data(), rhsLength);
		}

		template<jsonifier::concepts::string_t value_type_newer> jsonifier_inline friend bool operator==(const string_base& lhs, const value_type_newer& rhs) {
			if (rhs.size() != lhs.size()) {
				return false;
			}
			return jsonifier_internal::jsonifier_core_internal::compare(rhs.data(), lhs.data(), rhs.size());
		}

		jsonifier_inline void swap(string_base& other) {
			std::swap(capacityVal, other.capacityVal);
			std::swap(sizeVal, other.sizeVal);
			std::swap(dataVal, other.dataVal);
		}

		template<typename value_type_newer, size_type size> jsonifier_inline friend string_base operator+(const value_type_newer (&lhs)[size], const string_base& rhs) {
			string_base newLhs{ lhs };
			newLhs += rhs;
			return newLhs;
		}

		template<typename value_type_newer, size_type size> jsonifier_inline friend string_base operator+=(const value_type_newer (&lhs)[size], const string_base& rhs) {
			string_base newLhs{ lhs };
			newLhs += rhs;
			return newLhs;
		}

		template<jsonifier::concepts::pointer_t string_type_new> jsonifier_inline friend string_base operator+(string_type_new&& lhs, const string_base& rhs) {
			string_base newLhs{ lhs };
			newLhs += rhs;
			return newLhs;
		}

		template<jsonifier::concepts::pointer_t string_type_new> jsonifier_inline friend string_base operator+=(string_type_new&& lhs, const string_base& rhs) {
			string_base newLhs{ lhs };
			newLhs += rhs;
			return newLhs;
		}

		jsonifier_inline string_base operator+(const value_type& rhs) {
			string_base newLhs{ *this };
			newLhs.push_back(rhs);
			return newLhs;
		}

		jsonifier_inline string_base& operator+=(const value_type& rhs) {
			push_back(rhs);
			return *this;
		}

		template<jsonifier::concepts::string_t string_type_new> jsonifier_inline string_base operator+(const string_type_new& rhs) const {
			string_base newLhs{ *this };
			newLhs += rhs;
			return newLhs;
		}

		template<jsonifier::concepts::string_t string_type_new> jsonifier_inline string_base& operator+=(const string_type_new& rhs) {
			append(static_cast<string_base>(rhs));
			return *this;
		}

		template<jsonifier::concepts::pointer_t string_type_new> jsonifier_inline string_base operator+(string_type_new&& rhs) {
			string_base newLhs{ *this };
			newLhs += rhs;
			return newLhs;
		}

		template<jsonifier::concepts::pointer_t string_type_new> jsonifier_inline string_base& operator+=(string_type_new&& rhs) {
			string_base newRhs{ rhs };
			*this += newRhs;
			return *this;
		}

		template<typename value_type_newer, size_type size> jsonifier_inline string_base operator+(const value_type_newer (&rhs)[size]) const {
			string_base newLhs{ *this };
			newLhs += rhs;
			return newLhs;
		}

		template<typename value_type_newer, size_type size> jsonifier_inline string_base& operator+=(const value_type_newer (&rhs)[size]) {
			string_base newRhs{ rhs };
			*this += newRhs;
			return *this;
		}

		jsonifier_inline ~string_base() {
			reset();
		}

	  protected:
		size_type capacityVal{};
		size_type sizeVal{};
		pointer dataVal{};

		jsonifier_inline allocator& getAlloc() {
			return *this;
		}

		jsonifier_inline void reset() {
			if (dataVal && capacityVal) {
				if (sizeVal) {
					std::destroy(dataVal, dataVal + sizeVal);
					sizeVal = 0;
				}
				getAlloc().deallocate(dataVal, capacityVal + 1);
				dataVal		= nullptr;
				capacityVal = 0;
			}
		}
	};

	using string = string_base<char>;

	template<typename value_type> jsonifier_inline std::ostream& operator<<(std::ostream& os, const string_base<value_type>& string) {
		os << string.operator typename std::string();
		return os;
	}
}// namespace jsonifier

namespace jsonifier_internal {

	template<typename value_type_new> class buffer_string : public jsonifier::string_base<value_type_new> {
	  public:
		jsonifier_inline buffer_string() {
			jsonifier::string_base<value_type_new>::resize(16384);
		}
	};

}