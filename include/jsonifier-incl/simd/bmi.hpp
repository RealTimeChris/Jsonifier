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

namespace jsonifier::simd {

	template<typename value_type> JSONIFIER_INLINE constexpr value_type blsr_constexpr(value_type value) noexcept {
		return value & (value - 1);
	}

	template<concepts::uint8_types value_type> JSONIFIER_INLINE constexpr value_type blsr(const value_type value) noexcept {
		return blsr_constexpr(value);
	}

	template<concepts::uint16_types value_type> JSONIFIER_INLINE constexpr value_type blsr(const value_type value) noexcept {
		if (std::is_constant_evaluated()) {
			return blsr_constexpr(value);
		}
		return static_cast<value_type>(value & (value - 1));
	}

	template<concepts::uint32_types value_type> JSONIFIER_INLINE constexpr value_type blsr(const value_type value) noexcept {
		if (std::is_constant_evaluated()) {
			return blsr_constexpr(value);
		}
#if (JSONIFIER_COMPILER_MSVC || JSONIFIER_COMPILER_CLANG || JSONIFIER_COMPILER_GCC) && !JSONIFIER_ARCH_ARM64
		return static_cast<value_type>(_blsr_u32(static_cast<unsigned int>(value)));
#else
		return blsr_constexpr(value);
#endif
	}
	template<concepts::uint64_types value_type> JSONIFIER_INLINE constexpr value_type blsr(const value_type value) noexcept {
		if (std::is_constant_evaluated()) {
			return blsr_constexpr(value);
		}
#if (JSONIFIER_COMPILER_MSVC || JSONIFIER_COMPILER_CLANG || JSONIFIER_COMPILER_GCC) && !JSONIFIER_ARCH_ARM64
		return static_cast<value_type>(_blsr_u64(static_cast<uint64_t>(value)));
#else
		return blsr_constexpr(value);
#endif
	}

	template<typename value_type> JSONIFIER_INLINE constexpr value_type tzcnt(const value_type value) noexcept {
		return static_cast<value_type>(std::countr_zero(value));
	}

}
