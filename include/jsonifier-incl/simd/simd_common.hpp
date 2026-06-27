/*
	MIT License

	Copyright (c) 2024 RealTimeChris

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

#include <jsonifier-incl/utilities/type_entities.hpp>
#include <jsonifier-incl/containers/array.hpp>
#include <jsonifier-incl/simd/fallback.hpp>
#include <jsonifier-incl/simd/popcount.hpp>
#include <jsonifier-incl/simd/avx.hpp>
#include <jsonifier-incl/simd/neon.hpp>
#include <iostream>
#include <concepts>
#include <sstream>
#include <cstring>
#include <cstdint>
#include <bitset>

namespace jsonifier::simd {

#if JSONIFIER_PLATFORM_MAC
	using avx_list = internal::type_list<internal::type_holder<64, internal::avx_type_wrapper<internal::avx_type::m512>, uint64_t, std::numeric_limits<uint64_t>::max()>,
		internal::type_holder<32, internal::avx_type_wrapper<internal::avx_type::m256>, uint32_t, std::numeric_limits<uint32_t>::max()>,
		internal::type_holder<16, internal::avx_type_wrapper<internal::avx_type::m128>, uint64_t, std::numeric_limits<uint64_t>::max()>>;
#else
	using avx_list = internal::type_list<internal::type_holder<64, internal::avx_type_wrapper<internal::avx_type::m512>, uint64_t, std::numeric_limits<uint64_t>::max()>,
		internal::type_holder<32, internal::avx_type_wrapper<internal::avx_type::m256>, uint32_t, std::numeric_limits<uint32_t>::max()>,
		internal::type_holder<16, internal::avx_type_wrapper<internal::avx_type::m128>, uint16_t, std::numeric_limits<uint16_t>::max()>>;
#endif

#if JSONIFIER_PLATFORM_MAC
	using avx_integer_list = internal::type_list<internal::type_holder<64, internal::avx_type_wrapper<internal::avx_type::m512>, uint64_t, 64>,
		internal::type_holder<32, internal::avx_type_wrapper<internal::avx_type::m256>, uint32_t, 32>,
		internal::type_holder<16, internal::avx_type_wrapper<internal::avx_type::m128>, uint64_t, 16>>;
#else
	using avx_integer_list = internal::type_list<internal::type_holder<64, internal::avx_type_wrapper<internal::avx_type::m512>, uint64_t, 64>,
		internal::type_holder<32, internal::avx_type_wrapper<internal::avx_type::m256>, uint32_t, 32>,
		internal::type_holder<16, internal::avx_type_wrapper<internal::avx_type::m128>, uint16_t, 16>>;
#endif

}

namespace jsonifier::internal {

	inline constexpr array<bool, 256ULL> whitespaceTable{ []() constexpr {
		array<bool, 256ULL> returnValues{};
		returnValues[static_cast<uint64_t>('\t')] = true;
		returnValues[static_cast<uint64_t>(' ')]  = true;
		returnValues[static_cast<uint64_t>('\n')] = true;
		returnValues[static_cast<uint64_t>('\r')] = true;
		return returnValues;
	}() };

}
