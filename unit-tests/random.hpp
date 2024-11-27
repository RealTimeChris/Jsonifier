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
/// Sep 17, 2024
#pragma once

#include "common.hpp"

struct friend_element {
	int64_t id{};
	std::string name{};
	std::string phone{};
};

struct result_data {
	int64_t id{};
	std::string avatar{};
	int64_t age{};
	bool admin{};
	std::string name{};
	std::string company{};
	std::string phone{};
	std::string email{};
	std::string birthDate{};
	std::vector<friend_element> friends{};
	std::string field{};
};

struct random_message {
	int64_t id{};
	std::string jsonrpc{};
	int64_t total{};
	std::vector<result_data> result{};
};

template<> struct jsonifier::core<friend_element> {
	using value_type				 = friend_element;
	static constexpr auto parseValue = createValue<&value_type::id, &value_type::name, &value_type::phone>();
};

template<> struct jsonifier::core<result_data> {
	using value_type				 = result_data;
	static constexpr auto parseValue = createValue<&value_type::id, &value_type::avatar, &value_type::age, &value_type::admin, &value_type::name, &value_type::company,
		&value_type::phone, &value_type::email, &value_type::birthDate, &value_type::friends, &value_type::field>();
};

template<> struct jsonifier::core<random_message> {
	using value_type				 = random_message;
	static constexpr auto parseValue = createValue<&value_type::id, &value_type::jsonrpc, &value_type::total, &value_type::result>();
};
