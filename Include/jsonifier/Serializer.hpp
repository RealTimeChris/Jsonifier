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
/// Feb 20, 2023
#pragma once

#include <jsonifier/NumberParsing.hpp>
#include <jsonifier/StringParsing.hpp>

namespace JsonifierInternal {

	struct SerializeNoKeys {
		template<NullT ValueType, VectorLike BufferType> static void op(ValueType& value, BufferType& buffer, size_t& index);

		template<BoolT ValueType, VectorLike BufferType> static void op(ValueType& value, BufferType& buffer, size_t& index);

		template<NumT ValueType, VectorLike BufferType> static void op(ValueType& value, BufferType& buffer, size_t& index);

		template<EnumT ValueType, VectorLike BufferType> static void op(ValueType& value, BufferType& buffer, size_t& index);

		template<CharT ValueType, VectorLike BufferType> static void op(ValueType& value, BufferType& buffer, size_t& index);

		template<StringT ValueType, VectorLike BufferType> static void op(ValueType& value, BufferType& buffer, size_t& index);

		template<RawJsonT ValueType, VectorLike BufferType> static void op(ValueType& value, BufferType& buffer, size_t& index);

		template<RawArrayT ValueType, VectorLike BufferType> static void op(ValueType& value, BufferType& buffer, size_t& index);

		template<ArrayT ValueType, VectorLike BufferType> static void op(ValueType& value, BufferType& buffer, size_t& index);

		template<ArrayTupleT ValueType, VectorLike BufferType> static void op(ValueType& value, BufferType& buffer, size_t& index);

		template<ObjectT ValueType, VectorLike BufferType> static void op(ValueType& value, BufferType& buffer, size_t& index);

		template<typename ValueType, VectorLike BufferType> static void op(ValueType& value, BufferType& buffer, size_t& index);
	};

	struct SerializeWithKeys {
		template<NullT ValueType, VectorLike BufferType> static void op(ValueType& value, BufferType& buffer, size_t& index);

		template<BoolT ValueType, VectorLike BufferType> static void op(ValueType& value, BufferType& buffer, size_t& index);

		template<NumT ValueType, VectorLike BufferType> static void op(ValueType& value, BufferType& buffer, size_t& index);

		template<EnumT ValueType, VectorLike BufferType> static void op(ValueType& value, BufferType& buffer, size_t& index);

		template<CharT ValueType, VectorLike BufferType> static void op(ValueType& value, BufferType& buffer, size_t& index);

		template<StringT ValueType, VectorLike BufferType> static void op(ValueType& value, BufferType& buffer, size_t& index);

		template<RawJsonT ValueType, VectorLike BufferType> static void op(ValueType& value, BufferType& buffer, size_t& index);

		template<RawArrayT ValueType, VectorLike BufferType> static void op(ValueType& value, BufferType& buffer, size_t& index);

		template<ArrayT ValueType, VectorLike BufferType> static void op(ValueType& value, BufferType& buffer, size_t& index);

		template<ArrayTupleT ValueType, VectorLike BufferType> static void op(ValueType& value, BufferType& buffer, size_t& index);

		template<ObjectT ValueType, VectorLike BufferType, HasFind KeyType>
		static void op(ValueType& value, BufferType& buffer, size_t& index, const KeyType& excludedKeys);

		template<ObjectT ValueType, VectorLike BufferType> static void op(ValueType& value, BufferType& buffer, size_t& index);

		template<typename ValueType, VectorLike BufferType> static void op(ValueType& value, BufferType& buffer, size_t& index);
	};

	class Serializer {
	  public:
		inline constexpr Serializer() noexcept = default;

		template<bool excludeKeys = false> inline void serializeJson(CoreType auto& data, VectorLike auto& buffer) {
			size_t index{};
			using ValueType = decltype(data);
			if constexpr (excludeKeys) {
				if constexpr (HasExcludedKeys<ValueType>) {
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
