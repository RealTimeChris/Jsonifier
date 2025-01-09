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

#include "Jsonifier.hpp"
#include "Simdjson.hpp"
#include "Glaze.hpp"

namespace tests {

	enum class test_type {
		parse_and_serialize				  = 0,
		minify							  = 1,
		prettify						  = 2,
		validate						  = 3,
		parse_and_serialize_raw_json_data = 3,
	};

	enum class json_library {
		jsonifier = 0,
		glaze	  = 1,
		simdjson  = 2,
	};

	template<json_library lib, test_type type, typename test_data_type, bool minified, size_t iterations, const bnch_swt::string_literal testName> struct json_test_helper;

	template<typename value_type> struct parse_raw_json_data {};

	template<> struct parse_raw_json_data<test_struct> {
		test_struct impl(jsonifier::raw_json_data& rawJsonData) {
			test_struct output{};
			if (rawJsonData.getType() == jsonifier::json_type::object) {
				auto& json_object = rawJsonData.getObject();
				for (auto& [key, value]: json_object) {
					if (key == "testVals01" && value.getType() == jsonifier::json_type::array) {
						for (auto& element: value.getArray()) {
							if (element.getType() == jsonifier::json_type::string) {
								output.testVals01.emplace_back(static_cast<std::string>(element.getString()));
							}
						}
					} else if (key == "testVals02" && value.getType() == jsonifier::json_type::array) {
						for (auto& element: value.getArray()) {
							if (element.getType() == jsonifier::json_type::number) {
								output.testVals02.emplace_back(element.getUint());
							}
						}
					} else if (key == "testVals03" && value.getType() == jsonifier::json_type::array) {
						for (auto& element: value.getArray()) {
							if (element.getType() == jsonifier::json_type::number) {
								output.testVals03.emplace_back(element.getInt());
							}
						}
					} else if (key == "testVals04" && value.getType() == jsonifier::json_type::array) {
						for (auto& element: value.getArray()) {
							if (element.getType() == jsonifier::json_type::number) {
								output.testVals04.emplace_back(element.getDouble());
							}
						}
					} else if (key == "testVals05" && value.getType() == jsonifier::json_type::array) {
						for (auto& element: value.getArray()) {
							if (element.getType() == jsonifier::json_type::boolean) {
								output.testVals05.emplace_back(element.getBool());
							}
						}
					}
				}
			}
			return output;
		}
	};

	template<typename test_data_type, bool minified, size_t iterations, const bnch_swt::string_literal testNameNew>
	struct json_test_helper<json_library::jsonifier, test_type::parse_and_serialize_raw_json_data, test_data_type, minified, iterations, testNameNew> {
		static auto run(const test_data_type& testData) {
			static constexpr bnch_swt::string_literal testName{ testNameNew };
			static constexpr bnch_swt::string_literal testNameRead{ testName + "-Read-Raw-Json-Data" };
			static constexpr bnch_swt::string_literal testNameWrite{ testName + "-Write-Raw-Json-Data" };
			static constexpr bool partialRead{ std::is_same_v<test_data_type, partial_test<test_struct>> || std::is_same_v<test_data_type, twitter_partial_message> };
			static constexpr bool knownOrder{ !std::is_same_v<test_data_type, twitter_partial_message> };
			std::array<std::string, iterations> newStrings{};
			results_data r{ jsonifierLibraryName, testName, jsonifierCommitUrl, iterations };
			jsonifier::jsonifier_core parser{};
			std::string newBuffer{};
			parser.parseJson<jsonifier::parse_options{ .partialRead = partialRead, .knownOrder = knownOrder, .minified = minified }>(testData, newBuffer);
			for (auto& value: parser.getErrors()) {
				std::cout << "Jsonifier Error: " << value << std::endl;
			}
			std::string newerBuffer{};
			parser.serializeJson<jsonifier::serialize_options{ .prettify = !minified }>(testData, newerBuffer);
			std::string newestBuffer{ newBuffer };
			if (auto error =
					glz::read<glz::opts{ .error_on_unknown_keys = !partialRead, .skip_null_members = false, .prettify = !minified, .minified = minified }>(testData, newestBuffer);
				error) {
				std::cout << "Glaze Error: " << glz::format_error(error, newestBuffer) << std::endl;
			}
			newestBuffer.clear();
			if (auto error =
					glz::write<glz::opts{ .error_on_unknown_keys = !partialRead, .skip_null_members = false, .prettify = !minified, .minified = minified }>(testData, newestBuffer);
				error) {
				std::cout << "Glaze Error: " << glz::format_error(error, newestBuffer) << std::endl;
			}
			for (size_t x = 0; x < newestBuffer.size() && x < newerBuffer.size(); ++x) {
				if (newestBuffer[x] != newerBuffer[x]) {
					std::cout << "DIFFERENT AT INDEX: " << x;
					std::cout << "JSONIFIER VALUES: " << jsonifier::string_view{ newerBuffer.data() + x, 128 } << std::endl;
					std::cout << "GLAZE VALUES: " << jsonifier::string_view{ newestBuffer.data() + x, 128 } << std::endl;
					break;
				}
			}
			bnch_swt::performance_metrics readResult = bnch_swt::benchmark_stage<testNameRead, iterations>::template runBenchmark<jsonifierLibraryName, "teal">([&]() mutable {
				parser.parseJson<jsonifier::parse_options{ .partialRead = partialRead, .knownOrder = knownOrder, .minified = minified }>(testData, newBuffer);
				bnch_swt::doNotOptimizeAway(testData);
				return newerBuffer.size();
			});
			for (auto& value: parser.getErrors()) {
				std::cout << "Jsonifier Error: " << value << std::endl;
			}
			parser.serializeJson<jsonifier::serialize_options{ .prettify = !minified }>(testData, newerBuffer);
			bnch_swt::performance_metrics writeResult =
				bnch_swt::benchmark_stage<testNameWrite, iterations>::template runBenchmark<jsonifierLibraryName, "steelblue">([&]() mutable {
					parser.serializeJson<jsonifier::serialize_options{ .prettify = !minified }>(testData, newerBuffer);
					bnch_swt::doNotOptimizeAway(newerBuffer);
					return newerBuffer.size();
				});
			r.readResult  = result<result_type::read>{ "teal", readResult };
			r.writeResult = result<result_type::write>{ "steelblue", writeResult };
			bnch_swt::file_loader::saveFile(static_cast<std::string>(newerBuffer), jsonOutPath.operator std::string() + "/" + testName.operator std::string() + "-jsonifier.json");
			return r;
		}
	};

	template<typename test_data_type, bool minified, size_t iterations, const bnch_swt::string_literal testNameNew>
	struct json_test_helper<json_library::jsonifier, test_type::parse_and_serialize, test_data_type, minified, iterations, testNameNew> {
		static auto run(const test_data_type& testDataNew) {
			static constexpr bnch_swt::string_literal testName{ testNameNew };
			static constexpr bnch_swt::string_literal testNameRead{ testName + "-Read" };
			static constexpr bnch_swt::string_literal testNameWrite{ testName + "-Write" };
			static constexpr bool partialRead{ std::is_same_v<test_data_type, partial_test<test_struct>> || std::is_same_v<test_data_type, twitter_partial_message> };
			static constexpr bool knownOrder{ !std::is_same_v<test_data_type, abc_test<abc_test_struct>> && !std::is_same_v<test_data_type, twitter_partial_message> };
			results_data r{ jsonifierLibraryName, testName, jsonifierCommitUrl, iterations };
			jsonifier::jsonifier_core parser{};
			std::array<std::string, iterations * 2> newStrings{};
			std::string newString{};
			std::array<test_data_type, iterations * 2> testDatas{};
			parser.serializeJson<jsonifier::serialize_options{ .prettify = !minified }>(testDataNew, newString);
			size_t currentIndex{};
			bnch_swt::performance_metrics readResult = bnch_swt::benchmark_stage<testNameRead, iterations>::template runBenchmark<jsonifierLibraryName, "teal">([&]() mutable {
				parser.parseJson<jsonifier::parse_options{ .partialRead = partialRead, .knownOrder = knownOrder, .minified = minified }>(testDatas[currentIndex], newString);
				bnch_swt::doNotOptimizeAway(testDatas[currentIndex]);
				auto newSize = newString.size();
				++currentIndex;
				return newSize;
			});
			for (auto& value: parser.getErrors()) {
				std::cout << "Jsonifier Error: " << value << std::endl;
			}
			currentIndex = 0;
			bnch_swt::performance_metrics writeResult =
				bnch_swt::benchmark_stage<testNameWrite, iterations>::template runBenchmark<jsonifierLibraryName, "steelblue">([&]() mutable {
					parser.serializeJson<jsonifier::serialize_options{ .prettify = !minified }>(testDatas[currentIndex], newStrings[currentIndex]);
					bnch_swt::doNotOptimizeAway(newStrings[currentIndex]);
					auto newSize = newStrings[currentIndex].size();
					++currentIndex;
					return newSize;
				});
			r.readResult  = result<result_type::read>{ "teal", readResult };
			r.writeResult = result<result_type::write>{ "steelblue", writeResult };
			bnch_swt::file_loader::saveFile(static_cast<std::string>(newStrings[0]),
				jsonOutPath.operator std::string() + "/" + testName.operator std::string() + "-jsonifier.json");
			return r;
		}
	};

	template<size_t iterations, bnch_swt::string_literal testNameNew>
	struct json_test_helper<json_library::jsonifier, test_type::prettify, std::string, false, iterations, testNameNew> {
		static auto run(std::string& newBuffer) {
			static constexpr bnch_swt::string_literal testName{ testNameNew };
			results_data r{ jsonifierLibraryName, testName, jsonifierCommitUrl, iterations };
			jsonifier::jsonifier_core parser{};
			std::array<std::string, iterations * 2> newStrings{};
			size_t currentIndex{};
			auto writeResult = bnch_swt::benchmark_stage<testName, iterations>::template runBenchmark<jsonifierLibraryName, "steelblue">([&]() mutable {
				parser.prettifyJson(newBuffer, newStrings[currentIndex]);
				bnch_swt::doNotOptimizeAway(newStrings[currentIndex]);
				auto newSize = newStrings[currentIndex].size();
				++currentIndex;
				return newSize;
			});

			for (auto& value: parser.getErrors()) {
				std::cout << "Jsonifier Error: " << value << std::endl;
			}
			bnch_swt::file_loader::saveFile(newStrings[0], jsonOutPath.operator std::string() + "/" + testName.operator std::string() + "-jsonifier.json");
			r.writeResult = result<result_type::write>{ "steelblue", writeResult };
			return r;
		}
	};

	template<size_t iterations, bnch_swt::string_literal testNameNew>
	struct json_test_helper<json_library::jsonifier, test_type::minify, std::string, false, iterations, testNameNew> {
		static auto run(std::string& newBuffer) {
			static constexpr bnch_swt::string_literal testName{ testNameNew };
			std::array<std::string, iterations * 2> newStrings{};
			results_data r{ jsonifierLibraryName, testName, jsonifierCommitUrl, iterations };
			jsonifier::jsonifier_core parser{};
			size_t currentIndex{};
			auto writeResult = bnch_swt::benchmark_stage<testName, iterations>::template runBenchmark<jsonifierLibraryName, "steelblue">([&]() mutable {
				parser.minifyJson(newBuffer, newStrings[currentIndex]);
				bnch_swt::doNotOptimizeAway(newStrings[currentIndex]);
				auto newSize = newStrings[currentIndex].size();
				++currentIndex;
				return newSize;
			});
			for (auto& value: parser.getErrors()) {
				std::cout << "Jsonifier Error: " << value << std::endl;
			}
			bnch_swt::file_loader::saveFile(newStrings[0], jsonOutPath.operator std::string() + "/" + testName.operator std::string() + "-jsonifier.json");
			r.writeResult = result<result_type::write>{ "steelblue", writeResult };
			return r;
		}
	};

	template<size_t iterations, bnch_swt::string_literal testNameNew>
	struct json_test_helper<json_library::jsonifier, test_type::validate, std::string, false, iterations, testNameNew> {
		static auto run(std::string& newBuffer) {
			static constexpr bnch_swt::string_literal testName{ testNameNew };
			results_data r{ jsonifierLibraryName, testName, jsonifierCommitUrl, iterations };
			jsonifier::jsonifier_core parser{};
			bnch_swt::performance_metrics readResult = bnch_swt::benchmark_stage<testName, iterations>::template runBenchmark<jsonifierLibraryName, "teal">([&]() mutable {
				bnch_swt::doNotOptimizeAway(parser.validateJson(newBuffer));
				return newBuffer.size();
			});

			for (auto& value: parser.getErrors()) {
				std::cout << "Jsonifier Error: " << value << std::endl;
			}
			bnch_swt::file_loader::saveFile(newBuffer, jsonOutPath.operator std::string() + "/" + testName.operator std::string() + "-jsonifier.json");
			r.readResult = result<result_type::read>{ "teal", readResult };
			return r;
		}
	};

	template<typename test_data_type, bool minified, size_t iterations, const bnch_swt::string_literal testNameNew>
	struct json_test_helper<json_library::glaze, test_type::parse_and_serialize, test_data_type, minified, iterations, testNameNew> {
		static auto run(const test_data_type& testDataNew) {
			static constexpr bnch_swt::string_literal testName{ testNameNew };
			static constexpr bnch_swt::string_literal testNameRead{ testName + "-Read" };
			static constexpr bnch_swt::string_literal testNameWrite{ testName + "-Write" };
			static constexpr bool partialRead{ std::is_same_v<test_data_type, partial_test<test_struct>> || std::is_same_v<test_data_type, twitter_partial_message> };
			results_data r{ glazeLibraryName, testName, glazeCommitUrl, iterations };
			std::array<std::string, iterations * 2> newStrings{};
			std::string newString{};
			std::array<test_data_type, iterations * 2> testDatas{};
			auto newResult = glz::write<glz::opts{ .skip_null_members = false, .prettify = !minified, .minified = minified }>(testDataNew, newString);
			( void )newResult;
			size_t currentIndex{};
			bnch_swt::performance_metrics readResult  = bnch_swt::benchmark_stage<testNameRead, iterations>::template runBenchmark<glazeLibraryName, "dodgerblue">([&]() mutable {
				 if (auto error = glz::read<
						 glz::opts{ .error_on_unknown_keys = !partialRead, .skip_null_members = false, .prettify = !minified, .minified = minified, .partial_read = partialRead }>(
						 testDatas[currentIndex], newString);
					 error) {
					 std::cout << "Glaze Error: " << glz::format_error(error, newStrings[currentIndex]) << std::endl;
				 }
				 bnch_swt::doNotOptimizeAway(testDatas[currentIndex]);
				 auto newSize = newString.size();
				 ++currentIndex;
				 return newSize;
			 });
			currentIndex							  = 0;
			bnch_swt::performance_metrics writeResult = bnch_swt::benchmark_stage<testNameWrite, iterations>::template runBenchmark<glazeLibraryName, "skyblue">([&]() mutable {
				auto newResult =
					glz::write<glz::opts{ .skip_null_members = false, .prettify = !minified, .minified = minified }>(testDatas[currentIndex], newStrings[currentIndex]);
				bnch_swt::doNotOptimizeAway(newStrings[currentIndex]);
				( void )newResult;
				auto newSize = newStrings[currentIndex].size();
				++currentIndex;
				return newSize;
			});
			r.readResult							  = result<result_type::read>{ "dodgerblue", readResult };
			r.writeResult							  = result<result_type::write>{ "skyblue", writeResult };
			bnch_swt::file_loader::saveFile(static_cast<std::string>(newStrings[0]), jsonOutPath.operator std::string() + "/" + testName.operator std::string() + "-glaze.json");
			return r;
		}
	};

	template<size_t iterations, bnch_swt::string_literal testNameNew>
	struct json_test_helper<json_library::glaze, test_type::prettify, std::string, false, iterations, testNameNew> {
		static auto run(std::string& newBuffer) {
			static constexpr bnch_swt::string_literal testName{ testNameNew };
			std::array<std::string, iterations * 2> newStrings{};
			results_data r{ glazeLibraryName, testName, glazeCommitUrl, iterations };
			size_t currentIndex{};

			auto writeResult = bnch_swt::benchmark_stage<testName, iterations>::template runBenchmark<glazeLibraryName, "skyblue">([&]() mutable {
				glz::prettify_json(newBuffer, newStrings[currentIndex]);
				bnch_swt::doNotOptimizeAway(newStrings[currentIndex]);
				auto newSize = newStrings[currentIndex].size();
				++currentIndex;
				return newSize;
			});

			bnch_swt::file_loader::saveFile(newStrings[0], jsonOutPath.operator std::string() + "/" + testName.operator std::string() + "-glaze.json");
			r.writeResult = result<result_type::write>{ "skyblue", writeResult };

			return r;
		}
	};

	template<size_t iterations, bnch_swt::string_literal testNameNew> struct json_test_helper<json_library::glaze, test_type::minify, std::string, false, iterations, testNameNew> {
		static auto run(std::string& newBuffer) {
			static constexpr bnch_swt::string_literal testName{ testNameNew };

			std::string newestBuffer{ newBuffer };
			std::array<std::string, iterations * 2> newStrings{};
			size_t currentIndex{};
			results_data r{ glazeLibraryName, testName, glazeCommitUrl, iterations };
			auto writeResult = bnch_swt::benchmark_stage<testName, iterations>::template runBenchmark<glazeLibraryName, "skyblue">([&]() mutable {
				glz::minify_json(newestBuffer, newStrings[currentIndex]);
				bnch_swt::doNotOptimizeAway(newStrings[currentIndex]);
				auto newSize = newStrings[currentIndex].size();
				++currentIndex;
				return newSize;
			});

			bnch_swt::file_loader::saveFile(newStrings[0], jsonOutPath.operator std::string() + "/" + testName.operator std::string() + "-glaze.json");
			r.writeResult = result<result_type::write>{ "skyblue", writeResult };

			return r;
		}
	};

	template<size_t iterations, bnch_swt::string_literal testNameNew>
	struct json_test_helper<json_library::glaze, test_type::validate, std::string, false, iterations, testNameNew> {
		static auto run(std::string& newBuffer) {
			static constexpr bnch_swt::string_literal testName{ testNameNew };
			results_data r{ glazeLibraryName, testName, glazeCommitUrl, iterations };
			bnch_swt::performance_metrics readResult = bnch_swt::benchmark_stage<testName, iterations>::template runBenchmark<glazeLibraryName, "skyblue">([&]() mutable {
				bnch_swt::doNotOptimizeAway(glz::validate_json(newBuffer));
				return newBuffer.size();
			});

			bnch_swt::file_loader::saveFile(newBuffer, jsonOutPath.operator std::string() + "/" + testName.operator std::string() + "-glaze.json");
			r.readResult = result<result_type::read>{ "skyblue", readResult };

			return r;
		}
	};

	template<typename test_data_type, bool minified, size_t iterations, const bnch_swt::string_literal testNameNew>
	struct json_test_helper<json_library::simdjson, test_type::parse_and_serialize, test_data_type, minified, iterations, testNameNew> {
		static auto run(const test_data_type& testDataNew) {
			static constexpr bnch_swt::string_literal testName{ testNameNew };
			static constexpr bnch_swt::string_literal testNameRead{ testName + "-Read" };
			static constexpr bnch_swt::string_literal testNameWrite{ testName + "-Write" };
			results_data r{ simdjsonLibraryName, testName, simdjsonCommitUrl, iterations };
			jsonifier::jsonifier_core parserNew{};
			simdjson::ondemand::parser parser{};
			std::string newString{};
			std::array<test_data_type, iterations * 2> testDatas{};
			parserNew.serializeJson<jsonifier::serialize_options{ .prettify = !minified }>(testDataNew, newString);
			size_t currentIndex{};
			bnch_swt::performance_metrics readResult = bnch_swt::benchmark_stage<testNameRead, iterations>::template runBenchmark<simdjsonLibraryName, "cadetblue">([&]() mutable {
				try {
					getValue(testDatas[currentIndex], parser.iterate(newString).value());
					bnch_swt::doNotOptimizeAway(testDatas[currentIndex]);
					auto newSize = newString.size();
					++currentIndex;
					return newSize;

				} catch (const std::exception& error) {
					std::cout << "Simdjson Error: " << error.what() << std::endl;
				}
				++currentIndex;
				return newString.size();
			});
			r.readResult							 = result<result_type::read>{ "cadetblue", readResult };
			bnch_swt::file_loader::saveFile(static_cast<std::string>(newString), jsonOutPath.operator std::string() + "/" + testName.operator std::string() + "-simdjson.json");
			return r;
		}
	};

	template<size_t iterations, bnch_swt::string_literal testNameNew>
	struct json_test_helper<json_library::simdjson, test_type::minify, std::string, false, iterations, testNameNew> {
		static constexpr bnch_swt::string_literal testName{ testNameNew };
		static auto run(std::string& newBuffer) {
			results_data r{ simdjsonLibraryName, testName, simdjsonCommitUrl, iterations };

			simdjson::dom::parser parser{};
			std::array<std::string, iterations * 2> newStrings{};
			size_t currentIndex{};

			auto writeResult = bnch_swt::benchmark_stage<testName, iterations>::template runBenchmark<simdjsonLibraryName, "cornflowerblue">([&]() mutable {
				try {
					newStrings[currentIndex] = simdjson::minify(parser.parse(newBuffer));
					bnch_swt::doNotOptimizeAway(newStrings[currentIndex]);
					auto newSize = newStrings[currentIndex].size();
					++currentIndex;
					return newSize;
				} catch (std::exception& error) {
					std::cout << "Simdjson Error: " << error.what() << std::endl;
					return newStrings[currentIndex].size();
				}
			});

			bnch_swt::file_loader::saveFile(newStrings[0], jsonOutPath.operator std::string() + "/" + testName.operator std::string() + "-simdjson.json");
			r.writeResult = result<result_type::write>{ "cornflowerblue", writeResult };

			return r;
		}
	};

#if defined(JSONIFIER_MAC)
	constexpr bnch_swt::string_literal table_header = bnch_swt::string_literal{
		R"(
| Library | Read (MB/S) | Read Percentage Deviation (+/-%) | Read Length (Bytes) | Read Time (ns) | Write (MB/S) | Write Percentage Deviation (+/-%) | Write Length (Bytes) | Write Time (ns) |
| ------- | ----------- | -------------------------------- | ------------------- | -------------- | ------------ | --------------------------------- | -------------------- | --------------- |  )"
	};

	constexpr bnch_swt::string_literal read_table_header = bnch_swt::string_literal{ R"(
| Library | Read (MB/S) | Read Percentage Deviation (+/-%) | Read Length (Bytes) | Read Time (ns) |
| ------- | ----------- | -------------------------------- | ------------------- | -------------- |  )" };

	constexpr bnch_swt::string_literal write_table_header = bnch_swt::string_literal{
		R"(
| Library | Write (MB/S) | Write Percentage Deviation (+/-%) | Write Length (Bytes) | Write Time (ns) |
| ------- | ------------ | --------------------------------- | -------------------- | --------------- |  )"
	};
#else
	constexpr bnch_swt::string_literal table_header = bnch_swt::string_literal{
		R"(
| Library | Read (MB/S) | Read Percentage Deviation (+/-%) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Write (MB/S) | Write Percentage Deviation (+/-%) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) |
| ------- | ----------- | -------------------------------- | -----------------| ------------------- | -------------- | ------------ | --------------------------------- | ------------------| -------------------- | --------------- |  )"
	};

	constexpr bnch_swt::string_literal read_table_header = bnch_swt::string_literal{ R"(
| Library | Read (MB/S) | Read Percentage Deviation (+/-%) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) |
| ------- | ----------- | -------------------------------- | ---------------- | ------------------- | -------------- |  )" };

	constexpr bnch_swt::string_literal write_table_header = bnch_swt::string_literal{
		R"(
| Library | Write (MB/S) | Write Percentage Deviation (+/-%) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) |
| ------- | ------------ | --------------------------------- | ----------------- | -------------------- | --------------- |  )"
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

	std::string generateSection(const std::string& testName, const std::string& currentPathNew) {
		return R"(

### )" + testName +
			R"( Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/)" + currentPathNew + R"(/)" +
			urlEncode(testName) +
			R"(.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/)" +
			currentPathNew + R"(/)" + urlEncode(testName) + R"(_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/)" +
			currentPathNew + R"(/)" + urlEncode(testName) + R"(_Results.png?raw=true" 
alt="" width="400"/></p>
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/)" +
			currentPathNew + R"(/)" + urlEncode(testName) +
			R"(_Cumulative_Speedup.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/)" + currentPathNew + R"(/)" + testName +
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
		static test_results run(const test_data_type& jsonDataNew) {
			static constexpr bnch_swt::string_literal testName{ testNameNew };
			test_results jsonResults{};
			jsonResults.testName = testName.operator std::string();
			results_data jsonifierResults{};
			results_data simdjsonResults{};
			results_data glazeResults{};

#if !defined(ASAN_ENABLED)
			simdjsonResults = json_test_helper<json_library::simdjson, type, test_data_type, minified, iterations, testName>::run(jsonDataNew);
			if constexpr (!std::is_same_v<test_data_type, partial_test<test_struct>> && !std::is_same_v<test_data_type, twitter_partial_message>) {
				glazeResults = json_test_helper<json_library::glaze, type, test_data_type, minified, iterations, testName>::run(jsonDataNew);
				jsonResults.results.emplace_back(glazeResults);
			}
#endif
			jsonifierResults = json_test_helper<json_library::jsonifier, type, test_data_type, minified, iterations, testName>::run(jsonDataNew);
#if !defined(ASAN_ENABLED)
			jsonResults.results.emplace_back(simdjsonResults);
#endif
			jsonResults.results.emplace_back(jsonifierResults);
			jsonResults.markdownResults += generateSection(testName, currentPath);
			jsonResults.markdownResults += table_header.operator std::string() + "\n";
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
			jsonResults.testName = testName.operator std::string();
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
			jsonResults.markdownResults += generateSection(testName, currentPath);
			jsonResults.markdownResults += write_table_header.operator std::string() + "\n";
			std::sort(jsonResults.results.begin(), jsonResults.results.end(), std::greater<results_data>());
			for (auto iter = jsonResults.results.begin(); iter != jsonResults.results.end();) {
				jsonResults.markdownResults += iter->jsonStats();
				if (static_cast<size_t>(iter - jsonResults.results.begin()) != jsonResults.results.size() - 1) {
					jsonResults.markdownResults += "\n";
				}
				++iter;
			}
			bnch_swt::benchmark_stage<testNameNew, iterations>::printResults();
			return jsonResults;
		}
	};

	template<uint64_t iterations, bnch_swt::string_literal testNameNew> struct json_tests_helper<test_type::minify, std::string, false, iterations, testNameNew> {
		static test_results run(std::string& jsonDataNew) {
			static constexpr bnch_swt::string_literal testName{ testNameNew };
			test_results jsonResults{};
			jsonResults.testName = testName.operator std::string();
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
			jsonResults.markdownResults += generateSection(testName, currentPath);
			jsonResults.markdownResults += write_table_header.operator std::string() + "\n";
			std::sort(jsonResults.results.begin(), jsonResults.results.end(), std::greater<results_data>());
			for (auto iter = jsonResults.results.begin(); iter != jsonResults.results.end();) {
				jsonResults.markdownResults += iter->jsonStats();
				if (static_cast<size_t>(iter - jsonResults.results.begin()) != jsonResults.results.size() - 1) {
					jsonResults.markdownResults += "\n";
				}
				++iter;
			}
			bnch_swt::benchmark_stage<testNameNew, iterations>::printResults();
			return jsonResults;
		}
	};

	template<uint64_t iterations, bnch_swt::string_literal testNameNew> struct json_tests_helper<test_type::validate, std::string, false, iterations, testNameNew> {
		static test_results run(std::string& jsonDataNew) {
			static constexpr bnch_swt::string_literal testName{ testNameNew };
			test_results jsonResults{};
			jsonResults.testName = testName.operator std::string();
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
			jsonResults.markdownResults += generateSection(testName.operator std::string(), currentPath.operator std::string());
			jsonResults.markdownResults += read_table_header.operator std::string() + "\n";
			std::sort(jsonResults.results.begin(), jsonResults.results.end(), std::greater<results_data>());
			for (auto iter = jsonResults.results.begin(); iter != jsonResults.results.end();) {
				jsonResults.markdownResults += iter->jsonStats();
				if (static_cast<size_t>(iter - jsonResults.results.begin()) != jsonResults.results.size() - 1) {
					jsonResults.markdownResults += "\n";
				}
				++iter;
			}
			bnch_swt::benchmark_stage<testNameNew, iterations>::printResults();
			return jsonResults;
		}
	};

	void testFunction() {
		test_generator<test_struct> testJsonData{};
		std::string jsonDataNew{};
		jsonifier::jsonifier_core parser{};
		parser.serializeJson<jsonifier::serialize_options{ .prettify = true }>(testJsonData, jsonDataNew);
		bnch_swt::file_loader::saveFile(jsonDataNew, jsonOutPath.operator std::string() + "/Json Test (Prettified).json");
		bnch_swt::file_loader::saveFile(jsonDataNew, jsonOutPath.operator std::string() + "/Abc (Out of Order) Test (Prettified).json");
		bnch_swt::file_loader::saveFile(jsonDataNew, jsonOutPath.operator std::string() + "/Partial Test (Prettified).json");
		std::string jsonMinifiedData{ glz::minify_json(jsonDataNew) };
		bnch_swt::file_loader::saveFile(jsonMinifiedData, jsonOutPath.operator std::string() + "/Json Test (Minified).json");
		bnch_swt::file_loader::saveFile(jsonMinifiedData, jsonOutPath.operator std::string() + "/Abc (Out of Order) Test (Minified).json");
		bnch_swt::file_loader::saveFile(jsonMinifiedData, jsonOutPath.operator std::string() + "/Partial Test (Minified).json");
		std::string discordData{ bnch_swt::file_loader::loadFile(jsonPath.operator std::string() + "/" + "/Discord Test (Prettified).json") };
		bnch_swt::file_loader::saveFile(discordData, jsonOutPath.operator std::string() + "/Discord Test (Prettified).json");
		std::string discordMinifiedData{ bnch_swt::file_loader::loadFile(jsonPath.operator std::string() + "/" + "/Discord Test (Minified).json") };
		bnch_swt::file_loader::saveFile(discordMinifiedData, jsonOutPath.operator std::string() + "/Discord Test (Minified).json");
		std::string canadaData{ bnch_swt::file_loader::loadFile(jsonPath.operator std::string() + "/" + "/Canada Test (Prettified).json") };
		bnch_swt::file_loader::saveFile(canadaData, jsonOutPath.operator std::string() + "/Canada Test (Prettified).json");
		std::string canadaMinifiedData{ bnch_swt::file_loader::loadFile(jsonPath.operator std::string() + "/" + "/Canada Test (Minified).json") };
		bnch_swt::file_loader::saveFile(canadaMinifiedData, jsonOutPath.operator std::string() + "/Canada Test (Minified).json");
		std::string citmCatalogData{ bnch_swt::file_loader::loadFile(jsonPath.operator std::string() + "/" + "/CitmCatalog Test (Prettified).json") };
		bnch_swt::file_loader::saveFile(citmCatalogData, jsonOutPath.operator std::string() + "/CitmCatalog Test (Prettified).json");
		std::string citmCatalogMinifiedData{ bnch_swt::file_loader::loadFile(jsonPath.operator std::string() + "/" + "/CitmCatalog Test (Minified).json") };
		bnch_swt::file_loader::saveFile(citmCatalogMinifiedData, jsonOutPath.operator std::string() + "/CitmCatalog Test (Minified).json");
		std::string twitterData{ bnch_swt::file_loader::loadFile(jsonPath.operator std::string() + "/" + "/Twitter Test (Prettified).json") };
		bnch_swt::file_loader::saveFile(twitterData, jsonOutPath.operator std::string() + "/Twitter Test (Prettified).json");
		std::string twitterMinifiedData{ bnch_swt::file_loader::loadFile(jsonPath.operator std::string() + "/" + "/Twitter Test (Minified).json") };
		bnch_swt::file_loader::saveFile(twitterMinifiedData, jsonOutPath.operator std::string() + "/Twitter Test (Minified).json");
		bnch_swt::file_loader::saveFile(twitterData, jsonOutPath.operator std::string() + "/Minify Test.json");
		bnch_swt::file_loader::saveFile(twitterData, jsonOutPath.operator std::string() + "/Validate Test.json");
		bnch_swt::file_loader::saveFile(twitterMinifiedData, jsonOutPath.operator std::string() + "/Prettify Test.json");
		std::string newTimeString{};
		newTimeString.resize(1024);
		std::tm resultTwo{ getTime() };
		std::vector<test_results> benchmark_data{};
		newTimeString.resize(strftime(newTimeString.data(), 1024, "%b %d, %Y", &resultTwo));
		std::string newerString{ section00.operator std::string() + newTimeString + ")\n" + static_cast<std::string>(section002.operator std::string()) +
			static_cast<std::string>(section001) };
		test_generator<test_struct> testDataNew{};
		parser.serializeJson(testDataNew, jsonDataNew);
		test<test_struct> jsonDataNewer{};
		parser.parseJson<jsonifier::parse_options{ .knownOrder = true }>(jsonDataNewer, jsonDataNew);
		test_results testResults{ json_tests_helper<test_type::parse_and_serialize, test<test_struct>, false, maxIterations, "Json Test (Prettified)">::run(jsonDataNewer) };
		newerString += testResults.markdownResults;
		benchmark_data.emplace_back(testResults);
		testResults = json_tests_helper<test_type::parse_and_serialize, test<test_struct>, true, maxIterations, "Json Test (Minified)">::run(jsonDataNewer);
		newerString += testResults.markdownResults;
		benchmark_data.emplace_back(testResults);
		partial_test<test_struct> jsonPartialDataNewer{};
		parser.parseJson(jsonPartialDataNewer, jsonDataNew);
		testResults = json_tests_helper<test_type::parse_and_serialize, partial_test<test_struct>, false, maxIterations, "Partial Test (Prettified)">::run(jsonPartialDataNewer);
		newerString += testResults.markdownResults;
		benchmark_data.emplace_back(testResults);
		testResults = json_tests_helper<test_type::parse_and_serialize, partial_test<test_struct>, true, maxIterations, "Partial Test (Minified)">::run(jsonPartialDataNewer);
		newerString += testResults.markdownResults;
		benchmark_data.emplace_back(testResults);
		abc_test<abc_test_struct> abcDataNew{};
		parser.parseJson(abcDataNew, jsonDataNew);
		testResults = json_tests_helper<test_type::parse_and_serialize, abc_test<abc_test_struct>, false, maxIterations, "Abc (Out of Order) Test (Prettified)">::run(abcDataNew);
		newerString += testResults.markdownResults;
		benchmark_data.emplace_back(testResults);
		testResults = json_tests_helper<test_type::parse_and_serialize, abc_test<abc_test_struct>, true, maxIterations, "Abc (Out of Order) Test (Minified)">::run(abcDataNew);
		newerString += testResults.markdownResults;
		benchmark_data.emplace_back(testResults);
		discord_message discordDataNew{};
		parser.parseJson<jsonifier::parse_options{ .knownOrder = true }>(discordDataNew, discordData);
		testResults = json_tests_helper<test_type::parse_and_serialize, discord_message, false, maxIterations, "Discord Test (Prettified)">::run(discordDataNew);
		newerString += testResults.markdownResults;
		benchmark_data.emplace_back(testResults);
		testResults = json_tests_helper<test_type::parse_and_serialize, discord_message, true, maxIterations, "Discord Test (Minified)">::run(discordDataNew);
		newerString += testResults.markdownResults;
		benchmark_data.emplace_back(testResults);
		canada_message canadaDataNew{};
		parser.parseJson<jsonifier::parse_options{ .knownOrder = true }>(canadaDataNew, canadaData);
		testResults = json_tests_helper<test_type::parse_and_serialize, canada_message, false, maxIterations, "Canada Test (Prettified)">::run(canadaDataNew);
		newerString += testResults.markdownResults;
		benchmark_data.emplace_back(testResults);
		testResults = json_tests_helper<test_type::parse_and_serialize, canada_message, true, maxIterations, "Canada Test (Minified)">::run(canadaDataNew);
		newerString += testResults.markdownResults;
		benchmark_data.emplace_back(testResults);
		citm_catalog_message citmCatalogDataNew{};
		parser.parseJson<jsonifier::parse_options{ .knownOrder = true }>(citmCatalogDataNew, citmCatalogData);
		testResults = json_tests_helper<test_type::parse_and_serialize, citm_catalog_message, false, maxIterations, "CitmCatalog Test (Prettified)">::run(citmCatalogDataNew);
		newerString += testResults.markdownResults;
		benchmark_data.emplace_back(testResults);
		testResults = json_tests_helper<test_type::parse_and_serialize, citm_catalog_message, true, maxIterations, "CitmCatalog Test (Minified)">::run(citmCatalogDataNew);
		newerString += testResults.markdownResults;
		benchmark_data.emplace_back(testResults);
		twitter_partial_message twitterPartialDataNew{};
		parser.parseJson(twitterPartialDataNew, twitterData);
		testResults =
			json_tests_helper<test_type::parse_and_serialize, twitter_partial_message, false, maxIterations, "Twitter Partial Test (Prettified)">::run(twitterPartialDataNew);
		newerString += testResults.markdownResults;
		benchmark_data.emplace_back(testResults);
		testResults =
			json_tests_helper<test_type::parse_and_serialize, twitter_partial_message, true, maxIterations, "Twitter Partial Test (Minified)">::run(twitterPartialDataNew);
		newerString += testResults.markdownResults;
		benchmark_data.emplace_back(testResults);
		twitter_message twitterDataNew{};
		parser.parseJson<jsonifier::parse_options{ .knownOrder = true }>(twitterDataNew, twitterData);
		testResults = json_tests_helper<test_type::parse_and_serialize, twitter_message, false, maxIterations, "Twitter Test (Prettified)">::run(twitterDataNew);
		newerString += testResults.markdownResults;
		benchmark_data.emplace_back(testResults);
		testResults = json_tests_helper<test_type::parse_and_serialize, twitter_message, true, maxIterations, "Twitter Test (Minified)">::run(twitterDataNew);
		newerString += testResults.markdownResults;
		benchmark_data.emplace_back(testResults);
		testResults = json_tests_helper<test_type::minify, std::string, false, maxIterations, "Minify Test">::run(twitterData);
		newerString += testResults.markdownResults;
		benchmark_data.emplace_back(testResults);
		testResults = json_tests_helper<test_type::prettify, std::string, false, maxIterations, "Prettify Test">::run(twitterMinifiedData);
		newerString += testResults.markdownResults;
		benchmark_data.emplace_back(testResults);
		testResults = json_tests_helper<test_type::validate, std::string, false, maxIterations, "Validate Test">::run(twitterData);
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
		bnch_swt::file_loader::saveFile(resultsStringJson, jsonOutPath.operator std::string() + "/Results.json");
		bnch_swt::file_loader::saveFile(static_cast<std::string>(newerString), readMePath.operator std::string() + "/" + currentPath.operator std::string() + ".md");
		std::cout << "Md Data: " << newerString << std::endl;
		executePythonScript(basePath.operator std::string() + "/GenerateGraphs.py", jsonOutPath.operator std::string() + "/Results.json", graphsPath.operator std::string());
#if !defined(NDEBUG)
		for (auto& value: jsonifier::internal::types) {
			std::cout << "TYPE: " << value.first << ", HASH-TYPE: " << value.second << std::endl;
		}
#endif
	};

}