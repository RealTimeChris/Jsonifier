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

#include <jsonifier/StructuralIterator.hpp>
#include <jsonifier/HashMap.hpp>
#include <jsonifier/String.hpp>
#include <jsonifier/Error.hpp>
#include <jsonifier/Simd.hpp>

namespace jsonifier_internal {

	class parser;

	template<bool excludeKeys, typename value_type> struct parse_impl {};

	template<bool excludeKeys> struct parse {
		template<typename value_type> jsonifier_inline static void op(value_type& value, structural_iterator& iter, parser& parserNew) {
			parse_impl<excludeKeys, value_type>::op(value, iter, parserNew);
		}

		template<typename value_type, jsonifier::concepts::has_find KeyType>
		jsonifier_inline static void op(value_type& value, structural_iterator& iter, parser& parserNew, const KeyType& keys) {
			parse_impl<excludeKeys, value_type>::op(value, iter, parserNew, keys);
		}
	};

	class parser {
	  public:
		template<bool, typename value_type> friend struct parse_impl;
		template<bool> friend struct parse;

		template<bool excludeKeys = false, bool refreshString = false, jsonifier::concepts::core_type value_type, jsonifier::concepts::string_t buffer_type>
		jsonifier_inline void parseJson(value_type&& data, buffer_type& stringNew) {
			if (stringNew.empty()) {
				return;
			}
			section.reset<refreshString>(std::forward<buffer_type>(stringNew));
			errors.clear();
			auto newIter = section.begin();
			if (!*newIter) {
				return;
			}
			if jsonifier_constexpr (excludeKeys) {
				if jsonifier_constexpr (jsonifier::concepts::has_excluded_keys<jsonifier::concepts::unwrap<decltype(data)>>) {
					parse<excludeKeys>::op(data, newIter, data.excludedKeys, *this);
				} else {
					parse<excludeKeys>::op(data, newIter, *this);
				}
			} else {
				parse<excludeKeys>::op(data, newIter, *this);
			}
		}

		jsonifier_inline jsonifier::vector<error>& getErrors() {
			return errors;
		}

	  protected:
		buffer_string<uint8_t> currentStringBuffer{};
		buffer_string<char> currentKeyBuffer{};
		jsonifier::vector<error> errors{};
		simd_string_reader section{};
	};
};
