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

	template<bool printErrors, bool excludeKeys, typename ValueType> struct ParseImpl;

	template<bool printErrors, bool excludeKeys> struct Parse {
		template<typename ValueType> inline static void op(ValueType&& value, StructuralIterator& iter) {
			ParseImpl<printErrors, excludeKeys, std::remove_cvref_t<ValueType>>::op(std::forward<std::remove_cvref_t<ValueType>>(value), iter);
		}

		template<typename ValueType, HasFind KeyType> inline static void op(ValueType&& value, StructuralIterator& iter, const KeyType& keys) {
			ParseImpl<printErrors, excludeKeys, std::remove_cvref_t<ValueType>>::op(std::forward<std::remove_cvref_t<ValueType>>(value), iter,
				keys);
		}
	};

	class Parser {
	  public:
		inline Parser() noexcept = default;

		inline Parser& operator=(Parser&& other) noexcept {
			std::swap(section, other.section);
			std::swap(string, other.string);
			return *this;
		};

		inline Parser(Parser&& other) noexcept {
			*this = std::move(other);
		};

		inline Parser& operator=(const Parser&) = delete;
		inline Parser(const Parser&)			= delete;

		template<bool printErrors = false, bool refreshString = false, bool excludeKeys = false, CoreType ValueType, StringT BufferType>
		void parseJson(ValueType&& data, BufferType&& inStringNew) {
			if (inStringNew.empty()) {
				return;
			}
			refreshString ? reset(std::forward<BufferType>(inStringNew)) : (string != inStringNew ? reset(std::forward<BufferType>(inStringNew)) : void());
			auto newIter = begin();
			if (!*newIter) {
				return;
			}
			if constexpr (excludeKeys) {
				if constexpr (HasExcludedKeys<decltype(data)>) {
					Parse<printErrors, excludeKeys>::op(std::forward<ValueType>(data), newIter, data.excludedKeys);
				} else {
					Parse<printErrors, excludeKeys>::op(std::forward<ValueType>(data), newIter);
				}
			} else {
				Parse<printErrors, excludeKeys>::op(std::forward<ValueType>(data), newIter);
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
