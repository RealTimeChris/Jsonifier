/*
	Jsonifier - For parsing and serializing Json - very rapidly.
	Copyright (C) 2023 Chris M. (RealTimeChris)

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
	Lesser General Public License for more details.

	You should have received a copy of the GNU Lesser General Public
	License along with this library; if not, write to the Free Software
	Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301
	USA
*/
/// https://github.com/RealTimeChris/Jsonifier
/// Feb 3, 2023
#pragma once

#include <jsonifier/StructuralIterator.hpp>
#include <jsonifier/HashMap.hpp>
#include <jsonifier/String.hpp>
#include <jsonifier/Simd.hpp>

namespace JsonifierInternal {

	struct ParseNoKeys {
		template<bool printErrors, BoolT OTy> static void op(OTy& value, StructuralIterator& buffer);

		template<bool printErrors, NumT OTy> static void op(OTy& value, StructuralIterator& buffer);

		template<bool printErrors, EnumT OTy> static void op(OTy& value, StructuralIterator& buffer);

		template<bool printErrors, UniquePtrT OTy> static void op(OTy& value, StructuralIterator& it);

		template<bool printErrors, RawJsonT OTy> static void op(OTy& value, StructuralIterator& it);

		template<bool printErrors, StringT OTy> static void op(OTy& value, StructuralIterator& buffer);

		template<bool printErrors, RawArrayT OTy> static void op(OTy& value, StructuralIterator& buffer);

		template<bool printErrors, VectorT OTy> static void op(OTy& value, StructuralIterator& buffer);

		template<bool printErrors, StdTupleT OTy> static void op(OTy& value, StructuralIterator& buffer);

		template<bool printErrors, ObjectT OTy> static void op(OTy& value, StructuralIterator& buffer);

		template<bool printErrors, typename OTy> static void op(OTy& value, StructuralIterator& buffer);
	};

	struct ParseWithKeys {
		template<bool printErrors, BoolT OTy> static void op(OTy& value, StructuralIterator& buffer);

		template<bool printErrors, NumT OTy> static void op(OTy& value, StructuralIterator& buffer);

		template<bool printErrors, EnumT OTy> static void op(OTy& value, StructuralIterator& buffer);

		template<bool printErrors, UniquePtrT OTy> static void op(OTy& value, StructuralIterator& it);

		template<bool printErrors, RawJsonT OTy> static void op(OTy& value, StructuralIterator& it);

		template<bool printErrors, StringT OTy> static void op(OTy& value, StructuralIterator& buffer);

		template<bool printErrors, RawArrayT OTy> static void op(OTy& value, StructuralIterator& buffer);

		template<bool printErrors, VectorT OTy> static void op(OTy& value, StructuralIterator& buffer);

		template<bool printErrors, StdTupleT OTy> static void op(OTy& value, StructuralIterator& buffer);

		template<bool printErrors, ObjectT OTy, HasFind KTy> static void op(OTy& value, StructuralIterator& buffer, const KTy& excludedKeys);

		template<bool printErrors, ObjectT OTy> static void op(OTy& value, StructuralIterator& buffer);

		template<bool printErrors, typename OTy> static void op(OTy& value, StructuralIterator& buffer);
	};

	class Parser {
	  public:
		inline Parser() noexcept = default;

		inline Parser& operator=(Parser&& other) noexcept {
			swapF(inString, other.inString);
			swapF(section, other.section);
			return *this;
		};

		inline Parser(Parser&& other) noexcept {
			*this = std::move(other);
		};

		inline Parser& operator=(const Parser&) = delete;
		inline Parser(const Parser&) = delete;

		template<bool printErrors = false, bool excludeKeys = false> void parseJson(CoreType auto& data, StringT auto& inStringNew) {
			if (inStringNew.empty()) {
				return;
			}
			if (inString != inStringNew && inStringNew.size() != 0) [[unlikely]] {
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
		Jsonifier::StringView inString{};
		SimdStringReader section{};

		template<typename OTy> inline void reset(OTy& str) {
			if (str.size() == 0) [[unlikely]] {
				return;
			}
			inString = Jsonifier::StringView{ str.data(), str.size() };
			section.reset(inString.size(), reinterpret_cast<StringViewPtr>(inString.data()));
			section.generateJsonIndices();
		}

		inline StructuralIterator begin() noexcept {
			return { &section };
		}
	};
};
