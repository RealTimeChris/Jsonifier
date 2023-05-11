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
        License along with this library; if not, Write to the Free Software
        Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301
        USA
*/
/// https://github.com/RealTimeChris/Jsonifier
/// Feb 20, 2023
#pragma once

#include <jsonifier/NumberParsing.hpp>
#include <jsonifier/StringParsing.hpp>

namespace JsonifierInternal {

	struct SerializeNoKeys {
		template<NullT OTy, VectorLike BTy> static void op(OTy& value, BTy& buffer, size_t& index);

		template<BoolT OTy, VectorLike BTy> static void op(OTy& value, BTy& buffer, size_t& index);

		template<NumT OTy, VectorLike BTy> static void op(OTy& value, BTy& buffer, size_t& index);

		template<EnumT OTy, VectorLike BTy> static void op(OTy& value, BTy& buffer, size_t& index);

		template<CharT OTy, VectorLike BTy> static void op(OTy& value, BTy& buffer, size_t& index);

		template<StringT OTy, VectorLike BTy> static void op(OTy& value, BTy& buffer, size_t& index);

		template<RawJsonT OTy, VectorLike BTy> static void op(OTy& value, BTy& buffer, size_t& index);

		template<RawArrayT OTy, VectorLike BTy> static void op(OTy& value, BTy& buffer, size_t& index);

		template<VectorT OTy, VectorLike BTy> static void op(OTy& value, BTy& buffer, size_t& index);

		template<ArrayTupleT OTy, VectorLike BTy> static void op(OTy& value, BTy& buffer, size_t& index);

		template<StdTupleT OTy, VectorLike BTy> static void op(OTy& value, BTy& buffer, size_t& index);

		template<ObjectT OTy, VectorLike BTy> static void op(OTy& value, BTy& buffer, size_t& index);

		template<typename OTy, VectorLike BTy> static void op(OTy& value, BTy& buffer, size_t& index);
	};

	struct SerializeWithKeys {
		template<NullT OTy, VectorLike BTy> static void op(OTy& value, BTy& buffer, size_t& index);

		template<BoolT OTy, VectorLike BTy> static void op(OTy& value, BTy& buffer, size_t& index);

		template<NumT OTy, VectorLike BTy> static void op(OTy& value, BTy& buffer, size_t& index);

		template<EnumT OTy, VectorLike BTy> static void op(OTy& value, BTy& buffer, size_t& index);

		template<CharT OTy, VectorLike BTy> static void op(OTy& value, BTy& buffer, size_t& index);

		template<StringT OTy, VectorLike BTy> static void op(OTy& value, BTy& buffer, size_t& index);

		template<RawJsonT OTy, VectorLike BTy> static void op(OTy& value, BTy& buffer, size_t& index);

		template<RawArrayT OTy, VectorLike BTy> static void op(OTy& value, BTy& buffer, size_t& index);

		template<VectorT OTy, VectorLike BTy> static void op(OTy& value, BTy& buffer, size_t& index);

		template<ArrayTupleT OTy, VectorLike BTy> static void op(OTy& value, BTy& buffer, size_t& index);

		template<StdTupleT OTy, VectorLike BTy> static void op(OTy& value, BTy& buffer, size_t& index);

		template<ObjectT OTy, VectorLike BTy, HasFind KTy> static void op(OTy& value, BTy& buffer, size_t& index, const KTy& excludedKeys);

		template<ObjectT OTy, VectorLike BTy> static void op(OTy& value, BTy& buffer, size_t& index);

		template<typename OTy, VectorLike BTy> static void op(OTy& value, BTy& buffer, size_t& index);
	};

	class Serializer {
	  public:
		inline constexpr Serializer() noexcept = default;

		template<bool excludeKeys = false> inline void serializeJson(CoreType auto& data, VectorLike auto& buffer) {
			size_t index{};
			using OTy = decltype(data);
			using BTy = decltype(buffer);
			if constexpr (excludeKeys) {
				if constexpr (HasExcludedKeys<OTy>) {
					SerializeWithKeys::op(data, stringBuffer, index, data.excludedKeys);
				} else {
					SerializeWithKeys::op(data, stringBuffer, index);
				}
			} else {
				SerializeNoKeys::op(data, stringBuffer, index);
			}
			if (buffer.size() != index) [[unlikely]] {
				buffer.resize(index);
			}
			std::memcpy(buffer.data(), stringBuffer.data(), index);
		}

	  protected:
		Jsonifier::String stringBuffer{};
	};

}
