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

#include <BnchSwt/BenchmarkSuite.hpp>
#include "UnicodeEmoji.hpp"
#include "CitmCatalog.hpp"
#include "Twitter.hpp"
#include "Discord.hpp"
#include "Canada.hpp"
#include <thread>

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
		for (const auto& entry: std::filesystem::directory_iterator(std::string{ JSON_TEST_PATH } + testType)) {
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
	std::vector<std::string> testStrings{};
	std::vector<uint64_t> testUints{};
	std::vector<double> testDoubles{};
	std::vector<int64_t> testInts{};
	std::vector<bool> testBools{};
};

#if defined(JSONIFIER_MAC)
constexpr bnch_swt::result_type resultType{ bnch_swt::result_type::time };
#else
constexpr bnch_swt::result_type resultType{ bnch_swt::result_type::cycles };
#endif

template<typename value_type> struct test {
	std::vector<value_type> a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v, w, x, y, z;
};

template<typename value_type> struct partial_test {
	std::vector<value_type> m, s;
};

inline static std::random_device randomEngine{};
inline static std::mt19937_64 gen{ randomEngine() };
static constexpr std::string_view charset{ "!#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[]^_`abcdefghijklmnopqrstuvwxyz{|}~\"\\\r\b\f\t\n" };

template<typename value_type> struct test_generator {
	std::vector<value_type> a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v, w, x, y, z;

	template<typename value_type01, typename value_type02> static value_type01 randomizeNumberUniform(value_type01 start, value_type02 end) {
		std::uniform_real_distribution<value_type01> dis{ start, static_cast<value_type01>(end) };
		return dis(gen);
	}

	template<jsonifier::concepts::integer_t value_type01, jsonifier::concepts::integer_t value_type02>
	static value_type01 randomizeNumberUniform(value_type01 start, value_type02 end) {
		std::uniform_int_distribution<value_type01> dis{ start, static_cast<value_type01>(end) };
		return dis(gen);
	}

	static void insertUnicodeInJSON(std::string& jsonString) {
		auto newStringView = unicode_emoji::unicodeEmoji[randomizeNumberUniform(0ull, std::size(unicode_emoji::unicodeEmoji) - 1)];
		jsonString += static_cast<std::string>(newStringView);
	}

	static std::string generateString() {
		auto length{ randomizeNumberUniform(32ull, 64ull) };
		constexpr size_t charsetSize = charset.size();
		auto unicodeCount			 = length / 4ull;
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
		int32_t insertedUnicode = 0;
		auto iter				= unicodeIndices.begin();
		for (int32_t x = 0; x < length; ++x) {
			if (iter < unicodeIndices.end() && x == *iter) [[unlikely]] {
				insertUnicodeInJSON(result);
				insertedUnicode++;
				++iter;
			} else {
				result += charset[randomizeNumberUniform(0ull, charsetSize - 1)];
			}
		}

		return result;
	}

	static double generateDouble() {
		double min = std::numeric_limits<double>::min();
		double max = std::numeric_limits<double>::max();
		std::uniform_real_distribution<double> dis(log(min), log(max));
		double logValue = dis(gen);
		bool negative{ generateBool() };
		return negative ? -std::exp(logValue) : std::exp(logValue);
	}

	static bool generateBool() {
		return static_cast<bool>(randomizeNumberUniform(0, 100) >= 50);
	};

	static size_t generateUint() {
		return randomizeNumberUniform(std::numeric_limits<size_t>::min(), std::numeric_limits<size_t>::max());
	};

	static int64_t generateInt() {
		return randomizeNumberUniform(std::numeric_limits<int64_t>::min(), std::numeric_limits<int64_t>::max());
	};

	test_generator() {
		auto fill = [&](auto& v) {
			auto arraySize01 = randomizeNumberUniform(15ull, 25ull);
			v.resize(arraySize01);
			for (size_t x = 0; x < arraySize01; ++x) {
				auto arraySize02 = randomizeNumberUniform(15ull, 35ull);
				auto arraySize03 = randomizeNumberUniform(0ull, arraySize02);
				for (size_t y = 0; y < arraySize03; ++y) {
					auto newString = generateString();
					v[x].testStrings.emplace_back(newString);
				}
				arraySize03 = randomizeNumberUniform(0ull, arraySize02);
				for (size_t y = 0; y < arraySize03; ++y) {
					v[x].testUints.emplace_back(generateUint());
				}
				arraySize03 = randomizeNumberUniform(0ull, arraySize02);
				for (size_t y = 0; y < arraySize03; ++y) {
					v[x].testInts.emplace_back(generateInt());
				}
				arraySize03 = randomizeNumberUniform(0ull, arraySize02);
				for (size_t y = 0; y < arraySize03; ++y) {
					auto newBool = generateBool();
					v[x].testBools.emplace_back(newBool);
				}
				arraySize03 = randomizeNumberUniform(0ull, arraySize02);
				for (size_t y = 0; y < arraySize03; ++y) {
					v[x].testDoubles.emplace_back(generateDouble());
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

struct test_element_final {
	std::string libraryName{};
	std::string resultType{};
	double iterationCount{};
	double resultSpeed{};
	std::string color{};
	bool operator>(const test_element_final& other) const noexcept {
		return resultSpeed > other.resultSpeed;
	}
};

struct test_element_pair {
	test_element_final writeData{};
	test_element_final readData{};
	bool operator>(const test_element_pair& other) const noexcept {
		return writeData > other.writeData && readData > other.readData;
	}
};

struct test_elements_final {
	std::vector<test_element_final> results{};
	std::string testName{};
};

using test_results_final = std::vector<test_elements_final>;

template<typename value_type> struct abc_test {
	std::vector<value_type> z, y, x, w, v, u, t, s, r, q, p, o, n, m, l, k, j, i, h, g, f, e, d, c, b, a;
};

enum class result_type { read = 0, write = 1 };

template<result_type type> constexpr auto enumToString() {
	if constexpr (type == result_type::read) {
		return jsonifier_internal::string_literal{ "Read" };
	} else {
		return jsonifier_internal::string_literal{ "Write" };
	}
}

template<result_type type> struct result {
	std::optional<double> iterationCount{};
	std::optional<size_t> byteLength{};
	std::optional<double> jsonSpeed{};
	std::optional<double> jsonTime{};
	std::optional<double> cv{};
	std::string color{};

	result& operator=(result&&) noexcept	  = default;
	result(result&&) noexcept				  = default;
	result& operator=(const result&) noexcept = default;
	result(const result&) noexcept			  = default;

	result() noexcept = default;

	double getResultValueMbs(double input01, uint64_t input02) const {
#if !defined(JSONIFIER_MAC)
		input01 = bnch_swt::cyclesToTime(input01, bnch_swt::getCpuFrequency());
#endif
		auto mbWrittenCount	  = static_cast<double>(input02) / 1e+6l;
		auto writeSecondCount = input01 / 1e+9l;
		return mbWrittenCount / writeSecondCount;
	}

	double getResultValueCyclesMb(double input01, uint64_t input02) const {
		auto mbWrittenCount	  = static_cast<double>(input02) / 1e+6l;
		auto writeSecondCount = input01 / mbWrittenCount;
		return writeSecondCount;
	}

	result(const std::string& colorNew, size_t byteLengthNew, const bnch_swt::benchmark_result_final& results) {
		iterationCount.emplace(results.iterationCount);
		byteLength.emplace(byteLengthNew);
		jsonTime.emplace(results.median);
		cv.emplace(results.cv * 100.0f);
		jsonSpeed.emplace(getResultValueMbs(jsonTime.value(), byteLength.value()));
		color = colorNew;
	}

	operator bool() const noexcept {
		return jsonSpeed.has_value();
	}

	bool operator>(const result& other) const noexcept {
		if (jsonSpeed.has_value() && other.jsonSpeed.has_value()) {
			return jsonSpeed.value() > other.jsonSpeed.value();
		} else if (!jsonSpeed.has_value()) {
			return false;
		} else if (!other.jsonSpeed.has_value()) {
			return true;
		}
		return false;
	}
};

struct results_data {
	std::unordered_set<std::string> jsonifierExcludedKeys{};
	result<result_type::write> writeResult{};
	result<result_type::read> readResult{};
	std::string name{};
	std::string test{};
	std::string url{};
	size_t iterations{};

	bool operator>(const results_data& other) const noexcept {
		if (readResult && other.readResult) {
			return readResult > other.readResult;
		} else if (writeResult && other.writeResult) {
			return writeResult > other.writeResult;
		} else {
			return false;
		}
	}

	results_data& operator=(results_data&&) noexcept	  = default;
	results_data(results_data&&) noexcept				  = default;
	results_data& operator=(const results_data&) noexcept = default;
	results_data(const results_data&) noexcept			  = default;

	results_data() noexcept = default;

	results_data(const std::string& nameNew, const std::string& testNew, const std::string& urlNew, size_t iterationsNew) {
		iterations = iterationsNew;
		name	   = nameNew;
		test	   = testNew;
		url		   = urlNew;
	}

	void checkForMissingKeys() {
		if (!writeResult.jsonSpeed.has_value()) {
			jsonifierExcludedKeys.emplace("writeResult");
		} else if (!readResult.jsonSpeed.has_value()) {
			jsonifierExcludedKeys.emplace("readResult");
		}
	}

	void print() const noexcept {
		std::cout << std::string{ "| " } + name + " " + test + ": " + url + "\n" +
				"| ------------------------------------------------------------ "
				"|\n";
		if (readResult.byteLength.has_value() && readResult.jsonSpeed.has_value()) {
			std::cout << enumToString<result_type::read>() + " Speed (MB/S): " << std::setprecision(6) << readResult.jsonSpeed.value() << std::endl;
#if !defined(JSONIFIER_MAC)
			std::cout << enumToString<result_type::read>() + " Speed (Cycles/MB): " << std::setprecision(6)
					  << readResult.getResultValueCyclesMb(readResult.jsonTime.value(), readResult.byteLength.value()) << std::endl;
#endif
			std::cout << enumToString<result_type::read>() + " Length (Bytes): " << readResult.byteLength.value() << std::endl;
			std::cout << enumToString<result_type::read>() + " Runtime (ns): " << std::setprecision(6) << readResult.jsonTime.value() << std::endl;
			std::cout << enumToString<result_type::read>() + " Iteration Count: " << std::setprecision(4) << readResult.iterationCount.value() << std::endl;
			std::cout << enumToString<result_type::read>() + " Coefficient of Variance (%): " << std::setprecision(4) << readResult.cv.value() << std::endl;
		}
		if (writeResult.byteLength.has_value() && writeResult.jsonSpeed.has_value()) {
			std::cout << enumToString<result_type::write>() + " Speed (MB/S): " << std::setprecision(6) << writeResult.jsonSpeed.value() << std::endl;
#if !defined(JSONIFIER_MAC)
			std::cout << enumToString<result_type::write>() + " Speed (Cycles/MB): " << std::setprecision(6)
					  << writeResult.getResultValueCyclesMb(writeResult.jsonTime.value(), writeResult.byteLength.value()) << std::endl;
#endif
			std::cout << enumToString<result_type::write>() + " Length (Bytes): " << writeResult.byteLength.value() << std::endl;
			std::cout << enumToString<result_type::write>() + " Runtime (ns): " << std::setprecision(6) << writeResult.jsonTime.value() << std::endl;
			std::cout << enumToString<result_type::write>() + " Iteration Count: " << std::setprecision(4) << writeResult.iterationCount.value() << std::endl;
			std::cout << enumToString<result_type::write>() + " Coefficient of Variance (%): " << std::setprecision(4) << writeResult.cv.value() << std::endl;
		}
		std::cout << "\n---" << std::endl;
	}

	std::string jsonStats() const noexcept {
		std::string writeLength{};
		std::string writeTime{};
		std::string writeIterationCount{};
		std::string writeCv{};
		std::string write{};
		std::string write02{};
		std::string readLength{};
		std::string readTime{};
		std::string readIterationCount{};
		std::string readCv{};
		std::string read{};
		std::string read02{};
		std::string finalString{ "| [" + name + "](" + url + ") | " };
		if (readResult.jsonTime.has_value() && readResult.byteLength.has_value()) {
#if !defined(JSONIFIER_MAC)
			std::stringstream stream00{};
			stream00 << std::setprecision(6) << readResult.getResultValueCyclesMb(readResult.jsonTime.value(), readResult.byteLength.value());
			read02 = stream00.str();
#endif
			std::stringstream stream01{};
			stream01 << std::setprecision(6) << readResult.jsonSpeed.value();
			read = stream01.str();
			std::stringstream stream02{};
			stream02 << readResult.byteLength.value();
			readLength = stream02.str();
			std::stringstream stream03{};
			stream03 << std::setprecision(6) << readResult.jsonTime.value();
			readTime = stream03.str();
			std::stringstream stream04{};
			stream04 << std::setprecision(4) << readResult.iterationCount.value();
			readIterationCount = stream04.str();
			finalString += read + " | ";
#if !defined(JSONIFIER_MAC)
			finalString += read02 + " | ";
#endif
			finalString += readLength + " | " + readTime + " | " + readIterationCount + " | ";
		}
		if (writeResult.jsonTime.has_value() && writeResult.byteLength.has_value()) {
#if !defined(JSONIFIER_MAC)
			std::stringstream stream00{};
			stream00 << std::setprecision(6) << writeResult.getResultValueCyclesMb(writeResult.jsonTime.value(), writeResult.byteLength.value());
			write02 = stream00.str();
#endif
			std::stringstream stream01{};
			stream01 << std::setprecision(6) << writeResult.jsonSpeed.value();
			write = stream01.str();
			std::stringstream stream02{};
			stream02 << writeResult.byteLength.value();
			writeLength = stream02.str();
			std::stringstream stream03{};
			stream03 << std::setprecision(6) << writeResult.jsonTime.value();
			writeTime = stream03.str();
			std::stringstream stream04{};
			stream04 << std::setprecision(4) << writeResult.iterationCount.value();
			writeIterationCount = stream04.str();
			finalString += write + " | ";
#if !defined(JSONIFIER_MAC)
			finalString += write02 + " | ";
#endif
			finalString += writeLength + " | " + writeTime + " | " + writeIterationCount + " | ";
		}
		return finalString;
	}
};

struct test_results {
	std::vector<results_data> results{};
	std::string markdownResults{};
	std::string testName{};
};
