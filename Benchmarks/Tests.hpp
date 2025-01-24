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
		JSONIFIER_CLANG_INLINE static test_struct impl(jsonifier::raw_json_data& rawJsonData) {
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

	template<> struct parse_raw_json_data<test<test_struct>> {
		JSONIFIER_CLANG_INLINE static test<test_struct> impl(jsonifier::raw_json_data& rawJsonData) {
			test<test_struct> output{};
			for (auto& value: rawJsonData["a"].getArray()) {
				output.a.emplace_back(parse_raw_json_data<test_struct>::impl(value));
			}
			for (auto& value: rawJsonData["b"].getArray()) {
				output.b.emplace_back(parse_raw_json_data<test_struct>::impl(value));
			}
			for (auto& value: rawJsonData["c"].getArray()) {
				output.c.emplace_back(parse_raw_json_data<test_struct>::impl(value));
			}
			for (auto& value: rawJsonData["d"].getArray()) {
				output.d.emplace_back(parse_raw_json_data<test_struct>::impl(value));
			}
			for (auto& value: rawJsonData["e"].getArray()) {
				output.e.emplace_back(parse_raw_json_data<test_struct>::impl(value));
			}
			for (auto& value: rawJsonData["f"].getArray()) {
				output.f.emplace_back(parse_raw_json_data<test_struct>::impl(value));
			}
			for (auto& value: rawJsonData["g"].getArray()) {
				output.g.emplace_back(parse_raw_json_data<test_struct>::impl(value));
			}
			for (auto& value: rawJsonData["h"].getArray()) {
				output.h.emplace_back(parse_raw_json_data<test_struct>::impl(value));
			}
			for (auto& value: rawJsonData["i"].getArray()) {
				output.i.emplace_back(parse_raw_json_data<test_struct>::impl(value));
			}
			for (auto& value: rawJsonData["j"].getArray()) {
				output.j.emplace_back(parse_raw_json_data<test_struct>::impl(value));
			}
			for (auto& value: rawJsonData["k"].getArray()) {
				output.k.emplace_back(parse_raw_json_data<test_struct>::impl(value));
			}
			for (auto& value: rawJsonData["l"].getArray()) {
				output.l.emplace_back(parse_raw_json_data<test_struct>::impl(value));
			}
			for (auto& value: rawJsonData["m"].getArray()) {
				output.m.emplace_back(parse_raw_json_data<test_struct>::impl(value));
			}
			for (auto& value: rawJsonData["n"].getArray()) {
				output.n.emplace_back(parse_raw_json_data<test_struct>::impl(value));
			}
			for (auto& value: rawJsonData["o"].getArray()) {
				output.o.emplace_back(parse_raw_json_data<test_struct>::impl(value));
			}
			for (auto& value: rawJsonData["p"].getArray()) {
				output.p.emplace_back(parse_raw_json_data<test_struct>::impl(value));
			}
			for (auto& value: rawJsonData["q"].getArray()) {
				output.q.emplace_back(parse_raw_json_data<test_struct>::impl(value));
			}
			for (auto& value: rawJsonData["r"].getArray()) {
				output.r.emplace_back(parse_raw_json_data<test_struct>::impl(value));
			}
			for (auto& value: rawJsonData["s"].getArray()) {
				output.s.emplace_back(parse_raw_json_data<test_struct>::impl(value));
			}
			for (auto& value: rawJsonData["t"].getArray()) {
				output.t.emplace_back(parse_raw_json_data<test_struct>::impl(value));
			}
			for (auto& value: rawJsonData["u"].getArray()) {
				output.u.emplace_back(parse_raw_json_data<test_struct>::impl(value));
			}
			for (auto& value: rawJsonData["v"].getArray()) {
				output.v.emplace_back(parse_raw_json_data<test_struct>::impl(value));
			}
			for (auto& value: rawJsonData["w"].getArray()) {
				output.w.emplace_back(parse_raw_json_data<test_struct>::impl(value));
			}
			for (auto& value: rawJsonData["x"].getArray()) {
				output.x.emplace_back(parse_raw_json_data<test_struct>::impl(value));
			}
			for (auto& value: rawJsonData["y"].getArray()) {
				output.y.emplace_back(parse_raw_json_data<test_struct>::impl(value));
			}
			for (auto& value: rawJsonData["z"].getArray()) {
				output.z.emplace_back(parse_raw_json_data<test_struct>::impl(value));
			}
			return output;
		}
	};

	template<typename test_data_type, bool minified, size_t iterations, const bnch_swt::string_literal testNameNew>
	struct json_test_helper<json_library::jsonifier, test_type::parse_and_serialize_raw_json_data, test_data_type, minified, iterations, testNameNew> {
		JSONIFIER_CLANG_INLINE static auto run(std::vector<std::string>& testDataNew) {
			static constexpr bnch_swt::string_literal testName{ testNameNew };
			static constexpr bnch_swt::string_literal testNameRead{ testName + "-Read" };
			static constexpr bnch_swt::string_literal testNameWrite{ testName + "-Write" };
			static constexpr bool partialRead{ std::is_same_v<test_data_type, partial_test<test_struct>> || std::is_same_v<test_data_type, twitter_partial_message> };
			static constexpr bool knownOrder{ !std::is_same_v<test_data_type, twitter_partial_message> && !std::is_same_v<test_data_type, abc_test<abc_test_struct>> };
			results_data r{ jsonifierLibraryName, testName, jsonifierCommitUrl };
			jsonifier::jsonifier_core parser{};
			std::vector<jsonifier::raw_json_data> testDatas{ iterations + 1 };
			std::vector<test_data_type> testDatasFinal{ iterations + 1 };
			size_t currentIndex{};
			bnch_swt::performance_metrics readResult =
				bnch_swt::benchmark_stage<testNameRead, iterations, measuredIterations>::template runBenchmark<jsonifierLibraryName, "teal">([&]() mutable {
					parser.parseJson<jsonifier::parse_options{ .partialRead = partialRead, .knownOrder = knownOrder, .minified = minified }>(testDatas[currentIndex],
						testDataNew[currentIndex]);
					testDatasFinal[currentIndex] = parse_raw_json_data<test_data_type>::impl(testDatas[currentIndex]);
					bnch_swt::doNotOptimizeAway(testDatas[currentIndex]);
					auto newSize = testDataNew[currentIndex].size();
					++currentIndex;
					return newSize;
				});
			for (auto& value: parser.getErrors()) {
				std::cout << "Jsonifier Error: " << value << std::endl;
			}
			for (size_t x = 0; x < iterations; ++x) {
				testDataNew[x] = std::string{};
			}
			currentIndex = 0;
			bnch_swt::performance_metrics writeResult =
				bnch_swt::benchmark_stage<testNameWrite, iterations, measuredIterations>::template runBenchmark<jsonifierLibraryName, "steelblue">([&]() mutable {
					parser.serializeJson<jsonifier::serialize_options{ .prettify = !minified }>(testDatasFinal[currentIndex], testDataNew[currentIndex]);
					bnch_swt::doNotOptimizeAway(testDataNew[currentIndex]);
					auto newSize = testDataNew[currentIndex].size();
					++currentIndex;
					return newSize;
				});
			r.readResult  = result<result_type::read>{ "teal", readResult };
			r.writeResult = result<result_type::write>{ "steelblue", writeResult };
			bnch_swt::file_loader::saveFile(static_cast<std::string>(testDataNew[0]),
				jsonOutPath.operator std::string() + "/" + testName.operator std::string() + "-jsonifier.json");
			return r;
		}
	};

	template<typename test_data_type, bool minified, size_t iterations, const bnch_swt::string_literal testNameNew>
	struct json_test_helper<json_library::jsonifier, test_type::parse_and_serialize, test_data_type, minified, iterations, testNameNew> {
		JSONIFIER_CLANG_INLINE static auto run(std::vector<std::string>& testDataNew) {
			static constexpr bnch_swt::string_literal testName{ testNameNew };
			static constexpr bnch_swt::string_literal testNameRead{ testName + "-Read" };
			static constexpr bnch_swt::string_literal testNameWrite{ testName + "-Write" };
			static constexpr bool partialRead{ std::is_same_v<test_data_type, partial_test<test_struct>> || std::is_same_v<test_data_type, twitter_partial_message> };
			static constexpr bool knownOrder{ !std::is_same_v<test_data_type, twitter_partial_message> && !std::is_same_v<test_data_type, abc_test<abc_test_struct>> };
			results_data r{ jsonifierLibraryName, testName, jsonifierCommitUrl };
			jsonifier::jsonifier_core parser{};
			std::vector<test_data_type> testDatas{ iterations + 1 };
			size_t currentIndex{};
			bnch_swt::performance_metrics readResult =
				bnch_swt::benchmark_stage<testNameRead, iterations, measuredIterations>::template runBenchmark<jsonifierLibraryName, "teal">([&]() mutable {
					parser.parseJson<jsonifier::parse_options{ .partialRead = partialRead, .knownOrder = knownOrder, .minified = minified }>(testDatas[currentIndex],
						testDataNew[currentIndex]);
					bnch_swt::doNotOptimizeAway(testDatas[currentIndex]);
					auto newSize = testDataNew[currentIndex].size();
					++currentIndex;
					return newSize;
				});
			currentIndex = 0;
			for (auto& value: parser.getErrors()) {
				std::cout << "Jsonifier Error: " << value << std::endl;
			}
			for (size_t x = 0; x < iterations; ++x) {
				testDataNew[x] = std::string{};
			}
			bnch_swt::performance_metrics writeResult =
				bnch_swt::benchmark_stage<testNameWrite, iterations, measuredIterations>::template runBenchmark<jsonifierLibraryName, "steelblue">([&]() mutable {
					parser.serializeJson<jsonifier::serialize_options{ .prettify = !minified }>(testDatas[currentIndex], testDataNew[currentIndex]);
					bnch_swt::doNotOptimizeAway(testDataNew[currentIndex]);
					auto newSize = testDataNew[currentIndex].size();
					++currentIndex;
					return newSize;
				});
			r.readResult  = result<result_type::read>{ "teal", readResult };
			r.writeResult = result<result_type::write>{ "steelblue", writeResult };
			bnch_swt::file_loader::saveFile(static_cast<std::string>(testDataNew[0]),
				jsonOutPath.operator std::string() + "/" + testName.operator std::string() + "-jsonifier.json");
			return r;
		}
	};

	template<size_t iterations, bnch_swt::string_literal testNameNew>
	struct json_test_helper<json_library::jsonifier, test_type::prettify, std::string, false, iterations, testNameNew> {
		JSONIFIER_CLANG_INLINE static auto run(std::string& newBuffer) {
			static constexpr bnch_swt::string_literal testName{ testNameNew };
			results_data r{ jsonifierLibraryName, testName, jsonifierCommitUrl };
			jsonifier::jsonifier_core parser{};
			std::vector<std::string> testDataNew{ iterations + 1 };
			size_t currentIndex{};
			auto writeResult = bnch_swt::benchmark_stage<testName, iterations, measuredIterations>::template runBenchmark<jsonifierLibraryName, "steelblue">([&]() mutable {
				parser.prettifyJson(newBuffer, testDataNew[currentIndex]);
				bnch_swt::doNotOptimizeAway(testDataNew[currentIndex]);
				auto newSize = testDataNew[currentIndex].size();
				++currentIndex;
				return newSize;
			});

			for (auto& value: parser.getErrors()) {
				std::cout << "Jsonifier Error: " << value << std::endl;
			}
			bnch_swt::file_loader::saveFile(testDataNew[0], jsonOutPath.operator std::string() + "/" + testName.operator std::string() + "-jsonifier.json");
			r.writeResult = result<result_type::write>{ "steelblue", writeResult };
			return r;
		}
	};

	template<size_t iterations, bnch_swt::string_literal testNameNew>
	struct json_test_helper<json_library::jsonifier, test_type::minify, std::string, false, iterations, testNameNew> {
		JSONIFIER_CLANG_INLINE static auto run(std::string& newBuffer) {
			static constexpr bnch_swt::string_literal testName{ testNameNew };
			std::vector<std::string> testDataNew{ iterations + 1 };
			results_data r{ jsonifierLibraryName, testName, jsonifierCommitUrl };
			jsonifier::jsonifier_core parser{};
			size_t currentIndex{};
			auto writeResult = bnch_swt::benchmark_stage<testName, iterations, measuredIterations>::template runBenchmark<jsonifierLibraryName, "steelblue">([&]() mutable {
				parser.minifyJson(newBuffer, testDataNew[currentIndex]);
				bnch_swt::doNotOptimizeAway(testDataNew[currentIndex]);
				auto newSize = testDataNew[currentIndex].size();
				++currentIndex;
				return newSize;
			});
			for (auto& value: parser.getErrors()) {
				std::cout << "Jsonifier Error: " << value << std::endl;
			}
			bnch_swt::file_loader::saveFile(testDataNew[0], jsonOutPath.operator std::string() + "/" + testName.operator std::string() + "-jsonifier.json");
			r.writeResult = result<result_type::write>{ "steelblue", writeResult };
			return r;
		}
	};

	template<size_t iterations, bnch_swt::string_literal testNameNew>
	struct json_test_helper<json_library::jsonifier, test_type::validate, std::string, false, iterations, testNameNew> {
		JSONIFIER_CLANG_INLINE static auto run(std::string& newBuffer) {
			static constexpr bnch_swt::string_literal testName{ testNameNew };
			results_data r{ jsonifierLibraryName, testName, jsonifierCommitUrl };
			jsonifier::jsonifier_core parser{};
			bnch_swt::performance_metrics readResult =
				bnch_swt::benchmark_stage<testName, iterations, measuredIterations>::template runBenchmark<jsonifierLibraryName, "teal">([&]() mutable {
					if (auto result = parser.validateJson(newBuffer); !result) {
						bnch_swt::doNotOptimizeAway(result);
						return uint64_t{};
					}
					return static_cast<uint64_t>(newBuffer.size());
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
		JSONIFIER_CLANG_INLINE static auto run(std::vector<std::string>& testDataNew) {
			static constexpr bnch_swt::string_literal testName{ testNameNew };
			static constexpr bnch_swt::string_literal testNameRead{ testName + "-Read" };
			static constexpr bnch_swt::string_literal testNameWrite{ testName + "-Write" };
			static constexpr bool partialRead{ std::is_same_v<test_data_type, partial_test<test_struct>> || std::is_same_v<test_data_type, twitter_partial_message> };
			results_data r{ glazeLibraryName, testName, glazeCommitUrl };
			std::vector<test_data_type> testDatas{ iterations + 1 };
			size_t currentIndex{};
			bnch_swt::performance_metrics readResult = bnch_swt::benchmark_stage<testNameRead, iterations, measuredIterations>::template runBenchmark<glazeLibraryName,
				"dodgerblue">([&]() mutable {
				if (auto error = glz::read<
						glz::opts{ .error_on_unknown_keys = !partialRead, .skip_null_members = false, .prettify = !minified, .minified = minified, .partial_read = partialRead }>(
						testDatas[currentIndex], testDataNew[currentIndex]);
					error) {
					std::cout << "Glaze Error: " << glz::format_error(error, testDataNew[currentIndex]) << std::endl;
				}
				bnch_swt::doNotOptimizeAway(testDatas[currentIndex]);
				auto newSize = testDataNew[currentIndex].size();
				++currentIndex;
				return newSize;
			});
			for (size_t x = 0; x < iterations; ++x) {
				testDataNew[x] = std::string{};
			}
			currentIndex = 0;
			bnch_swt::performance_metrics writeResult =
				bnch_swt::benchmark_stage<testNameWrite, iterations, measuredIterations>::template runBenchmark<glazeLibraryName, "skyblue">([&]() mutable {
					auto newResult =
						glz::write<glz::opts{ .skip_null_members = false, .prettify = !minified, .minified = minified }>(testDatas[currentIndex], testDataNew[currentIndex]);
					bnch_swt::doNotOptimizeAway(testDataNew[currentIndex]);
					( void )newResult;
					auto newSize = testDataNew[currentIndex].size();
					++currentIndex;
					return newSize;
				});
			r.readResult  = result<result_type::read>{ "dodgerblue", readResult };
			r.writeResult = result<result_type::write>{ "skyblue", writeResult };
			bnch_swt::file_loader::saveFile(static_cast<std::string>(testDataNew[0]), jsonOutPath.operator std::string() + "/" + testName.operator std::string() + "-glaze.json");
			return r;
		}
	};

	template<size_t iterations, bnch_swt::string_literal testNameNew>
	struct json_test_helper<json_library::glaze, test_type::prettify, std::string, false, iterations, testNameNew> {
		JSONIFIER_CLANG_INLINE static auto run(std::string& newBuffer) {
			static constexpr bnch_swt::string_literal testName{ testNameNew };
			std::vector<std::string> testDataNew{ iterations + 1 };
			results_data r{ glazeLibraryName, testName, glazeCommitUrl };
			size_t currentIndex{};

			auto writeResult = bnch_swt::benchmark_stage<testName, iterations, measuredIterations>::template runBenchmark<glazeLibraryName, "skyblue">([&]() mutable {
				glz::prettify_json(newBuffer, testDataNew[currentIndex]);
				bnch_swt::doNotOptimizeAway(testDataNew[currentIndex]);
				auto newSize = testDataNew[currentIndex].size();
				++currentIndex;
				return newSize;
			});

			bnch_swt::file_loader::saveFile(testDataNew[0], jsonOutPath.operator std::string() + "/" + testName.operator std::string() + "-glaze.json");
			r.writeResult = result<result_type::write>{ "skyblue", writeResult };

			return r;
		}
	};

	template<size_t iterations, bnch_swt::string_literal testNameNew> struct json_test_helper<json_library::glaze, test_type::minify, std::string, false, iterations, testNameNew> {
		JSONIFIER_CLANG_INLINE static auto run(std::string& newBuffer) {
			static constexpr bnch_swt::string_literal testName{ testNameNew };

			std::string newestBuffer{ newBuffer };
			std::vector<std::string> testDataNew{ iterations + 1 };
			size_t currentIndex{};
			results_data r{ glazeLibraryName, testName, glazeCommitUrl };
			auto writeResult = bnch_swt::benchmark_stage<testName, iterations, measuredIterations>::template runBenchmark<glazeLibraryName, "skyblue">([&]() mutable {
				glz::minify_json(newestBuffer, testDataNew[currentIndex]);
				bnch_swt::doNotOptimizeAway(testDataNew[currentIndex]);
				auto newSize = testDataNew[currentIndex].size();
				++currentIndex;
				return newSize;
			});

			bnch_swt::file_loader::saveFile(testDataNew[0], jsonOutPath.operator std::string() + "/" + testName.operator std::string() + "-glaze.json");
			r.writeResult = result<result_type::write>{ "skyblue", writeResult };

			return r;
		}
	};

	template<size_t iterations, bnch_swt::string_literal testNameNew>
	struct json_test_helper<json_library::glaze, test_type::validate, std::string, false, iterations, testNameNew> {
		JSONIFIER_CLANG_INLINE static auto run(std::string& newBuffer) {
			static constexpr bnch_swt::string_literal testName{ testNameNew };
			results_data r{ glazeLibraryName, testName, glazeCommitUrl };
			bnch_swt::performance_metrics readResult =
				bnch_swt::benchmark_stage<testName, iterations, measuredIterations>::template runBenchmark<glazeLibraryName, "dodgerblue">([&]() mutable {
					if (auto result = glz::validate_json(newBuffer); result) {
						bnch_swt::doNotOptimizeAway(result);
						return size_t{};
					}
					return newBuffer.size();
				});

			bnch_swt::file_loader::saveFile(newBuffer, jsonOutPath.operator std::string() + "/" + testName.operator std::string() + "-glaze.json");
			r.readResult = result<result_type::read>{ "dodgerblue", readResult };

			return r;
		}
	};

	template<typename test_data_type, bool minified, size_t iterations, const bnch_swt::string_literal testNameNew>
	struct json_test_helper<json_library::simdjson, test_type::parse_and_serialize, test_data_type, minified, iterations, testNameNew> {
		JSONIFIER_CLANG_INLINE static auto run(std::vector<std::string>& testDataNew) {
			static constexpr bnch_swt::string_literal testName{ testNameNew };
			static constexpr bnch_swt::string_literal testNameRead{ testName + "-Read" };
			static constexpr bnch_swt::string_literal testNameWrite{ testName + "-Write" };
			results_data r{ simdjsonLibraryName, testName, simdjsonCommitUrl };
			jsonifier::jsonifier_core parserNew{};
			simdjson::ondemand::parser parser{};
			std::vector<test_data_type> testDatas{ iterations + 1 };
			size_t currentIndex{};
			bnch_swt::performance_metrics readResult =
				bnch_swt::benchmark_stage<testNameRead, iterations, measuredIterations>::template runBenchmark<simdjsonLibraryName, "cadetblue">([&]() mutable {
					try {
						getValue(testDatas[currentIndex], parser.iterate(testDataNew[currentIndex]).value());
						bnch_swt::doNotOptimizeAway(testDatas[currentIndex]);
						auto newSize = testDataNew[currentIndex].size();
						++currentIndex;
						return newSize;

					} catch (const std::exception& error) {
						std::cout << "Simdjson Error: " << error.what() << std::endl;
						std::cout << "Index: " << currentIndex << std::endl;
						++currentIndex;
					}
					return testDataNew[currentIndex].size();
				});
			r.readResult = result<result_type::read>{ "cadetblue", readResult };
			bnch_swt::file_loader::saveFile(static_cast<std::string>(testDataNew[0]),
				jsonOutPath.operator std::string() + "/" + testName.operator std::string() + "-simdjson.json");
			return r;
		}
	};

	template<size_t iterations, bnch_swt::string_literal testNameNew>
	struct json_test_helper<json_library::simdjson, test_type::minify, std::string, false, iterations, testNameNew> {
		static constexpr bnch_swt::string_literal testName{ testNameNew };
		JSONIFIER_CLANG_INLINE static auto run(std::string& newBuffer) {
			results_data r{ simdjsonLibraryName, testName, simdjsonCommitUrl };

			simdjson::dom::parser parser{};
			std::vector<std::string> testDataNew{ iterations + 1 };
			size_t currentIndex{};

			auto writeResult = bnch_swt::benchmark_stage<testName, iterations, measuredIterations>::template runBenchmark<simdjsonLibraryName, "cornflowerblue">([&]() mutable {
				try {
					testDataNew[currentIndex] = simdjson::minify(parser.parse(newBuffer));
					bnch_swt::doNotOptimizeAway(testDataNew[currentIndex]);
					auto newSize = testDataNew[currentIndex].size();
					++currentIndex;
					return newSize;
				} catch (std::exception& error) {
					std::cout << "Simdjson Error: " << error.what() << std::endl;
					return testDataNew[currentIndex].size();
				}
			});

			bnch_swt::file_loader::saveFile(testDataNew[0], jsonOutPath.operator std::string() + "/" + testName.operator std::string() + "-simdjson.json");
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
 > )" + jsonifier::toString(maxIterations) +
		R"( iterations on a ()" + getCPUInfo() + R"(), where the most stable 20 subsequent iterations are sampled.

#### Note:
These benchmarks were executed using the CPU benchmark library [BenchmarkSuite](https://github.com/RealTimeChris/BenchmarkSuite).)" };

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
		JSONIFIER_CLANG_INLINE static test_results run(std::vector<std::string>& jsonDataNew) {
			static constexpr bnch_swt::string_literal testName{ testNameNew };
			test_results jsonResults{};
			jsonResults.testName = testName.operator std::string();

#if !defined(ASAN_ENABLED)
			jsonResults.results.emplace_back(json_test_helper<json_library::simdjson, type, test_data_type, minified, iterations, testName>::run(jsonDataNew));
			if constexpr (!std::is_same_v<test_data_type, partial_test<test_struct>> && !std::is_same_v<test_data_type, twitter_partial_message>) {
				jsonResults.results.emplace_back(json_test_helper<json_library::glaze, type, test_data_type, minified, iterations, testName>::run(jsonDataNew));
			}
#endif
			jsonResults.results.emplace_back(json_test_helper<json_library::jsonifier, type, test_data_type, minified, iterations, testName>::run(jsonDataNew));
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
			bnch_swt::benchmark_stage<testNameNew + "-Read", iterations, measuredIterations>::printResults();
			bnch_swt::benchmark_stage<testNameNew + "-Write", iterations, measuredIterations>::printResults();
			return jsonResults;
		}
	};

	template<typename test_data_type, bool minified, uint64_t iterations, bnch_swt::string_literal testNameNew>
	struct json_tests_helper<test_type::parse_and_serialize_raw_json_data, test_data_type, minified, iterations, testNameNew> {
		JSONIFIER_CLANG_INLINE static test_results run(std::vector<std::string>& jsonDataNew) {
			static constexpr bnch_swt::string_literal testName{ testNameNew };
			test_results jsonResults{};
			jsonResults.testName = testName.operator std::string();

#if !defined(ASAN_ENABLED)
			jsonResults.results.emplace_back(
				json_test_helper<json_library::simdjson, test_type::parse_and_serialize, test_data_type, minified, iterations, testName>::run(jsonDataNew));
			if constexpr (!std::is_same_v<test_data_type, partial_test<test_struct>> && !std::is_same_v<test_data_type, twitter_partial_message>) {
				jsonResults.results.emplace_back(
					json_test_helper<json_library::glaze, test_type::parse_and_serialize, test_data_type, minified, iterations, testName>::run(jsonDataNew));
			}
#endif
			jsonResults.results.emplace_back(
				json_test_helper<json_library::jsonifier, test_type::parse_and_serialize_raw_json_data, test_data_type, minified, iterations, testName>::run(jsonDataNew));

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
			bnch_swt::benchmark_stage<testNameNew + "-Read", iterations, measuredIterations>::printResults();
			bnch_swt::benchmark_stage<testNameNew + "-Write", iterations, measuredIterations>::printResults();
			return jsonResults;
		}
	};

	template<uint64_t iterations, bnch_swt::string_literal testNameNew> struct json_tests_helper<test_type::prettify, std::string, false, iterations, testNameNew> {
		JSONIFIER_CLANG_INLINE static test_results run(std::string& jsonDataNew) {
			static constexpr bnch_swt::string_literal testName{ testNameNew };
			test_results jsonResults{};
			jsonResults.testName = testName.operator std::string();
#if !defined(ASAN_ENABLED)
			jsonResults.results.emplace_back(json_test_helper<json_library::glaze, test_type::prettify, std::string, false, iterations, testName>::run(jsonDataNew));
#endif
			jsonResults.results.emplace_back(json_test_helper<json_library::jsonifier, test_type::prettify, std::string, false, iterations, testName>::run(jsonDataNew));
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
			bnch_swt::benchmark_stage<testNameNew, iterations, measuredIterations>::printResults();
			return jsonResults;
		}
	};

	template<uint64_t iterations, bnch_swt::string_literal testNameNew> struct json_tests_helper<test_type::minify, std::string, false, iterations, testNameNew> {
		JSONIFIER_CLANG_INLINE static test_results run(std::string& jsonDataNew) {
			static constexpr bnch_swt::string_literal testName{ testNameNew };
			test_results jsonResults{};
			jsonResults.testName = testName.operator std::string();
#if !defined(ASAN_ENABLED)
			jsonResults.results.emplace_back(json_test_helper<json_library::simdjson, test_type::minify, std::string, false, iterations, testName>::run(jsonDataNew));
			jsonResults.results.emplace_back(json_test_helper<json_library::glaze, test_type::minify, std::string, false, iterations, testName>::run(jsonDataNew));
#endif
			jsonResults.results.emplace_back(json_test_helper<json_library::jsonifier, test_type::minify, std::string, false, iterations, testName>::run(jsonDataNew));
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
			bnch_swt::benchmark_stage<testNameNew, iterations, measuredIterations>::printResults();
			return jsonResults;
		}
	};

	template<uint64_t iterations, bnch_swt::string_literal testNameNew> struct json_tests_helper<test_type::validate, std::string, false, iterations, testNameNew> {
		JSONIFIER_CLANG_INLINE static test_results run(std::string& jsonDataNew) {
			static constexpr bnch_swt::string_literal testName{ testNameNew };
			test_results jsonResults{};
			jsonResults.testName = testName.operator std::string();
#if !defined(ASAN_ENABLED)
			jsonResults.results.emplace_back(json_test_helper<json_library::glaze, test_type::validate, std::string, false, iterations, testName>::run(jsonDataNew));
#endif
			jsonResults.results.emplace_back(json_test_helper<json_library::jsonifier, test_type::validate, std::string, false, iterations, testName>::run(jsonDataNew));
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
			bnch_swt::benchmark_stage<testNameNew, iterations, measuredIterations>::printResults();
			return jsonResults;
		}
	};

	JSONIFIER_CLANG_INLINE void testFunction() {
		std::vector<test<test_struct>> testJsonData{ [] {
			std::vector<test<test_struct>> returnValues{};
			for (size_t x = 0; x < maxIterations; ++x) {
				returnValues.emplace_back(test_generator::generateTest());
			}
			return returnValues;
		}() };
		std::vector<std::string> jsonData{ maxIterations };
		jsonifier::jsonifier_core parser{};
		for (size_t x = 0; x < maxIterations; ++x) {
			parser.serializeJson<jsonifier::serialize_options{ .prettify = true }>(testJsonData[x], jsonData[x]);
		}
		
		bnch_swt::file_loader::saveFile(jsonData[0], jsonOutPath.operator std::string() + "/Json Test (Prettified).json");
		bnch_swt::file_loader::saveFile(jsonData[0], jsonOutPath.operator std::string() + "/Abc (Out of Order) Test (Prettified).json");
		bnch_swt::file_loader::saveFile(jsonData[0], jsonOutPath.operator std::string() + "/Partial Test (Prettified).json");
		std::vector<std::string> jsonMinifiedData{ [&] {
			std::vector<std::string> returnValues{};
			for (size_t x = 0; x < maxIterations; ++x) {
				returnValues.emplace_back(parser.minifyJson(jsonData[x]));
			}
			return returnValues;
		} () };
		bnch_swt::file_loader::saveFile(jsonMinifiedData[0], jsonOutPath.operator std::string() + "/Json Test (Minified).json");
		bnch_swt::file_loader::saveFile(jsonMinifiedData[0], jsonOutPath.operator std::string() + "/Abc (Out of Order) Test (Minified).json");
		bnch_swt::file_loader::saveFile(jsonMinifiedData[0], jsonOutPath.operator std::string() + "/Partial Test (Minified).json");
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
		test_results testResults{
			json_tests_helper<test_type::parse_and_serialize_raw_json_data, test<test_struct>, false, maxIterations, "Json Test (Raw-Json-Data) (Prettified)">::run(jsonData)
		};
		newerString += testResults.markdownResults;
		benchmark_data.emplace_back(testResults);
		testResults =
			json_tests_helper<test_type::parse_and_serialize_raw_json_data, test<test_struct>, true, maxIterations, "Json Test (Raw-Json-Data) (Minified)">::run(jsonMinifiedData);
		newerString += testResults.markdownResults;
		benchmark_data.emplace_back(testResults);
		testResults = json_tests_helper<test_type::parse_and_serialize, test<test_struct>, false, maxIterations, "Json Test (Prettified)">::run(jsonData);
		newerString += testResults.markdownResults;
		benchmark_data.emplace_back(testResults);
		testResults = json_tests_helper<test_type::parse_and_serialize, test<test_struct>, true, maxIterations, "Json Test (Minified)">::run(jsonMinifiedData);
		newerString += testResults.markdownResults;
		benchmark_data.emplace_back(testResults);
		testResults = json_tests_helper<test_type::parse_and_serialize, partial_test<test_struct>, false, maxIterations, "Partial Test (Prettified)">::run(jsonData);
		newerString += testResults.markdownResults;
		benchmark_data.emplace_back(testResults);
		testResults = json_tests_helper<test_type::parse_and_serialize, partial_test<test_struct>, true, maxIterations, "Partial Test (Minified)">::run(jsonMinifiedData);
		newerString += testResults.markdownResults;
		benchmark_data.emplace_back(testResults);
		testResults = json_tests_helper<test_type::parse_and_serialize, abc_test<abc_test_struct>, false, maxIterations, "Abc (Out of Order) Test (Prettified)">::run(jsonData);
		newerString += testResults.markdownResults;
		benchmark_data.emplace_back(testResults);
		testResults =
			json_tests_helper<test_type::parse_and_serialize, abc_test<abc_test_struct>, true, maxIterations, "Abc (Out of Order) Test (Minified)">::run(jsonMinifiedData);
		newerString += testResults.markdownResults;
		benchmark_data.emplace_back(testResults);
		{
			std::vector<std::string> discordDataNew{ maxIterations };
			for (size_t x = 0; x < maxIterations; ++x) {
				discordDataNew[x] = discordData;
			}
			testResults = json_tests_helper<test_type::parse_and_serialize, discord_message, false, maxIterations, "Discord Test (Prettified)">::run(discordDataNew);
			newerString += testResults.markdownResults;
			benchmark_data.emplace_back(testResults);
			std::vector<std::string> discordMinifiedDataNew{ maxIterations };
			for (size_t x = 0; x < maxIterations; ++x) {
				parser.minifyJson(discordData, discordMinifiedDataNew[x]);
			}
			testResults = json_tests_helper<test_type::parse_and_serialize, discord_message, true, maxIterations, "Discord Test (Minified)">::run(discordMinifiedDataNew);
			newerString += testResults.markdownResults;
			benchmark_data.emplace_back(testResults);
		}
		{
			std::vector<std::string> canadaDataNew{ maxIterations };
			for (size_t x = 0; x < maxIterations; ++x) {
				canadaDataNew[x] = canadaData;
			}
			testResults = json_tests_helper<test_type::parse_and_serialize, canada_message, false, maxIterations, "Canada Test (Prettified)">::run(canadaDataNew);
			newerString += testResults.markdownResults;
			benchmark_data.emplace_back(testResults);
			std::vector<std::string> canadaMinifiedDataNew{ maxIterations };
			for (size_t x = 0; x < maxIterations; ++x) {
				parser.minifyJson(canadaData, canadaMinifiedDataNew[x]);
			}
			testResults = json_tests_helper<test_type::parse_and_serialize, canada_message, true, maxIterations, "Canada Test (Minified)">::run(canadaMinifiedDataNew);
			newerString += testResults.markdownResults;
			benchmark_data.emplace_back(testResults);
		}
		{
			std::vector<std::string> citmCatalogDataNew{ maxIterations };
			for (size_t x = 0; x < maxIterations; ++x) {
				citmCatalogDataNew[x] = citmCatalogData;
			}
			testResults = json_tests_helper<test_type::parse_and_serialize, citm_catalog_message, false, maxIterations, "Citm Catalog Test (Prettified)">::run(citmCatalogDataNew);
			newerString += testResults.markdownResults;
			benchmark_data.emplace_back(testResults);
			std::vector<std::string> citmCatalogMinifiedDataNew{ maxIterations };
			for (size_t x = 0; x < maxIterations; ++x) {
				parser.minifyJson(citmCatalogData, citmCatalogMinifiedDataNew[x]);
			}
			testResults =
				json_tests_helper<test_type::parse_and_serialize, citm_catalog_message, true, maxIterations, "Citm Catalog Test (Minified)">::run(citmCatalogMinifiedDataNew);
			newerString += testResults.markdownResults;
			benchmark_data.emplace_back(testResults);
		}
		{
			std::vector<std::string> twitterPartialDataNew{ maxIterations };
			for (size_t x = 0; x < maxIterations; ++x) {
				twitterPartialDataNew[x] = twitterData;
			}
			testResults = json_tests_helper<test_type::parse_and_serialize, twitter_partial_message, false, maxIterations, "Twitter Partial Test (Prettified)">::run(twitterPartialDataNew);
			newerString += testResults.markdownResults;
			benchmark_data.emplace_back(testResults);
			std::vector<std::string> twitterPartialMinifiedDataNew{ maxIterations };
			for (size_t x = 0; x < maxIterations; ++x) {
				parser.minifyJson(twitterData, twitterPartialMinifiedDataNew[x]);
			}
			testResults = json_tests_helper<test_type::parse_and_serialize, twitter_partial_message, true, maxIterations, "Twitter Partial Test (Minified)">::run(
				twitterPartialMinifiedDataNew);
			newerString += testResults.markdownResults;
			benchmark_data.emplace_back(testResults);
		}
		{
			std::vector<std::string> twitterDataNew{ maxIterations };
			for (size_t x = 0; x < maxIterations; ++x) {
				twitterDataNew[x] = twitterData;
			}
			testResults = json_tests_helper<test_type::parse_and_serialize, twitter_message, false, maxIterations, "Twitter Test (Prettified)">::run(twitterDataNew);
			newerString += testResults.markdownResults;
			benchmark_data.emplace_back(testResults);
			std::vector<std::string> twitterMinifiedDataNew{ maxIterations };
			for (size_t x = 0; x < maxIterations; ++x) {
				parser.minifyJson(twitterData, twitterMinifiedDataNew[x]);
			}
			testResults = json_tests_helper<test_type::parse_and_serialize, twitter_message, true, maxIterations, "Twitter Test (Minified)">::run(twitterMinifiedDataNew);
			newerString += testResults.markdownResults;
			benchmark_data.emplace_back(testResults);
		}
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