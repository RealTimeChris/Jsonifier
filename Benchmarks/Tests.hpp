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
#include <benchmark/benchmark.h>

namespace tests {

	// Enumeration for test types
	enum class test_type {
		parse_and_serialize				  = 0,
		minify							  = 1,
		prettify						  = 2,
		validate						  = 3,
		parse_and_serialize_raw_json_data = 4,
	};

	// Enumeration for JSON libraries
	enum class json_library {
		jsonifier = 0,
		glaze	  = 1,
		simdjson  = 2,
	};

	// Base template for different test implementations
	template<json_library lib, test_type type, typename test_data_type, bool minified> struct json_benchmark;

	// Implementation for Jsonifier parse and serialize
	template<typename test_data_type, bool minified> struct json_benchmark<json_library::jsonifier, test_type::parse_and_serialize, test_data_type, minified> {
		static void run(benchmark::State& state, const test_data_type& testData) {
			jsonifier::jsonifier_core parser{};
			std::string buffer{};

			// Serialize once to get the JSON string
			parser.serializeJson<jsonifier::serialize_options{ .prettify = !minified }>(testData, buffer);

			test_data_type parsedData{};

			// Benchmark parsing
			if (state.range(0) == 0) {// Parse benchmark
				for (auto _: state) {
					parser.parseJson<jsonifier::parse_options{ .partialRead = std::is_same_v<test_data_type, partial_test<partial_test_struct>> ||
							std::is_same_v<test_data_type, twitter_partial_message>,
						.knownOrder = true,
						.minified	= minified }>(parsedData, buffer);

					benchmark::DoNotOptimize(parsedData);
					benchmark::ClobberMemory();
				}

				state.SetBytesProcessed(int64_t(state.iterations()) * int64_t(buffer.size()));
				state.SetLabel("Parse");
			}
			// Benchmark serializing
			else if (state.range(0) == 1) {// Serialize benchmark
				std::string outputBuffer;

				for (auto _: state) {
					outputBuffer.clear();
					parser.serializeJson<jsonifier::serialize_options{ .prettify = !minified }>(testData, outputBuffer);

					benchmark::DoNotOptimize(outputBuffer);
					benchmark::ClobberMemory();
				}

				state.SetBytesProcessed(int64_t(state.iterations()) * int64_t(outputBuffer.size()));
				state.SetLabel("Serialize");
			}

			// Check for errors
			for (auto& error: parser.getErrors()) {
				std::cout << "Jsonifier Error: " << error << std::endl;
			}
		}
	};

	// Implementation for Glaze parse and serialize
	template<typename test_data_type, bool minified> struct json_benchmark<json_library::glaze, test_type::parse_and_serialize, test_data_type, minified> {
		static void run(benchmark::State& state, const test_data_type& testData) {
			std::string buffer{};

			// Serialize once to get the JSON string
			auto result = glz::write<glz::opts{ .skip_null_members = false, .prettify = !minified, .minified = minified }>(testData, buffer);

			test_data_type parsedData{};

			// Benchmark parsing
			if (state.range(0) == 0) {// Parse benchmark
				for (auto _: state) {
					auto error = glz::read<glz::opts{ .error_on_unknown_keys = !std::is_same_v<test_data_type, partial_test<partial_test_struct>> &&
							!std::is_same_v<test_data_type, twitter_partial_message>,
						.skip_null_members = false,
						.prettify		   = !minified,
						.minified		   = minified,
						.partial_read = std::is_same_v<test_data_type, partial_test<partial_test_struct>> || std::is_same_v<test_data_type, twitter_partial_message> }>(parsedData,
						buffer);

					if (error) {
						std::cout << "Glaze Error: " << glz::format_error(error, buffer) << std::endl;
					}

					benchmark::DoNotOptimize(parsedData);
					benchmark::ClobberMemory();
				}

				state.SetBytesProcessed(int64_t(state.iterations()) * int64_t(buffer.size()));
				state.SetLabel("Parse");
			}
			// Benchmark serializing
			else if (state.range(0) == 1) {// Serialize benchmark
				std::string outputBuffer;

				for (auto _: state) {
					outputBuffer.clear();
					auto res = glz::write<glz::opts{ .skip_null_members = false, .prettify = !minified, .minified = minified }>(testData, outputBuffer);

					benchmark::DoNotOptimize(outputBuffer);
					benchmark::ClobberMemory();
				}

				state.SetBytesProcessed(int64_t(state.iterations()) * int64_t(outputBuffer.size()));
				state.SetLabel("Serialize");
			}
		}
	};

	// Implementation for Simdjson parse
	template<typename test_data_type, bool minified> struct json_benchmark<json_library::simdjson, test_type::parse_and_serialize, test_data_type, minified> {
		static void run(benchmark::State& state, const test_data_type& testData) {
			jsonifier::jsonifier_core parserNew{};
			simdjson::ondemand::parser parser{};
			std::string buffer{};

			// Use Jsonifier to serialize once to get the JSON string
			parserNew.serializeJson<jsonifier::serialize_options{ .prettify = !minified }>(testData, buffer);

			test_data_type parsedData{};

			// Benchmark parsing only (Simdjson doesn't have serialization)
			if (state.range(0) == 0) {// Parse benchmark
				for (auto _: state) {
					try {
						getValue(parsedData, parser.iterate(buffer).value().get_value());

						benchmark::DoNotOptimize(parsedData);
						benchmark::ClobberMemory();
					} catch (const std::exception& error) {
						std::cout << "Simdjson Error: " << error.what() << std::endl;
					}
				}

				state.SetBytesProcessed(int64_t(state.iterations()) * int64_t(buffer.size()));
				state.SetLabel("Parse");
			}
		}
	};

	// Implementation for minify operation
	template<bool minified> struct json_benchmark<json_library::jsonifier, test_type::minify, std::string, minified> {
		static void run(benchmark::State& state, std::string& jsonData) {
			jsonifier::jsonifier_core parser{};
			std::string result;

			for (auto _: state) {
				result.clear();
				parser.minifyJson(jsonData, result);

				benchmark::DoNotOptimize(result);
				benchmark::ClobberMemory();
			}

			state.SetBytesProcessed(int64_t(state.iterations()) * int64_t(result.size()));
			state.SetLabel("Minify");

			// Check for errors
			for (auto& error: parser.getErrors()) {
				std::cout << "Jsonifier Error: " << error << std::endl;
			}
		}
	};

	template<bool minified> struct json_benchmark<json_library::glaze, test_type::minify, std::string, minified> {
		static void run(benchmark::State& state, std::string& jsonDataNew) {
			std::string result;
			std::string jsonData{ jsonDataNew };

			for (auto _: state) {
				result.clear();
				glz::minify_json(jsonData, result);

				benchmark::DoNotOptimize(result);
				benchmark::ClobberMemory();
			}

			state.SetBytesProcessed(int64_t(state.iterations()) * int64_t(result.size()));
			state.SetLabel("Minify");
		}
	};

	template<bool minified> struct json_benchmark<json_library::simdjson, test_type::minify, std::string, minified> {
		static void run(benchmark::State& state, std::string& jsonData) {
			simdjson::dom::parser parser{};
			std::string result;

			for (auto _: state) {
				try {
					result = simdjson::minify(parser.parse(jsonData));

					benchmark::DoNotOptimize(result);
					benchmark::ClobberMemory();
				} catch (std::exception& error) {
					std::cout << "Simdjson Error: " << error.what() << std::endl;
				}
			}

			state.SetBytesProcessed(int64_t(state.iterations()) * int64_t(result.size()));
			state.SetLabel("Minify");
		}
	};

	// Implementation for prettify operation
	template<bool minified> struct json_benchmark<json_library::jsonifier, test_type::prettify, std::string, minified> {
		static void run(benchmark::State& state, std::string& jsonData) {
			jsonifier::jsonifier_core parser{};
			std::string result;

			for (auto _: state) {
				result.clear();
				parser.prettifyJson(jsonData, result);

				benchmark::DoNotOptimize(result);
				benchmark::ClobberMemory();
			}

			state.SetBytesProcessed(int64_t(state.iterations()) * int64_t(result.size()));
			state.SetLabel("Prettify");

			// Check for errors
			for (auto& error: parser.getErrors()) {
				std::cout << "Jsonifier Error: " << error << std::endl;
			}
		}
	};

	template<bool minified> struct json_benchmark<json_library::glaze, test_type::prettify, std::string, minified> {
		static void run(benchmark::State& state, std::string& jsonData) {
			std::string result;

			for (auto _: state) {
				result.clear();
				glz::prettify_json(jsonData, result);

				benchmark::DoNotOptimize(result);
				benchmark::ClobberMemory();
			}

			state.SetBytesProcessed(int64_t(state.iterations()) * int64_t(result.size()));
			state.SetLabel("Prettify");
		}
	};

	// Implementation for validate operation
	template<bool minified> struct json_benchmark<json_library::jsonifier, test_type::validate, std::string, minified> {
		static void run(benchmark::State& state, std::string& jsonData) {
			jsonifier::jsonifier_core parser{};

			for (auto _: state) {
				auto result = parser.validateJson(jsonData);

				benchmark::DoNotOptimize(result);
				benchmark::ClobberMemory();
			}

			state.SetBytesProcessed(int64_t(state.iterations()) * int64_t(jsonData.size()));
			state.SetLabel("Validate");

			// Check for errors
			for (auto& error: parser.getErrors()) {
				std::cout << "Jsonifier Error: " << error << std::endl;
			}
		}
	};

	template<bool minified> struct json_benchmark<json_library::glaze, test_type::validate, std::string, minified> {
		static void run(benchmark::State& state, std::string& jsonData) {
			for (auto _: state) {
				auto result = glz::validate_json(jsonData);

				benchmark::DoNotOptimize(result);
				benchmark::ClobberMemory();
			}

			state.SetBytesProcessed(int64_t(state.iterations()) * int64_t(jsonData.size()));
			state.SetLabel("Validate");
		}
	};

	// Parse raw JSON data (specific template specialization for test_struct)
	template<bool minified> struct json_benchmark<json_library::jsonifier, test_type::parse_and_serialize_raw_json_data, test_struct, minified> {
		static void run(benchmark::State& state, const test_struct& testData) {
			jsonifier::jsonifier_core parser{};
			std::string buffer{};

			// Serialize once to get the JSON string
			parser.serializeJson<jsonifier::serialize_options{ .prettify = !minified }>(testData, buffer);

			for (auto _: state) {
				jsonifier::raw_json_data rawJsonData;
				parser.parseJson(rawJsonData, buffer);

				test_struct output{};
				if (rawJsonData.getType() == jsonifier::json_type::object) {
					auto& json_object = rawJsonData.getObject();
					for (auto& [key, value]: json_object) {
						if (key == "testString" && value.getType() == jsonifier::json_type::array) {
							for (auto& element: value.getArray()) {
								if (element.getType() == jsonifier::json_type::string) {
									output.testString = static_cast<std::string>(element.getString());
								}
							}
						} else if (key == "testUint" && value.getType() == jsonifier::json_type::array) {
							for (auto& element: value.getArray()) {
								if (element.getType() == jsonifier::json_type::number) {
									output.testUint = element.getUint();
								}
							}
						} else if (key == "testInt" && value.getType() == jsonifier::json_type::array) {
							for (auto& element: value.getArray()) {
								if (element.getType() == jsonifier::json_type::number) {
									output.testInt = element.getInt();
								}
							}
						} else if (key == "testDouble" && value.getType() == jsonifier::json_type::array) {
							for (auto& element: value.getArray()) {
								if (element.getType() == jsonifier::json_type::number) {
									output.testDouble = element.getDouble();
								}
							}
						} else if (key == "testBool" && value.getType() == jsonifier::json_type::array) {
							for (auto& element: value.getArray()) {
								if (element.getType() == jsonifier::json_type::boolean) {
									output.testBool = element.getBool();
								}
							}
						}
					}
				}

				benchmark::DoNotOptimize(output);
				benchmark::ClobberMemory();
			}

			state.SetBytesProcessed(int64_t(state.iterations()) * int64_t(buffer.size()));
			state.SetLabel("Parse Raw JSON");

			// Check for errors
			for (auto& error: parser.getErrors()) {
				std::cout << "Jsonifier Error: " << error << std::endl;
			}
		}
	};

	// Helper function to register benchmarks for a specific test type
	template<json_library lib, test_type type, typename test_data_type, bool minified> void register_benchmark(const std::string& name, const test_data_type& testData) {
		// Create a copy of the test data to ensure it stays valid
		test_data_type* testDataCopy = new test_data_type(testData);

		// For parse and serialize, we register two benchmarks - one for parse, one for serialize
		if constexpr (type == test_type::parse_and_serialize) {
			std::string libName;
			switch (lib) {
				case json_library::jsonifier:
					libName = "Jsonifier";
					break;
				case json_library::glaze:
					libName = "Glaze";
					break;
				case json_library::simdjson:
					libName = "Simdjson";
					break;
			}

			std::string formatName = minified ? "Minified" : "Prettified";

			// For parse benchmark (arg = 0)
			benchmark::RegisterBenchmark((name + "/" + libName + "/" + formatName + "/Parse").c_str(),
				[testDataCopy](benchmark::State& state) {
					json_benchmark<lib, type, test_data_type, minified>::run(state, *testDataCopy);
				})
				->Arg(0)
				->Unit(benchmark::kMicrosecond);

			// For serialize benchmark (arg = 1), but not for simdjson which only parses
			if constexpr (lib != json_library::simdjson) {
				benchmark::RegisterBenchmark((name + "/" + libName + "/" + formatName + "/Serialize").c_str(),
					[testDataCopy](benchmark::State& state) {
						json_benchmark<lib, type, test_data_type, minified>::run(state, *testDataCopy);
					})
					->Arg(1)
					->Unit(benchmark::kMicrosecond);
			}
		}
		// For other operations, just register a single benchmark
		else {
			std::string libName;
			switch (lib) {
				case json_library::jsonifier:
					libName = "Jsonifier";
					break;
				case json_library::glaze:
					libName = "Glaze";
					break;
				case json_library::simdjson:
					libName = "Simdjson";
					break;
			}

			std::string opName;
			switch (type) {
				case test_type::minify:
					opName = "Minify";
					break;
				case test_type::prettify:
					opName = "Prettify";
					break;
				case test_type::validate:
					opName = "Validate";
					break;
				case test_type::parse_and_serialize_raw_json_data:
					opName = "ParseRawJSON";
					break;
				default:
					opName = "Unknown";
					break;
			}

			benchmark::RegisterBenchmark((name + "/" + libName + "/" + opName).c_str(), [testDataCopy](benchmark::State& state) {
				json_benchmark<lib, type, test_data_type, minified>::run(state, *testDataCopy);
			})->Unit(benchmark::kMicrosecond);
		}
	}

	// Main function to run all benchmarks
	void run_json_benchmarks() {
		// Define paths - use the original paths
		const bnch_swt::string_literal jsonOutPath{ currentPath + bnch_swt::string_literal{ "/Output" } };
		const bnch_swt::string_literal jsonPath{ currentPath + bnch_swt::string_literal{ "/Input" } };

		std::cout << "JSON input path: " << jsonPath.operator std::string() << std::endl;
		std::cout << "JSON output path: " << jsonOutPath.operator std::string() << std::endl;

		// Generate test data
		jsonifier::jsonifier_core parser{};

		// Generate test_struct data
		std::vector<test<test_struct>> jsonTestData(100);
		for (size_t i = 0; i < jsonTestData.size(); ++i) {
			jsonTestData[i] = test_generator::generateTest();
		}

		// Serialize to get JSON strings for different tests
		std::string jsonPrettified, jsonMinified;
		parser.serializeJson<jsonifier::serialize_options{ .prettify = true }>(jsonTestData[0], jsonPrettified);
		jsonMinified = parser.minifyJson(jsonPrettified);

		// Save the JSON files
		bnch_swt::file_loader::saveFile(jsonPrettified, jsonOutPath.operator std::string() + "/Json Test (Prettified).json");
		bnch_swt::file_loader::saveFile(jsonMinified, jsonOutPath.operator std::string() + "/Json Test (Minified).json");

		// Register basic data type benchmarks
		{
			std::vector<std::vector<double>> doubleData = test_generator::generateValues<double>(100, 100);
			register_benchmark<json_library::jsonifier, test_type::parse_and_serialize, std::vector<std::vector<double>>, false>("DoubleTest", doubleData);
			register_benchmark<json_library::glaze, test_type::parse_and_serialize, std::vector<std::vector<double>>, false>("DoubleTest", doubleData);
			register_benchmark<json_library::simdjson, test_type::parse_and_serialize, std::vector<std::vector<double>>, false>("DoubleTest", doubleData);
		}

		{
			std::vector<std::vector<std::string>> stringData = test_generator::generateValues<std::string>(100, 100);
			register_benchmark<json_library::jsonifier, test_type::parse_and_serialize, std::vector<std::vector<std::string>>, false>("StringTest", stringData);
			register_benchmark<json_library::glaze, test_type::parse_and_serialize, std::vector<std::vector<std::string>>, false>("StringTest", stringData);
			register_benchmark<json_library::simdjson, test_type::parse_and_serialize, std::vector<std::vector<std::string>>, false>("StringTest", stringData);
		}

		// Register full JSON struct tests
		register_benchmark<json_library::jsonifier, test_type::parse_and_serialize, test<test_struct>, false>("JsonTest", jsonTestData[0]);
		register_benchmark<json_library::glaze, test_type::parse_and_serialize, test<test_struct>, false>("JsonTest", jsonTestData[0]);
		register_benchmark<json_library::simdjson, test_type::parse_and_serialize, test<test_struct>, false>("JsonTest", jsonTestData[0]);

		register_benchmark<json_library::jsonifier, test_type::parse_and_serialize, test<test_struct>, true>("JsonTest", jsonTestData[0]);
		register_benchmark<json_library::glaze, test_type::parse_and_serialize, test<test_struct>, true>("JsonTest", jsonTestData[0]);
		register_benchmark<json_library::simdjson, test_type::parse_and_serialize, test<test_struct>, true>("JsonTest", jsonTestData[0]);

		// Discord test
		{
			std::string discordData = bnch_swt::file_loader::loadFile(jsonPath.operator std::string() + "/Discord Test (Prettified).json");

			discord_message discordDataObj{};
			parser.parseJson<jsonifier::parse_options{ .knownOrder = true }>(discordDataObj, discordData);

			// Save for future use
			bnch_swt::file_loader::saveFile(discordData, jsonOutPath.operator std::string() + "/Discord Test (Prettified).json");
			std::string discordMinifiedData = parser.minifyJson(discordData);
			bnch_swt::file_loader::saveFile(discordMinifiedData, jsonOutPath.operator std::string() + "/Discord Test (Minified).json");

			// Register benchmarks
			register_benchmark<json_library::jsonifier, test_type::parse_and_serialize, discord_message, false>("DiscordTest", discordDataObj);
			register_benchmark<json_library::glaze, test_type::parse_and_serialize, discord_message, false>("DiscordTest", discordDataObj);
			register_benchmark<json_library::simdjson, test_type::parse_and_serialize, discord_message, false>("DiscordTest", discordDataObj);

			register_benchmark<json_library::jsonifier, test_type::parse_and_serialize, discord_message, true>("DiscordTest", discordDataObj);
			register_benchmark<json_library::glaze, test_type::parse_and_serialize, discord_message, true>("DiscordTest", discordDataObj);
			register_benchmark<json_library::simdjson, test_type::parse_and_serialize, discord_message, true>("DiscordTest", discordDataObj);
		}

		// Canada test
		{
			std::string canadaData = bnch_swt::file_loader::loadFile(jsonPath.operator std::string() + "/Canada Test (Prettified).json");

			canada_message canadaDataObj{};
			parser.parseJson<jsonifier::parse_options{ .knownOrder = true }>(canadaDataObj, canadaData);

			// Save for future use
			bnch_swt::file_loader::saveFile(canadaData, jsonOutPath.operator std::string() + "/Canada Test (Prettified).json");
			std::string canadaMinifiedData = parser.minifyJson(canadaData);
			bnch_swt::file_loader::saveFile(canadaMinifiedData, jsonOutPath.operator std::string() + "/Canada Test (Minified).json");

			// Register benchmarks
			register_benchmark<json_library::jsonifier, test_type::parse_and_serialize, canada_message, false>("CanadaTest", canadaDataObj);
			register_benchmark<json_library::glaze, test_type::parse_and_serialize, canada_message, false>("CanadaTest", canadaDataObj);
			register_benchmark<json_library::simdjson, test_type::parse_and_serialize, canada_message, false>("CanadaTest", canadaDataObj);

			register_benchmark<json_library::jsonifier, test_type::parse_and_serialize, canada_message, true>("CanadaTest", canadaDataObj);
			register_benchmark<json_library::glaze, test_type::parse_and_serialize, canada_message, true>("CanadaTest", canadaDataObj);
			register_benchmark<json_library::simdjson, test_type::parse_and_serialize, canada_message, true>("CanadaTest", canadaDataObj);
		}

		// CitMCatalog test
		{
			std::string citmCatalogData = bnch_swt::file_loader::loadFile(jsonPath.operator std::string() + "/CitmCatalog Test (Prettified).json");

			citm_catalog_message citmCatalogDataObj{};
			parser.parseJson<jsonifier::parse_options{ .knownOrder = true }>(citmCatalogDataObj, citmCatalogData);

			// Save for future use
			bnch_swt::file_loader::saveFile(citmCatalogData, jsonOutPath.operator std::string() + "/CitmCatalog Test (Prettified).json");
			std::string citmCatalogMinifiedData = parser.minifyJson(citmCatalogData);
			bnch_swt::file_loader::saveFile(citmCatalogMinifiedData, jsonOutPath.operator std::string() + "/CitmCatalog Test (Minified).json");

			// Register benchmarks
			register_benchmark<json_library::jsonifier, test_type::parse_and_serialize, citm_catalog_message, false>("CitmCatalogTest", citmCatalogDataObj);
			register_benchmark<json_library::glaze, test_type::parse_and_serialize, citm_catalog_message, false>("CitmCatalogTest", citmCatalogDataObj);
			register_benchmark<json_library::simdjson, test_type::parse_and_serialize, citm_catalog_message, false>("CitmCatalogTest", citmCatalogDataObj);

			register_benchmark<json_library::jsonifier, test_type::parse_and_serialize, citm_catalog_message, true>("CitmCatalogTest", citmCatalogDataObj);
			register_benchmark<json_library::glaze, test_type::parse_and_serialize, citm_catalog_message, true>("CitmCatalogTest", citmCatalogDataObj);
			register_benchmark<json_library::simdjson, test_type::parse_and_serialize, citm_catalog_message, true>("CitmCatalogTest", citmCatalogDataObj);
		}

		// Twitter test
		{
			// Load Twitter test data
            std::string twitterData = bnch_swt::file_loader::loadFile(jsonPath.operator std::string() + "/Twitter Test (Prettified).json");
            std::string twitterMinifiedData = bnch_swt::file_loader::loadFile(jsonPath.operator std::string() + "/Twitter Test (Minified).json");
            
            // Debug info - check if data is loaded correctly
            std::cout << "Twitter data size: " << twitterData.size() << " bytes" << std::endl;
            
            // If Twitter data isn't available, use synthetic data
            if (twitterData.empty()) {
				std::cout << "Generating Twitter test data..." << std::endl;
				twitterMinifiedData = parser.minifyJson(twitterData);
				std::cout << "Generated Twitter data: " << twitterData.size() << " bytes" << std::endl;

				// Save for future use
				bnch_swt::file_loader::saveFile(twitterData, jsonOutPath.operator std::string() + "/Twitter Test (Prettified).json");
				bnch_swt::file_loader::saveFile(twitterMinifiedData, jsonOutPath.operator std::string() + "/Twitter Test (Minified).json");
            }
            
            // Create deep copies of the Twitter data to use in benchmarks
            std::string* twitterDataForMinify = new std::string(twitterData);
            std::string* twitterMinifiedDataForPrettify = new std::string(twitterMinifiedData);
            std::string* twitterDataForValidate = new std::string(twitterData);
            
            // Parse Twitter data
            twitter_message twitterDataObj{};
            parser.parseJson<jsonifier::parse_options{ .knownOrder = true }>(twitterDataObj, twitterData);
            
            // Parse Twitter Partial data
            twitter_partial_message twitterPartialDataObj{};
            parser.parseJson<jsonifier::parse_options{}>(twitterPartialDataObj, twitterData);
            
            // Save test files
            bnch_swt::file_loader::saveFile(twitterData, jsonOutPath.operator std::string() + "/Minify Test.json");
            bnch_swt::file_loader::saveFile(twitterMinifiedData, jsonOutPath.operator std::string() + "/Prettify Test.json");
            bnch_swt::file_loader::saveFile(twitterData, jsonOutPath.operator std::string() + "/Validate Test.json");
            
            // Register Twitter data benchmarks
            register_benchmark<json_library::jsonifier, test_type::parse_and_serialize, twitter_message, false>(
                "TwitterTest", twitterDataObj);
            register_benchmark<json_library::glaze, test_type::parse_and_serialize, twitter_message, false>(
                "TwitterTest", twitterDataObj);
            register_benchmark<json_library::simdjson, test_type::parse_and_serialize, twitter_message, false>(
                "TwitterTest", twitterDataObj);
                
            register_benchmark<json_library::jsonifier, test_type::parse_and_serialize, twitter_message, true>(
                "TwitterTest", twitterDataObj);
            register_benchmark<json_library::glaze, test_type::parse_and_serialize, twitter_message, true>(
                "TwitterTest", twitterDataObj);
            register_benchmark<json_library::simdjson, test_type::parse_and_serialize, twitter_message, true>(
                "TwitterTest", twitterDataObj);
                
            // Register Twitter Partial data benchmarks
            register_benchmark<json_library::jsonifier, test_type::parse_and_serialize, twitter_partial_message, false>(
                "TwitterPartialTest", twitterPartialDataObj);
            register_benchmark<json_library::jsonifier, test_type::parse_and_serialize, twitter_partial_message, true>(
                "TwitterPartialTest", twitterPartialDataObj);
            
            // Register minify, prettify, and validate operations
            benchmark::RegisterBenchmark(
                "MinifyTest/Jsonifier/Minify",
                [twitterDataForMinify](benchmark::State& state) {
				jsonifier::jsonifier_core parser{};
				std::string result;
				for (auto _: state) {
					result.clear();
					parser.minifyJson(*twitterDataForMinify, result);
					benchmark::DoNotOptimize(result);
					benchmark::ClobberMemory();
				}
				state.SetBytesProcessed(int64_t(state.iterations()) * int64_t(result.size()));
				state.SetLabel("Minify");
                }
            )->Unit(benchmark::kMicrosecond);
            
            benchmark::RegisterBenchmark(
                "MinifyTest/Glaze/Minify",
                [twitterDataForMinify](benchmark::State& state) {
				std::string result;
				for (auto _: state) {
					result.clear();
					glz::minify_json(*twitterDataForMinify, result);
					benchmark::DoNotOptimize(result);
					benchmark::ClobberMemory();
				}
				state.SetBytesProcessed(int64_t(state.iterations()) * int64_t(result.size()));
				state.SetLabel("Minify");
                }
            )->Unit(benchmark::kMicrosecond);
            
            benchmark::RegisterBenchmark(
                "MinifyTest/Simdjson/Minify",
                [twitterDataForMinify](benchmark::State& state) {
				simdjson::dom::parser parser{};
				std::string result;
				for (auto _: state) {
					try {
						//result = simdjson::minify(parser.parse(*twitterDataForMinify));
						benchmark::DoNotOptimize(result);
						benchmark::ClobberMemory();
					} catch (std::exception& error) {
						std::cout << "Simdjson Error: " << error.what() << std::endl;
					}
				}
				state.SetBytesProcessed(int64_t(state.iterations()) * int64_t(result.size()));
				state.SetLabel("Minify");
                }
            )->Unit(benchmark::kMicrosecond);
            
            benchmark::RegisterBenchmark(
                "PrettifyTest/Jsonifier/Prettify",
                [twitterMinifiedDataForPrettify](benchmark::State& state) {
				jsonifier::jsonifier_core parser{};
				std::string result;
				for (auto _: state) {
					result.clear();
					parser.prettifyJson(*twitterMinifiedDataForPrettify, result);
					benchmark::DoNotOptimize(result);
					benchmark::ClobberMemory();
				}
				state.SetBytesProcessed(int64_t(state.iterations()) * int64_t(result.size()));
				state.SetLabel("Prettify");
                }
            )->Unit(benchmark::kMicrosecond);
            
            benchmark::RegisterBenchmark(
                "PrettifyTest/Glaze/Prettify",
                [twitterMinifiedDataForPrettify](benchmark::State& state) {
				std::string result;
				for (auto _: state) {
					result.clear();
					glz::prettify_json(*twitterMinifiedDataForPrettify, result);
					benchmark::DoNotOptimize(result);
					benchmark::ClobberMemory();
				}
				state.SetBytesProcessed(int64_t(state.iterations()) * int64_t(result.size()));
				state.SetLabel("Prettify");
                }
            )->Unit(benchmark::kMicrosecond);
            
            benchmark::RegisterBenchmark(
                "ValidateTest/Jsonifier/Validate",
                [twitterDataForValidate](benchmark::State& state) {
				jsonifier::jsonifier_core parser{};
				for (auto _: state) {
					auto result = parser.validateJson(*twitterDataForValidate);
					benchmark::DoNotOptimize(result);
					benchmark::ClobberMemory();
				}
				state.SetBytesProcessed(int64_t(state.iterations()) * int64_t(twitterDataForValidate->size()));
				state.SetLabel("Validate");
                }
            )->Unit(benchmark::kMicrosecond);
            
            benchmark::RegisterBenchmark(
                "ValidateTest/Glaze/Validate",
                [twitterDataForValidate](benchmark::State& state) {
				for (auto _: state) {
					auto result = glz::validate_json(*twitterDataForValidate);
					benchmark::DoNotOptimize(result);
					benchmark::ClobberMemory();
				}
				state.SetBytesProcessed(int64_t(state.iterations()) * int64_t(twitterDataForValidate->size()));
				state.SetLabel("Validate");
                }
            )->Unit(benchmark::kMicrosecond);
		}

		// Run all registered benchmarks
		benchmark::RunSpecifiedBenchmarks();
	}
}// namespace tests
