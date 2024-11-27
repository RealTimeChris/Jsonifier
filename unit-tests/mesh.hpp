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

struct batch {
	std::vector<int64_t> indexRange{};
	std::vector<int64_t> vertexRange{};
	std::vector<int64_t> usedBones{};
};

struct morph_targets {};

struct mesh_message {
	std::vector<batch> batches{};
	morph_targets morphTargets{};
	std::vector<double> positions{};
	std::vector<double> tex0{};
	std::vector<int64_t> colors{};
	std::vector<std::vector<int64_t>> influences{};
	std::vector<double> normals{};
	std::vector<int64_t> indices{};
};

template<> struct jsonifier::core<batch> {
	using value_type				 = batch;
	static constexpr auto parseValue = createValue<&value_type::indexRange, &value_type::vertexRange, &value_type::usedBones>();
};

template<> struct jsonifier::core<morph_targets> {
	using value_type				 = morph_targets;
	static constexpr auto parseValue = createValue();
};

template<> struct jsonifier::core<mesh_message> {
	using value_type				 = mesh_message;
	static constexpr auto parseValue = createValue<&value_type::batches, &value_type::morphTargets, &value_type::positions, &value_type::tex0, &value_type::colors,
		&value_type::influences, &value_type::normals, &value_type::indices>();
};
