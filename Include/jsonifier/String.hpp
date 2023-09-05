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

namespace JsonifierInternal {

	template<typename ValueType01, typename ValueType02> constexpr bool stringConstCompare(const ValueType01& string01, const ValueType02& string02) {
		if (string01.size() != string02.size()) [[unlikely]] {
			return false;
		}
		using CharType = ValueType01::value_type;
		for (uint64_t x = 0; x < string01.size(); ++x) {
			if (string01[x] != static_cast<CharType>(string02[x])) [[unlikely]] {
				return false;
			}
		}
		return true;
	}

	template<typename ValueType> using RefUnwrap = std::decay_t<ValueType>;

	template<typename ValueType>
	concept HasSize = requires(ValueType value) {
		{ value.size() };
	};

	template<typename ValueType>
	concept HasData = requires(ValueType value) {
		{ value.data() };
	};

	template<typename ValueType>
	concept Indexable = requires(ValueType value) { value[std::declval<uint64_t>()]; } || requires(ValueType value) { value[std::declval<uint64_t>()]; };

	template<typename ValueType>
	concept CharT = std::same_as<char, ValueType> || std::same_as<char8_t, ValueType> || std::same_as<char16_t, ValueType> || std::same_as<char32_t, ValueType> ||
		std::same_as<wchar_t, ValueType>;

	template<typename ValueType>
	concept SearchableStringValue = CharT<ValueType> || Indexable<ValueType>;

	template<typename ValueType>
	concept HasDataAndSize = HasData<ValueType> && HasSize<ValueType>;

	template<typename ValueType>
	concept HasSubstr = requires(ValueType value) {
		{ value.substr(std::declval<uint64_t>(), std::declval<uint64_t>()) };
	};

	template<typename ValueType>
	concept StringT = ( HasSubstr<std::remove_const_t<ValueType>> && HasData<std::remove_const_t<ValueType>> && HasSize<std::remove_const_t<ValueType>> &&
						  !std::same_as<char, std::remove_const_t<ValueType>> ) ||
		std::derived_from<ValueType, std::string> || std::same_as<std::string, ValueType>;

	template<typename ValueType> class CharTraits : public std::char_traits<ValueType> {};

	template<> class CharTraits<uint8_t> {
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

}// namespace JsonifierInternal

namespace Jsonifier {

	template<typename ValueType> class StringBase;

	template<typename ValueTypeNew, typename ValueTypeInternal> class StringOpBase {
	  public:
		using value_type = ValueTypeInternal;
		using size_type	 = uint64_t;

		inline const ValueTypeNew& getDerivedType() const {
			return *static_cast<const ValueTypeNew*>(this);
		}

		template<JsonifierInternal::SearchableStringValue StringType> inline size_type findLastOf(const StringType& valueToFind, size_type position = 0) const {
			if constexpr (std::same_as<JsonifierInternal::RefUnwrap<StringType>, value_type> || std::same_as<JsonifierInternal::RefUnwrap<StringType>, value_type> ||
				std::same_as<JsonifierInternal::RefUnwrap<StringType>, char>) {
				if (getDerivedType().sizeVal && position < getDerivedType().sizeVal) {
					return JsonifierInternal::findLastSingleCharacter(getDerivedType().dataVal, getDerivedType().sizeVal - position, valueToFind);
				} else {
					return ValueTypeNew::npos;
				}
			} else if constexpr (std::is_pointer_v<StringType> || std::is_array_v<StringType>) {
				auto sizeValNew = JsonifierInternal::CharTraits<std::remove_pointer_t<std::remove_all_extents_t<StringType>>>::length(valueToFind);
				if (getDerivedType().sizeVal == 0 || sizeValNew == 0 || position >= getDerivedType().sizeVal) {
					return ValueTypeNew::npos;
				}
				size_type currentHighestIndex{ 0 };
				for (size_type x = 0; x < sizeValNew; ++x) {
					auto newIndex = JsonifierInternal::findLastSingleCharacter(getDerivedType().dataVal, getDerivedType().sizeVal - position, valueToFind[x]);
					if (newIndex > currentHighestIndex && newIndex != ValueTypeNew::npos) {
						currentHighestIndex = newIndex;
					}
				}
				return currentHighestIndex == 0 ? ValueTypeNew::npos : currentHighestIndex - 1;
			} else {
				auto sizeValNew = valueToFind.size();
				if (getDerivedType().sizeVal == 0 || sizeValNew == 0 || position >= getDerivedType().sizeVal) {
					return ValueTypeNew::npos;
				}
				size_type currentHighestIndex{ 0 };
				for (size_type x = 0; x < sizeValNew; ++x) {
					auto newIndex = JsonifierInternal::findLastSingleCharacter(getDerivedType().dataVal, getDerivedType().sizeVal - position, valueToFind[x]);
					if (newIndex > currentHighestIndex && newIndex != ValueTypeNew::npos) {
						currentHighestIndex = newIndex;
					}
				}
				return currentHighestIndex == 0 ? ValueTypeNew::npos : currentHighestIndex - 1;
			}
		}

		template<JsonifierInternal::SearchableStringValue StringType> inline size_type findFirstOf(const StringType& valueToFind, size_type position = 0) const {
			if constexpr (std::same_as<JsonifierInternal::RefUnwrap<StringType>, value_type> || std::same_as<JsonifierInternal::RefUnwrap<StringType>, value_type> ||
				std::same_as<JsonifierInternal::RefUnwrap<StringType>, char>) {
				if (getDerivedType().sizeVal && position < getDerivedType().sizeVal) {
					return JsonifierInternal::findSingleCharacter(getDerivedType().dataVal, getDerivedType().sizeVal, valueToFind);
				} else {
					return ValueTypeNew::npos;
				}
			} else if constexpr (std::is_pointer_v<StringType> || std::is_array_v<StringType>) {
				auto sizeValNew = JsonifierInternal::CharTraits<std::remove_pointer_t<std::remove_all_extents_t<StringType>>>::length(valueToFind);
				if (getDerivedType().sizeVal == 0 || sizeValNew == 0 || position >= getDerivedType().sizeVal) {
					return ValueTypeNew::npos;
				}
				size_type currentHighestIndex{ ValueTypeNew::npos };
				for (size_type x = 0; x < sizeValNew; ++x) {
					auto newIndex = JsonifierInternal::findSingleCharacter(getDerivedType().dataVal + position, getDerivedType().sizeVal, valueToFind[x]);
					if (newIndex < currentHighestIndex && newIndex != ValueTypeNew::npos) {
						currentHighestIndex = newIndex;
					}
				}
				return currentHighestIndex;
			} else {
				auto sizeValNew = valueToFind.size();
				if (getDerivedType().sizeVal == 0 || sizeValNew == 0 || position >= getDerivedType().sizeVal) {
					return ValueTypeNew::npos;
				}
				size_type currentHighestIndex{ ValueTypeNew::npos };
				for (size_type x = 0; x < sizeValNew; ++x) {
					auto newIndex = JsonifierInternal::findSingleCharacter(getDerivedType().dataVal + position, getDerivedType().sizeVal, valueToFind[x]);
					if (newIndex < currentHighestIndex && newIndex != ValueTypeNew::npos) {
						currentHighestIndex = newIndex;
					}
				}
				return currentHighestIndex;
			}
		}

		template<JsonifierInternal::SearchableStringValue StringType> inline size_type findFirstNotOf(const StringType& valueToFind, size_type position = 0) const {
			if constexpr (std::same_as<JsonifierInternal::RefUnwrap<StringType>, value_type> || std::same_as<JsonifierInternal::RefUnwrap<StringType>, value_type> ||
				std::same_as<JsonifierInternal::RefUnwrap<StringType>, char>) {
				if (getDerivedType().sizeVal && position < getDerivedType().sizeVal) {
					return JsonifierInternal::findFirstCharacterNotEqual(getDerivedType().dataVal, getDerivedType().sizeVal, valueToFind);
				} else {
					return ValueTypeNew::npos;
				}
			} else if constexpr (std::is_pointer_v<StringType> || std::is_array_v<StringType>) {
				auto sizeValNew = JsonifierInternal::CharTraits<std::remove_pointer_t<std::remove_all_extents_t<StringType>>>::length(valueToFind);
				if (getDerivedType().sizeVal == 0 || sizeValNew == 0 || position >= getDerivedType().sizeVal) {
					return ValueTypeNew::npos;
				}
				size_type currentHighestIndex{ ValueTypeNew::npos };
				for (size_type x = position; x < getDerivedType().sizeVal; ++x) {
					bool didWeFindOne{ false };
					for (size_type y = 0; y < sizeValNew; ++y) {
						if (valueToFind[y] == getDerivedType().dataVal[x]) {
							didWeFindOne = true;
							break;
						}
					}
					if (!didWeFindOne) {
						return x;
					}
				}
				return currentHighestIndex;
			} else {
				auto sizeValNew = valueToFind.size();
				if (getDerivedType().sizeVal == 0 || sizeValNew == 0 || position >= getDerivedType().sizeVal) {
					return ValueTypeNew::npos;
				}
				size_type currentHighestIndex{ ValueTypeNew::npos };
				for (size_type x = position; x < getDerivedType().sizeVal; ++x) {
					bool didWeFindOne{ false };
					for (size_type y = 0; y < sizeValNew; ++y) {
						if (valueToFind[y] == getDerivedType().dataVal[x]) {
							didWeFindOne = true;
							break;
						}
					}
					if (!didWeFindOne) {
						return x;
					}
				}
				return currentHighestIndex;
			}
		}

		template<JsonifierInternal::SearchableStringValue StringType> inline size_type find(StringType valueToFind, size_type position = 0) const {
			if constexpr (std::same_as<JsonifierInternal::RefUnwrap<StringType>, value_type> || std::same_as<JsonifierInternal::RefUnwrap<StringType>, value_type> ||
				std::same_as<JsonifierInternal::RefUnwrap<StringType>, char>) {
				if (getDerivedType().sizeVal && position < getDerivedType().sizeVal) {
					return JsonifierInternal::findSingleCharacter(getDerivedType().dataVal, getDerivedType().sizeVal, valueToFind);
				} else {
					return ValueTypeNew::npos;
				}
			} else if constexpr (std::is_pointer_v<StringType> || std::is_array_v<StringType>) {
				auto sizeValNew = JsonifierInternal::CharTraits<std::remove_pointer_t<std::remove_all_extents_t<StringType>>>::length(valueToFind);
				if (getDerivedType().sizeVal == 0 || sizeValNew == 0 || position >= getDerivedType().sizeVal) {
					return ValueTypeNew::npos;
				}
				auto firstVal = valueToFind[0];
				int64_t currentIndex{ static_cast<int64_t>(position) };
				while (currentIndex < static_cast<int64_t>(getDerivedType().sizeVal)) {
					if (auto newIndex = JsonifierInternal::findSingleCharacter(getDerivedType().dataVal + currentIndex, getDerivedType().sizeVal - currentIndex, firstVal);
						newIndex != ValueTypeNew::npos) {
						bool doWeBreak{ false };
						if (currentIndex + sizeValNew <= getDerivedType().sizeVal) {
							if (memcmp(valueToFind, getDerivedType().dataVal + currentIndex, sizeValNew) == 0) {
								doWeBreak = true;
							}
						} else {
							return ValueTypeNew::npos;
						}
						if (doWeBreak) {
							return currentIndex;
						}
						if (newIndex == ValueTypeNew::npos) {
							return ValueTypeNew::npos;
						} else if (!newIndex) {
							++currentIndex;
						} else {
							currentIndex += newIndex;
						}
					} else {
						++currentIndex;
					}
				}
			} else {
				auto sizeValNew = valueToFind.size();
				if (getDerivedType().sizeVal == 0 || sizeValNew == 0 || position >= getDerivedType().sizeVal) {
					return ValueTypeNew::npos;
				}
				auto firstVal = valueToFind[0];
				int64_t currentIndex{ static_cast<int64_t>(position) };
				while (currentIndex < static_cast<int64_t>(getDerivedType().sizeVal)) {
					if (auto newIndex = JsonifierInternal::findSingleCharacter(getDerivedType().dataVal + currentIndex, getDerivedType().sizeVal - currentIndex, firstVal);
						newIndex != ValueTypeNew::npos) {
						bool doWeBreak{ false };
						if (currentIndex + sizeValNew <= getDerivedType().sizeVal) {
							if (memcmp(valueToFind.data(), getDerivedType().dataVal + currentIndex, sizeValNew) == 0) {
								doWeBreak = true;
							}
						} else {
							return ValueTypeNew::npos;
						}
						if (doWeBreak) {
							return currentIndex;
						}
						if (newIndex == ValueTypeNew::npos) {
							return ValueTypeNew::npos;
						} else if (!newIndex) {
							++currentIndex;
						} else {
							currentIndex += newIndex;
						}
					} else {
						++currentIndex;
					}
				}
				return ValueTypeNew::npos;
			}
		}
	};

	template<typename ValueType> class StringViewBase;

	template<typename ValueType> class StringBase : public StringOpBase<StringBase<ValueType>, ValueType>, protected JsonifierInternal::AllocWrapper<ValueType> {
	  public:
		using value_type			 = ValueType;
		using pointer				 = value_type*;
		using const_pointer			 = const value_type*;
		using reference				 = value_type&;
		using const_reference		 = const value_type&;
		using iterator				 = JsonifierInternal::Iterator<StringBase::value_type>;
		using const_iterator		 = JsonifierInternal::Iterator<const StringBase::value_type>;
		using reverse_iterator		 = std::reverse_iterator<iterator>;
		using const_reverse_iterator = std::reverse_iterator<const_iterator>;
		using size_type				 = uint64_t;
		using allocator				 = JsonifierInternal::AllocWrapper<value_type>;
		using traits_type			 = JsonifierInternal::CharTraits<value_type>;

		friend StringOpBase<StringBase, value_type>;

		inline StringBase() noexcept : capacityVal{}, sizeVal{}, dataVal{} {};

		static inline size_type npos{ std::numeric_limits<size_type>::max() };

		inline StringBase& operator=(StringBase&& other) noexcept {
			if (this != &other) {
				reset();
				swap(other);
			}
			return *this;
		}

		inline explicit StringBase(StringBase&& other) noexcept : capacityVal{}, sizeVal{}, dataVal{} {
			*this = std::move(other);
		}

		inline StringBase& operator=(const StringBase& other) {
			if (this != &other) {
				size_type sizeNew = other.size();
				if (sizeNew > 0) {
					reset();
					StringBase temp{};
					temp.resize(sizeNew);
					std::memcpy(temp.dataVal, other.data(), sizeNew);
					swap(temp);
				}
			}
			return *this;
		}

		inline StringBase(const StringBase& other) noexcept : capacityVal{}, sizeVal{}, dataVal{} {
			*this = other;
		}

		template<JsonifierInternal::StringT ValueTypeNew> inline StringBase& operator=(const ValueTypeNew& other) noexcept {
			size_type sizeNew = other.size();
			if (sizeNew > 0) {
				reset();
				StringBase temp{};
				temp.resize(sizeNew);
				std::memcpy(temp.dataVal, other.data(), sizeNew);
				swap(temp);
			}
			return *this;
		}

		template<JsonifierInternal::StringT ValueTypeNew> inline explicit StringBase(const ValueTypeNew& other) noexcept : capacityVal{}, sizeVal{}, dataVal{} {
			*this = other;
		}

		template<typename ValueTypeNew> inline StringBase& operator=(const ValueTypeNew* other) noexcept {
			auto sizeNew = JsonifierInternal::CharTraits<ValueTypeNew>::length(other);
			if (sizeNew) [[likely]] {
				reset();
				StringBase temp{};
				temp.resize(sizeNew);
				std::memcpy(temp.dataVal, other, sizeNew);
				swap(temp);
			}
			return *this;
		}

		template<typename ValueTypeNew> inline StringBase(const ValueTypeNew* other) noexcept : capacityVal{}, sizeVal{}, dataVal{} {
			*this = other;
		}

		inline StringBase(const_pointer other, uint64_t sizeNew) noexcept : capacityVal{}, sizeVal{}, dataVal{} {
			if (sizeNew) [[likely]] {
				reset();
				StringBase temp{};
				temp.resize(sizeNew);
				std::memcpy(temp.dataVal, other, sizeNew);
				swap(temp);
			}
		}

		inline StringBase substr(size_type position, size_type count = std::numeric_limits<size_type>::max()) const {
			if (position >= sizeVal) {
				throw std::out_of_range("Substring position is out of range.");
			}

			count = std::min(count, sizeVal - position);

			StringBase result{};
			result.resize(count);
			std::memcpy(result.dataVal, dataVal + position, count * sizeof(value_type));
			result.getAlloc().construct(&result.dataVal[count], '\0');

			return result;
		}

		inline iterator begin() noexcept {
			return iterator(dataVal);
		}

		inline const_iterator begin() const noexcept {
			return const_iterator(dataVal);
		}

		inline iterator end() noexcept {
			return iterator(dataVal + sizeVal);
		}

		inline const_iterator end() const noexcept {
			return const_iterator(dataVal + sizeVal);
		}

		inline reverse_iterator rbegin() noexcept {
			return reverse_iterator(end());
		}

		inline const_reverse_iterator rbegin() const noexcept {
			return const_reverse_iterator(end());
		}

		inline reverse_iterator rend() noexcept {
			return reverse_iterator(begin());
		}

		inline const_reverse_iterator rend() const noexcept {
			return const_reverse_iterator(begin());
		}

		inline const_iterator cbegin() const noexcept {
			return begin();
		}

		inline const_iterator cend() const noexcept {
			return end();
		}

		inline const_reverse_iterator crbegin() const noexcept {
			return rbegin();
		}

		inline const_reverse_iterator crend() const noexcept {
			return rend();
		}

		inline void append(const StringBase& sizeNew) {
			if (sizeVal + sizeNew.size() > capacityVal) {
				reserve(sizeVal + sizeNew.size());
			}
			std::memcpy(dataVal + sizeVal, sizeNew.data(), sizeNew.size());
			sizeVal += sizeNew.size();
			getAlloc().construct(&dataVal[sizeVal], '\0');
		}

		inline void append(const_pointer valuesNew, uint64_t sizeNew) {
			if (sizeVal + sizeNew > capacityVal) {
				reserve(sizeVal + sizeNew);
			}
			std::memcpy(dataVal + sizeVal, valuesNew, sizeNew);
			sizeVal += sizeNew;
			getAlloc().construct(&dataVal[sizeVal], '\0');
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
			getAlloc().construct(&dataVal[sizeVal], '\0');
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
			getAlloc().construct(&dataVal[sizeVal], '\0');
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
			getAlloc().construct(&dataVal[sizeVal], '\0');
		}

		inline void pushBack(value_type value) {
			if (sizeVal + 1 >= capacityVal) {
				reserve((sizeVal + 2) * 4);
			}
			getAlloc().construct(&dataVal[sizeVal++], value);
			getAlloc().construct(&dataVal[sizeVal], '\0');
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

		inline const_reference operator[](size_type index) const noexcept {
			return dataVal[index];
		}

		inline reference operator[](size_type index) noexcept {
			return dataVal[index];
		}

		inline std::string_view stringView(size_type offSet, size_type count) const noexcept {
			return std::string_view{ data() + offSet, count };
		}

		template<typename ValueTypeNew = value_type> inline explicit operator std::enable_if_t<std::same_as<ValueTypeNew, char>, std::string_view>() const {
			return { dataVal, sizeVal };
		}

		inline operator std::string() const {
			std::basic_string<char> returnValue{};
			returnValue.resize(sizeVal);
			std::memcpy(returnValue.data(), data(), returnValue.size());
			return returnValue;
		}

		inline void clear() noexcept {
			sizeVal = 0;
		}

		inline size_type maxSize() const {
			return std::numeric_limits<size_type>::max();
		}

		inline void resize(size_type sizeNew) {
			if (sizeNew > 0) [[likely]] {
				if (sizeNew > capacityVal) [[likely]] {
					pointer newPtr = getAlloc().allocate(sizeNew + 1);
					try {
						if (dataVal) [[likely]] {
							std::uninitialized_move(dataVal, dataVal + sizeVal, newPtr);
							getAlloc().deallocate(dataVal, capacityVal + 1);
						}
					} catch (...) {
						getAlloc().deallocate(newPtr, sizeNew + 1);
						throw;
					}
					capacityVal = sizeNew;
					dataVal		= newPtr;
					std::uninitialized_value_construct(dataVal + sizeVal, dataVal + capacityVal);
				} else if (sizeNew > sizeVal) [[unlikely]] {
					std::uninitialized_value_construct(dataVal + sizeVal, dataVal + capacityVal);
				}
				sizeVal = sizeNew;
				getAlloc().construct(&dataVal[sizeVal], '\0');
			} else {
				sizeVal = 0;
			}
		}

		inline void reserve(size_t capacityValNew) {
			if (capacityValNew > capacityVal) [[likely]] {
				pointer newPtr = getAlloc().allocate(capacityValNew + 1);
				try {
					if (dataVal && sizeVal) [[likely]] {
						std::uninitialized_move(dataVal, dataVal + sizeVal, newPtr);
						getAlloc().deallocate(dataVal, capacityVal + 1);
					}
				} catch (...) {
					getAlloc().deallocate(newPtr, capacityValNew + 1);
					throw;
				}

				capacityVal = capacityValNew;
				dataVal		= newPtr;
			}
		}

		inline size_type capacity() const noexcept {
			return capacityVal;
		}

		inline size_type size() const noexcept {
			return sizeVal;
		}

		inline bool empty() const noexcept {
			return sizeVal == 0;
		}

		inline pointer data() const noexcept {
			return dataVal;
		}

		inline bool operator==(const pointer rhs) const {
			if (traits_type::length(rhs) != size()) {
				return false;
			}
			return JsonifierInternal::JsonifierCoreInternal::compare(rhs, data(), size());
		}

		template<uint64_t strLength> inline bool operator==(const value_type (&other)[strLength]) const {
			if ((strLength - 1) != size()) {
				return false;
			}
			return JsonifierInternal::JsonifierCoreInternal::compare(other, data(), size());
		}

		template<JsonifierInternal::StringT ValueTypeNew> inline friend bool operator==(const StringBase& lhs, const ValueTypeNew& rhs) {
			if (rhs.size() != lhs.size()) {
				return false;
			}
			return JsonifierInternal::JsonifierCoreInternal::compare(rhs.data(), lhs.data(), rhs.size());
		}

		template<typename ValueTypeNew> inline friend bool operator==(const ValueTypeNew* lhs, const StringBase& rhs) {
			auto lhsSize = traits_type::length(lhs);
			if (rhs.size() != lhsSize) {
				return false;
			}
			return JsonifierInternal::JsonifierCoreInternal::compare(rhs.data(), lhs, lhsSize);
		}

		inline void swap(StringBase& other) {
			std::swap(capacityVal, other.capacityVal);
			std::swap(sizeVal, other.sizeVal);
			std::swap(dataVal, other.dataVal);
		}

		template<JsonifierInternal::StringT StringTypeNew> inline friend StringBase operator+(const StringTypeNew& lhs, const StringBase& rhs) {
			StringBase newLhs{ lhs };
			newLhs += rhs;
			return newLhs;
		}

		template<JsonifierInternal::StringT StringTypeNew> inline friend StringBase operator+=(const StringTypeNew& lhs, const StringBase& rhs) {
			StringBase newLhs{ lhs };
			newLhs += rhs;
			return newLhs;
		}

		template<size_type size> inline friend StringBase operator+(const value_type (&lhs)[size], const StringBase& rhs) {
			StringBase newLhs{ lhs };
			newLhs += rhs;
			return newLhs;
		}

		template<size_type size> inline friend StringBase operator+=(const value_type (&lhs)[size], const StringBase& rhs) {
			StringBase newLhs{ lhs };
			newLhs += rhs;
			return newLhs;
		}

		template<uint64_t strLength> inline StringBase operator+(const value_type (&rhs)[strLength]) {
			StringBase newLhs{ *this };
			newLhs += rhs;
			return newLhs;
		}

		template<uint64_t strLength> inline StringBase& operator+=(const value_type (&rhs)[strLength]) {
			StringBase newRhs{ rhs };
			append(newRhs);
			return *this;
		}

		inline StringBase operator+(const value_type& rhs) {
			StringBase newLhs{ *this };
			newLhs.pushBack(rhs);
			return newLhs;
		}

		inline StringBase& operator+=(const value_type& rhs) {
			pushBack(rhs);
			return *this;
		}

		template<JsonifierInternal::StringT StringTypeNew> inline StringBase operator+(const StringTypeNew& rhs) noexcept {
			StringBase newLhs{ *this };
			newLhs += rhs;
			return newLhs;
		}

		template<JsonifierInternal::StringT StringTypeNew> inline StringBase& operator+=(const StringTypeNew& rhs) noexcept {
			append(rhs.data(), rhs.size());
			return *this;
		}

		inline ~StringBase() {
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
				getAlloc().deallocate(dataVal, capacityVal + 1);
				dataVal		= nullptr;
				sizeVal		= 0;
				capacityVal = 0;
			}
		}
	};

	using String = StringBase<char>;

	template<typename ValueType> inline std::ostream& operator<<(std::ostream& os, const StringBase<ValueType>& string) {
		os << string.operator typename std::string();
		return os;
	}
}// namespace Jsonifier
