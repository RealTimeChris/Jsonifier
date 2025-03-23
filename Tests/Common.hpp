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

#include "UnicodeEmoji.hpp"
#include <filesystem>
#include <fstream>
#include <thread>
#include <random>
#include <rt-ut>

constexpr jsonifier::internal::string_literal basePath{ BASE_PATH };
constexpr jsonifier::internal::string_literal testPath{ basePath };

class test_base {
  public:
	test_base() noexcept = default;

	test_base(const std::string& stringNew, const std::string& fileContentsNew, bool areWeAFailingTestNew)
		: fileContents{ fileContentsNew }, areWeAFailingTest{ areWeAFailingTestNew }, testName{ stringNew } {};
	std::string fileContents{};
	bool areWeAFailingTest{};
	std::string testName{};
};

bool processFilesInFolder(std::unordered_map<std::string, test_base>& resultFileContents, const std::string& testType) noexcept {
	try {
		for (const auto& entry: std::filesystem::directory_iterator(std::string{ testPath.operator std::string() } + testType)) {
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

struct test_struct {
	std::vector<std::string> testString{};
	std::vector<uint64_t> testUint{};
	std::vector<double> testDouble{};
	std::vector<int64_t> testInt{};
	std::vector<bool> testBool{};
};

struct partial_test_struct {
	std::vector<std::string> testString{};
	std::vector<bool> testBool{};
};

struct abc_test_struct {
	std::vector<bool> testBool{};
	std::vector<int64_t> testInt{};
	std::vector<double> testDouble{};
	std::vector<uint64_t> testUint{};
	std::vector<std::string> testString{};
};

template<typename value_type> struct test {
	std::vector<value_type> a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v, w, x, y, z;
};

template<typename value_type> struct partial_test {
	std::vector<value_type> m, s;
};

template<typename value_type> struct abc_test {
	std::vector<value_type> z, y, x, w, v, u, t, s, r, q, p, o, n, m, l, k, j, i, h, g, f, e, d, c, b, a;
};

template<typename value_type> struct test_generator {
	std::vector<value_type> a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v, w, x, y, z;

	inline static constexpr std::string_view charset{ "!#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[]^_`abcdefghijklmnopqrstuvwxyz{|}~\"\\\r\b\f\t\n" };
	inline static std::uniform_real_distribution<double> disDouble{ log(std::numeric_limits<double>::min()), log(std::numeric_limits<double>::max()) };
	inline static std::uniform_int_distribution<int64_t> disInt{ std::numeric_limits<int64_t>::min(), std::numeric_limits<int64_t>::max() };
	inline static std::uniform_int_distribution<size_t> disUint{ std::numeric_limits<size_t>::min(), std::numeric_limits<size_t>::max() };
	inline static std::uniform_int_distribution<size_t> disCharSet{ 0ull, charset.size() - 1 };
	inline static std::uniform_int_distribution<size_t> disString{ 32ull, 64ull };
	inline static std::uniform_int_distribution<size_t> disUnicodeEmoji{ 0ull, std::size(unicode_emoji::unicodeEmoji) - 1 };
	inline static std::uniform_int_distribution<size_t> disBool{ 0, 100 };
	inline static std::random_device randomEngine{};
	inline static std::mt19937_64 gen{ randomEngine() };

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
		std::vector<size_t> unicodeIndices{};
		static constexpr auto checkForPresenceOfIndex = [](auto& indices, auto index, auto length, auto&& checkForPresenceOfIndexNew) -> void {
			if (std::find(indices.begin(), indices.end(), index) != indices.end()) {
				index = randomizeNumberUniform(0ull, length);
				checkForPresenceOfIndexNew(indices, index, length, checkForPresenceOfIndexNew);
			} else {
				indices.emplace_back(index);
			}
		};
		for (size_t x = 0; x < unicodeCount; ++x) {
			auto newValue = randomizeNumberUniform(0ull, length);
			checkForPresenceOfIndex(unicodeIndices, newValue, length, checkForPresenceOfIndex);
		}
		std::sort(unicodeIndices.begin(), unicodeIndices.end(), std::less<size_t>{});

		std::string result{};
		size_t insertedUnicode = 0;
		auto iter			   = unicodeIndices.begin();
		for (size_t x = 0; x < length; ++x) {
			if (iter < unicodeIndices.end() && x == *iter) [[unlikely]] {
				insertUnicodeInJSON(result);
				insertedUnicode++;
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

	static size_t generateUint() {
		return disUint(gen);
	}

	static int64_t generateInt() {
		return disInt(gen);
	}

	test_generator() {
		auto fill = [&](auto& v) {
			auto arraySize01 = randomizeNumberUniform(5ull, 15ull);
			v.resize(arraySize01);
			for (size_t x = 0; x < arraySize01; ++x) {
				auto arraySize02 = randomizeNumberUniform(25ull, 35ull);
				auto arraySize03 = randomizeNumberUniform(0ull, 10ull);
				for (size_t y = 0; y < arraySize03; ++y) {
					auto newString = generateString();
					v[x].testString.emplace_back(newString);
				}
				arraySize03 = randomizeNumberUniform(0ull, 10ull);
				for (size_t y = 0; y < arraySize03; ++y) {
					v[x].testUint.emplace_back(generateUint());
				}
				arraySize03 = randomizeNumberUniform(0ull, 10ull);
				for (size_t y = 0; y < arraySize03; ++y) {
					v[x].testInt.emplace_back(generateInt());
				}
				arraySize03 = randomizeNumberUniform(0ull, 10ull);
				for (size_t y = 0; y < arraySize03; ++y) {
					auto newBool = generateBool();
					v[x].testBool.emplace_back(newBool);
				}
				arraySize03 = randomizeNumberUniform(0ull, 10ull);
				for (size_t y = 0; y < arraySize03; ++y) {
					v[x].testDouble.emplace_back(generateDouble());
				}
			}
		};

		fill(a);
		fill(b);
		fill(c);
		fill(d);
		fill(e);
		fill(f);
		fill(g);
		fill(h);
		fill(i);
		fill(j);
		fill(k);
		fill(l);
		fill(m);
		fill(n);
		fill(o);
		fill(p);
		fill(q);
		fill(r);
		fill(s);
		fill(t);
		fill(u);
		fill(v);
		fill(w);
		fill(x);
		fill(y);
		fill(z);
	}
};

template<rt_ut::string_literal test_suite, const auto& pass_tests, const auto& pass_values> struct pass_tests_runner {
	template<uint64_t index> static void impl(jsonifier::jsonifier_core<>& parser) {
		rt_ut::unit_test<test_suite + rt_ut::string_literal{ rt_ut::create_string_literal<pass_tests[index].size()>(pass_tests[index].data()) }, true>::assert_eq(
			pass_values[index], [&]() {
				using value_type = std::remove_cvref_t<decltype(pass_values[0])>;
				value_type data{};
				parser.parseJson(data, pass_tests[index]);
				for (auto& value: parser.getErrors()) {
					std::cout << "Jsonifier Error: " << value << std::endl;
				}
				return data;
			});
	}
};

template<rt_ut::string_literal test_suite, const auto& fail_tests, const auto fail_value> struct fail_tests_runner {
	template<uint64_t index> static void impl(jsonifier::jsonifier_core<>& parser) {
		rt_ut::unit_test<test_suite + rt_ut::string_literal{ rt_ut::create_string_literal<fail_tests[index].size()>(fail_tests[index].data()) }, true>::assert_ne(fail_value,
			[&]() {
				int64_t data{};
				parser.parseJson(data, fail_tests[index]);
				return parser.getErrors().size();
			});
	}
};

template<rt_ut::string_literal test_suite, const auto& pass_tests, const auto& pass_values, template<auto, const auto&, const auto&> typename functor, typename integer_sequence>
struct pass_test_runner;

template<rt_ut::string_literal test_suite, const auto& pass_tests, const auto& pass_values, template<auto, const auto&, const auto&> typename functor, uint64_t... indices>
struct pass_test_runner<test_suite, pass_tests, pass_values, functor, std::integer_sequence<uint64_t, indices...>> {
	static void impl() {
		jsonifier::jsonifier_core<> parser{};
		(functor<test_suite, pass_tests, pass_values>::template impl<indices>(parser), ...);
	}
};

template<rt_ut::string_literal test_suite, const auto& fail_tests, const auto fail_values, template<auto, const auto&, const auto&> typename functor, typename integer_sequence>
struct fail_test_runner;

template<rt_ut::string_literal test_suite, const auto& fail_tests, const auto fail_values, template<auto, const auto&, const auto&> typename functor, uint64_t... indices>
struct fail_test_runner<test_suite, fail_tests, fail_values, functor, std::integer_sequence<uint64_t, indices...>> {
	static void impl() {
		jsonifier::jsonifier_core<> parser{};
		(functor<test_suite, fail_tests, fail_values>::template impl<indices>(parser), ...);
	}
};