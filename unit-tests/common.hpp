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

#include "unicode_emoji.hpp"
#include <glaze/glaze.hpp>
#include <filesystem>
#include <jsonifier>
#include <fstream>
#include <random>
#include <rt-ut>

struct abc_partial_test_struct {
	bool test_bool{};
	std::string test_string{};
	uint64_t test_uint{};
};

struct abc_test_struct {
	bool test_bool{};
	double test_double{};
	int64_t test_int{};
	uint64_t test_uint{};
	std::string test_string{};
};

template<typename value_type> struct abc_test {
	std::vector<value_type> z, y, x, w, v, u, t, s, r, q, p, o, n, m, l, k, j, i, h, g, f, e, d, c, b, a;
};

template<typename value_type> struct abc_partial_test {
	std::vector<value_type> z, a;
};

inline static std::vector<std::string> stringToVector(const std::string& input) {
	std::vector<std::string> result;

	const std::string startTag = "[START]";

	const std::string endTag = "[END]";

	uint64_t pos = 0;

	while (true) {
		uint64_t startPos = input.find(startTag, pos);

		if (startPos == std::string::npos) {
			break;
		}

		startPos += startTag.size();

		uint64_t endPos = input.find(endTag, startPos);

		if (endPos == std::string::npos) {
			result.emplace_back(input.substr(startPos));

			break;
		}

		result.emplace_back(input.substr(startPos, endPos - startPos));

		pos = endPos + endTag.size();
	}

	return result;
}

inline static std::string vectorToString(const std::vector<std::string>& vec) {
	std::string result;
	for (uint64_t i = 0; i < vec.size(); ++i) {
		const std::string& s = vec[i];
		result += "[START]";
		result += s;
		result += "[END]\n";
	}
	return result;
}

template<> struct jsonifier::core<abc_partial_test_struct> {
	using value_type				 = abc_partial_test_struct;
	static constexpr auto parseValue = createValue<&value_type::test_uint>();
};

template<> struct jsonifier::core<abc_test_struct> {
	using value_type				 = abc_test_struct;
	static constexpr auto parseValue = createValue<&value_type::test_bool, &value_type::test_double, &value_type::test_int, &value_type::test_uint, &value_type::test_string>();
};

template<typename value_type_new> struct jsonifier::core<abc_partial_test<value_type_new>> {
	using value_type				 = abc_partial_test<value_type_new>;
	static constexpr auto parseValue = createValue<&value_type::z, &value_type::a>();
};

template<typename value_type_new> struct jsonifier::core<abc_test<value_type_new>> {
	using value_type				 = abc_test<value_type_new>;
	static constexpr auto parseValue = createValue<&value_type::z, &value_type::y, &value_type::x, &value_type::w, &value_type::v, &value_type::u, &value_type::t, &value_type::s,
		&value_type::r, &value_type::q, &value_type::p, &value_type::o, &value_type::n, &value_type::m, &value_type::l, &value_type::k, &value_type::j, &value_type::i,
		&value_type::h, &value_type::g, &value_type::f, &value_type::e, &value_type::d, &value_type::c, &value_type::b, &value_type::a>();
};

constexpr jsonifier::internal::string_literal basePath{ BASE_PATH };

class test_base {
  public:
	test_base() noexcept = default;

	test_base(const std::string& stringNew, const std::string& fileContentsNew, bool areWeAFailingTestNew)
		: fileContents{ fileContentsNew }, areWeAFailingTest{ areWeAFailingTestNew }, testName{ stringNew } {
	}
	std::string fileContents{};
	bool areWeAFailingTest{};
	std::string testName{};
};

inline static bool processFilesInFolder(std::unordered_map<std::string, test_base>& resultFileContents, const std::string& testType) noexcept {
	try {
		for (const auto& entry: std::filesystem::directory_iterator(std::string{ basePath.operator std::string() } + testType)) {
			if (entry.is_regular_file()) {
				const std::string fileName = entry.path().filename().string();

				if (fileName.size() >= 5 && fileName.substr(fileName.size() - 5) == std::string{ ".json" }) {
					std::ifstream file(entry.path());
					if (file.is_open()) {
						std::string fileContents((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
						bool returnValue					= (fileName.find(".json") != std::string::npos);
						resultFileContents[fileName.data()] = { fileName, fileContents, returnValue };
						file.close();
					} else {
						std::cerr << "Error opening file: " << fileName << std::endl;
						return false;
					}
				}
			}
		}
	} catch (const std::exception& e) {
		std::cerr << "Error while processing files: " << e.what() << std::endl;
		return false;
	}

	return true;
}

class file_handle {
  public:
	static void save_file(const std::string& data, const std::string& path) {
		std::filesystem::path abs_path = std::filesystem::absolute(path);
		std::filesystem::create_directories(abs_path.parent_path());
		std::fstream stream{ abs_path, std::ios::out | std::ios::trunc };
		if (stream.is_open()) {
			stream << data;
			stream.flush();
			bool ok = stream.good();
			stream.close();
			std::cout << (ok ? "Saved: " : "Write error: ") << abs_path.string() << std::endl;
		} else {
			std::cout << "Failed to open for writing: " << abs_path.string() << std::endl;
		}
	}

	static std::string get(const std::string& path) {
		std::fstream stream{ std::filesystem::absolute(path), std::ios::in };
		if (stream.is_open()) {
			return std::string(std::istreambuf_iterator<char>(stream), std::istreambuf_iterator<char>());
		}
		return {};
	}
};

template<typename value_type_new> struct abc_test_generator {
	std::vector<value_type_new> a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v, w, x, y, z;

	inline static constexpr std::string_view charset{ "!#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[]^_`abcdefghijklmnopqrstuvwxyz{|}~\"\\\r\b\f\t\n" };
#if defined(__clang__)
	#pragma clang diagnostic push
	#pragma clang diagnostic ignored "-Wglobal-constructors"
	#pragma clang diagnostic ignored "-Wexit-time-destructors"
#endif
	inline static std::uniform_real_distribution<double> disDouble{ log(std::numeric_limits<double>::min()), log(std::numeric_limits<double>::max()) };
	inline static std::uniform_int_distribution<int64_t> disInt{ std::numeric_limits<int64_t>::min(), std::numeric_limits<int64_t>::max() };
	inline static std::uniform_int_distribution<uint64_t> disUint{ std::numeric_limits<uint64_t>::min(), std::numeric_limits<uint64_t>::max() };
	inline static std::uniform_int_distribution<uint64_t> disCharSet{ 0ull, charset.size() - 1 };
	inline static std::uniform_int_distribution<uint64_t> disString{ 32ull, 64ull };
	inline static std::uniform_int_distribution<uint64_t> disUnicodeEmoji{ 0ull, std::size(unicode_emoji::unicodeEmoji) - 1 };
	inline static std::uniform_int_distribution<uint64_t> disBool{ 0, 100 };
#if defined(__clang__)
	#pragma clang diagnostic pop
#endif
	inline static std::random_device& randomEngine() {
		static std::random_device* randomEngine{ new std::random_device{} };
		return *randomEngine;
	}

	inline static std::mt19937_64 gen{ randomEngine()() };

	template<jsonifier::concepts::integer_t value_type01, jsonifier::concepts::integer_t value_type02>
	static value_type01 randomizeNumberUniform(value_type01 start, value_type02 end) {
		std::uniform_int_distribution<value_type01> dis{ start, static_cast<value_type01>(end) };
		return dis(gen);
	}

	static void insertUnicodeInJSON(std::string& jsonString) {
		auto newStringView = unicode_emoji::unicodeEmoji[disUnicodeEmoji(gen)];
		jsonString += static_cast<std::string>(newStringView);
	}

	static std::string generateString() {
		auto length{ disString(gen) };
		auto unicodeCount = length / 32ull;
		std::vector<uint64_t> unicodeIndices{};
		static constexpr auto checkForPresenceOfIndex = [](auto& indices, auto index, auto lengthNew, auto&& checkForPresenceOfIndexNew) -> void {
			if (std::find(indices.begin(), indices.end(), index) != indices.end()) {
				index = randomizeNumberUniform(0ull, lengthNew);
				checkForPresenceOfIndexNew(indices, index, lengthNew, checkForPresenceOfIndexNew);
			} else {
				indices.emplace_back(index);
			}
		};
		for (uint64_t x = 0; x < unicodeCount; ++x) {
			auto newValue = randomizeNumberUniform(0ull, length);
			checkForPresenceOfIndex(unicodeIndices, newValue, length, checkForPresenceOfIndex);
		}
		std::sort(unicodeIndices.begin(), unicodeIndices.end(), std::less<uint64_t>{});

		std::string result{};
		auto iter				 = unicodeIndices.begin();
		for (uint64_t x = 0; x < length; ++x) {
			if (iter < unicodeIndices.end() && x == *iter) [[unlikely]] {
				insertUnicodeInJSON(result);
				++iter;
			} else {
				result += charset[disCharSet(gen)];
			}
		}

		return result;
	}

	static double generateDouble() {
		double logValue = disDouble(gen);
		bool negative{ generateBool() };
		return negative ? -std::exp(logValue) : std::exp(logValue);
	}

	static bool generateBool() {
		return static_cast<bool>(disBool(gen) >= 50);
	}

	static uint64_t generateUint() {
		return disUint(gen);
	}

	static int64_t generateInt() {
		return disInt(gen);
	}

	static abc_test_struct generate_test_struct() {
		abc_test_struct return_values{};
		return_values.test_bool	  = generateBool();
		return_values.test_double = generateDouble();
		return_values.test_int	  = generateInt();
		return_values.test_uint	  = generateUint();
		return_values.test_string = generateString();
		return return_values;
	}

	static abc_test<abc_test_struct> generate_test() {
		abc_test<abc_test_struct> return_values{};
		auto fill = [&](auto& v) {
			const auto array_size_01 = generateUint() % 15;
			v.resize(array_size_01);
			for (uint64_t x = 0; x < array_size_01; ++x) {
				v[x] = generate_test_struct();
			}
		};

		fill(return_values.a);
		fill(return_values.b);
		fill(return_values.c);
		fill(return_values.d);
		fill(return_values.e);
		fill(return_values.f);
		fill(return_values.g);
		fill(return_values.h);
		fill(return_values.i);
		fill(return_values.j);
		fill(return_values.k);
		fill(return_values.l);
		fill(return_values.m);
		fill(return_values.n);
		fill(return_values.o);
		fill(return_values.p);
		fill(return_values.q);
		fill(return_values.r);
		fill(return_values.s);
		fill(return_values.t);
		fill(return_values.u);
		fill(return_values.v);
		fill(return_values.w);
		fill(return_values.x);
		fill(return_values.y);
		fill(return_values.z);
		return return_values;
	}
};

template<typename value_type_new> struct jsonifier::core<abc_test_generator<value_type_new>> {
	using value_type				 = abc_test_generator<value_type_new>;
	static constexpr auto parseValue = createValue<&value_type::a, &value_type::b, &value_type::c, &value_type::d, &value_type::e, &value_type::f, &value_type::g, &value_type::h,
		&value_type::i, &value_type::j, &value_type::k, &value_type::l, &value_type::m, &value_type::n, &value_type::o, &value_type::p, &value_type::q, &value_type::r,
		&value_type::s, &value_type::t, &value_type::u, &value_type::v, &value_type::w, &value_type::x, &value_type::y, &value_type::z>();
};

template<typename value_type_out, typename value_type_in, const auto& pass_tests, const auto& pass_values> struct pass_tests_runner {
	template<uint64_t index> static void impl(jsonifier::jsonifier_core<>& parser) {
		value_type_in data{};
		rt_ut::unit_test<rt_ut::string_literal{ rt_ut::create_string_literal<pass_tests[index].size()>(pass_tests[index].data()) }, true>::assert_eq(pass_values[index], [&]() {
			parser.parseJson(data, pass_tests[index]);
			for (auto& value: parser.getErrors()) {
				std::cout << "Jsonifier Error: " << value << std::endl;
			}
			return static_cast<value_type_out>(data);
		});
	}
};

template<typename value_type_in, const auto& fail_tests> struct fail_tests_runner {
	template<uint64_t index> static void impl(jsonifier::jsonifier_core<>& parser) {
		value_type_in data{};
		rt_ut::unit_test<rt_ut::string_literal{ rt_ut::create_string_literal<fail_tests[index].size()>(fail_tests[index].data()) }, true>::assert_ne(true, [&]() {
			return parser.parseJson(data, fail_tests[index]);
		});
	}
};

template<typename value_type_out, typename value_type_in, const auto& pass_tests, const auto& pass_values, template<typename, typename, const auto&, const auto&> typename functor,
	typename integer_sequence>
struct pass_test_runner {};

template<typename value_type_out, typename value_type_in, const auto& pass_tests, const auto& pass_values, template<typename, typename, const auto&, const auto&> typename functor,
	uint64_t... indices>
struct pass_test_runner<value_type_out, value_type_in, pass_tests, pass_values, functor, jsonifier::internal::integer_sequence<indices...>> {
	static void impl() {
		jsonifier::jsonifier_core<> parser{};
		(functor<value_type_out, value_type_in, pass_tests, pass_values>::template impl<indices>(parser), ...);
	}
};

template<typename value_type_in, const auto& fail_tests, template<typename, const auto&> typename functor, typename integer_sequence> struct fail_test_runner {};

template<typename value_type_in, const auto& fail_tests, template<typename, const auto&> typename functor, uint64_t... indices>
struct fail_test_runner<value_type_in, fail_tests, functor, jsonifier::internal::integer_sequence<indices...>> {
	static void impl() {
		jsonifier::jsonifier_core<> parser{};
		(functor<value_type_in, fail_tests>::template impl<indices>(parser), ...);
	}
};
