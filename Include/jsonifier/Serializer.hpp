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

	template<typename value_type = void> struct serialize_impl;

	struct serialize {
		template<jsonifier::concepts::core_type value_type, jsonifier::concepts::buffer_like buffer_type, jsonifier::concepts::uint64_type index_type>
		JSONIFIER_INLINE static void impl(value_type&& value, buffer_type&& buffer, index_type&& index) {
			if constexpr (jsonifier::concepts::has_excluded_keys<value_type>) {
				serialize_impl<jsonifier::concepts::unwrap_t<value_type>>::impl(std::forward<value_type>(value), std::forward<buffer_type>(buffer), std::forward<index_type>(index),
					value.jsonifierExcludedKeys);
			} else {
				serialize_impl<jsonifier::concepts::unwrap_t<value_type>>::impl(std::forward<value_type>(value), std::forward<buffer_type>(buffer),
					std::forward<index_type>(index));
			}
		}
	};

	template<typename derived_type> class serializer {
	  public:
		template<typename value_type> friend struct serialize_impl;

		JSONIFIER_INLINE serializer& operator=(const serializer& other) = delete;
		JSONIFIER_INLINE serializer(const serializer& other)			= delete;

		template<bool prettify = false, jsonifier::concepts::core_type value_type, jsonifier::concepts::buffer_like buffer_type>
		JSONIFIER_INLINE void serializeJson(value_type&& data, buffer_type&& iter) {
			derivedRef.errors.clear();
			index = 0;
			serialize::impl(std::forward<value_type>(data), derivedRef.stringBuffer, index);
			if (iter.size() != index) {
				iter.resize(index);
			}
			std::memcpy(iter.data(), derivedRef.stringBuffer.data(), index);
			if constexpr (prettify) {
				iter = derivedRef.prettify(iter);
			}
		}

	  protected:
		derived_type& derivedRef{ initializeSelfRef() };
		uint64_t index{};

		JSONIFIER_INLINE serializer() noexcept : derivedRef{ initializeSelfRef() } {};

		JSONIFIER_INLINE ~serializer() noexcept = default;

		JSONIFIER_INLINE derived_type& initializeSelfRef() {
			return *static_cast<derived_type*>(this);
		}
	};

}