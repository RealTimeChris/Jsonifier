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

#include <jsonifier/NumberUtils.hpp>
#include <jsonifier/StringUtils.hpp>
#include <jsonifier/Error.hpp>

namespace jsonifier_internal {

	template<jsonifier::concepts::buffer_like buffer_type, jsonifier::concepts::uint64_type index_type, typename char_type>
	JSONIFIER_INLINE void writeCharacter(buffer_type&& buffer, index_type&& index, const char_type c) noexcept {
		const auto k = index + 1;
		if (k >= buffer.size()) [[unlikely]] {
			buffer.resize(max(buffer.size() * 2, k));
		}

		buffer[index] = static_cast<typename jsonifier::concepts::unwrap_t<buffer_type>::value_type>(c);
		++index;
	}

	template<json_structural_type c, jsonifier::concepts::buffer_like buffer_type, jsonifier::concepts::uint64_type index_type>
	JSONIFIER_INLINE void writeCharacter(buffer_type&& buffer, index_type&& index) noexcept {
		const auto k = index + 1;
		if (k >= buffer.size()) [[unlikely]] {
			buffer.resize(max(buffer.size() * 2, k));
		}

		buffer[index] = static_cast<typename jsonifier::concepts::unwrap_t<buffer_type>::value_type>(c);
		++index;
	}

	template<jsonifier::concepts::buffer_like buffer_type, jsonifier::concepts::uint64_type index_type, typename char_type>
	JSONIFIER_INLINE void writeCharacterUnchecked(buffer_type&& buffer, index_type&& index, const char_type c) noexcept {
		buffer[index] = static_cast<typename jsonifier::concepts::unwrap_t<buffer_type>::value_type>(c);
		++index;
	}

	template<jsonifier::concepts::buffer_like buffer_type, jsonifier::concepts::uint64_type index_type, uint64_t size, typename char_type>
	JSONIFIER_INLINE void writeCharacters(buffer_type&& buffer, index_type&& index, const char_type (&str)[size]) noexcept {
		static constexpr auto sizeNew = size - 1;
		const auto k				  = index + sizeNew;
		if (k >= buffer.size()) [[unlikely]] {
			buffer.resize(max(buffer.size() * 2, k));
		}
		std::copy(str, str + sizeNew, buffer.data() + std::forward<index_type>(index));
		index += sizeNew;
	}

	template<const jsonifier::string_view& str, jsonifier::concepts::buffer_like buffer_type, jsonifier::concepts::uint64_type index_type>
	JSONIFIER_INLINE void writeCharacters(buffer_type&& buffer, index_type&& index) noexcept {
		const auto sizeNew = str.size();
		const auto k	   = index + sizeNew;
		if (k >= buffer.size()) [[unlikely]] {
			buffer.resize(max(buffer.size() * 2, k));
		}
		std::copy(str.data(), str.data() + sizeNew, buffer.data() + std::forward<index_type>(index));
		index += sizeNew;
	}

	template<typename derived_type, typename value_type> struct serialize_impl;

	template<typename derived_type> class serializer {
	  public:
		template<typename derived_type_new, typename value_type> friend struct serialize_impl;
		template<uint64_t n, uint64_t indexNew, bool areWeFirst, typename derived_type_new, jsonifier::concepts::jsonifier_value_t value_type,
			jsonifier::concepts::buffer_like buffer_type, jsonifier::concepts::uint64_type index_type, jsonifier::concepts::has_find... key_type>
		friend void serializeObjects(value_type&& value, buffer_type&& buffer, index_type&& index, key_type&&... excludedKeys);

		JSONIFIER_INLINE serializer& operator=(const serializer& other) = delete;
		JSONIFIER_INLINE serializer(const serializer& other)			= delete;

		template<bool prettify = false, bool newLineInPrettyArrays = false, jsonifier::concepts::core_type value_type, jsonifier::concepts::buffer_like buffer_type>
		JSONIFIER_INLINE bool serializeJson(value_type&& data, buffer_type&& iter) {
			derivedRef.errors.clear();
			derivedRef.index = 0;
			impl(std::forward<value_type>(data), iter, derivedRef.index);
			if (iter.size() != derivedRef.index) {
				iter.resize(derivedRef.index);
			}
			if constexpr (prettify) {
				iter = static_cast<jsonifier::concepts::unwrap_t<buffer_type>>(derivedRef.template prettify<newLineInPrettyArrays, false>(iter));
			}
			return true;
		}

	  protected:
		derived_type& derivedRef{ initializeSelfRef() };

		JSONIFIER_INLINE serializer() noexcept : derivedRef{ initializeSelfRef() } {};

		JSONIFIER_INLINE derived_type& initializeSelfRef() {
			return *static_cast<derived_type*>(this);
		}

		template<jsonifier::concepts::core_type value_type, jsonifier::concepts::buffer_like buffer_type, jsonifier::concepts::uint64_type index_type>
		JSONIFIER_INLINE static void impl(value_type&& value, buffer_type&& buffer, index_type&& index) {
			if constexpr (jsonifier::concepts::has_excluded_keys<value_type>) {
				serialize_impl<derived_type, jsonifier::concepts::unwrap_t<value_type>>::impl(std::forward<value_type>(value), std::forward<buffer_type>(buffer),
					std::forward<index_type>(index), value.jsonifierExcludedKeys);
			} else {
				serialize_impl<derived_type, jsonifier::concepts::unwrap_t<value_type>>::impl(std::forward<value_type>(value), std::forward<buffer_type>(buffer),
					std::forward<index_type>(index));
			}
		}

		JSONIFIER_INLINE ~serializer() noexcept = default;
	};

}