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

	template<typename value_type>
	concept pod_type = std::is_same_v<std::vector<std::vector<bool>>, value_type> || std::is_same_v<std::vector<std::vector<std::string>>, value_type> ||
		std::is_same_v<std::vector<std::vector<int64_t>>, value_type> || std::is_same_v<std::vector<std::vector<uint64_t>>, value_type> ||
		std::is_same_v<std::vector<std::vector<double>>, value_type>;

	template<typename value_type>
	concept iterative_test_type = std::is_same_v<test<test_struct>, value_type> || std::is_same_v<abc_test<abc_test_struct>, value_type> ||
		std::is_same_v<partial_test<partial_test_struct>, value_type> || std::is_same_v<test_struct, value_type>;

	template<pod_type test_data_type, bool minified, size_t iterations, const bnch_swt::string_literal testNameNew>
	struct json_test_helper<json_library::jsonifier, test_type::parse_and_serialize, test_data_type, minified, iterations, testNameNew> {
		JSONIFIER_CLANG_INLINE static auto run(const test_data_type& testDataNew) {
			static constexpr bnch_swt::string_literal testName{ testNameNew };
			static constexpr bnch_swt::string_literal testNameRead{ testName + "-Read" };
			static constexpr bnch_swt::string_literal testNameWrite{ testName + "-Write" };
			static constexpr bool partialRead{ std::is_same_v<test_data_type, partial_test<partial_test_struct>> || std::is_same_v<test_data_type, twitter_partial_message> };
			static constexpr bool knownOrder{ true };
			results_data r{ jsonifierLibraryName, testName, jsonifierCommitUrl };
			jsonifier::jsonifier_core parser{};
			std::vector<std::vector<std::string>> newStrings{ iterations };
			for (size_t x = 0; x < iterations; ++x) {
				newStrings[x].resize(testDataNew[x].size());
			}
			for (size_t x = 0; x < iterations; ++x) {
				for (size_t y = 0; y < testDataNew[x].size(); ++y) {
					parser.serializeJson<jsonifier::serialize_options{ .prettify = !minified }>(testDataNew[x][y], newStrings[x][y]);
				}
			}
			test_data_type testDatas{ iterations };
			for (size_t x = 0; x < iterations; ++x) {
				testDatas[x].resize(testDataNew[x].size());
			}
			size_t currentIndex{};
			struct parse_test_struct {
				BNCH_SWT_HOST static size_t impl(jsonifier::jsonifier_core<>& parserNew, test_data_type& testDatas, size_t& currentIndex,
					std::vector<std::vector<std::string>>& newStrings) {
					size_t newSize{};
					for (size_t x = 0; x < testDatas[currentIndex].size(); ++x) {
						parserNew.parseJson<jsonifier::parse_options{ .partialRead = partialRead, .knownOrder = knownOrder, .minified = minified }>(testDatas[currentIndex][x],
							newStrings[currentIndex][x]);
						bnch_swt::do_not_optimize_away(testDatas[currentIndex]);
						newSize += newStrings[currentIndex][x].size();
					}
					++currentIndex;
					return newSize;
				}
			};
			auto readResult =
				bnch_swt::benchmark_stage<testNameRead, iterations, measuredIterations>::template run_benchmark<jsonifierLibraryName,  parse_test_struct>(parser, testDatas,
					currentIndex, newStrings);
			for (auto& value: parser.getErrors()) {
				std::cout << "Jsonifier Error: " << value << std::endl;
			}
			for (size_t x = 0; x < iterations; ++x) {
				for (size_t y = 0; y < testDataNew[x].size(); ++y) {
					newStrings[x][y] = std::string{};
				}
			}
			currentIndex = 0;
			struct serialize_test_struct {
				BNCH_SWT_HOST static size_t impl(jsonifier::jsonifier_core<>& parserNew, test_data_type& testDatas, size_t& currentIndex,
					std::vector<std::vector<std::string>>& newStrings) {
					size_t newSize{};
					for (size_t x = 0; x < testDatas[currentIndex].size(); ++x) {
						parserNew.serializeJson<jsonifier::serialize_options{ .prettify = !minified }>(testDatas[currentIndex][x], newStrings[currentIndex][x]);
						bnch_swt::do_not_optimize_away(newStrings[currentIndex][x]);
						newSize += newStrings[currentIndex][x].size();
					}
					++currentIndex;
					return newSize;
				}
			};
			auto writeResult =
				bnch_swt::benchmark_stage<testNameWrite, iterations, measuredIterations>::template run_benchmark<jsonifierLibraryName,  serialize_test_struct>(parser,
					testDatas, currentIndex, newStrings);
			r.readResult  = result<result_type::read>{ "teal", readResult };
			r.writeResult = result<result_type::write>{ "steelblue", writeResult };
			bnch_swt::file_loader::saveFile(static_cast<std::string>(parser.serializeJson(testDatas)),
				jsonOutPath.operator std::string() + "/" + testName.operator std::string() + "-jsonifier.json");
			return r;
		}
	};

	template<iterative_test_type test_data_type, bool minified, size_t iterations, const bnch_swt::string_literal testNameNew>
	struct json_test_helper<json_library::jsonifier, test_type::parse_and_serialize, test_data_type, minified, iterations, testNameNew> {
		JSONIFIER_CLANG_INLINE static auto run(const std::vector<test_data_type>& testDataNew) {
			static constexpr bnch_swt::string_literal testName{ testNameNew };
			static constexpr bnch_swt::string_literal testNameRead{ testName + "-Read" };
			static constexpr bnch_swt::string_literal testNameWrite{ testName + "-Write" };
			static constexpr bool partialRead{ std::is_same_v<test_data_type, partial_test<partial_test_struct>> || std::is_same_v<test_data_type, twitter_partial_message> };
			static constexpr bool knownOrder{ true };
			results_data r{ jsonifierLibraryName, testName, jsonifierCommitUrl };
			jsonifier::jsonifier_core parser{};
			std::vector<std::string> newStrings{ iterations };
			for (size_t x = 0; x < iterations; ++x) {
				parser.serializeJson<jsonifier::serialize_options{ .prettify = !minified }>(testDataNew[x], newStrings[x]);
			}
			std::vector<test_data_type> testDatas{ iterations };
			size_t currentIndex{};
			struct parse_test_struct {
				BNCH_SWT_HOST static size_t impl(jsonifier::jsonifier_core<>& parserNew, std::vector<test_data_type>& testDatas, size_t& currentIndex,
					std::vector<std::string>& newStrings) {
					parserNew.parseJson<jsonifier::parse_options{ .partialRead = partialRead, .knownOrder = knownOrder, .minified = minified }>(testDatas[currentIndex],
						newStrings[currentIndex]);
					bnch_swt::do_not_optimize_away(testDatas[currentIndex]);
					auto newSize = newStrings[currentIndex].size();
					++currentIndex;
					return newSize;
				}
			};
			auto readResult =
				bnch_swt::benchmark_stage<testNameRead, iterations, measuredIterations>::template run_benchmark<jsonifierLibraryName, parse_test_struct>(parser, testDatas,
					currentIndex, newStrings);
			for (auto& value: parser.getErrors()) {
				std::cout << "Jsonifier Error: " << value << std::endl;
			}
			for (size_t x = 0; x < iterations; ++x) {
				newStrings[x] = std::string{};
			}
			currentIndex = 0;
			struct serialize_test_struct {
				BNCH_SWT_HOST static size_t impl(jsonifier::jsonifier_core<>& parserNew, std::vector<test_data_type>& testDatas, size_t& currentIndex,
					std::vector<std::string>& newStrings) {
					parserNew.serializeJson<jsonifier::serialize_options{ .prettify = !minified }>(testDatas[currentIndex], newStrings[currentIndex]);
					bnch_swt::do_not_optimize_away(newStrings[currentIndex]);
					auto newSize = newStrings[currentIndex].size();
					++currentIndex;
					return newSize;
				}
			};
			auto writeResult = bnch_swt::benchmark_stage<testNameWrite, iterations, measuredIterations>::template run_benchmark<jsonifierLibraryName, serialize_test_struct>(parser,
				testDatas, currentIndex, newStrings);
			r.readResult  = result<result_type::read>{ "teal", readResult };
			r.writeResult = result<result_type::write>{ "steelblue", writeResult };
			bnch_swt::file_loader::saveFile(static_cast<std::string>(newStrings[0]),
				jsonOutPath.operator std::string() + "/" + testName.operator std::string() + "-jsonifier.json");
			return r;
		}
	};

	template<typename test_data_type, bool minified, size_t iterations, const bnch_swt::string_literal testNameNew>
	struct json_test_helper<json_library::jsonifier, test_type::parse_and_serialize, test_data_type, minified, iterations, testNameNew> {
		JSONIFIER_CLANG_INLINE static auto run(const test_data_type& testDataNew) {
			static constexpr bnch_swt::string_literal testName{ testNameNew };
			static constexpr bnch_swt::string_literal testNameRead{ testName + "-Read" };
			static constexpr bnch_swt::string_literal testNameWrite{ testName + "-Write" };
			static constexpr bool partialRead{ std::is_same_v<test_data_type, partial_test<partial_test_struct>> || std::is_same_v<test_data_type, twitter_partial_message> };
			static constexpr bool knownOrder{ true };
			results_data r{ jsonifierLibraryName, testName, jsonifierCommitUrl };
			jsonifier::jsonifier_core parser{};
			std::vector<std::string> newStrings{ iterations };
			std::string newString{ parser.serializeJson<jsonifier::serialize_options{ .prettify = !minified }>(testDataNew) };
			std::vector<test_data_type> testDatas{ iterations };
			size_t currentIndex{};
			struct parse_test_struct {
				BNCH_SWT_HOST static size_t impl(jsonifier::jsonifier_core<>& parserNew, std::vector<test_data_type>& testDatas, size_t& currentIndex,
					std::string& newStrings) {
					parserNew.parseJson<jsonifier::parse_options{ .partialRead = partialRead, .knownOrder = knownOrder, .minified = minified }>(testDatas[currentIndex],
						newStrings);
					bnch_swt::do_not_optimize_away(testDatas[currentIndex]);
					auto newSize = newStrings.size();
					++currentIndex;
					return newSize;
				}
			};
			auto readResult =
				bnch_swt::benchmark_stage<testNameRead, iterations, measuredIterations>::template run_benchmark<jsonifierLibraryName, parse_test_struct>(parser, testDatas,
					currentIndex, newString);
			for (auto& value: parser.getErrors()) {
				std::cout << "Jsonifier Error: " << value << std::endl;
			}
			for (size_t x = 0; x < iterations; ++x) {
				newStrings[x] = std::string{};
			}
			currentIndex = 0;
			struct serialize_test_struct {
				BNCH_SWT_HOST static size_t impl(jsonifier::jsonifier_core<>& parserNew, std::vector<test_data_type>& testDatas, size_t& currentIndex,
					std::vector<std::string>& newStrings) {
					parserNew.serializeJson<jsonifier::serialize_options{ .prettify = !minified }>(testDatas[currentIndex], newStrings[currentIndex]);
					bnch_swt::do_not_optimize_away(newStrings[currentIndex]);
					auto newSize = newStrings[currentIndex].size();
					++currentIndex;
					return newSize;
				}
			};
			auto writeResult = bnch_swt::benchmark_stage<testNameWrite, iterations, measuredIterations>::template run_benchmark<jsonifierLibraryName, serialize_test_struct>(parser,
				testDatas, currentIndex, newStrings);
			r.readResult	 = result<result_type::read>{ "teal", readResult };
			r.writeResult	 = result<result_type::write>{ "steelblue", writeResult };
			bnch_swt::file_loader::saveFile(static_cast<std::string>(newStrings[0]),
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
			std::vector<std::string> newStrings{ iterations };
			size_t currentIndex{};
			struct prettify_test_struct {
				BNCH_SWT_HOST static size_t impl(jsonifier::jsonifier_core<>& parserNew, size_t& currentIndex, std::vector<std::string>& newStrings,
					std::string& newerBuffer) {
					parserNew.prettifyJson(newerBuffer, newStrings[currentIndex]);
					bnch_swt::do_not_optimize_away(newStrings[currentIndex]);
					auto newSize = newStrings[currentIndex].size();
					++currentIndex;
					return newSize;
				}
			};
			auto writeResult = bnch_swt::benchmark_stage<testName, iterations, measuredIterations>::template run_benchmark<jsonifierLibraryName, prettify_test_struct>(parser,
				currentIndex, newStrings, newBuffer);

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
		JSONIFIER_CLANG_INLINE static auto run(std::string& newBuffer) {
			static constexpr bnch_swt::string_literal testName{ testNameNew };
			std::vector<std::string> newStrings{ iterations };
			results_data r{ jsonifierLibraryName, testName, jsonifierCommitUrl };
			jsonifier::jsonifier_core parser{};
			size_t currentIndex{};
			struct minify_test_struct {
				BNCH_SWT_HOST static size_t impl(jsonifier::jsonifier_core<>& parserNew, size_t& currentIndex, std::vector<std::string>& newStrings, std::string& newerBuffer) {
					parserNew.minifyJson(newerBuffer, newStrings[currentIndex]);
					bnch_swt::do_not_optimize_away(newStrings[currentIndex]);
					auto newSize = newStrings[currentIndex].size();
					++currentIndex;
					return newSize;
				}
			};
			auto writeResult = bnch_swt::benchmark_stage<testName, iterations, measuredIterations>::template run_benchmark<jsonifierLibraryName, minify_test_struct>(parser,
				currentIndex, newStrings, newBuffer);
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
		JSONIFIER_CLANG_INLINE static auto run(std::string& newBuffer) {
			static constexpr bnch_swt::string_literal testName{ testNameNew };
			results_data r{ jsonifierLibraryName, testName, jsonifierCommitUrl };
			struct validate_test_struct {
				BNCH_SWT_HOST static size_t impl(jsonifier::jsonifier_core<>& parserNew, std::string& newerBuffer) {
					if (auto result = parserNew.validateJson(newerBuffer); !result) {
						bnch_swt::do_not_optimize_away(result);
						return uint64_t{};
					}
					return static_cast<uint64_t>(newerBuffer.size());
				}
			};
			jsonifier::jsonifier_core parser{};
			auto readResult =
				bnch_swt::benchmark_stage<testName, iterations, measuredIterations>::template run_benchmark<jsonifierLibraryName, validate_test_struct>(parser, newBuffer);

			for (auto& value: parser.getErrors()) {
				std::cout << "Jsonifier Error: " << value << std::endl;
			}
			bnch_swt::file_loader::saveFile(newBuffer, jsonOutPath.operator std::string() + "/" + testName.operator std::string() + "-jsonifier.json");
			r.readResult = result<result_type::read>{ "teal", readResult };
			return r;
		}
	};

	template<typename value_type> struct get_ref {
		using type = value_type&;
	};

	template<jsonifier::concepts::bool_t value_type> struct get_ref<value_type> {
		using type = value_type;
	};

	template<typename value_type> using get_ref_t = typename get_ref<value_type>::type;

	template<pod_type test_data_type, bool minified, size_t iterations, const bnch_swt::string_literal testNameNew>
	struct json_test_helper<json_library::glaze, test_type::parse_and_serialize, test_data_type, minified, iterations, testNameNew> {
		JSONIFIER_CLANG_INLINE static auto run(const test_data_type& testDataNew) {
			static constexpr bnch_swt::string_literal testName{ testNameNew };
			static constexpr bnch_swt::string_literal testNameRead{ testName + "-Read" };
			static constexpr bnch_swt::string_literal testNameWrite{ testName + "-Write" };
			static constexpr bool partialRead{ std::is_same_v<test_data_type, partial_test<partial_test_struct>> || std::is_same_v<test_data_type, twitter_partial_message> };
			results_data r{ glazeLibraryName, testName, glazeCommitUrl };
			std::vector<std::vector<std::string>> newStrings{ iterations };
			for (size_t x = 0; x < iterations; ++x) {
				newStrings[x].resize(testDataNew[x].size());
			}
			for (size_t x = 0; x < iterations; ++x) {
				for (size_t y = 0; y < testDataNew[x].size(); ++y) {
					auto newResult = glz::write<glz::opts{ .skip_null_members = false, .prettify = !minified, .minified = minified }>(testDataNew[x][y], newStrings[x][y]);
					( void )newResult;
				}
			}
			test_data_type testDatas{ iterations };
			for (size_t x = 0; x < iterations; ++x) {
				testDatas[x].resize(testDataNew[x].size());
			}
			size_t currentIndex{};
			struct parse_test_struct {
				BNCH_SWT_HOST static size_t impl(test_data_type& testDatas, size_t& currentIndex, std::vector<std::vector<std::string>>& newStrings) {
					size_t newSize{};
					for (size_t x = 0; x < testDatas[currentIndex].size(); ++x) {
						get_ref_t<decltype(testDatas[currentIndex][x])> reference = testDatas[currentIndex][x];
						if (auto error = glz::read<glz::opts{ .error_on_unknown_keys = !partialRead,
								.skip_null_members									 = false,
								.prettify											 = !minified,
								.minified											 = minified,
								.partial_read										 = partialRead }>(reference, newStrings[currentIndex][x]);
							error) {
							std::cout << "Glaze Error: " << glz::format_error(error, newStrings[currentIndex][x]) << std::endl;
						}
						bnch_swt::do_not_optimize_away(testDatas[currentIndex]);
						newSize += newStrings[currentIndex][x].size();
					}
					++currentIndex;
					return newSize;
				}
			};
			auto readResult =
				bnch_swt::benchmark_stage<testNameRead, iterations, measuredIterations>::template run_benchmark<glazeLibraryName,  parse_test_struct>(testDatas, currentIndex,
					newStrings);
			for (size_t x = 0; x < iterations; ++x) {
				for (size_t y = 0; y < testDataNew[x].size(); ++y) {
					newStrings[x][y] = std::string{};
				}
			}
			currentIndex = 0;
			struct serialize_test_struct {
				BNCH_SWT_HOST static size_t impl(test_data_type& testDatas, size_t& currentIndex, std::vector<std::vector<std::string>>& newStrings) {
					size_t newSize{};
					for (size_t x = 0; x < testDatas[currentIndex].size(); ++x) {
						auto newResult = glz::write<glz::opts{ .skip_null_members = false, .prettify = !minified, .minified = minified }>(testDatas[currentIndex][x],
							newStrings[currentIndex][x]);
						( void )newResult;
						bnch_swt::do_not_optimize_away(newStrings[currentIndex][x]);
						newSize += newStrings[currentIndex][x].size();
					}
					++currentIndex;
					return newSize;
				}
			};
			auto writeResult =
				bnch_swt::benchmark_stage<testNameWrite, iterations, measuredIterations>::template run_benchmark<glazeLibraryName,  serialize_test_struct>(testDatas,
					currentIndex, newStrings);
			r.readResult  = result<result_type::read>{ "dodgerblue", readResult };
			r.writeResult = result<result_type::write>{ "skyblue", writeResult };
			std::string newString{};
			auto newResult = glz::write_json(testDatas, newString);
			( void )newResult;
			bnch_swt::file_loader::saveFile(newString, jsonOutPath.operator std::string() + "/" + testName.operator std::string() + "-glaze.json");
			return r;
		}
	};

	template<iterative_test_type test_data_type, bool minified, size_t iterations, const bnch_swt::string_literal testNameNew>
	struct json_test_helper<json_library::glaze, test_type::parse_and_serialize, test_data_type, minified, iterations, testNameNew> {
		JSONIFIER_CLANG_INLINE static auto run(const std::vector<test_data_type>& testDataNew) {
			static constexpr bnch_swt::string_literal testName{ testNameNew };
			static constexpr bnch_swt::string_literal testNameRead{ testName + "-Read" };
			static constexpr bnch_swt::string_literal testNameWrite{ testName + "-Write" };
			static constexpr bool partialRead{ std::is_same_v<test_data_type, partial_test<partial_test_struct>> || std::is_same_v<test_data_type, twitter_partial_message> };
			results_data r{ glazeLibraryName, testName, glazeCommitUrl };
			std::vector<std::string> newStrings{ iterations };
			std::vector<test_data_type> testDatas{ iterations };
			for (size_t x = 0; x < iterations; ++x) {
				auto newResult = glz::write<glz::opts{ .skip_null_members = false, .prettify = !minified, .minified = minified }>(testDataNew[x], newStrings[x]);
				( void )newResult;
			}
			size_t currentIndex{};
			struct parse_test_struct {
				BNCH_SWT_HOST static size_t impl(std::vector<test_data_type>& testDatas, size_t& currentIndex, std::vector<std::string>& newStrings) {
					if (auto error = glz::read<glz::opts{ .error_on_unknown_keys = !partialRead,
							.skip_null_members									 = false,
							.prettify											 = !minified,
							.minified											 = minified,
							.partial_read										 = partialRead }>(testDatas[currentIndex], newStrings[currentIndex]);
						error) {
						std::cout << "Glaze Error: " << glz::format_error(error, newStrings[currentIndex]) << std::endl;
					}
					bnch_swt::do_not_optimize_away(testDatas[currentIndex]);
					auto newSize = newStrings[currentIndex].size();
					++currentIndex;
					return newSize;
				}
			};
			auto readResult =
				bnch_swt::benchmark_stage<testNameRead, iterations, measuredIterations>::template run_benchmark<glazeLibraryName,  parse_test_struct>(testDatas,
					currentIndex, newStrings);
			for (size_t x = 0; x < iterations; ++x) {
				newStrings[x] = std::string{};
			}
			currentIndex = 0;
			struct serialize_test_struct {
				BNCH_SWT_HOST static size_t impl(std::vector<test_data_type>& testDatas, size_t& currentIndex, std::vector<std::string>& newStrings) {
					auto newResult =
						glz::write<glz::opts{ .skip_null_members = false, .prettify = !minified, .minified = minified }>(testDatas[currentIndex], newStrings[currentIndex]);
					bnch_swt::do_not_optimize_away(newStrings[currentIndex]);
					( void )newResult;
					auto newSize = newStrings[currentIndex].size();
					++currentIndex;
					return newSize;
				}
			};
			auto writeResult =
				bnch_swt::benchmark_stage<testNameWrite, iterations, measuredIterations>::template run_benchmark<glazeLibraryName,  serialize_test_struct>(testDatas,
					currentIndex, newStrings);
			r.readResult  = result<result_type::read>{ "dodgerblue", readResult };
			r.writeResult = result<result_type::write>{ "skyblue", writeResult };
			bnch_swt::file_loader::saveFile(static_cast<std::string>(newStrings[0]), jsonOutPath.operator std::string() + "/" + testName.operator std::string() + "-glaze.json");
			return r;
		}
	};

	template<typename test_data_type, bool minified, size_t iterations, const bnch_swt::string_literal testNameNew>
	struct json_test_helper<json_library::glaze, test_type::parse_and_serialize, test_data_type, minified, iterations, testNameNew> {
		JSONIFIER_CLANG_INLINE static auto run(const test_data_type& testDataNew) {
			static constexpr bnch_swt::string_literal testName{ testNameNew };
			static constexpr bnch_swt::string_literal testNameRead{ testName + "-Read" };
			static constexpr bnch_swt::string_literal testNameWrite{ testName + "-Write" };
			static constexpr bool partialRead{ std::is_same_v<test_data_type, partial_test<partial_test_struct>> || std::is_same_v<test_data_type, twitter_partial_message> };
			results_data r{ glazeLibraryName, testName, glazeCommitUrl };
			std::vector<std::string> newStrings{ iterations };
			std::string newString{};
			std::vector<test_data_type> testDatas{ iterations };
			auto newResult = glz::write<glz::opts{ .skip_null_members = false, .prettify = !minified, .minified = minified }>(testDataNew, newString);
			( void )newResult;
			size_t currentIndex{};
			struct parse_test_struct {
				BNCH_SWT_HOST static size_t impl(std::vector<test_data_type>& testDatas, size_t& currentIndex, std::string& newString, std::vector<std::string>& newStrings) {
					if (auto error = glz::read<glz::opts{ .error_on_unknown_keys = !partialRead,
							.skip_null_members									 = false,
							.prettify											 = !minified,
							.minified											 = minified,
							.partial_read										 = partialRead }>(testDatas[currentIndex], newString);
						error) {
						std::cout << "Glaze Error: " << glz::format_error(error, newStrings[currentIndex]) << std::endl;
					}
					bnch_swt::do_not_optimize_away(testDatas[currentIndex]);
					auto newSize = newString.size();
					++currentIndex;
					return newSize;
				}
			};
			auto readResult =
				bnch_swt::benchmark_stage<testNameRead, iterations, measuredIterations>::template run_benchmark<glazeLibraryName,  parse_test_struct>(testDatas,
					currentIndex, newString, newStrings);
			for (size_t x = 0; x < iterations; ++x) {
				testDatas[x] = testDatas[0];
			}
			currentIndex = 0;
			struct serialize_test_struct {
				BNCH_SWT_HOST static size_t impl(std::vector<test_data_type>& testDatas, size_t& currentIndex, std::vector<std::string>& newStrings) {
					auto newResultNew =
						glz::write<glz::opts{ .skip_null_members = false, .prettify = !minified, .minified = minified }>(testDatas[currentIndex], newStrings[currentIndex]);
					bnch_swt::do_not_optimize_away(newStrings[currentIndex]);
					( void )newResultNew;
					auto newSize = newStrings[currentIndex].size();
					++currentIndex;
					return newSize;
				}
			};
			auto writeResult =
				bnch_swt::benchmark_stage<testNameWrite, iterations, measuredIterations>::template run_benchmark<glazeLibraryName,  serialize_test_struct>(testDatas,
					currentIndex, newStrings);
			r.readResult  = result<result_type::read>{ "dodgerblue", readResult };
			r.writeResult = result<result_type::write>{ "skyblue", writeResult };
			bnch_swt::file_loader::saveFile(static_cast<std::string>(newStrings[0]), jsonOutPath.operator std::string() + "/" + testName.operator std::string() + "-glaze.json");
			return r;
		}
	};

	template<size_t iterations, bnch_swt::string_literal testNameNew>
	struct json_test_helper<json_library::glaze, test_type::prettify, std::string, false, iterations, testNameNew> {
		JSONIFIER_CLANG_INLINE static auto run(std::string& newBuffer) {
			static constexpr bnch_swt::string_literal testName{ testNameNew };
			std::vector<std::string> newStrings{ iterations };
			results_data r{ glazeLibraryName, testName, glazeCommitUrl };
			size_t currentIndex{};

			struct prettify_test_struct {
				BNCH_SWT_HOST static size_t impl(std::string& newBuffer, std::vector<std::string>& newStrings, size_t& currentIndex) {
					glz::prettify_json(newBuffer, newStrings[currentIndex]);
					bnch_swt::do_not_optimize_away(newStrings[currentIndex]);
					auto newSize = newStrings[currentIndex].size();
					++currentIndex;
					return newSize;
				}
			};

			auto writeResult = bnch_swt::benchmark_stage<testName, iterations, measuredIterations>::template run_benchmark<glazeLibraryName,  prettify_test_struct>(
				newBuffer, newStrings, currentIndex);

			bnch_swt::file_loader::saveFile(newStrings[0], jsonOutPath.operator std::string() + "/" + testName.operator std::string() + "-glaze.json");
			r.writeResult = result<result_type::write>{ "skyblue", writeResult };

			return r;
		}
	};

	template<size_t iterations, bnch_swt::string_literal testNameNew> struct json_test_helper<json_library::glaze, test_type::minify, std::string, false, iterations, testNameNew> {
		JSONIFIER_CLANG_INLINE static auto run(std::string& newBuffer) {
			static constexpr bnch_swt::string_literal testName{ testNameNew };

			std::string newestBuffer{ newBuffer };
			std::vector<std::string> newStrings{ iterations };
			size_t currentIndex{};
			results_data r{ glazeLibraryName, testName, glazeCommitUrl };

			struct minify_test_struct {
				BNCH_SWT_HOST static size_t impl(std::string& newestBuffer, std::vector<std::string>& newStrings, size_t& currentIndex) {
					glz::minify_json(newestBuffer, newStrings[currentIndex]);
					bnch_swt::do_not_optimize_away(newStrings[currentIndex]);
					auto newSize = newStrings[currentIndex].size();
					++currentIndex;
					return newSize;
				}
			};

			auto writeResult = bnch_swt::benchmark_stage<testName, iterations, measuredIterations>::template run_benchmark<glazeLibraryName,  minify_test_struct>(
				newestBuffer, newStrings, currentIndex);

			bnch_swt::file_loader::saveFile(newStrings[0], jsonOutPath.operator std::string() + "/" + testName.operator std::string() + "-glaze.json");
			r.writeResult = result<result_type::write>{ "skyblue", writeResult };

			return r;
		}
	};

	template<size_t iterations, bnch_swt::string_literal testNameNew>
	struct json_test_helper<json_library::glaze, test_type::validate, std::string, false, iterations, testNameNew> {
		JSONIFIER_CLANG_INLINE static auto run(std::string& newBuffer) {
			static constexpr bnch_swt::string_literal testName{ testNameNew };
			results_data r{ glazeLibraryName, testName, glazeCommitUrl };

			struct validate_test_struct {
				BNCH_SWT_HOST static size_t impl(std::string& newBuffer) {
					if (auto result = glz::validate_json(newBuffer); result) {
						bnch_swt::do_not_optimize_away(result);
						return size_t{};
					}
					return newBuffer.size();
				}
			};

			auto readResult =
				bnch_swt::benchmark_stage<testName, iterations, measuredIterations>::template run_benchmark<glazeLibraryName,  validate_test_struct>(newBuffer);

			bnch_swt::file_loader::saveFile(newBuffer, jsonOutPath.operator std::string() + "/" + testName.operator std::string() + "-glaze.json");
			r.readResult = result<result_type::read>{ "dodgerblue", readResult };

			return r;
		}
	};

	template<pod_type test_data_type, bool minified, size_t iterations, const bnch_swt::string_literal testNameNew>
	struct json_test_helper<json_library::simdjson, test_type::parse_and_serialize, test_data_type, minified, iterations, testNameNew> {
		JSONIFIER_CLANG_INLINE static auto run(const test_data_type& testDataNew) {
			static constexpr bnch_swt::string_literal testName{ testNameNew };
			static constexpr bnch_swt::string_literal testNameRead{ testName + "-Read" };
			static constexpr bnch_swt::string_literal testNameWrite{ testName + "-Write" };
			results_data r{ simdjsonLibraryName, testName, simdjsonCommitUrl };
			std::vector<std::vector<std::string>> newStrings{ iterations };
			simdjson::ondemand::parser parser{};
			for (size_t x = 0; x < iterations; ++x) {
				newStrings[x].resize(testDataNew[x].size());
			}
			for (size_t x = 0; x < iterations; ++x) {
				for (size_t y = 0; y < testDataNew[x].size(); ++y) {
					auto newResult = glz::write<glz::opts{ .skip_null_members = false, .prettify = !minified, .minified = minified }>(testDataNew[x][y], newStrings[x][y]);
					( void )newResult;
				}
			}
			test_data_type testDatas{ iterations };
			for (size_t x = 0; x < iterations; ++x) {
				testDatas[x].resize(testDataNew[x].size());
			}
			size_t currentIndex{};

			struct parse_test_struct {
				BNCH_SWT_HOST static size_t impl(simdjson::ondemand::parser& parser, test_data_type& testDatas, size_t& currentIndex,
					std::vector<std::vector<std::string>>& newStrings) {
					size_t newSize{};
					for (size_t x = 0; x < testDatas[currentIndex].size(); ++x) {
						getValue(testDatas[currentIndex][x], static_cast<simdjson::ondemand::document>(parser.iterate(newStrings[currentIndex][x])));
						bnch_swt::do_not_optimize_away(testDatas[currentIndex]);
						newSize += newStrings[currentIndex][x].size();
					}
					++currentIndex;
					return newSize;
				}
			};

			auto readResult =
				bnch_swt::benchmark_stage<testNameRead, iterations, measuredIterations>::template run_benchmark<simdjsonLibraryName,  parse_test_struct>(parser,
					testDatas, currentIndex, newStrings);
			r.readResult = result<result_type::read>{ "cadetblue", readResult };
			std::string newString{};
			auto newResult = glz::write_json(testDatas, newString);
			( void )newResult;
			bnch_swt::file_loader::saveFile(newString, jsonOutPath.operator std::string() + "/" + testName.operator std::string() + "-simdjson.json");
			return r;
		}
	};

	template<iterative_test_type test_data_type, bool minified, size_t iterations, const bnch_swt::string_literal testNameNew>
	struct json_test_helper<json_library::simdjson, test_type::parse_and_serialize, test_data_type, minified, iterations, testNameNew> {
		JSONIFIER_CLANG_INLINE static auto run(const std::vector<test_data_type>& testDataNew) {
			static constexpr bnch_swt::string_literal testName{ testNameNew };
			static constexpr bnch_swt::string_literal testNameRead{ testName + "-Read" };
			static constexpr bnch_swt::string_literal testNameWrite{ testName + "-Write" };
			results_data r{ simdjsonLibraryName, testName, simdjsonCommitUrl };
			jsonifier::jsonifier_core parserNew{};
			std::vector<std::string> newStrings{ iterations };
			simdjson::ondemand::parser parser{};
			std::vector<test_data_type> testDatas{ iterations };
			for (size_t x = 0; x < iterations; ++x) {
				parserNew.serializeJson<jsonifier::serialize_options{ .prettify = !minified }>(testDataNew[x], newStrings[x]);
			}
			size_t currentIndex{};

			struct parse_test_struct {
				BNCH_SWT_HOST static size_t impl(simdjson::ondemand::parser& parser, std::vector<test_data_type>& testDatas, size_t& currentIndex,
					std::vector<std::string>& newStrings) {
					try {
						getValue(testDatas[currentIndex], parser.iterate(newStrings[currentIndex]).value().get_value());
						bnch_swt::do_not_optimize_away(testDatas[currentIndex]);
						auto newSize = newStrings[currentIndex].size();
						++currentIndex;
						return newSize;

					} catch (const std::exception& error) {
						std::cout << "Simdjson Error: " << error.what() << std::endl;
					}
					++currentIndex;
					return size_t{};
				}
			};

			auto readResult =
				bnch_swt::benchmark_stage<testNameRead, iterations, measuredIterations>::template run_benchmark<simdjsonLibraryName,  parse_test_struct>(parser,
					testDatas, currentIndex, newStrings);
			r.readResult = result<result_type::read>{ "cadetblue", readResult };
			bnch_swt::file_loader::saveFile(static_cast<std::string>(newStrings[0]), jsonOutPath.operator std::string() + "/" + testName.operator std::string() + "-simdjson.json");
			return r;
		}
	};

	template<typename test_data_type, bool minified, size_t iterations, const bnch_swt::string_literal testNameNew>
	struct json_test_helper<json_library::simdjson, test_type::parse_and_serialize, test_data_type, minified, iterations, testNameNew> {
		JSONIFIER_CLANG_INLINE static auto run(const test_data_type& testDataNew) {
			static constexpr bnch_swt::string_literal testName{ testNameNew };
			static constexpr bnch_swt::string_literal testNameRead{ testName + "-Read" };
			static constexpr bnch_swt::string_literal testNameWrite{ testName + "-Write" };
			results_data r{ simdjsonLibraryName, testName, simdjsonCommitUrl };
			jsonifier::jsonifier_core parserNew{};
			simdjson::ondemand::parser parser{};
			std::string newString{};
			test_data_type testDataNewer{};
			std::vector<test_data_type> testDatas{ iterations };
			parserNew.serializeJson<jsonifier::serialize_options{ .prettify = !minified }>(testDataNew, newString);
			getValue(testDataNewer, parser.iterate(newString).value().get_value());
			newString.clear();
			parserNew.serializeJson<jsonifier::serialize_options{ .prettify = !minified }>(testDataNewer, newString);
			size_t currentIndex{};

			struct parse_test_struct {
				BNCH_SWT_HOST static size_t impl(simdjson::ondemand::parser& parser, std::vector<test_data_type>& testDatas, size_t& currentIndex, std::string& newString) {
					try {
						getValue(testDatas[currentIndex], parser.iterate(newString).value().get_value());
						bnch_swt::do_not_optimize_away(testDatas[currentIndex]);
						auto newSize = newString.size();
						++currentIndex;
						return newSize;

					} catch (const std::exception& error) {
						std::cout << "Simdjson Error: " << error.what() << std::endl;
					}
					++currentIndex;
					return newString.size();
				}
			};

			auto readResult =
				bnch_swt::benchmark_stage<testNameRead, iterations, measuredIterations>::template run_benchmark<simdjsonLibraryName,  parse_test_struct>(parser,
					testDatas, currentIndex, newString);
			r.readResult = result<result_type::read>{ "cadetblue", readResult };
			bnch_swt::file_loader::saveFile(static_cast<std::string>(newString), jsonOutPath.operator std::string() + "/" + testName.operator std::string() + "-simdjson.json");
			return r;
		}
	};

	template<size_t iterations, bnch_swt::string_literal testNameNew>
	struct json_test_helper<json_library::simdjson, test_type::minify, std::string, false, iterations, testNameNew> {
		static constexpr bnch_swt::string_literal testName{ testNameNew };
		JSONIFIER_CLANG_INLINE static auto run(std::string& newBuffer) {
			results_data r{ simdjsonLibraryName, testName, simdjsonCommitUrl };

			simdjson::dom::parser parser{};
			std::vector<std::string> newStrings{ iterations };
			size_t currentIndex{};

			struct minify_test_struct {
				BNCH_SWT_HOST static size_t impl(simdjson::dom::parser& parser, std::string& newBuffer, std::vector<std::string>& newStrings, size_t& currentIndex) {
					try {
						newStrings[currentIndex] = simdjson::minify(parser.parse(newBuffer));
						bnch_swt::do_not_optimize_away(newStrings[currentIndex]);
						auto newSize = newStrings[currentIndex].size();
						++currentIndex;
						return newSize;
					} catch (std::exception& error) {
						std::cout << "Simdjson Error: " << error.what() << std::endl;
						return newStrings[currentIndex].size();
					}
				}
			};

			auto writeResult =
				bnch_swt::benchmark_stage<testName, iterations, measuredIterations>::template run_benchmark<simdjsonLibraryName, minify_test_struct>(parser,
					newBuffer, newStrings, currentIndex);

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
 > )" + jsonifier::toString(maxIterations) +
		R"( iterations on a ()" + getCPUInfo() + R"(), where the most stable 20 subsequent iterations are sampled.

#### Note:
These benchmarks were executed using the CPU benchmark library [benchmarksuite](https://github.com/RealTimeChris/benchmarksuite).)" };

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
		template<typename test_data_type_new> JSONIFIER_CLANG_INLINE static test_results run(const test_data_type_new& jsonDataNew) {
			static constexpr bnch_swt::string_literal testName{ testNameNew };
			test_results jsonResults{};
			jsonResults.testName = testName.operator std::string();
			results_data jsonifierResults{};
			results_data simdjsonResults{};
			results_data glazeResults{};

#if !defined(ASAN_ENABLED)
			simdjsonResults = json_test_helper<json_library::simdjson, type, test_data_type, minified, iterations, testName>::run(jsonDataNew);
			if constexpr (!std::is_same_v<test_data_type, partial_test<partial_test_struct>> && !std::is_same_v<test_data_type, twitter_partial_message>) {
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
			bnch_swt::benchmark_stage<testNameNew + "-Read", iterations, measuredIterations>::print_results();
			bnch_swt::benchmark_stage<testNameNew + "-Write", iterations, measuredIterations>::print_results();
			return jsonResults;
		}
	};

	template<uint64_t iterations, bnch_swt::string_literal testNameNew> struct json_tests_helper<test_type::prettify, std::string, false, iterations, testNameNew> {
		JSONIFIER_CLANG_INLINE static test_results run(std::string& jsonDataNew) {
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
			bnch_swt::benchmark_stage<testNameNew, iterations, measuredIterations>::print_results();
			return jsonResults;
		}
	};

	template<uint64_t iterations, bnch_swt::string_literal testNameNew> struct json_tests_helper<test_type::minify, std::string, false, iterations, testNameNew> {
		JSONIFIER_CLANG_INLINE static test_results run(std::string& jsonDataNew) {
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
			bnch_swt::benchmark_stage<testNameNew, iterations, measuredIterations>::print_results();
			return jsonResults;
		}
	};

	template<uint64_t iterations, bnch_swt::string_literal testNameNew> struct json_tests_helper<test_type::validate, std::string, false, iterations, testNameNew> {
		JSONIFIER_CLANG_INLINE static test_results run(std::string& jsonDataNew) {
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
			bnch_swt::benchmark_stage<testNameNew, iterations, measuredIterations>::print_results();
			return jsonResults;
		}
	};

	JSONIFIER_CLANG_INLINE void testFunction() {
		std::string jsonDataNew{};
		jsonifier::jsonifier_core parser{};
		std::vector<test<test_struct>> jsonDataNewer{ maxIterations };
		for (size_t x = 0; x < maxIterations; ++x) {
			jsonDataNewer[x] = test_generator::generateTest();
		}
		parser.serializeJson<jsonifier::serialize_options{ .prettify = true }>(jsonDataNewer[0], jsonDataNew);
		bnch_swt::file_loader::saveFile(jsonDataNew, jsonOutPath.operator std::string() + "/Json Test (Prettified).json");
		std::string jsonMinifiedData = parser.minifyJson(jsonDataNew);
		bnch_swt::file_loader::saveFile(jsonMinifiedData, jsonOutPath.operator std::string() + "/Json Test (Minified).json");
		bnch_swt::file_loader::saveFile(jsonDataNew, jsonOutPath.operator std::string() + "/Partial Test (Prettified).json");
		bnch_swt::file_loader::saveFile(jsonMinifiedData, jsonOutPath.operator std::string() + "/Partial Test (Minified).json");
		bnch_swt::file_loader::saveFile(jsonDataNew, jsonOutPath.operator std::string() + "/Abc (Out of Order) Test (Prettified).json");
		bnch_swt::file_loader::saveFile(jsonMinifiedData, jsonOutPath.operator std::string() + "/Abc (Out of Order) Test (Minified).json");
		std::string newTimeString{};
		newTimeString.resize(1024);
		std::tm resultTwo{ getTime() };
		std::vector<test_results> benchmark_data{};
		newTimeString.resize(strftime(newTimeString.data(), 1024, "%b %d, %Y", &resultTwo));
		std::string newerString{ section00.operator std::string() + newTimeString + ")\n" + static_cast<std::string>(section002.operator std::string()) +
			static_cast<std::string>(section001) };
		test_results testResults{};
		{
			std::vector<std::vector<double>> doubleData{ test_generator::generateValues<double>(maxIterations, 100) };
			parser.serializeJson<jsonifier::serialize_options{ .prettify = true }>(doubleData, jsonDataNew);
			bnch_swt::file_loader::saveFile(jsonDataNew, jsonOutPath.operator std::string() + "/Double Test.json");
			testResults = json_tests_helper<test_type::parse_and_serialize, std::vector<std::vector<double>>, false, maxIterations, "Double Test">::run(doubleData);
			newerString += testResults.markdownResults;
			benchmark_data.emplace_back(testResults);
		}
		{
			std::vector<std::vector<std::string>> stringData{ test_generator::generateValues<std::string>(maxIterations, 100) };
			parser.serializeJson<jsonifier::serialize_options{ .prettify = true }>(stringData, jsonDataNew);
			bnch_swt::file_loader::saveFile(jsonDataNew, jsonOutPath.operator std::string() + "/String Test.json");
			testResults = json_tests_helper<test_type::parse_and_serialize, std::vector<std::vector<std::string>>, false, maxIterations, "String Test">::run(stringData);
			newerString += testResults.markdownResults;
			benchmark_data.emplace_back(testResults);
		}
		{
			std::vector<std::vector<uint64_t>> uintData{ test_generator::generateValues<uint64_t>(maxIterations, 100) };
			parser.serializeJson<jsonifier::serialize_options{ .prettify = true }>(uintData, jsonDataNew);
			bnch_swt::file_loader::saveFile(jsonDataNew, jsonOutPath.operator std::string() + "/Uint Test.json");
			testResults = json_tests_helper<test_type::parse_and_serialize, std::vector<std::vector<uint64_t>>, false, maxIterations, "Uint Test">::run(uintData);
			newerString += testResults.markdownResults;
			benchmark_data.emplace_back(testResults);
		}
		{
			std::vector<std::vector<int64_t>> intData{ test_generator::generateValues<int64_t>(maxIterations, 100) };
			parser.serializeJson<jsonifier::serialize_options{ .prettify = true }>(intData, jsonDataNew);
			bnch_swt::file_loader::saveFile(jsonDataNew, jsonOutPath.operator std::string() + "/Int Test.json");
			testResults = json_tests_helper<test_type::parse_and_serialize, std::vector<std::vector<int64_t>>, false, maxIterations, "Int Test">::run(intData);
			newerString += testResults.markdownResults;
			benchmark_data.emplace_back(testResults);
		}
		{
			std::vector<std::vector<bool>> boolData{ test_generator::generateValues<bool>(maxIterations, 100) };
			parser.serializeJson<jsonifier::serialize_options{ .prettify = true }>(boolData, jsonDataNew);
			bnch_swt::file_loader::saveFile(jsonDataNew, jsonOutPath.operator std::string() + "/Bool Test.json");
			testResults = json_tests_helper<test_type::parse_and_serialize, std::vector<std::vector<bool>>, false, maxIterations, "Bool Test">::run(boolData);
			newerString += testResults.markdownResults;
			benchmark_data.emplace_back(testResults);
		}
		testResults = json_tests_helper<test_type::parse_and_serialize, test<test_struct>, false, maxIterations, "Json Test (Prettified)">::run(jsonDataNewer);
		newerString += testResults.markdownResults;
		benchmark_data.emplace_back(testResults);
		testResults = json_tests_helper<test_type::parse_and_serialize, test<test_struct>, true, maxIterations, "Json Test (Minified)">::run(jsonDataNewer);
		newerString += testResults.markdownResults;
		benchmark_data.emplace_back(testResults);
		std::vector<partial_test<partial_test_struct>> jsonPartialDataNewer{ maxIterations };
		std::vector<std::string> newStrings{ maxIterations };
		for (size_t x = 0; x < maxIterations; ++x) {
			parser.serializeJson(jsonDataNewer[x], newStrings[x]);
			parser.parseJson(jsonPartialDataNewer[x], newStrings[x]);
		}
		testResults =
			json_tests_helper<test_type::parse_and_serialize, partial_test<partial_test_struct>, false, maxIterations, "Partial Test (Prettified)">::run(jsonPartialDataNewer);
		newerString += testResults.markdownResults;
		benchmark_data.emplace_back(testResults);
		testResults =
			json_tests_helper<test_type::parse_and_serialize, partial_test<partial_test_struct>, true, maxIterations, "Partial Test (Minified)">::run(jsonPartialDataNewer);
		newerString += testResults.markdownResults;
		benchmark_data.emplace_back(testResults);
		std::vector<abc_test<abc_test_struct>> abcDataNew{ maxIterations };
		for (size_t x = 0; x < maxIterations; ++x) {
			parser.serializeJson(jsonDataNewer[x], newStrings[x]);
			parser.parseJson(abcDataNew[x], newStrings[x]);
		}
		testResults = json_tests_helper<test_type::parse_and_serialize, abc_test<abc_test_struct>, false, maxIterations, "Abc (Out of Order) Test (Prettified)">::run(abcDataNew);
		newerString += testResults.markdownResults;
		benchmark_data.emplace_back(testResults);
		testResults = json_tests_helper<test_type::parse_and_serialize, abc_test<abc_test_struct>, true, maxIterations, "Abc (Out of Order) Test (Minified)">::run(abcDataNew);
		newerString += testResults.markdownResults;
		benchmark_data.emplace_back(testResults);
		{
			std::string discordData{ bnch_swt::file_loader::loadFile(jsonPath.operator std::string() + "/" + "/Discord Test (Prettified).json") };
			discord_message discordDataNew{};
			parser.parseJson<jsonifier::parse_options{ .knownOrder = true }>(discordDataNew, discordData);
			for (auto& value: parser.getErrors()) {
				std::cout << "Error: " << value << std::endl;
			}
			bnch_swt::file_loader::saveFile(discordData, jsonOutPath.operator std::string() + "/Discord Test (Prettified).json");
			std::string discordMinifiedData{ bnch_swt::file_loader::loadFile(jsonPath.operator std::string() + "/" + "/Discord Test (Minified).json") };
			bnch_swt::file_loader::saveFile(discordMinifiedData, jsonOutPath.operator std::string() + "/Discord Test (Minified).json");
			testResults = json_tests_helper<test_type::parse_and_serialize, discord_message, false, maxIterations, "Discord Test (Prettified)">::run(discordDataNew);
			newerString += testResults.markdownResults;
			benchmark_data.emplace_back(testResults);
			testResults = json_tests_helper<test_type::parse_and_serialize, discord_message, true, maxIterations, "Discord Test (Minified)">::run(discordDataNew);
			newerString += testResults.markdownResults;
			benchmark_data.emplace_back(testResults);
		}
		{
			std::string canadaData{ bnch_swt::file_loader::loadFile(jsonPath.operator std::string() + "/" + "/Canada Test (Prettified).json") };
			canada_message canadaDataNew{};
			parser.parseJson<jsonifier::parse_options{ .knownOrder = true }>(canadaDataNew, canadaData);
			for (auto& value: parser.getErrors()) {
				std::cout << "Error: " << value << std::endl;
			}
			bnch_swt::file_loader::saveFile(canadaData, jsonOutPath.operator std::string() + "/Canada Test (Prettified).json");
			std::string canadaMinifiedData{ bnch_swt::file_loader::loadFile(jsonPath.operator std::string() + "/" + "/Canada Test (Minified).json") };
			bnch_swt::file_loader::saveFile(canadaMinifiedData, jsonOutPath.operator std::string() + "/Canada Test (Minified).json");
			testResults = json_tests_helper<test_type::parse_and_serialize, canada_message, false, maxIterations, "Canada Test (Prettified)">::run(canadaDataNew);
			newerString += testResults.markdownResults;
			benchmark_data.emplace_back(testResults);
			testResults = json_tests_helper<test_type::parse_and_serialize, canada_message, true, maxIterations, "Canada Test (Minified)">::run(canadaDataNew);
			newerString += testResults.markdownResults;
			benchmark_data.emplace_back(testResults);
		}
		{
			std::string citmCatalogData{ bnch_swt::file_loader::loadFile(jsonPath.operator std::string() + "/" + "/CitmCatalog Test (Prettified).json") };
			citm_catalog_message citmCatalogDataNew{};
			parser.parseJson<jsonifier::parse_options{ .knownOrder = true }>(citmCatalogDataNew, citmCatalogData);
			for (auto& value: parser.getErrors()) {
				std::cout << "Error: " << value << std::endl;
			}
			bnch_swt::file_loader::saveFile(citmCatalogData, jsonOutPath.operator std::string() + "/CitmCatalog Test (Prettified).json");
			std::string citmCatalogMinifiedData{ bnch_swt::file_loader::loadFile(jsonPath.operator std::string() + "/" + "/CitmCatalog Test (Minified).json") };
			bnch_swt::file_loader::saveFile(citmCatalogMinifiedData, jsonOutPath.operator std::string() + "/CitmCatalog Test (Minified).json");
			testResults = json_tests_helper<test_type::parse_and_serialize, citm_catalog_message, false, maxIterations, "CitmCatalog Test (Prettified)">::run(citmCatalogDataNew);
			newerString += testResults.markdownResults;
			benchmark_data.emplace_back(testResults);
			testResults = json_tests_helper<test_type::parse_and_serialize, citm_catalog_message, true, maxIterations, "CitmCatalog Test (Minified)">::run(citmCatalogDataNew);
			newerString += testResults.markdownResults;
			benchmark_data.emplace_back(testResults);
		}
		{
			std::string twitterData{ bnch_swt::file_loader::loadFile(jsonPath.operator std::string() + "/" + "/Twitter Test (Prettified).json") };
			twitter_message twitterDataNew{};
			parser.parseJson<jsonifier::parse_options{ .knownOrder = true }>(twitterDataNew, twitterData);
			for (auto& value: parser.getErrors()) {
				std::cout << "Error: " << value << std::endl;
			}
			bnch_swt::file_loader::saveFile(twitterData, jsonOutPath.operator std::string() + "/Twitter Test (Prettified).json");
			std::string twitterMinifiedData{ bnch_swt::file_loader::loadFile(jsonPath.operator std::string() + "/" + "/Twitter Test (Minified).json") };
			bnch_swt::file_loader::saveFile(twitterMinifiedData, jsonOutPath.operator std::string() + "/Twitter Test (Minified).json");
			bnch_swt::file_loader::saveFile(twitterData, jsonOutPath.operator std::string() + "/Minify Test.json");
			bnch_swt::file_loader::saveFile(twitterMinifiedData, jsonOutPath.operator std::string() + "/Prettify Test.json");
			bnch_swt::file_loader::saveFile(twitterData, jsonOutPath.operator std::string() + "/Validate Test.json");
			twitter_partial_message twitterPartialDataNew{};
			parser.parseJson<jsonifier::parse_options{}>(twitterPartialDataNew, twitterData);
			testResults =
				json_tests_helper<test_type::parse_and_serialize, twitter_partial_message, false, maxIterations, "Twitter Partial Test (Prettified)">::run(twitterPartialDataNew);
			newerString += testResults.markdownResults;
			benchmark_data.emplace_back(testResults);
			testResults =
				json_tests_helper<test_type::parse_and_serialize, twitter_partial_message, true, maxIterations, "Twitter Partial Test (Minified)">::run(twitterPartialDataNew);
			newerString += testResults.markdownResults;
			benchmark_data.emplace_back(testResults);

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
		}
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