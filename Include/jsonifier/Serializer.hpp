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

namespace jsonifier_internal {

	template<bool excludeKeys, typename value_type = void> struct serialize_impl;

	template<bool excludeKeys> struct serialize {
		template<typename value_type, jsonifier::concepts::vector_like buffer_type> jsonifier_inline static void op(value_type&& value, buffer_type&& buffer, uint64_t& index) {
			serialize_impl<excludeKeys, jsonifier::concepts::unwrap<value_type>>::op(std::forward<value_type>(value), std::forward<buffer_type>(buffer), index);
		}

		template<typename value_type, jsonifier::concepts::vector_like buffer_type, jsonifier::concepts::has_find KeyType>
		jsonifier_inline static void op(value_type&& value, buffer_type&& buffer, uint64_t& index, const KeyType& keys) {
			serialize_impl<excludeKeys, jsonifier::concepts::unwrap<value_type>>::op(std::forward<value_type>(value), std::forward<buffer_type>(buffer), index, keys);
		}
	};

	class serializer {
	  public:
		template<bool excludeKeys = false, jsonifier::concepts::core_type value_type, jsonifier::concepts::vector_like buffer_type>
		jsonifier_inline void serializeJson(value_type&& data, buffer_type&& buffer) {
			uint64_t index{};
			if jsonifier_constexpr (excludeKeys) {
				if jsonifier_constexpr (jsonifier::concepts::has_excluded_keys<value_type>) {
					serialize<excludeKeys>::op(std::forward<value_type>(data), stringBuffer, index, data.excludedKeys);
				} else {
					serialize<excludeKeys>::op(std::forward<value_type>(data), stringBuffer, index);
				}
			} else {
				serialize<excludeKeys>::op(std::forward<value_type>(data), stringBuffer, index);
			}
			if (buffer.size() != index) [[unlikely]] {
				buffer.resize(index);
			}
			std::memcpy(buffer.data(), stringBuffer.data(), index);
		}

	  protected:
		jsonifier::string stringBuffer{};
	};

}
