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
#include "CitmCatalog.hpp"
#include "Twitter.hpp"
#include "Discord.hpp"
#include "Canada.hpp"

struct test_struct {
	std::vector<std::string> testStrings{};
	std::vector<uint64_t> testUints{};
	std::vector<double> testDoubles{};
	std::vector<int64_t> testInts{};
	std::vector<bool> testBools{};
};

template<typename value_type> struct test {
	std::vector<value_type> a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v, w, x, y, z;
};

template<typename value_type> struct test_generator {
	std::vector<value_type> a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v, w, x, y, z;

	std::random_device randomEngine{};
	std::mt19937_64 gen{ randomEngine() };

	static constexpr std::string_view charset{ "!#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[]^_`abcdefghijklmnopqrstuvwxyz{|}~\"\\\r\b\f\t\n" };

	template<typename value_type01, typename value_type02> value_type01 randomizeNumberUniform(value_type01 start, value_type02 end) {
		std::uniform_real_distribution<value_type01> dis{ start, static_cast<value_type01>(end) };
		return dis(gen);
	}

	template<jsonifier::concepts::integer_t value_type01, jsonifier::concepts::integer_t value_type02> value_type01 randomizeNumberUniform(value_type01 start, value_type02 end) {
		std::uniform_int_distribution<value_type01> dis{ start, static_cast<value_type01>(end) };
		return dis(gen);
	}

	void insertUnicodeInJSON(std::string& jsonString) {
		auto newStringView = unicode_emoji::unicodeEmoji[randomizeNumberUniform(0ull, std::size(unicode_emoji::unicodeEmoji) - 1)];
		jsonString += static_cast<jsonifier::string>(newStringView);
	}

	std::string generateString() {
		auto length{ randomizeNumberUniform(32, 64) };
		constexpr size_t charsetSize = charset.size();
		auto unicodeCount			 = randomizeNumberUniform(1, length / 8);
		std::string result{};
		for (int32_t x = 0; x < length; ++x) {
			if (x % unicodeCount == 0) [[unlikely]] {
				insertUnicodeInJSON(result);
			}
			result += charset[randomizeNumberUniform(0ull, charsetSize - 1)];
		}
		return result;
	}

	double generateDouble() {
		return randomizeNumberUniform(std::numeric_limits<double>::min(), std::numeric_limits<double>::max());
	};

	bool generateBool() {
		return static_cast<bool>(randomizeNumberUniform(0, 100) >= 50);
	};

	size_t generateUint() {
		return randomizeNumberUniform(std::numeric_limits<size_t>::min(), std::numeric_limits<size_t>::max());
	};

	int64_t generateInt() {
		return randomizeNumberUniform(std::numeric_limits<int64_t>::min(), std::numeric_limits<int64_t>::max());
	};

	test_generator() {
		auto fill = [&](auto& v) {
			auto arraySize01 = randomizeNumberUniform(5ull, 25ull);
			v.resize(arraySize01);
			for (size_t x = 0; x < arraySize01; ++x) {
				auto arraySize02 = randomizeNumberUniform(5ull, 35ull);
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
	double resultSpeed{};
	double iterationCount{};
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

template<result_type type> std::string enumToString() {
	return type == result_type::read ? "Read" : "Write";
}

template<result_type type> struct result {
	std::optional<size_t> byteLength{};
	std::optional<double> jsonSpeed{};
	std::optional<double> jsonTime{};
	std::optional<double> iterationCount{};
	std::optional<double> cv{};
	std::string color{};

	result& operator=(result&&) noexcept	  = default;
	result(result&&) noexcept				  = default;
	result& operator=(const result&) noexcept = default;
	result(const result&) noexcept			  = default;

	result() noexcept = default;

	result(const std::string& colorNew, size_t byteLengthNew, const bnch_swt::benchmark_result_final& results) {
		iterationCount.emplace(results.iterationCount);
		byteLength.emplace(byteLengthNew);
		const auto cpuFrequency = bnch_swt::getCpuFrequency();
		jsonTime.emplace(results.median);
		cv.emplace(results.cv * 100.0f);
		auto mbWrittenCount	  = static_cast<double>(byteLength.value()) / 1e+6l;
		auto writeSecondCount = jsonTime.value() / 1e+9l;
		jsonSpeed.emplace(mbWrittenCount / writeSecondCount);
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
	jsonifier::string name{};
	jsonifier::string test{};
	jsonifier::string url{};
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

	results_data(const jsonifier::string& nameNew, const jsonifier::string& testNew, const jsonifier::string& urlNew, size_t iterationsNew) {
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
		std::cout << jsonifier::string{ "| " } + name + " " + test + ": " + url + "\n" +
				"| ------------------------------------------------------------ "
				"|\n";
		if (readResult.byteLength.has_value() && readResult.jsonSpeed.has_value()) {
			std::cout << enumToString<result_type::read>() + " Speed (MB/S): " << std::setprecision(6) << readResult.jsonSpeed.value() << std::endl;
			std::cout << enumToString<result_type::read>() + " Length (Bytes): " << readResult.byteLength.value() << std::endl;
			std::cout << enumToString<result_type::read>() + " Runtime (ns): " << std::setprecision(6) << readResult.jsonTime.value() << std::endl;
			std::cout << enumToString<result_type::read>() + " Iteration Count: " << std::setprecision(4) << readResult.iterationCount.value() << std::endl;
			std::cout << enumToString<result_type::read>() + " Coefficient of Variance (%): " << std::setprecision(4) << readResult.cv.value() << std::endl;
		}
		if (writeResult.byteLength.has_value() && writeResult.jsonSpeed.has_value()) {
			std::cout << enumToString<result_type::write>() + " Speed (MB/S): " << std::setprecision(6) << writeResult.jsonSpeed.value() << std::endl;
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
		std::string readLength{};
		std::string readTime{};
		std::string readIterationCount{};
		std::string readCv{};
		std::string read{};
		std::string finalString{ "| [" + name + "](" + url + ") | " };
		if (readResult.jsonTime.has_value() && readResult.byteLength.has_value()) {
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
			finalString += read + " | " + readLength + " | " + readTime + " | " + readIterationCount + " | ";
		} else {
			readLength		   = "N/A";
			readTime		   = "N/A";
			read			   = "N/A";
			readIterationCount = "N/A";
		}
		if (writeResult.jsonTime.has_value() && writeResult.byteLength.has_value()) {
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
			finalString += write + " | " + writeLength + " | " + writeTime + " | " + writeIterationCount + " | ";
		} else {
			writeLength			= "N/A";
			writeTime			= "N/A";
			write				= "N/A";
			writeIterationCount = "N/A";
		}
		return finalString;
	}
};

struct test_results {
	std::vector<results_data> results{};
	std::string markdownResults{};
	std::string testName{};
};
