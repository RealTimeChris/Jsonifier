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
#include <jsonifier/Simd.hpp>
#include <type_traits>
#include <utility>

namespace jsonifier_internal {

	template<bool printErrors, bool excludeKeys, typename value_type = void> struct parse_impl;

	template<bool printErrors, bool excludeKeys> struct parse {
		template<typename value_type> inline static void op(value_type&& value, structural_iterator& iter) {
			parse_impl<printErrors, excludeKeys, ref_unwrap<value_type>>::op(std::forward<ref_unwrap<value_type>>(value), iter);
		}

		template<typename value_type, has_find KeyType> inline static void op(value_type&& value, structural_iterator& iter, const KeyType& keys) {
			parse_impl<printErrors, excludeKeys, ref_unwrap<value_type>>::op(std::forward<ref_unwrap<value_type>>(value), iter, keys);
		}
	};

	class parser {
	  public:
		template<bool printErrors = false, bool refreshstring = false, bool excludeKeys = false, core_type value_type, string_t buffer_type>
		inline void parseJson(value_type&& data, buffer_type&& inStringNew) {
			if (inStringNew.empty()) {
				return;
			}
			if constexpr (refreshstring) {
				reset(std::forward<buffer_type>(inStringNew));
			} else if (inStringNew != string) {
				reset(std::forward<buffer_type>(inStringNew));
			}
			auto newIter = begin();
			if (!*newIter) {
				return;
			}
			if constexpr (excludeKeys) {
				if constexpr (has_excluded_keys<decltype(data)>) {
					parse<printErrors, excludeKeys>::op(std::forward<value_type>(data), newIter, data.excludedKeys);
				} else {
					parse<printErrors, excludeKeys>::op(std::forward<value_type>(data), newIter);
				}
			} else {
				parse<printErrors, excludeKeys>::op(std::forward<value_type>(data), newIter);
			}
		}

	  protected:
		jsonifier::string_base<uint8_t> string{};
		simd_string_reader section{};
		int64_t originalLength{};

		template<typename value_type> inline void reset(value_type&& stringNew) {
			string.resize(stringNew.size());
			originalLength = stringNew.size();
			std::memcpy(string.data(), stringNew.data(), stringNew.size());
			section.reset(string);
		}

		inline structural_iterator begin() {
			return structural_iterator{ section.getStructurals(), originalLength };
		}
	};
};
