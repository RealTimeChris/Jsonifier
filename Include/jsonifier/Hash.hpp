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
	static constexpr uint64_t fnv64OffsetBasis{ 0xcbf29ce484222325 };
	static constexpr uint64_t fnv64Prime{ 0x00000100000001B3 };

	struct fnv1a_hash {
		constexpr uint64_t operator()(string_view_ptr value, int64_t size, uint64_t seed = 0) const {
			uint64_t hash = fnv64OffsetBasis * fnv64Prime ^ seed;
			int64_t x{};
			if (!std::is_constant_evaluated()) {
				for (; x < size - 8; x += 8) {
					hash = (hash ^ static_cast<uint64_t>(value[x])) * fnv64Prime;
					hash = (hash ^ static_cast<uint64_t>(value[x + 1])) * fnv64Prime;
					hash = (hash ^ static_cast<uint64_t>(value[x + 2])) * fnv64Prime;
					hash = (hash ^ static_cast<uint64_t>(value[x + 3])) * fnv64Prime;
					hash = (hash ^ static_cast<uint64_t>(value[x + 4])) * fnv64Prime;
					hash = (hash ^ static_cast<uint64_t>(value[x + 5])) * fnv64Prime;
					hash = (hash ^ static_cast<uint64_t>(value[x + 6])) * fnv64Prime;
					hash = (hash ^ static_cast<uint64_t>(value[x + 7])) * fnv64Prime;
				}
			}
			for (; x < size; ++x) {
				hash = (hash ^ static_cast<uint64_t>(value[x])) * fnv64Prime;
			}
			return hash >> 8;
		}
	};

	template<typename value_type> struct hash;
}