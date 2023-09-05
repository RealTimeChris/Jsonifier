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

#include <jsonifier/NumberUtils.hpp>
#include <jsonifier/StringUtils.hpp>

namespace JsonifierInternal {

	template<bool excludeKeys, typename ValueType = void> struct SerializeImpl;

	template<bool excludeKeys> struct Serialize {
		template<typename ValueType, VectorLike BufferType> inline static void op(ValueType&& value, BufferType&& buffer, uint64_t& index) {
			SerializeImpl<excludeKeys, RefUnwrap<ValueType>>::op(std::forward<ValueType>(value), std::forward<BufferType>(buffer), index);
		}

		template<typename ValueType, VectorLike BufferType, HasFind KeyType> inline static void op(ValueType&& value, BufferType&& buffer, uint64_t& index, const KeyType& keys) {
			SerializeImpl<excludeKeys, RefUnwrap<ValueType>>::op(std::forward<ValueType>(value), std::forward<BufferType>(buffer), index, keys);
		}
	};

	class Serializer {
	  public:
		constexpr Serializer() noexcept = default;

		template<bool excludeKeys = false, CoreType ValueType, VectorLike BufferType> inline void serializeJson(ValueType&& data, BufferType&& buffer) {
			uint64_t index{};
			if constexpr (excludeKeys) {
				if constexpr (HasExcludedKeys<ValueType>) {
					Serialize<excludeKeys>::op(std::forward<ValueType>(data), stringBuffer, index, data.excludedKeys);
				} else {
					Serialize<excludeKeys>::op(std::forward<ValueType>(data), stringBuffer, index);
				}
			} else {
				Serialize<excludeKeys>::op(std::forward<ValueType>(data), stringBuffer, index);
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
