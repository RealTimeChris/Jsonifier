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
/// Feb 3, 2023
#pragma once

#include <jsonifier/SerialStructuralIterator.hpp>
#include <jsonifier/SimdStructuralIterator.hpp>
#include <jsonifier/HashMap.hpp>
#include <jsonifier/String.hpp>
#include <jsonifier/Error.hpp>
#include <jsonifier/Simd.hpp>

namespace jsonifier_internal {

	template<typename value_type> struct parse_impl {};

	struct parse {
		template<jsonifier::concepts::core_type value_type, jsonifier::concepts::is_fwd_iterator iterator_type, jsonifier::concepts::is_parser parser_type>
		inline static void op(value_type&& value, iterator_type&& iter, parser_type&& parserNew) {
			parse_impl<jsonifier::concepts::unwrap<value_type>>::op(std::forward<value_type>(value), std::forward<iterator_type>(iter), std::forward<parser_type>(parserNew));
		}

		template<jsonifier::concepts::core_type value_type, jsonifier::concepts::is_fwd_iterator iterator_type, jsonifier::concepts::is_parser parser_type,
			jsonifier::concepts::has_find key_type>
		inline static void op(value_type&& value, iterator_type&& iter, parser_type&& parserNew, key_type&& keys) {
			parse_impl<jsonifier::concepts::unwrap<value_type>>::op(std::forward<value_type>(value), std::forward<iterator_type>(iter), std::forward<parser_type>(parserNew),
				std::forward<key_type>(keys));
		}
	};

	template<typename derived_type> class parser {
	  public:
		template<typename value_type> friend struct parse_impl;
		friend struct parse;

		template<bool refreshString = true, jsonifier::concepts::core_type value_type, jsonifier::concepts::buffer_like buffer_type>
		inline void parseJson(value_type&& data, buffer_type&& stringNew) {
			section.reset<refreshString>(stringNew.data(), stringNew.size());
			static_cast<derived_type*>(this)->currentStringBuffer.clear();
			static_cast<derived_type*>(this)->errors.clear();
			parseValues(data, section.begin());
		}

		inline jsonifier::vector<error>& getErrors() {
			return static_cast<derived_type*>(this)->errors;
		}

		inline buffer_string<uint8_t>& getCurrentStringBuffer() {
			return static_cast<derived_type*>(this)->currentStringBuffer;
		}

	  protected:
		simd_string_reader section{};

		parser() noexcept = default;

		template<jsonifier::concepts::core_type value_type, jsonifier::concepts::is_fwd_iterator iterator_type> inline void parseValues(value_type&& value, iterator_type&& iter) {
			if (!iter.operator->()) {
				return;
			}
			if constexpr (jsonifier::concepts::has_excluded_keys<jsonifier::concepts::unwrap<value_type>>) {
				parse::op(std::forward<value_type>(value), std::forward<iterator_type>(iter), *this, value.jsonifierExcludedKeys);
			} else {
				parse::op(std::forward<value_type>(value), std::forward<iterator_type>(iter), *this);
			}
		}

		~parser() noexcept = default;
	};
};
