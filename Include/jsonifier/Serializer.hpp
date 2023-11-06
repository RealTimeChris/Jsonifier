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

	template<typename value_type> struct serialize_impl {};

	class jsonifier_core;

	struct serialize {
		template<jsonifier::concepts::core_type value_type, jsonifier::concepts::is_fwd_iterator buffer_iterator_type>
		inline static void op(value_type&& value, buffer_iterator_type&& buffer) {
			serialize_impl<jsonifier::concepts::unwrap<value_type>>::op(std::forward<value_type>(value), std::forward<buffer_iterator_type>(buffer));
		}

		template<jsonifier::concepts::core_type value_type, jsonifier::concepts::is_fwd_iterator buffer_iterator_type, jsonifier::concepts::has_find key_type>
		inline static void op(value_type&& value, buffer_iterator_type&& buffer, key_type&& keys) {
			serialize_impl<jsonifier::concepts::unwrap<value_type>>::op(std::forward<value_type>(value), std::forward<buffer_iterator_type>(buffer), std::forward<key_type>(keys));
		}
	};

	template<typename derived_type> class serializer {
	  public:
		template<jsonifier::concepts::core_type value_type, jsonifier::concepts::buffer_like buffer_type> inline void serializeJson(value_type&& data, buffer_type&& buffer) {
			iter.reset();
			static_cast<derived_type*>(this)->errors.clear();
			if constexpr (jsonifier::concepts::has_excluded_keys<jsonifier::concepts::unwrap<value_type>>) {
				serialize::op(std::forward<value_type>(data), static_cast<serialization_iterator<buffer_string<uint8_t>>&&>(iter), data.jsonifierExcludedKeys);
			} else {
				serialize::op(std::forward<value_type>(data), static_cast<serialization_iterator<buffer_string<uint8_t>>&&>(iter));
			}
			if constexpr (jsonifier::concepts::has_resize<buffer_type>) {
				if (buffer.size() != iter.currentSize) [[unlikely]] {
					buffer.resize(iter.currentSize);
				}
				std::memcpy(buffer.data(), getCurrentStringBuffer().data(), iter.currentSize);
			} else {
				static_cast<derived_type*>(this)->errors.emplace_back(createError(error_code::Serialize_Error));
			}
		}
		
		inline buffer_string<uint8_t>& getCurrentStringBuffer() {
			return static_cast<derived_type*>(this)->currentStringBuffer;
		}

	  protected:
		serialization_iterator<buffer_string<uint8_t>> iter{ getCurrentStringBuffer() };

		serializer() noexcept = default;

		~serializer() noexcept = default;
	};

}
