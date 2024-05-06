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

#include <source_location>
#include <unordered_map>
#include <exception>
#include <string>

namespace jsonifier_internal {

	// https://en.wikipedia.org/wiki/Fowler-Noll-Vo_hash_function
	// http://www.isthe.com/chongo/tech/comp/fnv/index.html#FNV-param
	static constexpr uint32_t fnv32OffsetBasis{ 0x811c9dc5u };
	static constexpr uint32_t fnv32Prime{ 0x01000193u };

	JSONIFIER_INLINE uint32_t fnv1aHashRt(const void* value, uint64_t size) {
		uint32_t hash		  = fnv32OffsetBasis * fnv32Prime;
		const uint8_t* newPtr = static_cast<const uint8_t*>(value);
		for (uint64_t x = 0; x < size; ++x) {
			hash = (hash ^ static_cast<uint32_t>(static_cast<std::byte>(newPtr[x]))) * fnv32Prime;
		}
		return static_cast<uint32_t>(hash >> 8);
	}

	JSONIFIER_INLINE uint32_t fnv1aHashRt(const void* value, uint64_t size, uint32_t seed) {
		uint32_t hash			= (fnv32OffsetBasis ^ seed) * fnv32Prime;
		const uint8_t* startPtr = static_cast<const uint8_t*>(value);
		while (size >= 8) {
			hash = (hash ^ static_cast<uint32_t>(*startPtr++)) * fnv32Prime;
			hash = (hash ^ static_cast<uint32_t>(*startPtr++)) * fnv32Prime;
			hash = (hash ^ static_cast<uint32_t>(*startPtr++)) * fnv32Prime;
			hash = (hash ^ static_cast<uint32_t>(*startPtr++)) * fnv32Prime;
			hash = (hash ^ static_cast<uint32_t>(*startPtr++)) * fnv32Prime;
			hash = (hash ^ static_cast<uint32_t>(*startPtr++)) * fnv32Prime;
			hash = (hash ^ static_cast<uint32_t>(*startPtr++)) * fnv32Prime;
			hash = (hash ^ static_cast<uint32_t>(*startPtr++)) * fnv32Prime;
			size -= 8;
		}
		while (size > 0) {
			hash = (hash ^ static_cast<uint32_t>(*startPtr++)) * fnv32Prime;
			--size;
		}
		return static_cast<uint32_t>(hash >> 8);
	}

	template<typename string_t> constexpr uint32_t fnv1aHashCt(const string_t& value, uint32_t seed) {
		uint32_t hash = (fnv32OffsetBasis ^ seed) * fnv32Prime;
		for (const auto& valueNew: value) {
			hash = (hash ^ static_cast<uint32_t>(static_cast<std::byte>(valueNew))) * fnv32Prime;
		}
		return static_cast<uint32_t>(hash >> 8);
	}

	template<typename value_type> struct hash;
}