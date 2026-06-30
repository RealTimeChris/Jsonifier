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
#pragma once

#include <jsonifier-incl/utilities/type_entities.hpp>
#include <cstdint>
#include <vector>

namespace jsonifier::concepts {

	template<typename, typename> inline constexpr bool is_same_v						  = false;
	template<typename value_type> inline constexpr bool is_same_v<value_type, value_type> = true;

	template<typename value_type> using base_t = internal::remove_cvref_t<value_type>;

	template<typename value_type_01, typename value_type_02>
	concept same_as_types = is_same_v<internal::remove_reference_t<value_type_01>, internal::remove_reference_t<value_type_02>>;

	template<typename value_type>
	concept enum_types = std::is_enum_v<base_t<value_type>> && std::is_unsigned_v<std::underlying_type_t<base_t<value_type>>>;

	template<typename value_type>
	concept integral_types = std::is_integral_v<base_t<value_type>> && !same_as_types<base_t<value_type>, bool>;

	template<typename value_type>
	concept trivially_copyable_types = __is_trivial(base_t<value_type>) || std::is_standard_layout_v<base_t<value_type>>;

	template<typename value_type>
	concept uint_types = std::is_unsigned_v<base_t<value_type>> && integral_types<value_type>;

	template<typename value_type>
	concept uint8_types = uint_types<value_type> && sizeof(internal::remove_cvref_t<value_type>) == 1;

	template<typename value_type>
	concept uint16_types = uint_types<value_type> && sizeof(internal::remove_cvref_t<value_type>) == 2;

	template<typename value_type>
	concept uint32_types = uint_types<value_type> && sizeof(internal::remove_cvref_t<value_type>) == 4;

	template<typename value_type>
	concept uint64_types = uint_types<value_type> && sizeof(internal::remove_cvref_t<value_type>) == 8;

	template<typename value_type>
	concept uintegral_or_enum_types = uint_types<value_type> || enum_types<value_type>;

	template<typename value_type_01, typename value_type_02>
	concept at_least_one_enum_types = ( enum_types<value_type_01> && uint_types<value_type_02> ) || (uint_types<value_type_01> && enum_types<value_type_02>);

	template<typename value_type_01, typename value_type_02>
	concept indexable_types =
		at_least_one_enum_types<value_type_01, value_type_02> || same_as_types<value_type_01, value_type_02> || (integral_types<value_type_01> && integral_types<value_type_02>);


}
