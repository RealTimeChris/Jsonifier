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

#include <jsonifier/SerializationIterator.hpp>
#include <jsonifier/NumberUtils.hpp>
#include <jsonifier/StringUtils.hpp>
#include <jsonifier/Error.hpp>

namespace jsonifier_internal {

	template<typename value_type = void> struct serialize_impl;

	struct serialize {
		template<bool shortStringsSupport, jsonifier::concepts::core_type value_type, jsonifier::concepts::is_fwd_iterator iterator_type>
		JSONIFIER_INLINE static void op(value_type&& value, iterator_type&& iter) {
			if constexpr (jsonifier::concepts::has_excluded_keys<value_type>) {
				serialize_impl<jsonifier::concepts::unwrap<value_type>>::template op<shortStringsSupport>(std::forward<value_type>(value), std::forward<iterator_type>(iter),
					value.jsonifierExcludedKeys);
			} else {
				serialize_impl<jsonifier::concepts::unwrap<value_type>>::template op<shortStringsSupport>(std::forward<value_type>(value), std::forward<iterator_type>(iter));
			}
		}
	};

	template<typename derived_type> class serializer : protected serialization_iterator<serializer<derived_type>, derived_type> {
	  public:
		using iterator_type = serialization_iterator<serializer<derived_type>, derived_type>;

		template<typename value_type> friend struct serialize_impl;

		JSONIFIER_INLINE serializer& operator=(const serializer& other) = delete;
		JSONIFIER_INLINE serializer(const serializer& other)			= delete;

		template<bool shortStringsSupport = false, jsonifier::concepts::core_type value_type, jsonifier::concepts::buffer_like buffer_type>
		JSONIFIER_INLINE void serializeJson(value_type&& data, buffer_type&& iter) {
			iterator_type::reset();
			derivedRef.errors.clear();
			serialize::template op<shortStringsSupport>(std::forward<value_type>(data), std::forward<iterator_type>(*this));
			if (iter.size() != iterator_type::currentSize) [[unlikely]] {
				iter.resize(iterator_type::currentSize);
			}
			std::memcpy(iter.data(), iterator_type::data(), iterator_type::currentSize);
		}

	  protected:
		derived_type& derivedRef{ initializeSelfRef() };

		JSONIFIER_INLINE serializer() noexcept : derivedRef{ initializeSelfRef() } {};

		JSONIFIER_INLINE ~serializer() noexcept = default;

		JSONIFIER_INLINE derived_type& initializeSelfRef() {
			return *static_cast<derived_type*>(this);
		}
	};

}