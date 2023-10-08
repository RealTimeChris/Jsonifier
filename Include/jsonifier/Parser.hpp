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
		template<typename value_type> inline static void op(value_type&& value, structural_iterator& iter, parser& parserNew) {
			parse_impl<excludeKeys, std::unwrap_ref_decay_t<value_type>>::op(std::forward<std::unwrap_ref_decay_t<value_type>>(value), iter, parserNew);
		}

		template<typename value_type, jsonifier::concepts::has_find KeyType>
		inline static void op(value_type&& value, structural_iterator& iter, const KeyType& keys, parser& parserNew) {
			parse_impl<excludeKeys, std::unwrap_ref_decay_t<value_type>>::op(std::forward<std::unwrap_ref_decay_t<value_type>>(value), iter, keys, parserNew);
		}
	};

	class parser {
	  public:
		template<bool, typename value_type> friend struct parse_impl;

		template<bool excludeKeys = false, bool refreshString = false, jsonifier::concepts::core_type value_type, jsonifier::concepts::string_t buffer_type>
		inline void parseJson(value_type&& data, buffer_type&& stringNew) {
			if (stringNew.empty()) {
				return;
			}
			if constexpr (refreshString) {
				reset(std::forward<buffer_type>(stringNew));
			} else if (stringNew != string) {
				reset(std::forward<buffer_type>(stringNew));
			}
			errors.clear();
			auto newIter = begin();
			if (!*newIter) {
				return;
			}
			if constexpr (excludeKeys) {
				if constexpr (jsonifier::concepts::has_excluded_keys<decltype(data)>) {
					parse<excludeKeys>::op(std::forward<value_type>(data), newIter, data.excludedKeys, *this);
				} else {
					parse<excludeKeys>::op(std::forward<value_type>(data), newIter, *this);
				}
			} else {
				parse<excludeKeys>::op(std::forward<value_type>(data), newIter, *this);
			}
		}

		inline jsonifier::vector<error>& getErrors() {
			return errors;
		}

	  protected:
		jsonifier::string_base<uint8_t> string{};
		jsonifier::vector<error> errors{};
		simd_string_reader section{};

		template<typename value_type> inline void reset(value_type&& stringNew) {
			string.resize(stringNew.size());
			std::memcpy(string.data(), stringNew.data(), stringNew.size());
			section.reset(string);
		}

		inline structural_iterator begin() {
			return structural_iterator{ section.getStructurals(), static_cast<int64_t>(section.getTapeLength()) };
		}
	};
};
