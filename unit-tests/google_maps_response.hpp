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
/// https://github.com/RealTimeChris/Json-Performance
#pragma once

#include "common.hpp"

struct distance_data {
	std::string text{};
	int64_t value{};
};

struct element {
	distance_data distance{};
	distance_data duration{};
	std::string status{};
};

struct row {
	std::vector<element> elements{};
};

struct google_maps_response_message {
	std::vector<std::string> destination_addresses{};
	std::vector<std::string> origin_addresses{};
	std::vector<row> rows{};
	std::string status{};
};

template<> struct jsonifier::core<distance_data> {
	using value_type				 = distance_data;
	static constexpr auto parseValue = createValue<&value_type::text, &value_type::value>();
};

template<> struct jsonifier::core<element> {
	using value_type				 = element;
	static constexpr auto parseValue = createValue<&value_type::distance, &value_type::duration, &value_type::status>();
};

template<> struct jsonifier::core<row> {
	using value_type				 = row;
	static constexpr auto parseValue = createValue<&value_type::elements>();
};

template<> struct jsonifier::core<google_maps_response_message> {
	using value_type				 = google_maps_response_message;
	static constexpr auto parseValue = createValue<&value_type::destination_addresses, &value_type::origin_addresses, &value_type::rows, &value_type::status>();
};
