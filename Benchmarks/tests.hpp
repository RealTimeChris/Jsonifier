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

#include "jsonifier.hpp"

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

	static constexpr bnch_swt::stage_config config{
		.clear_cpu_cache_before_all_executions = true,
		.measured_execution_count			   = measured_executions,
		.max_execution_count				   = max_executions,
	};

	template<json_library lib, test_type type, typename test_data_type, bool minified, size_t iterations, const bnch_swt::string_literal test_name> struct json_test_helper;

	template<typename value_type>
	concept pod_type = std::is_same_v<std::vector<std::vector<bool>>, value_type> || std::is_same_v<std::vector<std::vector<std::string>>, value_type> ||
		std::is_same_v<std::vector<std::vector<int64_t>>, value_type> || std::is_same_v<std::vector<std::vector<uint64_t>>, value_type> ||
		std::is_same_v<std::vector<std::vector<double>>, value_type>;

	template<typename value_type>
	concept iterative_test_type = std::is_same_v<test<test_struct>, value_type> || std::is_same_v<abc_test<abc_test_struct>, value_type> ||
		std::is_same_v<partial_test<partial_test_struct>, value_type> || std::is_same_v<test_struct, value_type>;

	static constexpr bnch_swt::string_literal stage_name{ "Json-Performance" };

	using benchmark_stage = bnch_swt::benchmark_stage<stage_name, config>;

	template<pod_type test_data_type, bool minified, size_t iterations, const bnch_swt::string_literal test_name_new>
	struct json_test_helper<json_library::jsonifier, test_type::parse_and_serialize, test_data_type, minified, iterations, test_name_new> {
		static auto run(const test_data_type& test_data_new) {
			static constexpr bnch_swt::string_literal test_name{ test_name_new };
			static constexpr bnch_swt::string_literal test_name_read{ test_name + "-Read" };
			static constexpr bnch_swt::string_literal test_name_write{ test_name + "-Write" };
			static constexpr bool partial_read{ std::is_same_v<test_data_type, partial_test<partial_test_struct>> || std::is_same_v<test_data_type, twitter_partial_message> };
			static constexpr bool known_order{ true };
			results_data r{ jsonifier_library_name, test_name, jsonifier_commit_url };
			jsonifier::jsonifier_core parser{};
			std::vector<std::vector<std::string>> new_strings{ iterations };
			for (size_t x = 0; x < iterations; ++x) {
				new_strings[x].resize(test_data_new[x].size());
			}
			for (size_t x = 0; x < iterations; ++x) {
				for (size_t y = 0; y < test_data_new[x].size(); ++y) {
					parser.serializeJson<jsonifier::serialize_options{ .prettify = !minified }>(test_data_new[x][y], new_strings[x][y]);
				}
			}
			test_data_type test_datas{ iterations };
			for (size_t x = 0; x < iterations; ++x) {
				test_datas[x].resize(test_data_new[x].size());
			}
			size_t current_index{};
			struct parse_test_struct {
				static size_t impl(jsonifier::jsonifier_core<>& parser_new, test_data_type& test_datas, size_t& current_index,
					std::vector<std::vector<std::string>>& new_strings) {
					current_index %= test_datas.size();
					size_t new_size{};
					for (size_t x = 0; x < test_datas[current_index].size(); ++x) {
						parser_new.parseJson<jsonifier::parse_options{ .partialRead = partial_read, .knownOrder = known_order, .minified = minified }>(test_datas[current_index][x],
							new_strings[current_index][x]);
						bnch_swt::do_not_optimize_away(test_datas[current_index]);
						new_size += new_strings[current_index][x].size();
					}
					++current_index;
					return new_size;
				}
			};
			auto read_result = benchmark_stage::template run_benchmark<test_name_read, jsonifier_library_name, parse_test_struct>(parser, test_datas, current_index, new_strings);
			for (auto& value: parser.getErrors()) {
				std::cout << "Jsonifier Error: " << value << std::endl;
			}
			for (size_t x = 0; x < iterations; ++x) {
				for (size_t y = 0; y < test_data_new[x].size(); ++y) {
					new_strings[x][y] = std::string{};
				}
			}
			current_index = 0;
			struct serialize_test_struct {
				static size_t impl(jsonifier::jsonifier_core<>& parser_new, test_data_type& test_datas, size_t& current_index, std::vector<std::vector<std::string>>& new_strings) {
					current_index %= test_datas.size();
					size_t new_size{};
					for (size_t x = 0; x < test_datas[current_index].size(); ++x) {
						parser_new.serializeJson<jsonifier::serialize_options{ .prettify = !minified }>(test_datas[current_index][x], new_strings[current_index][x]);
						bnch_swt::do_not_optimize_away(new_strings[current_index][x]);
						new_size += new_strings[current_index][x].size();
					}
					++current_index;
					return new_size;
				}
			};
			auto write_result =
				benchmark_stage::template run_benchmark<test_name_write, jsonifier_library_name, serialize_test_struct>(parser, test_datas, current_index, new_strings);
			r.read_result  = result<result_type::read>{ "teal", read_result };
			r.write_result = result<result_type::write>{ "steelblue", write_result };
			bnch_swt::file_handle::save_file(static_cast<std::string>(parser.serializeJson(test_datas)),
				json_out_path.operator std::string() + "/" + test_name.operator std::string() + "-jsonifier.json");
			return r;
		}
	};

	template<iterative_test_type test_data_type, bool minified, size_t iterations, const bnch_swt::string_literal test_name_new>
	struct json_test_helper<json_library::jsonifier, test_type::parse_and_serialize, test_data_type, minified, iterations, test_name_new> {
		static auto run(const std::vector<test_data_type>& test_data_new) {
			static constexpr bnch_swt::string_literal test_name{ test_name_new };
			static constexpr bnch_swt::string_literal test_name_read{ test_name + "-Read" };
			static constexpr bnch_swt::string_literal test_name_write{ test_name + "-Write" };
			static constexpr bool partial_read{ std::is_same_v<test_data_type, partial_test<partial_test_struct>> || std::is_same_v<test_data_type, twitter_partial_message> };
			static constexpr bool known_order{ true };
			results_data r{ jsonifier_library_name, test_name, jsonifier_commit_url };
			jsonifier::jsonifier_core parser{};
			std::vector<std::string> new_strings{ iterations };
			for (size_t x = 0; x < iterations; ++x) {
				parser.serializeJson<jsonifier::serialize_options{ .prettify = !minified }>(test_data_new[x], new_strings[x]);
			}
			std::vector<test_data_type> test_datas{ iterations };
			size_t current_index{};
			struct parse_test_struct {
				static size_t impl(jsonifier::jsonifier_core<>& parser_new, std::vector<test_data_type>& test_datas, size_t& current_index, std::vector<std::string>& new_strings) {
					current_index %= test_datas.size();
					parser_new.parseJson<jsonifier::parse_options{ .partialRead = partial_read, .knownOrder = known_order, .minified = minified }>(test_datas[current_index],
						new_strings[current_index]);
					bnch_swt::do_not_optimize_away(test_datas[current_index]);
					auto new_size = new_strings[current_index].size();
					++current_index;
					return new_size;
				}
			};
			auto read_result = benchmark_stage::template run_benchmark<test_name_read, jsonifier_library_name, parse_test_struct>(parser, test_datas, current_index, new_strings);
			for (auto& value: parser.getErrors()) {
				std::cout << "Jsonifier Error: " << value << std::endl;
			}
			for (size_t x = 0; x < iterations; ++x) {
				new_strings[x] = std::string{};
			}
			current_index = 0;
			struct serialize_test_struct {
				static size_t impl(jsonifier::jsonifier_core<>& parser_new, std::vector<test_data_type>& test_datas, size_t& current_index, std::vector<std::string>& new_strings) {
					current_index %= test_datas.size();
					parser_new.serializeJson<jsonifier::serialize_options{ .prettify = !minified }>(test_datas[current_index], new_strings[current_index]);
					bnch_swt::do_not_optimize_away(new_strings[current_index]);
					auto new_size = new_strings[current_index].size();
					++current_index;
					return new_size;
				}
			};
			auto write_result =
				benchmark_stage::template run_benchmark<test_name_write, jsonifier_library_name, serialize_test_struct>(parser, test_datas, current_index, new_strings);
			r.read_result  = result<result_type::read>{ "teal", read_result };
			r.write_result = result<result_type::write>{ "steelblue", write_result };
			bnch_swt::file_handle::save_file(static_cast<std::string>(new_strings[0]),
				json_out_path.operator std::string() + "/" + test_name.operator std::string() + "-jsonifier.json");
			return r;
		}
	};

	template<typename test_data_type, bool minified, size_t iterations, const bnch_swt::string_literal test_name_new>
	struct json_test_helper<json_library::jsonifier, test_type::parse_and_serialize, test_data_type, minified, iterations, test_name_new> {
		static auto run(const test_data_type& test_data_new) {
			static constexpr bnch_swt::string_literal test_name{ test_name_new };
			static constexpr bnch_swt::string_literal test_name_read{ test_name + "-Read" };
			static constexpr bnch_swt::string_literal test_name_write{ test_name + "-Write" };
			static constexpr bool partial_read{ std::is_same_v<test_data_type, partial_test<partial_test_struct>> || std::is_same_v<test_data_type, twitter_partial_message> };
			static constexpr bool known_order{ true };
			results_data r{ jsonifier_library_name, test_name, jsonifier_commit_url };
			jsonifier::jsonifier_core parser{};
			std::string new_string{ parser.serializeJson<jsonifier::serialize_options{ .prettify = !minified }>(test_data_new) };
			std::vector<test_data_type> test_datas{ iterations };
			std::vector<std::string> new_strings{ iterations };
			size_t current_index{};
			struct parse_test_struct {
				static size_t impl(jsonifier::jsonifier_core<>& parser_new, std::vector<test_data_type>& test_datas, size_t& current_index, std::string& new_string) {
					current_index %= test_datas.size();
					parser_new.parseJson<jsonifier::parse_options{ .partialRead = partial_read, .knownOrder = known_order, .minified = minified }>(test_datas[current_index],
						new_string);
					bnch_swt::do_not_optimize_away(test_datas[current_index]);
					auto new_size = new_string.size();
					++current_index;
					return new_size;
				}
			};
			auto read_result = benchmark_stage::template run_benchmark<test_name_read, jsonifier_library_name, parse_test_struct>(parser, test_datas, current_index, new_string);
			for (auto& value: parser.getErrors()) {
				std::cout << "Jsonifier Error: " << value << std::endl;
			}
			for (size_t x = 0; x < iterations; ++x) {
				test_datas[x] = test_datas[0];
			}
			current_index = 0;
			struct serialize_test_struct {
				static size_t impl(jsonifier::jsonifier_core<>& parser_new, std::vector<test_data_type>& test_datas, size_t& current_index, std::vector<std::string>& new_strings) {
					current_index %= test_datas.size();
					parser_new.serializeJson<jsonifier::serialize_options{ .prettify = !minified }>(test_datas[current_index], new_strings[current_index]);
					bnch_swt::do_not_optimize_away(new_strings[current_index]);
					auto new_size = new_strings[current_index].size();
					++current_index;
					return new_size;
				}
			};
			auto write_result =
				benchmark_stage::template run_benchmark<test_name_write, jsonifier_library_name, serialize_test_struct>(parser, test_datas, current_index, new_strings);
			for (auto& value: parser.getErrors()) {
				std::cout << "Jsonifier Error: " << value << std::endl;
			}
			r.read_result  = result<result_type::read>{ "teal", read_result };
			r.write_result = result<result_type::write>{ "steelblue", write_result };
			bnch_swt::file_handle::save_file(static_cast<std::string>(new_strings[0]),
				json_out_path.operator std::string() + "/" + test_name.operator std::string() + "-jsonifier.json");
			return r;
		}
	};

	template<size_t iterations, bnch_swt::string_literal test_name_new>
	struct json_test_helper<json_library::jsonifier, test_type::prettify, std::string, false, iterations, test_name_new> {
		static auto run(std::string& new_buffer) {
			static constexpr bnch_swt::string_literal test_name{ test_name_new };
			results_data r{ jsonifier_library_name, test_name, jsonifier_commit_url };
			jsonifier::jsonifier_core parser{};
			std::vector<std::string> new_strings{ iterations };
			size_t current_index{};
			struct prettify_test_struct {
				static size_t impl(jsonifier::jsonifier_core<>& parser_new, size_t& current_index, std::vector<std::string>& new_strings, std::string& newer_buffer) {
					current_index %= new_strings.size();
					parser_new.prettifyJson(newer_buffer, new_strings[current_index]);
					bnch_swt::do_not_optimize_away(new_strings[current_index]);
					auto new_size = new_strings[current_index].size();
					++current_index;
					return new_size;
				}
			};
			auto write_result = benchmark_stage::template run_benchmark<test_name, jsonifier_library_name, prettify_test_struct>(parser, current_index, new_strings, new_buffer);

			for (auto& value: parser.getErrors()) {
				std::cout << "Jsonifier Error: " << value << std::endl;
			}
			bnch_swt::file_handle::save_file(new_strings[0], json_out_path.operator std::string() + "/" + test_name.operator std::string() + "-jsonifier.json");
			r.write_result = result<result_type::write>{ "steelblue", write_result };
			return r;
		}
	};

	template<size_t iterations, bnch_swt::string_literal test_name_new>
	struct json_test_helper<json_library::jsonifier, test_type::minify, std::string, false, iterations, test_name_new> {
		static auto run(std::string& new_buffer) {
			static constexpr bnch_swt::string_literal test_name{ test_name_new };
			std::vector<std::string> new_strings{ iterations };
			results_data r{ jsonifier_library_name, test_name, jsonifier_commit_url };
			jsonifier::jsonifier_core parser{};
			size_t current_index{};
			struct minify_test_struct {
				static size_t impl(jsonifier::jsonifier_core<>& parser_new, size_t& current_index, std::vector<std::string>& new_strings, std::string& newer_buffer) {
					current_index %= new_strings.size();
					parser_new.minifyJson(newer_buffer, new_strings[current_index]);
					bnch_swt::do_not_optimize_away(new_strings[current_index]);
					auto new_size = new_strings[current_index].size();
					++current_index;
					return new_size;
				}
			};
			auto write_result = benchmark_stage::template run_benchmark<test_name, jsonifier_library_name, minify_test_struct>(parser, current_index, new_strings, new_buffer);
			for (auto& value: parser.getErrors()) {
				std::cout << "Jsonifier Error: " << value << std::endl;
			}
			bnch_swt::file_handle::save_file(new_strings[0], json_out_path.operator std::string() + "/" + test_name.operator std::string() + "-jsonifier.json");
			r.write_result = result<result_type::write>{ "steelblue", write_result };
			return r;
		}
	};

	template<size_t iterations, bnch_swt::string_literal test_name_new>
	struct json_test_helper<json_library::jsonifier, test_type::validate, std::string, false, iterations, test_name_new> {
		static auto run(std::string& new_buffer) {
			static constexpr bnch_swt::string_literal test_name{ test_name_new };
			results_data r{ jsonifier_library_name, test_name, jsonifier_commit_url };
			struct validate_test_struct {
				static size_t impl(jsonifier::jsonifier_core<>& parser_new, std::string& newer_buffer) {
					if (auto result = parser_new.validateJson(newer_buffer); !result) {
						bnch_swt::do_not_optimize_away(result);
						return uint64_t{};
					}
					return static_cast<uint64_t>(newer_buffer.size());
				}
			};
			jsonifier::jsonifier_core parser{};
			auto read_result = benchmark_stage::template run_benchmark<test_name, jsonifier_library_name, validate_test_struct>(parser, new_buffer);

			for (auto& value: parser.getErrors()) {
				std::cout << "Jsonifier Error: " << value << std::endl;
			}
			bnch_swt::file_handle::save_file(new_buffer, json_out_path.operator std::string() + "/" + test_name.operator std::string() + "-jsonifier.json");
			r.read_result = result<result_type::read>{ "teal", read_result };
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

#if JSONIFIER_PLATFORM_MAC
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

	std::string generate_section_01(const auto& cpu_name) {
		return std::string{ R"(
 > )" + std::to_string(max_executions) +
			R"( iterations on a ()" + cpu_name + R"(), where the most stable 20 subsequent iterations are sampled.

#### Note:
These benchmarks were executed using the CPU benchmark library [benchmarksuite](https://github.com/RealTimeChris/benchmarksuite).)" };
	}

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

	std::string generate_section(const auto& test_name_new, const auto& current_path_new) {
		std::string test_name{ static_cast<std::string>(test_name_new) };
		std::string current_path{ static_cast<std::string>(current_path_new) };
		return R"(

### )" + test_name +
			R"( Results [(View the data used in the following test)](./Json/)" + current_path + R"(/)" + url_encode(test_name) +
			R"(.json):

----
<p align="left"><a href="./Graphs/)" +
			current_path + R"(/)" + url_encode(test_name) + R"(_Results.png" target="_blank"><img src="./Graphs/)" + current_path + R"(/)" + url_encode(test_name) +
			R"(_Results.png?raw=true" 
alt="" width="400"/></p>
<p align="left"><a href="./Graphs/)" +
			current_path + R"(/)" + url_encode(test_name) + R"(_Cumulative_Speedup.png" target="_blank"><img src="./Graphs/)" + current_path + R"(/)" + test_name +
			R"(_Cumulative_Speedup.png?raw=true" 
alt="" width="400"/></p>

)" + (test_name.find("Abc (Out of Order) Test (Prettified)") == std::string::npos ? "" : "The JSON documents in the previous tests featured keys ranging from \"a\" to \"z\",\
 where each key corresponds to an array of values. Notably, the documents in this test arrange these keys in reverse order, deviating from the typical \"a\" to \"z\" arrangement.\n\n\
This test effectively demonstrates the challenges encountered when utilizing simdjson and iterative parsers that lack the ability to efficiently allocate memory locations through\
hashing. In cases where the keys are not in the expected sequence, performance is significantly compromised, with the severity escalating as the document size increases.\n\n\
In contrast, hash-based solutions offer a viable alternative by circumventing these issues and maintaining optimal performance regardless of the JSON document's scale, or ordering of the keys being parsed.\n");
	}

	struct ranked_result {
		results_data* data{};
		size_t rank{};
		bool tied{};
		size_t tie_count{};
	};

	inline double tie_throughput(const results_data& r) {
		if (r.read_result.json_speed.has_value()) {
			return r.read_result.json_speed.value();
		}
		if (r.write_result.json_speed.has_value()) {
			return r.write_result.json_speed.value();
		}
		return 0.0;
	}

	inline double tie_deviation(const results_data& r) {
		if (r.read_result.json_speed.has_value()) {
			return std::abs(r.read_result.json_speed_percentage_deviation.value()) / 100.0;
		}
		if (r.write_result.json_speed.has_value()) {
			return std::abs(r.write_result.json_speed_percentage_deviation.value()) / 100.0;
		}
		return 0.0;
	}

	inline std::vector<ranked_result> rank_with_ties(std::vector<results_data>& results) {
		static constexpr double confidence_multiplier = 1.96;
		static constexpr double epsilon				  = 1e-12;

		auto ranges_overlap = [&](const results_data* a, const results_data* b) -> bool {
			double a_stddev = tie_deviation(*a);
			double b_stddev = tie_deviation(*b);
			double a_tp		= tie_throughput(*a);
			double b_tp		= tie_throughput(*b);

			if (a_stddev < epsilon || b_stddev < epsilon) {
				return std::abs(a_tp - b_tp) < epsilon;
			}

			double a_min = a_tp * (1.0 - a_stddev * confidence_multiplier);
			double a_max = a_tp * (1.0 + a_stddev * confidence_multiplier);
			double b_min = b_tp * (1.0 - b_stddev * confidence_multiplier);
			double b_max = b_tp * (1.0 + b_stddev * confidence_multiplier);

			return !(a_max + epsilon < b_min || b_max + epsilon < a_min);
		};

		std::vector<results_data*> sorted;
		sorted.reserve(results.size());
		for (auto& r: results) {
			sorted.push_back(&r);
		}

		std::sort(sorted.begin(), sorted.end(), [&](auto a, auto b) {
			return tie_throughput(*a) > tie_throughput(*b);
		});

		std::vector<std::vector<size_t>> groups;
		std::vector<bool> grouped(sorted.size(), false);

		for (size_t i = 0; i < sorted.size(); ++i) {
			if (grouped[i]) {
				continue;
			}

			std::vector<size_t> group;
			group.push_back(i);
			grouped[i] = true;

			for (size_t j = i + 1; j < sorted.size(); ++j) {
				if (grouped[j]) {
					continue;
				}

				bool overlaps_group = false;
				for (size_t member: group) {
					if (ranges_overlap(sorted[member], sorted[j])) {
						overlaps_group = true;
						break;
					}
				}

				if (overlaps_group) {
					group.push_back(j);
					grouped[j] = true;
				}
			}

			groups.push_back(std::move(group));
		}

		std::sort(groups.begin(), groups.end(), [&](const auto& a, const auto& b) {
			double a_throughput = 0, b_throughput = 0;
			for (size_t idx: a) {
				a_throughput += tie_throughput(*sorted[idx]);
			}
			for (size_t idx: b) {
				b_throughput += tie_throughput(*sorted[idx]);
			}
			return (a_throughput / a.size()) > (b_throughput / b.size());
		});

		std::vector<ranked_result> out;
		out.reserve(sorted.size());
		size_t rank = 1;
		for (const auto& group: groups) {
			bool is_tied = group.size() > 1;
			for (size_t idx: group) {
				out.push_back(ranked_result{ sorted[idx], rank, is_tied, group.size() });
			}
			rank += group.size();
		}
		return out;
	}

	inline void append_markdown_row(std::string& md, const ranked_result& rr, bool last) {
		std::string row = rr.data->json_stats();
		if (rr.tied) {
			std::string marker = " **STATISTICAL TIE";
			if (rr.tie_count > 2) {
				marker += " (" + std::to_string(rr.tie_count) + "-way)";
			}
			marker += "**";

			auto first = row.find('|');
			if (first != std::string::npos) {
				auto second = row.find('|', first + 1);
				if (second != std::string::npos) {
					row.insert(second, marker + " ");
				} else {
					row += marker;
				}
			} else {
				row += marker;
			}
		}
		md += row;
		if (!last) {
			md += "\n";
		}
	}

	inline void append_ranked_rows(std::string& md, std::vector<results_data>& results) {
		auto ranked = rank_with_ties(results);
		for (size_t i = 0; i < ranked.size(); ++i) {
			append_markdown_row(md, ranked[i], i == ranked.size() - 1);
		}
	}

	template<test_type type, typename test_data_type, bool minified, uint64_t iterations, bnch_swt::string_literal test_name> struct json_tests_helper;

	template<test_type type, typename test_data_type, bool minified, uint64_t iterations, bnch_swt::string_literal test_name_new> struct json_tests_helper {
		template<typename test_data_type_new> static test_results run(const test_data_type_new& json_data_new) {
			static constexpr bnch_swt::string_literal test_name{ test_name_new };
			test_results json_results{};
			json_results.test_name = test_name.operator std::string();
			results_data jsonifier_results{};
			results_data simdjson_results{};
			results_data glaze_results{};
			jsonifier_results = json_test_helper<json_library::jsonifier, type, test_data_type, minified, iterations, test_name>::run(json_data_new);
			json_results.results.emplace_back(jsonifier_results);
			json_results.markdown_results += generate_section(test_name, current_path);
			json_results.markdown_results += table_header.operator std::string() + "\n";
			append_ranked_rows(json_results.markdown_results, json_results.results);
			bnch_swt::benchmark_stage<stage_name, config>::print_results();
			bnch_swt::benchmark_stage<stage_name, config>::clear_all_results();
			return json_results;
		}
	};

	template<uint64_t iterations, bnch_swt::string_literal test_name_new> struct json_tests_helper<test_type::prettify, std::string, false, iterations, test_name_new> {
		static test_results run(std::string& json_data_new) {
			static constexpr bnch_swt::string_literal test_name{ test_name_new };
			test_results json_results{};
			json_results.test_name = test_name.operator std::string();
			results_data jsonifier_results{};
			results_data glaze_results{};
			jsonifier_results = json_test_helper<json_library::jsonifier, test_type::prettify, std::string, false, iterations, test_name>::run(json_data_new);
			json_results.results.emplace_back(jsonifier_results);
			json_results.markdown_results += generate_section(test_name, current_path);
			json_results.markdown_results += write_table_header.operator std::string() + "\n";
			append_ranked_rows(json_results.markdown_results, json_results.results);
			bnch_swt::benchmark_stage<stage_name, config>::print_results();
			bnch_swt::benchmark_stage<stage_name, config>::clear_all_results();
			return json_results;
		}
	};

	template<uint64_t iterations, bnch_swt::string_literal test_name_new> struct json_tests_helper<test_type::minify, std::string, false, iterations, test_name_new> {
		static test_results run(std::string& json_data_new) {
			static constexpr bnch_swt::string_literal test_name{ test_name_new };
			test_results json_results{};
			json_results.test_name = test_name.operator std::string();
			results_data jsonifier_results{};
			results_data simdjson_results{};
			results_data glaze_results{};
			jsonifier_results = json_test_helper<json_library::jsonifier, test_type::minify, std::string, false, iterations, test_name>::run(json_data_new);
			json_results.results.emplace_back(jsonifier_results);
			json_results.markdown_results += generate_section(test_name, current_path);
			json_results.markdown_results += write_table_header.operator std::string() + "\n";
			append_ranked_rows(json_results.markdown_results, json_results.results);
			bnch_swt::benchmark_stage<stage_name, config>::print_results();
			bnch_swt::benchmark_stage<stage_name, config>::clear_all_results();
			return json_results;
		}
	};

	template<uint64_t iterations, bnch_swt::string_literal test_name_new> struct json_tests_helper<test_type::validate, std::string, false, iterations, test_name_new> {
		static test_results run(std::string& json_data_new) {
			static constexpr bnch_swt::string_literal test_name{ test_name_new };
			test_results json_results{};
			json_results.test_name = test_name.operator std::string();
			results_data jsonifier_results{};
			results_data glaze_results{};
			jsonifier_results = json_test_helper<json_library::jsonifier, test_type::validate, std::string, false, iterations, test_name>::run(json_data_new);
			json_results.results.emplace_back(jsonifier_results);
			json_results.markdown_results += generate_section(test_name.operator std::string(), current_path.operator std::string());
			json_results.markdown_results += read_table_header.operator std::string() + "\n";
			append_ranked_rows(json_results.markdown_results, json_results.results);
			bnch_swt::benchmark_stage<stage_name, config>::print_results();
			bnch_swt::benchmark_stage<stage_name, config>::clear_all_results();
			return json_results;
		}
	};

	void test_function() {
		std::string json_data_new{};
		jsonifier::jsonifier_core parser{};
		std::vector<test<test_struct>> json_data_newer{ max_executions };
		for (size_t x = 0; x < max_executions; ++x) {
			json_data_newer[x] = test_generator::generate_test();
		}
		parser.serializeJson<jsonifier::serialize_options{ .prettify = true }>(json_data_newer[0], json_data_new);
		bnch_swt::file_handle::save_file(json_data_new, json_out_path.operator std::string() + "/Json Test (Prettified).json");
		std::string json_minified_data = parser.minifyJson(json_data_new);
		bnch_swt::file_handle::save_file(json_minified_data, json_out_path.operator std::string() + "/Json Test (Minified).json");
		bnch_swt::file_handle::save_file(json_data_new, json_out_path.operator std::string() + "/Partial Test (Prettified).json");
		bnch_swt::file_handle::save_file(json_minified_data, json_out_path.operator std::string() + "/Partial Test (Minified).json");
		bnch_swt::file_handle::save_file(json_data_new, json_out_path.operator std::string() + "/Abc (Out of Order) Test (Prettified).json");
		bnch_swt::file_handle::save_file(json_minified_data, json_out_path.operator std::string() + "/Abc (Out of Order) Test (Minified).json");
		std::string new_time_string{};
		new_time_string.resize(1024);
		std::tm result_two{ get_time() };
		std::vector<test_results> benchmark_data{};
		new_time_string.resize(strftime(new_time_string.data(), 1024, "%b %d, %Y", &result_two));
		std::string newer_string{ section00.operator std::string() + new_time_string + ")\n" + static_cast<std::string>(section002.operator std::string()) +
			static_cast<std::string>(generate_section_01(bnch_swt::internal::get_device_info<bnch_swt::benchmark_types::cpu>())) };
		test_results test_results{};
		{
			std::vector<std::vector<double>> double_data{ test_generator::generate_values<double>(max_executions, 100) };
			parser.serializeJson<jsonifier::serialize_options{ .prettify = true }>(double_data, json_data_new);
			bnch_swt::file_handle::save_file(json_data_new, json_out_path.operator std::string() + "/Double Test.json");
			test_results = json_tests_helper<test_type::parse_and_serialize, std::vector<std::vector<double>>, false, max_executions, "Double Test">::run(double_data);
			newer_string += test_results.markdown_results;
			benchmark_data.emplace_back(test_results);
		}
		{
			std::vector<std::vector<std::string>> string_data{ test_generator::generate_values<std::string>(max_executions, 100) };
			parser.serializeJson<jsonifier::serialize_options{ .prettify = true }>(string_data, json_data_new);
			bnch_swt::file_handle::save_file(json_data_new, json_out_path.operator std::string() + "/String Test.json");
			test_results = json_tests_helper<test_type::parse_and_serialize, std::vector<std::vector<std::string>>, false, max_executions, "String Test">::run(string_data);
			newer_string += test_results.markdown_results;
			benchmark_data.emplace_back(test_results);
		}
		{
			std::vector<std::vector<uint64_t>> uint_data{ test_generator::generate_values<uint64_t>(max_executions, 100) };
			parser.serializeJson<jsonifier::serialize_options{ .prettify = true }>(uint_data, json_data_new);
			bnch_swt::file_handle::save_file(json_data_new, json_out_path.operator std::string() + "/Uint Test.json");
			test_results = json_tests_helper<test_type::parse_and_serialize, std::vector<std::vector<uint64_t>>, false, max_executions, "Uint Test">::run(uint_data);
			newer_string += test_results.markdown_results;
			benchmark_data.emplace_back(test_results);
		}
		{
			std::vector<std::vector<int64_t>> int_data{ test_generator::generate_values<int64_t>(max_executions, 100) };
			parser.serializeJson<jsonifier::serialize_options{ .prettify = true }>(int_data, json_data_new);
			bnch_swt::file_handle::save_file(json_data_new, json_out_path.operator std::string() + "/Int Test.json");
			test_results = json_tests_helper<test_type::parse_and_serialize, std::vector<std::vector<int64_t>>, false, max_executions, "Int Test">::run(int_data);
			newer_string += test_results.markdown_results;
			benchmark_data.emplace_back(test_results);
		}
		{
			std::vector<std::vector<bool>> bool_data{ test_generator::generate_values<bool>(max_executions, 100) };
			parser.serializeJson<jsonifier::serialize_options{ .prettify = true }>(bool_data, json_data_new);
			bnch_swt::file_handle::save_file(json_data_new, json_out_path.operator std::string() + "/Bool Test.json");
			test_results = json_tests_helper<test_type::parse_and_serialize, std::vector<std::vector<bool>>, false, max_executions, "Bool Test">::run(bool_data);
			newer_string += test_results.markdown_results;
			benchmark_data.emplace_back(test_results);
		}
		test_results = json_tests_helper<test_type::parse_and_serialize, test<test_struct>, false, max_executions, "Json Test (Prettified)">::run(json_data_newer);
		newer_string += test_results.markdown_results;
		benchmark_data.emplace_back(test_results);
		test_results = json_tests_helper<test_type::parse_and_serialize, test<test_struct>, true, max_executions, "Json Test (Minified)">::run(json_data_newer);
		newer_string += test_results.markdown_results;
		benchmark_data.emplace_back(test_results);
		std::vector<partial_test<partial_test_struct>> json_partial_data_newer{ max_executions };
		std::vector<std::string> new_strings{ max_executions };
		for (size_t x = 0; x < max_executions; ++x) {
			parser.serializeJson(json_data_newer[x], new_strings[x]);
			parser.parseJson(json_partial_data_newer[x], new_strings[x]);
		}
		test_results =
			json_tests_helper<test_type::parse_and_serialize, partial_test<partial_test_struct>, false, max_executions, "Partial Test (Prettified)">::run(json_partial_data_newer);
		newer_string += test_results.markdown_results;
		benchmark_data.emplace_back(test_results);
		test_results =
			json_tests_helper<test_type::parse_and_serialize, partial_test<partial_test_struct>, true, max_executions, "Partial Test (Minified)">::run(json_partial_data_newer);
		newer_string += test_results.markdown_results;
		benchmark_data.emplace_back(test_results);
		std::vector<abc_test<abc_test_struct>> abc_data_new{ max_executions };
		for (size_t x = 0; x < max_executions; ++x) {
			parser.serializeJson(json_data_newer[x], new_strings[x]);
			parser.parseJson(abc_data_new[x], new_strings[x]);
		}
		test_results =
			json_tests_helper<test_type::parse_and_serialize, abc_test<abc_test_struct>, false, max_executions, "Abc (Out of Order) Test (Prettified)">::run(abc_data_new);
		newer_string += test_results.markdown_results;
		benchmark_data.emplace_back(test_results);
		test_results = json_tests_helper<test_type::parse_and_serialize, abc_test<abc_test_struct>, true, max_executions, "Abc (Out of Order) Test (Minified)">::run(abc_data_new);
		newer_string += test_results.markdown_results;
		benchmark_data.emplace_back(test_results);
		{
			std::string discord_data{ bnch_swt::file_handle::get(json_path.operator std::string() + "/" + "/Discord Test (Prettified).json") };
			discord_message discord_data_new{};
			parser.parseJson<jsonifier::parse_options{ .knownOrder = true }>(discord_data_new, discord_data);
			for (auto& value: parser.getErrors()) {
				std::cout << "Error: " << value << std::endl;
			}
			bnch_swt::file_handle::save_file(discord_data, json_out_path.operator std::string() + "/Discord Test (Prettified).json");
			std::string discord_minified_data{ bnch_swt::file_handle::get(json_path.operator std::string() + "/" + "/Discord Test (Minified).json") };
			bnch_swt::file_handle::save_file(discord_minified_data, json_out_path.operator std::string() + "/Discord Test (Minified).json");
			test_results = json_tests_helper<test_type::parse_and_serialize, discord_message, false, max_executions, "Discord Test (Prettified)">::run(discord_data_new);
			newer_string += test_results.markdown_results;
			benchmark_data.emplace_back(test_results);
			test_results = json_tests_helper<test_type::parse_and_serialize, discord_message, true, max_executions, "Discord Test (Minified)">::run(discord_data_new);
			newer_string += test_results.markdown_results;
			benchmark_data.emplace_back(test_results);
		}
		{
			std::string canada_data{ bnch_swt::file_handle::get(json_path.operator std::string() + "/" + "/Canada Test (Prettified).json") };
			canada_message canada_data_new{};
			parser.parseJson<jsonifier::parse_options{ .knownOrder = true }>(canada_data_new, canada_data);
			for (auto& value: parser.getErrors()) {
				std::cout << "Error: " << value << std::endl;
			}
			bnch_swt::file_handle::save_file(canada_data, json_out_path.operator std::string() + "/Canada Test (Prettified).json");
			std::string canada_minified_data{ bnch_swt::file_handle::get(json_path.operator std::string() + "/" + "/Canada Test (Minified).json") };
			bnch_swt::file_handle::save_file(canada_minified_data, json_out_path.operator std::string() + "/Canada Test (Minified).json");
			test_results = json_tests_helper<test_type::parse_and_serialize, canada_message, false, max_executions, "Canada Test (Prettified)">::run(canada_data_new);
			newer_string += test_results.markdown_results;
			benchmark_data.emplace_back(test_results);
			test_results = json_tests_helper<test_type::parse_and_serialize, canada_message, true, max_executions, "Canada Test (Minified)">::run(canada_data_new);
			newer_string += test_results.markdown_results;
			benchmark_data.emplace_back(test_results);
		}
		{
			std::string citm_catalog_data{ bnch_swt::file_handle::get(json_path.operator std::string() + "/" + "/CitmCatalog Test (Prettified).json") };
			citm_catalog_message citm_catalog_data_new{};
			parser.parseJson<jsonifier::parse_options{ .knownOrder = true }>(citm_catalog_data_new, citm_catalog_data);
			for (auto& value: parser.getErrors()) {
				std::cout << "Error: " << value << std::endl;
			}
			bnch_swt::file_handle::save_file(citm_catalog_data, json_out_path.operator std::string() + "/CitmCatalog Test (Prettified).json");
			std::string citm_catalog_minified_data{ bnch_swt::file_handle::get(json_path.operator std::string() + "/" + "/CitmCatalog Test (Minified).json") };
			bnch_swt::file_handle::save_file(citm_catalog_minified_data, json_out_path.operator std::string() + "/CitmCatalog Test (Minified).json");
			test_results =
				json_tests_helper<test_type::parse_and_serialize, citm_catalog_message, false, max_executions, "CitmCatalog Test (Prettified)">::run(citm_catalog_data_new);
			newer_string += test_results.markdown_results;
			benchmark_data.emplace_back(test_results);
			test_results = json_tests_helper<test_type::parse_and_serialize, citm_catalog_message, true, max_executions, "CitmCatalog Test (Minified)">::run(citm_catalog_data_new);
			newer_string += test_results.markdown_results;
			benchmark_data.emplace_back(test_results);
		}
		{
			std::string twitter_data{ bnch_swt::file_handle::get(json_path.operator std::string() + "/" + "/Twitter Test (Prettified).json") };
			twitter_message twitter_data_new{};
			parser.parseJson<jsonifier::parse_options{ .knownOrder = true }>(twitter_data_new, twitter_data);
			for (auto& value: parser.getErrors()) {
				std::cout << "Error: " << value << std::endl;
			}
			bnch_swt::file_handle::save_file(twitter_data, json_out_path.operator std::string() + "/Twitter Test (Prettified).json");
			std::string twitter_minified_data{ bnch_swt::file_handle::get(json_path.operator std::string() + "/" + "/Twitter Test (Minified).json") };
			bnch_swt::file_handle::save_file(twitter_minified_data, json_out_path.operator std::string() + "/Twitter Test (Minified).json");
			bnch_swt::file_handle::save_file(twitter_data, json_out_path.operator std::string() + "/Minify Test.json");
			bnch_swt::file_handle::save_file(twitter_minified_data, json_out_path.operator std::string() + "/Prettify Test.json");
			bnch_swt::file_handle::save_file(twitter_data, json_out_path.operator std::string() + "/Validate Test.json");
			twitter_partial_message twitter_partial_data_new{};
			parser.parseJson<jsonifier::parse_options{}>(twitter_partial_data_new, twitter_data);
			test_results = json_tests_helper<test_type::parse_and_serialize, twitter_partial_message, false, max_executions, "Twitter Partial Test (Prettified)">::run(
				twitter_partial_data_new);
			newer_string += test_results.markdown_results;
			benchmark_data.emplace_back(test_results);
			test_results =
				json_tests_helper<test_type::parse_and_serialize, twitter_partial_message, true, max_executions, "Twitter Partial Test (Minified)">::run(twitter_partial_data_new);
			newer_string += test_results.markdown_results;
			benchmark_data.emplace_back(test_results);

			test_results = json_tests_helper<test_type::parse_and_serialize, twitter_message, false, max_executions, "Twitter Test (Prettified)">::run(twitter_data_new);
			newer_string += test_results.markdown_results;
			benchmark_data.emplace_back(test_results);
			test_results = json_tests_helper<test_type::parse_and_serialize, twitter_message, true, max_executions, "Twitter Test (Minified)">::run(twitter_data_new);
			newer_string += test_results.markdown_results;
			benchmark_data.emplace_back(test_results);
			test_results = json_tests_helper<test_type::minify, std::string, false, max_executions, "Minify Test">::run(twitter_data);
			newer_string += test_results.markdown_results;
			benchmark_data.emplace_back(test_results);
			test_results = json_tests_helper<test_type::prettify, std::string, false, max_executions, "Prettify Test">::run(twitter_minified_data);
			newer_string += test_results.markdown_results;
			benchmark_data.emplace_back(test_results);
			test_results = json_tests_helper<test_type::validate, std::string, false, max_executions, "Validate Test">::run(twitter_data);
			newer_string += test_results.markdown_results;
			benchmark_data.emplace_back(test_results);
		}
		std::string results_string_json{};
		test_results_final results_data{};
		for (auto& value: benchmark_data) {
			test_elements_final test_element{};
			test_element.test_name = value.test_name;
			for (auto& value_new: value.results) {
				test_element_final result_final{};
				if (value_new.read_result.json_speed.has_value()) {
					result_final.library_name = value_new.name;
					result_final.color		  = value_new.read_result.color;
					result_final.result_speed = value_new.read_result.json_speed.value();
					result_final.result_type  = "Read";
					test_element.results.emplace_back(result_final);
				}
				if (value_new.write_result.json_speed.has_value()) {
					result_final.library_name = value_new.name;
					result_final.color		  = value_new.write_result.color;
					result_final.result_speed = value_new.write_result.json_speed.value();
					result_final.result_type  = "Write";
					test_element.results.emplace_back(result_final);
				}
			}
			results_data.emplace_back(test_element);
		}
		parser.serializeJson<jsonifier::serialize_options{ .prettify = true }>(results_data, results_string_json);
		std::cout << "Json Data: " << results_string_json << std::endl;
		bnch_swt::file_handle::save_file(results_string_json, json_out_path.operator std::string() + "/Results.json");
		bnch_swt::file_handle::save_file(static_cast<std::string>(newer_string), read_me_path.operator std::string() + "/" + current_path.operator std::string() + ".md");
		std::cout << "Md Data: " << newer_string << std::endl;
		execute_python_script(base_path.operator std::string() + "/GenerateGraphs.py", json_out_path.operator std::string() + "/Results.json", graphs_path.operator std::string());
#if !defined(NDEBUG)
		for (auto& value: jsonifier::internal::types) {
			std::cout << "TYPE: " << value.first << ", HASH-TYPE: " << value.second << std::endl;
		}
#endif
	};

}