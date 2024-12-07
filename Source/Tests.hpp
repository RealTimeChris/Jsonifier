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

#include "Conformance.hpp"
#include "BoundsTest.hpp"
#include "RoundTrip.hpp"
#include "Jsonifier.hpp"
#include "Simdjson.hpp"
#include "String.hpp"
#include "Float.hpp"
#include "Glaze.hpp"
#include "Uint.hpp"
#include "Int.hpp"

namespace tests {

	enum class test_type {
		parse_and_serialize = 0,
		minify				= 1,
		prettify			= 2,
		validate			= 3,
	};

	enum class json_library {
		jsonifier = 0,
		glaze	  = 1,
		simdjson  = 2,
	};

	template<json_library lib, test_type type, typename test_data_type, bool minified, size_t iterations, const bnch_swt::string_literal testName> struct json_test_helper {};

	template<json_library lib, test_type type, typename test_data_type, bool minified, size_t iterations, const bnch_swt::string_literal testName> struct json_test_helper;

	template<typename test_data_type, bool minified, size_t iterations, const bnch_swt::string_literal testNameNew>
	struct json_test_helper<json_library::jsonifier, test_type::parse_and_serialize, test_data_type, minified, iterations, testNameNew> {
		static auto run(std::string& newBuffer) {
			static constexpr bnch_swt::string_literal testName{ testNameNew };
			static constexpr bnch_swt::string_literal testNameRead{ testName + "-Read" };
			static constexpr bnch_swt::string_literal testNameWrite{ testName + "-Write" };
			static constexpr bool partialRead{ std::is_same_v<test_data_type, partial_test<test_struct>> };
			static constexpr bool knownOrder{ !std::is_same_v<test_data_type, abc_test<test_struct>> };
			results_data r{ jsonifierLibraryName, testName, jsonifierCommitUrl, iterations };
			jsonifier::jsonifier_core parser{};
			test_data_type testData{};
			if constexpr (partialRead) {
				parser.parseJson(testData, newBuffer);
			}
			auto readResult = bnch_swt::benchmark_stage<testNameRead, iterations>::template runBenchmark<testName, jsonifierLibraryName, "teal">([&]() mutable {
				parser.parseJson<jsonifier::parse_options{ .partialRead = partialRead, .knownOrder = knownOrder, .minified = minified }>(testData, newBuffer);
				bnch_swt::doNotOptimizeAway(testData);
				return newBuffer.size();
			});
			for (auto& value: parser.getErrors()) {
				std::cout << "Jsonifier Error: " << value << std::endl;
			}
			std::string_view newerBuffer{};
			auto writeResult = bnch_swt::benchmark_stage<testNameWrite, iterations>::template runBenchmark<testName, jsonifierLibraryName, "steelblue">([&]() mutable {
				newerBuffer = parser.serializeJson<jsonifier::serialize_options{ .prettify = !minified }>(testData);
				bnch_swt::doNotOptimizeAway(newerBuffer);
				return newerBuffer.size();
			});

			for (auto& value: parser.getErrors()) {
				std::cout << "Jsonifier Error: " << value << std::endl;
			}

			auto readSize	 = newerBuffer.size();
			auto writtenSize = newerBuffer.size();
			r.readResult	 = result<result_type::read>{ "teal", readSize, readResult };
			r.writeResult	 = result<result_type::write>{ "steelblue", writtenSize, writeResult };
			bnch_swt::file_loader::saveFile(static_cast<std::string>(newerBuffer), jsonOutPath + "/" + testName + "-jsonifier.json");
			return r;
		}
	};

	template<size_t iterations, bnch_swt::string_literal testNameNew>
	struct json_test_helper<json_library::jsonifier, test_type::prettify, std::string, false, iterations, testNameNew> {
		static auto run(std::string& newBuffer) {
			static constexpr bnch_swt::string_literal testName{ testNameNew };
			results_data r{ jsonifierLibraryName, testName, jsonifierCommitUrl, iterations };
			jsonifier::jsonifier_core parser{};
			std::string newerBuffer{};
			auto writeResult = bnch_swt::benchmark_stage<testName>::template runBenchmark<testName, jsonifierLibraryName, "steelblue">([&]() mutable {
				parser.prettifyJson(newBuffer, newerBuffer);
				bnch_swt::doNotOptimizeAway(newerBuffer);
				return newerBuffer.size();
			});

			for (auto& value: parser.getErrors()) {
				std::cout << "Jsonifier Error: " << value << std::endl;
			}
			bnch_swt::file_loader::saveFile(newerBuffer, jsonOutPath + "/" + testName + "-jsonifier.json");
			r.writeResult = result<result_type::write>{ "steelblue", newerBuffer.size(), writeResult };
			return r;
		}
	};

	template<size_t iterations, bnch_swt::string_literal testNameNew>
	struct json_test_helper<json_library::jsonifier, test_type::minify, std::string, false, iterations, testNameNew> {
		static auto run(std::string& newBuffer) {
			static constexpr bnch_swt::string_literal testName{ testNameNew };
			std::string newerBuffer{};
			results_data r{ jsonifierLibraryName, testName, jsonifierCommitUrl, iterations };
			jsonifier::jsonifier_core parser{};
			auto writeResult = bnch_swt::benchmark_stage<testName>::template runBenchmark<testName, jsonifierLibraryName, "steelblue">([&]() mutable {
				parser.minifyJson(newBuffer, newerBuffer);
				bnch_swt::doNotOptimizeAway(newerBuffer);
				return newerBuffer.size();
			});
			for (auto& value: parser.getErrors()) {
				std::cout << "Jsonifier Error: " << value << std::endl;
			}
			bnch_swt::file_loader::saveFile(newerBuffer, jsonOutPath + "/" + testName + "-jsonifier.json");
			r.writeResult = result<result_type::write>{ "steelblue", newerBuffer.size(), writeResult };
			return r;
		}
	};

	template<size_t iterations, bnch_swt::string_literal testNameNew>
	struct json_test_helper<json_library::jsonifier, test_type::validate, std::string, false, iterations, testNameNew> {
		static auto run(std::string& newBuffer) {
			static constexpr bnch_swt::string_literal testName{ testNameNew };
			results_data r{ jsonifierLibraryName, testName, jsonifierCommitUrl, iterations };
			jsonifier::jsonifier_core parser{};
			auto readResult = bnch_swt::benchmark_stage<testName>::template runBenchmark<testName, jsonifierLibraryName, "steelblue">([&]() mutable {
				bnch_swt::doNotOptimizeAway(parser.validateJson(newBuffer));
				return newBuffer.size();
			});

			for (auto& value: parser.getErrors()) {
				std::cout << "Jsonifier Error: " << value << std::endl;
			}
			bnch_swt::file_loader::saveFile(newBuffer, jsonOutPath + "/" + testName + "-jsonifier.json");
			r.readResult = result<result_type::read>{ "teal", newBuffer.size(), readResult };
			return r;
		}
	};

	template<typename test_data_type, bool minified, size_t iterations, bnch_swt::string_literal testNameNew>
	struct json_test_helper<json_library::glaze, test_type::parse_and_serialize, test_data_type, minified, iterations, testNameNew> {
		static auto run(std::string& newBuffer) {
			static constexpr bnch_swt::string_literal testName{ testNameNew };
			static constexpr bnch_swt::string_literal testNameRead{ testName + "-Read" };
			static constexpr bnch_swt::string_literal testNameWrite{ testName + "-Write" };
			static constexpr bool partialRead{ std::is_same_v<test_data_type, partial_test<test_struct>> };
			results_data r{ glazeLibraryName, testName, glazeCommitUrl, iterations };
			test_data_type testData{};
			if constexpr (partialRead) {
				if (auto error =
						glz::read<glz::opts{ .error_on_unknown_keys = !partialRead, .skip_null_members = false, .prettify = !minified, .minified = minified }>(testData, newBuffer);
					error) {
					std::cout << "Glaze Error: " << glz::format_error(error, newBuffer) << std::endl;
				}
			}
			auto readResult = bnch_swt::benchmark_stage<testNameRead, iterations>::template runBenchmark<testName, glazeLibraryName, "dodgerblue">([&]() mutable {
				if (auto error = glz::read<glz::opts{ .error_on_unknown_keys = !partialRead,
						.skip_null_members									 = false,
						.prettify											 = !minified,
						.minified											 = minified,
						.partial_read										 = partialRead,
						.partial_read_nested								 = partialRead }>(testData, newBuffer);
					error) {
					std::cout << "Glaze Error: " << glz::format_error(error, newBuffer) << std::endl;
				}
				bnch_swt::doNotOptimizeAway(testData);
				return newBuffer.size();
			});
			std::string newerBuffer{};
			auto writeResult = bnch_swt::benchmark_stage<testNameWrite, iterations>::template runBenchmark<testName, glazeLibraryName, "steelblue">([&]() mutable {
				auto newResult = glz::write<glz::opts{ .skip_null_members = false, .prettify = !minified, .minified = minified }>(testData, newerBuffer);
				bnch_swt::doNotOptimizeAway(newResult);
				return newerBuffer.size();
			});

			auto readSize	 = newerBuffer.size();
			auto writtenSize = newerBuffer.size();
			r.readResult	 = result<result_type::read>{ "dodgerblue", readSize, readResult };
			r.writeResult	 = result<result_type::write>{ "skyblue", writtenSize, writeResult };
			bnch_swt::file_loader::saveFile(static_cast<std::string>(newerBuffer), jsonOutPath + "/" + testName + "-glaze.json");

			return r;
		}
	};

	template<size_t iterations, bnch_swt::string_literal testNameNew>
	struct json_test_helper<json_library::glaze, test_type::prettify, std::string, false, iterations, testNameNew> {
		static auto run(std::string& newBuffer) {
			static constexpr bnch_swt::string_literal testName{ testNameNew };

			results_data r{ glazeLibraryName, testName, glazeCommitUrl, iterations };
			std::string newerBuffer{};

			auto writeResult = bnch_swt::benchmark_stage<testName>::template runBenchmark<testName, glazeLibraryName, "steelblue">([&]() mutable {
				glz::prettify_json(newBuffer, newerBuffer);
				bnch_swt::doNotOptimizeAway(newerBuffer);
				return newerBuffer.size();
			});

			bnch_swt::file_loader::saveFile(newerBuffer, jsonOutPath + "/" + testName + "-glaze.json");
			r.writeResult = result<result_type::write>{ "skyblue", newerBuffer.size(), writeResult };

			return r;
		}
	};

	template<size_t iterations, bnch_swt::string_literal testNameNew> struct json_test_helper<json_library::glaze, test_type::minify, std::string, false, iterations, testNameNew> {
		static auto run(std::string& newBuffer) {
			static constexpr bnch_swt::string_literal testName{ testNameNew };

			std::string newestBuffer{ newBuffer };
			std::string newerBuffer{};

			results_data r{ glazeLibraryName, testName, glazeCommitUrl, iterations };
			auto writeResult = bnch_swt::benchmark_stage<testName>::template runBenchmark<testName, glazeLibraryName, "steelblue">([&]() mutable {
				glz::minify_json(newestBuffer, newerBuffer);
				bnch_swt::doNotOptimizeAway(newerBuffer);
				return newerBuffer.size();
			});

			bnch_swt::file_loader::saveFile(newerBuffer, jsonOutPath + "/" + testName + "-glaze.json");
			r.writeResult = result<result_type::write>{ "skyblue", newerBuffer.size(), writeResult };

			return r;
		}
	};

	template<size_t iterations, bnch_swt::string_literal testNameNew>
	struct json_test_helper<json_library::glaze, test_type::validate, std::string, false, iterations, testNameNew> {
		static auto run(std::string& newBuffer) {
			static constexpr bnch_swt::string_literal testName{ testNameNew };
			results_data r{ glazeLibraryName, testName, glazeCommitUrl, iterations };
			auto readResult = bnch_swt::benchmark_stage<testName>::template runBenchmark<testName, glazeLibraryName, "skyblue">([&]() mutable {
				bnch_swt::doNotOptimizeAway(glz::validate_json(newBuffer));
				return newBuffer.size();
			});

			bnch_swt::file_loader::saveFile(newBuffer, jsonOutPath + "/" + testName + "-glaze.json");
			r.readResult = result<result_type::read>{ "skyblue", newBuffer.size(), readResult };

			return r;
		}
	};

	template<typename test_data_type, bool minified, size_t iterations, bnch_swt::string_literal testNameNew>
	struct json_test_helper<json_library::simdjson, test_type::parse_and_serialize, test_data_type, minified, iterations, testNameNew> {
		JSONIFIER_INLINE static auto run(std::string& newestBuffer) {
			std::string newBuffer{ newestBuffer };
			newBuffer.reserve(newBuffer.size() + 256);
			static constexpr bnch_swt::string_literal testName{ testNameNew };
			static constexpr bnch_swt::string_literal testNameRead{ testName + "-Read" };
			results_data r{ simdjsonLibraryName, testName, simdjsonCommitUrl, iterations };
			simdjson::ondemand::parser parser{};
			test_data_type testData{};
			auto readSize	= newBuffer.size();
			auto readResult = bnch_swt::benchmark_stage<testNameRead, iterations>::template runBenchmark<testName, simdjsonLibraryName, "cadetblue">([&]() {
				try {
					getValue(testData, parser.iterate(newBuffer).value());
					bnch_swt::doNotOptimizeAway(testData);
					return newBuffer.size();
				} catch (std::exception& error) {
					std::cout << "Simdjson Error: " << error.what() << std::endl;
					return newBuffer.size();
				}
			});
			std::string newerBuffer{};
			auto resultNew = glz::write<glz::opts{ .skip_null_members = false, .prettify = !minified, .minified = minified }>(testData, newerBuffer);
			( void )resultNew;
			readSize = newerBuffer.size();

			r.readResult = result<result_type::read>{ "cadetblue", readSize, readResult };
			bnch_swt::file_loader::saveFile(newerBuffer, jsonOutPath + "/" + testName + "-simdjson.json");
			return r;
		}
	};

	template<size_t iterations, bnch_swt::string_literal testNameNew>
	struct json_test_helper<json_library::simdjson, test_type::minify, std::string, false, iterations, testNameNew> {
		static constexpr bnch_swt::string_literal testName{ testNameNew };
		static auto run(std::string& newBuffer) {
			results_data r{ simdjsonLibraryName, testName, simdjsonCommitUrl, iterations };

			simdjson::dom::parser parser{};
			std::string newerBuffer{};

			auto writeResult = bnch_swt::benchmark_stage<testName>::template runBenchmark<testName, simdjsonLibraryName, "cornflowerblue">([&]() mutable {
				try {
					newerBuffer = simdjson::minify(parser.parse(newBuffer));
					bnch_swt::doNotOptimizeAway(newerBuffer);
					return newBuffer.size();
				} catch (std::exception& error) {
					std::cout << "Simdjson Error: " << error.what() << std::endl;
					return newBuffer.size();
				}
			});

			bnch_swt::file_loader::saveFile(newerBuffer, jsonOutPath + "/" + testName + "-simdjson.json");
			r.writeResult = result<result_type::write>{ "cornflowerblue", newerBuffer.size(), writeResult };


			return r;
		}
	};

#if defined(JSONIFIER_MAC)
	constexpr bnch_swt::string_literal table_header = bnch_swt::string_literal{ R"(
| Library | Read (MB/S) | Read Percentage Deviation (+/-%) | Read Length (Bytes) | Read Time (ns) | Write (MB/S) | Write Percentage Deviation (+/-%) | Write Length (Bytes) | Write Time (ns) |
| ------- | ----------- | --------------------- | ------------------- | -------------- | ------------ | ---------------------- | -------------------- | ----------------|  )" };

	constexpr bnch_swt::string_literal read_table_header = bnch_swt::string_literal{ R"(
| Library | Read (MB/S) | Read Percentage Deviation (+/-%) | Read Length (Bytes) | Read Time (ns) |
| ------- | ----------- | --------------------- | ------------------- | -------------- |  )" };

	constexpr bnch_swt::string_literal write_table_header = bnch_swt::string_literal{
		R"(
| Library | Write (MB/S) | Write Percentage Deviation (+/-%) | Write Length (Bytes) | Write Time (ns) |
| ------- | ------------ | ---------------------- | -------------------- | --------------- |  )"
	};
#else
	constexpr bnch_swt::string_literal table_header = bnch_swt::string_literal{
		R"(
| Library | Read (MB/S) | Read Percentage Deviation (+/-%) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Write (MB/S) | Write Percentage Deviation (+/-%) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) |
| ------- | ----------- | --------------------- | -----------------| ------------------- | -------------- | ------------ | ---------------------- | ------------------| -------------------- | --------------- |  )"
	};

	constexpr bnch_swt::string_literal read_table_header = bnch_swt::string_literal{ R"(
| Library | Read (MB/S) | Read Percentage Deviation (+/-%) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) |
| ------- | ----------- | --------------------- | ---------------- | ------------------- | -------------- |  )" };

	constexpr bnch_swt::string_literal write_table_header = bnch_swt::string_literal{
		R"(
| Library | Write (MB/S) | Write Percentage Deviation (+/-%) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) |
| ------- | ------------ | ---------------------- | ----------------- | -------------------- | --------------- |  )"
	};
#endif

	static std::string section001{ R"(
 > )" + jsonifier::toString(100) +
		R"( iterations on a ()" + getCPUInfo() + R"().

)" };

	constexpr bnch_swt::string_literal section002{ bnch_swt::string_literal{ R"(#### Using the following commits:
----
| Jsonifier: [)" } +
		JSONIFIER_COMMIT + R"(](https://github.com/RealTimeChris/Jsonifier/commit/)" + JSONIFIER_COMMIT + ")  \n" + R"(| Glaze: [)" + GLAZE_COMMIT +
		R"(](https://github.com/stephenberry/glaze/commit/)" + GLAZE_COMMIT + ")  \n" + R"(| Simdjson: [)" + SIMDJSON_COMMIT + R"(](https://github.com/simdjson/simdjson/commit/)" +
		SIMDJSON_COMMIT + ")  \n" };

	constexpr bnch_swt::string_literal section00{ R"(# Json-Performance
Performance profiling of JSON libraries (Compiled and run on )" +
		bnch_swt::string_literal{ OPERATING_SYSTEM_NAME } + " " + OPERATING_SYSTEM_VERSION + R"( using the )" + COMPILER_ID + " " + COMPILER_VERSION +
		" compiler).  \n\nLatest Results: (" };

	std::string generateSection(const std::string& testName, const std::string& currentPath) {
		return R"(

### )" + testName +
			R"( Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/)" + currentPath + R"(/)" +
			bnch_swt::urlEncode(testName) +
			R"(.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/)" +
			currentPath + R"(/)" + bnch_swt::urlEncode(testName) +
			R"(_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/)" + currentPath + R"(/)" +
			bnch_swt::urlEncode(testName) + R"(_Results.png?raw=true" 
alt="" width="400"/></p>
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/)" +
			currentPath + R"(/)" + bnch_swt::urlEncode(testName) +
			R"(_Cumulative_Speedup.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/)" + currentPath + R"(/)" + testName +
			R"(_Cumulative_Speedup.png?raw=true" 
alt="" width="400"/></p>

)" + (testName.find("Abc (Out of Order) Test (Prettified)") == std::string::npos ? "" : "The JSON documents in the previous tests featured keys ranging from \"a\" to \"z\",\
 where each key corresponds to an array of values. Notably, the documents in this test arrange these keys in reverse order, deviating from the typical \"a\" to \"z\" arrangement.\n\n\
This test effectively demonstrates the challenges encountered when utilizing simdjson and iterative parsers that lack the ability to efficiently allocate memory locations through\
hashing. In cases where the keys are not in the expected sequence, performance is significantly compromised, with the severity escalating as the document size increases.\n\n\
In contrast, hash-based solutions offer a viable alternative by circumventing these issues and maintaining optimal performance regardless of the JSON document's scale, or ordering of the keys being parsed.\n");
	}

	template<test_type type, typename test_data_type, bool minified, uint64_t iterations, bnch_swt::string_literal testName> struct json_tests_helper;

	template<test_type type, typename test_data_type, bool minified, uint64_t iterations, bnch_swt::string_literal testNameNew> struct json_tests_helper {
		static test_results run(std::string& jsonDataNew) {
			static constexpr bnch_swt::string_literal testName{ testNameNew };
			test_results jsonResults{};
			jsonResults.testName = static_cast<std::string>(testName);
			results_data jsonifierResults{};
			results_data simdjsonResults{};
			results_data glazeResults{};

#if !defined(ASAN_ENABLED)
			simdjsonResults = json_test_helper<json_library::simdjson, type, test_data_type, minified, iterations, testName>::run(jsonDataNew);
			glazeResults	= json_test_helper<json_library::glaze, type, test_data_type, minified, iterations, testName>::run(jsonDataNew);
#endif
			jsonifierResults = json_test_helper<json_library::jsonifier, type, test_data_type, minified, iterations, testName>::run(jsonDataNew);
#if !defined(ASAN_ENABLED)
			jsonResults.results.emplace_back(simdjsonResults);
			jsonResults.results.emplace_back(glazeResults);
#endif
			jsonResults.results.emplace_back(jsonifierResults);
			jsonResults.markdownResults += generateSection(testName, getCurrentPathImpl());
			jsonResults.markdownResults += table_header + "\n";
			std::sort(jsonResults.results.begin(), jsonResults.results.end(), std::greater<results_data>());
			for (auto iter = jsonResults.results.begin(); iter != jsonResults.results.end();) {
				jsonResults.markdownResults += iter->jsonStats();
				if (static_cast<size_t>(iter - jsonResults.results.begin()) != jsonResults.results.size() - 1) {
					jsonResults.markdownResults += "\n";
				}
				++iter;
			}
			bnch_swt::benchmark_stage<testNameNew + "-Read", iterations>::printResults();
			bnch_swt::benchmark_stage<testNameNew + "-Write", iterations>::printResults();
			return jsonResults;
		}
	};

	template<uint64_t iterations, bnch_swt::string_literal testNameNew> struct json_tests_helper<test_type::prettify, std::string, false, iterations, testNameNew> {
		static test_results run(std::string& jsonDataNew) {
			static constexpr bnch_swt::string_literal testName{ testNameNew };
			test_results jsonResults{};
			jsonResults.testName = static_cast<std::string>(testName);
			results_data jsonifierResults{};
			results_data glazeResults{};
#if !defined(ASAN_ENABLED)
			glazeResults = json_test_helper<json_library::glaze, test_type::prettify, std::string, false, iterations, testName>::run(jsonDataNew);
#endif
			jsonifierResults = json_test_helper<json_library::jsonifier, test_type::prettify, std::string, false, iterations, testName>::run(jsonDataNew);
#if !defined(ASAN_ENABLED)
			jsonResults.results.emplace_back(glazeResults);
#endif
			jsonResults.results.emplace_back(jsonifierResults);
			jsonResults.markdownResults += generateSection(testName, getCurrentPathImpl());
			jsonResults.markdownResults += write_table_header + "\n";
			std::sort(jsonResults.results.begin(), jsonResults.results.end(), std::greater<results_data>());
			for (auto iter = jsonResults.results.begin(); iter != jsonResults.results.end();) {
				jsonResults.markdownResults += iter->jsonStats();
				if (static_cast<size_t>(iter - jsonResults.results.begin()) != jsonResults.results.size() - 1) {
					jsonResults.markdownResults += "\n";
				}
				++iter;
			}
			bnch_swt::benchmark_stage<testNameNew>::printResults();
			return jsonResults;
		}
	};

	template<uint64_t iterations, bnch_swt::string_literal testNameNew> struct json_tests_helper<test_type::minify, std::string, false, iterations, testNameNew> {
		static test_results run(std::string& jsonDataNew) {
			static constexpr bnch_swt::string_literal testName{ testNameNew };
			test_results jsonResults{};
			jsonResults.testName = static_cast<std::string>(testName);
			results_data jsonifierResults{};
			results_data simdjsonResults{};
			results_data glazeResults{};
#if !defined(ASAN_ENABLED)
			simdjsonResults = json_test_helper<json_library::simdjson, test_type::minify, std::string, false, iterations, testName>::run(jsonDataNew);
			glazeResults	= json_test_helper<json_library::glaze, test_type::minify, std::string, false, iterations, testName>::run(jsonDataNew);
#endif
			jsonifierResults = json_test_helper<json_library::jsonifier, test_type::minify, std::string, false, iterations, testName>::run(jsonDataNew);
#if !defined(ASAN_ENABLED)
			jsonResults.results.emplace_back(simdjsonResults);
			jsonResults.results.emplace_back(glazeResults);
#endif
			jsonResults.results.emplace_back(jsonifierResults);
			jsonResults.markdownResults += generateSection(testName, getCurrentPathImpl());
			jsonResults.markdownResults += write_table_header + "\n";
			std::sort(jsonResults.results.begin(), jsonResults.results.end(), std::greater<results_data>());
			for (auto iter = jsonResults.results.begin(); iter != jsonResults.results.end();) {
				jsonResults.markdownResults += iter->jsonStats();
				if (static_cast<size_t>(iter - jsonResults.results.begin()) != jsonResults.results.size() - 1) {
					jsonResults.markdownResults += "\n";
				}
				++iter;
			}
			bnch_swt::benchmark_stage<testNameNew>::printResults();
			return jsonResults;
		}
	};

	template<uint64_t iterations, bnch_swt::string_literal testNameNew> struct json_tests_helper<test_type::validate, std::string, false, iterations, testNameNew> {
		static test_results run(std::string& jsonDataNew) {
			static constexpr bnch_swt::string_literal testName{ testNameNew };
			test_results jsonResults{};
			jsonResults.testName = static_cast<std::string>(testName);
			results_data jsonifierResults{};
			results_data glazeResults{};
#if !defined(ASAN_ENABLED)
			glazeResults = json_test_helper<json_library::glaze, test_type::validate, std::string, false, iterations, testName>::run(jsonDataNew);
#endif
			jsonifierResults = json_test_helper<json_library::jsonifier, test_type::validate, std::string, false, iterations, testName>::run(jsonDataNew);
			jsonResults.results.emplace_back(jsonifierResults);
#if !defined(ASAN_ENABLED)
			jsonResults.results.emplace_back(glazeResults);
#endif
			jsonResults.markdownResults += generateSection(testName, getCurrentPathImpl());
			jsonResults.markdownResults += read_table_header + "\n";
			std::sort(jsonResults.results.begin(), jsonResults.results.end(), std::greater<results_data>());
			for (auto iter = jsonResults.results.begin(); iter != jsonResults.results.end();) {
				jsonResults.markdownResults += iter->jsonStats();
				if (static_cast<size_t>(iter - jsonResults.results.begin()) != jsonResults.results.size() - 1) {
					jsonResults.markdownResults += "\n";
				}
				++iter;
			}
			bnch_swt::benchmark_stage<testNameNew>::printResults();
			return jsonResults;
		}
	};

	void testFunction() {
		bounds_tests::boundsTests();
		conformance_tests::conformanceTests();
		round_trip_tests::roundTripTests();
		string_validation_tests::stringTests();
		float_validation_tests::floatTests();
		uint_validation_tests::uintTests();
		int_validation_tests::intTests();
		test_generator<test_struct> testJsonData{};
		std::string jsonDataNew{};
		jsonifier::jsonifier_core parser{};
		parser.serializeJson<jsonifier::serialize_options{ .prettify = true }>(testJsonData, jsonDataNew);
		bnch_swt::file_loader::saveFile(jsonDataNew, jsonOutPath + "/Json Test (Prettified).json");
		bnch_swt::file_loader::saveFile(jsonDataNew, jsonOutPath + "/Abc (Out of Order) Test (Prettified).json");
		bnch_swt::file_loader::saveFile(jsonDataNew, jsonOutPath + "/Partial Test (Prettified).json");
		std::string jsonMinifiedData{ glz::minify_json(jsonDataNew) };
		bnch_swt::file_loader::saveFile(jsonMinifiedData, jsonOutPath + "/Json Test (Minified).json");
		bnch_swt::file_loader::saveFile(jsonMinifiedData, jsonOutPath + "/Abc (Out of Order) Test (Minified).json");
		bnch_swt::file_loader::saveFile(jsonMinifiedData, jsonOutPath + "/Partial Test (Minified).json");
		std::string discordData{ bnch_swt::file_loader::loadFile(jsonPath + "/" + "/Discord Test (Prettified).json") };
		bnch_swt::file_loader::saveFile(discordData, jsonOutPath + "/Discord Test (Prettified).json");
		std::string discordMinifiedData{ bnch_swt::file_loader::loadFile(jsonPath + "/" + "/Discord Test (Minified).json") };
		bnch_swt::file_loader::saveFile(discordMinifiedData, jsonOutPath + "/Discord Test (Minified).json");
		std::string canadaData{ bnch_swt::file_loader::loadFile(jsonPath + "/" + "/Canada Test (Prettified).json") };
		bnch_swt::file_loader::saveFile(canadaData, jsonOutPath + "/Canada Test (Prettified).json");
		std::string canadaMinifiedData{ bnch_swt::file_loader::loadFile(jsonPath + "/" + "/Canada Test (Minified).json") };
		bnch_swt::file_loader::saveFile(canadaMinifiedData, jsonOutPath + "/Canada Test (Minified).json");
		std::string citmCatalogData{ bnch_swt::file_loader::loadFile(jsonPath + "/" + "/CitmCatalog Test (Prettified).json") };
		bnch_swt::file_loader::saveFile(citmCatalogData, jsonOutPath + "/CitmCatalog Test (Prettified).json");
		std::string citmCatalogMinifiedData{ bnch_swt::file_loader::loadFile(jsonPath + "/" + "/CitmCatalog Test (Minified).json") };
		bnch_swt::file_loader::saveFile(citmCatalogMinifiedData, jsonOutPath + "/CitmCatalog Test (Minified).json");
		std::string twitterData{ bnch_swt::file_loader::loadFile(jsonPath + "/" + "/Twitter Test (Prettified).json") };
		bnch_swt::file_loader::saveFile(twitterData, jsonOutPath + "/Twitter Test (Prettified).json");
		std::string twitterMinifiedData{ bnch_swt::file_loader::loadFile(jsonPath + "/" + "/Twitter Test (Minified).json") };
		bnch_swt::file_loader::saveFile(twitterMinifiedData, jsonOutPath + "/Twitter Test (Minified).json");
		bnch_swt::file_loader::saveFile(twitterData, jsonOutPath + "/Minify Test.json");
		bnch_swt::file_loader::saveFile(twitterData, jsonOutPath + "/Validate Test.json");
		bnch_swt::file_loader::saveFile(twitterMinifiedData, jsonOutPath + "/Prettify Test.json");
		std::string newTimeString{};
		newTimeString.resize(1024);
		std::tm resultTwo{ getTime() };
		std::vector<test_results> benchmark_data{};
		newTimeString.resize(strftime(newTimeString.data(), 1024, "%b %d, %Y", &resultTwo));
		std::string newerString{ static_cast<std::string>(section00) + newTimeString + ")\n" + static_cast<std::string>(section002) + section001 };
		test_results testResults{ json_tests_helper<test_type::parse_and_serialize, test<test_struct>, false, maxIterationCount, "Json Test (Prettified)">::run(jsonDataNew) };
		newerString += testResults.markdownResults;
		benchmark_data.emplace_back(testResults);
		testResults = json_tests_helper<test_type::parse_and_serialize, test<test_struct>, true, maxIterationCount, "Json Test (Minified)">::run(jsonMinifiedData);
		newerString += testResults.markdownResults;
		benchmark_data.emplace_back(testResults);
		testResults = json_tests_helper<test_type::parse_and_serialize, partial_test<test_struct>, false, maxIterationCount, "Partial Test (Prettified)">::run(jsonDataNew);
		newerString += testResults.markdownResults;
		benchmark_data.emplace_back(testResults);
		testResults = json_tests_helper<test_type::parse_and_serialize, partial_test<test_struct>, true, maxIterationCount, "Partial Test (Minified)">::run(jsonMinifiedData);
		newerString += testResults.markdownResults;
		benchmark_data.emplace_back(testResults);
		testResults =
			json_tests_helper<test_type::parse_and_serialize, abc_test<abc_test_struct>, false, maxIterationCount, "Abc (Out of Order) Test (Prettified)">::run(jsonDataNew);
		newerString += testResults.markdownResults;
		benchmark_data.emplace_back(testResults);
		testResults =
			json_tests_helper<test_type::parse_and_serialize, abc_test<abc_test_struct>, true, maxIterationCount, "Abc (Out of Order) Test (Minified)">::run(jsonMinifiedData);
		newerString += testResults.markdownResults;
		benchmark_data.emplace_back(testResults);
		testResults = json_tests_helper<test_type::parse_and_serialize, discord_message, false, maxIterationCount, "Discord Test (Prettified)">::run(discordData);
		newerString += testResults.markdownResults;
		benchmark_data.emplace_back(testResults);
		testResults = json_tests_helper<test_type::parse_and_serialize, discord_message, true, maxIterationCount, "Discord Test (Minified)">::run(discordMinifiedData);
		newerString += testResults.markdownResults;
		benchmark_data.emplace_back(testResults);
		testResults = json_tests_helper<test_type::parse_and_serialize, canada_message, false, maxIterationCount, "Canada Test (Prettified)">::run(canadaData);
		newerString += testResults.markdownResults;
		benchmark_data.emplace_back(testResults);
		testResults = json_tests_helper<test_type::parse_and_serialize, canada_message, true, maxIterationCount, "Canada Test (Minified)">::run(canadaMinifiedData);
		newerString += testResults.markdownResults;
		benchmark_data.emplace_back(testResults);
		testResults = json_tests_helper<test_type::parse_and_serialize, citm_catalog_message, false, maxIterationCount, "CitmCatalog Test (Prettified)">::run(citmCatalogData);
		newerString += testResults.markdownResults;
		benchmark_data.emplace_back(testResults);
		testResults = json_tests_helper<test_type::parse_and_serialize, citm_catalog_message, true, maxIterationCount, "CitmCatalog Test (Minified)">::run(citmCatalogMinifiedData);
		newerString += testResults.markdownResults;
		benchmark_data.emplace_back(testResults);
		testResults = json_tests_helper<test_type::parse_and_serialize, twitter_message, false, maxIterationCount, "Twitter Test (Prettified)">::run(twitterData);
		newerString += testResults.markdownResults;
		benchmark_data.emplace_back(testResults);
		testResults = json_tests_helper<test_type::parse_and_serialize, twitter_message, true, maxIterationCount, "Twitter Test (Minified)">::run(twitterMinifiedData);
		newerString += testResults.markdownResults;
		benchmark_data.emplace_back(testResults);
		testResults = json_tests_helper<test_type::minify, std::string, false, maxIterationCount, "Minify Test">::run(twitterData);
		newerString += testResults.markdownResults;
		benchmark_data.emplace_back(testResults);
		testResults = json_tests_helper<test_type::prettify, std::string, false, maxIterationCount, "Prettify Test">::run(twitterMinifiedData);
		newerString += testResults.markdownResults;
		benchmark_data.emplace_back(testResults);
		testResults = json_tests_helper<test_type::validate, std::string, false, maxIterationCount, "Validate Test">::run(twitterData);
		newerString += testResults.markdownResults;
		benchmark_data.emplace_back(testResults);
		std::string resultsStringJson{};
		test_results_final resultsData{};
		for (auto& value: benchmark_data) {
			test_elements_final testElement{};
			testElement.testName = value.testName;
			for (auto& valueNew: value.results) {
				test_element_final resultFinal{};
				if (valueNew.readResult.jsonSpeed.has_value()) {
					resultFinal.libraryName = valueNew.name;
					resultFinal.color		= valueNew.readResult.color;
					resultFinal.resultSpeed = valueNew.readResult.jsonSpeed.value();
					resultFinal.resultType	= "Read";
					testElement.results.emplace_back(resultFinal);
				}
				if (valueNew.writeResult.jsonSpeed.has_value()) {
					resultFinal.libraryName = valueNew.name;
					resultFinal.color		= valueNew.writeResult.color;
					resultFinal.resultSpeed = valueNew.writeResult.jsonSpeed.value();
					resultFinal.resultType	= "Write";
					testElement.results.emplace_back(resultFinal);
				}
			}
			resultsData.emplace_back(testElement);
		}
		parser.serializeJson<jsonifier::serialize_options{ .prettify = true }>(resultsData, resultsStringJson);
		std::cout << "Json Data: " << resultsStringJson << std::endl;
		bnch_swt::file_loader::saveFile(resultsStringJson, jsonOutPath + "/Results.json");
		bnch_swt::file_loader::saveFile(static_cast<std::string>(newerString), readMePath + "/" + getCurrentPathImpl() + ".md");
		std::cout << "Md Data: " << newerString << std::endl;
		executePythonScript(std::string{ basePath } + "/GenerateGraphs.py", jsonOutPath + "/Results.json", graphsPath);
#if !defined(NDEBUG)
		for (auto& value: jsonifier_internal::types) {
			std::cout << "TYPE: " << value.first << ", HASH-TYPE: " << value.second << std::endl;
		}
#endif
	};

}