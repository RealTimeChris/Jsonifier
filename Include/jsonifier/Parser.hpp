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
/// Feb 3, 2023
#pragma once

#include <jsonifier/StructuralIterator.hpp>
#include <jsonifier/HashMap.hpp>
#include <jsonifier/String.hpp>
#include <jsonifier/Simd.hpp>
#include <type_traits>
#include <utility>

namespace JsonifierInternal {

	struct ParseNoKeys {
		template<bool printErrors, typename ValueType> static void op(ValueType& value, StructuralIterator& iter);

		template<bool printErrors, BoolT ValueType> static void op(ValueType& value, StructuralIterator& iter);

		template<bool printErrors, NumT ValueType> static void op(ValueType& value, StructuralIterator& iter);

		template<bool printErrors, EnumT ValueType> static void op(ValueType& value, StructuralIterator& iter);

		template<bool printErrors, UniquePtrT ValueType> static void op(ValueType& value, StructuralIterator& iter);

		template<bool printErrors, RawJsonT ValueType> static void op(ValueType& value, StructuralIterator& iter);

		template<bool printErrors, StringT ValueType> static void op(ValueType& value, StructuralIterator& iter);

		template<bool printErrors, CharT ValueType> static void op(ValueType& value, StructuralIterator& iter);

		template<bool printErrors, RawArrayT ValueType> static void op(ValueType& value, StructuralIterator& iter);

		template<bool printErrors, ArrayT ValueType> static void op(ValueType& value, StructuralIterator& iter);

		template<bool printErrors, ObjectT ValueType> static void op(ValueType& value, StructuralIterator& iter);
	};

	struct ParseWithKeys {
		template<bool printErrors, typename ValueType> static void op(ValueType& value, StructuralIterator& iter);

		template<bool printErrors, BoolT ValueType> static void op(ValueType& value, StructuralIterator& iter);

		template<bool printErrors, NumT ValueType> static void op(ValueType& value, StructuralIterator& iter);

		template<bool printErrors, EnumT ValueType> static void op(ValueType& value, StructuralIterator& iter);

		template<bool printErrors, UniquePtrT ValueType> static void op(ValueType& value, StructuralIterator& iter);

		template<bool printErrors, RawJsonT ValueType> static void op(ValueType& value, StructuralIterator& iter);

		template<bool printErrors, StringT ValueType> static void op(ValueType& value, StructuralIterator& iter);

		template<bool printErrors, CharT ValueType> static void op(ValueType& value, StructuralIterator& iter);

		template<bool printErrors, RawArrayT ValueType> static void op(ValueType& value, StructuralIterator& iter);

		template<bool printErrors, ArrayT ValueType> static void op(ValueType& value, StructuralIterator& iter);

		template<bool printErrors, ObjectT ValueType, HasFind KeyType> static void op(ValueType& value, StructuralIterator& iter, const KeyType& excludedKeys);

		template<bool printErrors, ObjectT ValueType> static void op(ValueType& value, StructuralIterator& iter);
	};

	class Parser {
	  public:
		inline Parser() noexcept = default;

		inline Parser& operator=(Parser&& other) noexcept {
			std::swap(section, other.section);
			return *this;
		};

		inline Parser(Parser&& other) noexcept {
			*this = std::move(other);
		};

		inline Parser& operator=(const Parser&) = delete;
		inline Parser(const Parser&)			= delete;

		template<bool printErrors = false, bool refreshString = false, bool excludeKeys = false> void parseJson(CoreType auto& data, StringT auto& inStringNew) {
			if (inStringNew.empty()) {
				return;
			}
			bool refreshStringNew{ refreshString };
			if (refreshStringNew || (inStringNew.size() != string.size() && string != inStringNew)) {
				reset(inStringNew);
			}
			auto newIter = begin();
			if (!*newIter) {
				return;
			}
			if constexpr (excludeKeys) {
				if constexpr (HasExcludedKeys<decltype(data)>) {
					ParseWithKeys::op<printErrors>(data, newIter, data.excludedKeys);
				} else {
					ParseWithKeys::op<printErrors>(data, newIter);
				}
			} else {
				ParseNoKeys::op<printErrors>(data, newIter);
			}
		}

	  protected:
		SimdStringReader section{};
		Jsonifier::StringBase<uint8_t> string{};

		template<typename ValueType> inline void reset(const ValueType& stringNew) {
			string.resize(stringNew.size());
			std::memcpy(string.data(), stringNew.data(), stringNew.size());
			section.reset(static_cast<Jsonifier::StringViewBase<uint8_t>>(string));
		}

		inline StructuralIterator begin() noexcept {
			return { &section };
		}
	};
};
