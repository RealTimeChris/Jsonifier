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
#include "simdjson.hpp"
#include "glaze.hpp"

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

	template<json_library lib, test_type type, typename test_data_type, bool minified, size_t iterations, const bnch_swt::string_literal test_name> struct json_test_helper;

	template<typename value_type>
	concept pod_type = std::is_same_v<std::vector<std::vector<bool>>, value_type> || std::is_same_v<std::vector<std::vector<std::string>>, value_type> ||
		std::is_same_v<std::vector<std::vector<int64_t>>, value_type> || std::is_same_v<std::vector<std::vector<uint64_t>>, value_type> ||
		std::is_same_v<std::vector<std::vector<double>>, value_type>;

	template<typename value_type>
	concept iterative_test_type = std::is_same_v<test<test_struct>, value_type> || std::is_same_v<abc_test<abc_test_struct>, value_type> ||
		std::is_same_v<partial_test<partial_test_struct>, value_type> || std::is_same_v<test_struct, value_type>;

	template<pod_type test_data_type, bool minified, size_t iterations, const bnch_swt::string_literal test_name_new>
	struct json_test_helper<json_library::jsonifier, test_type::parse_and_serialize, test_data_type, minified, iterations, test_name_new> {
		JSONIFIER_INLINE static auto run(const test_data_type& test_data_new) {
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
				JSONIFIER_INLINE static size_t impl(jsonifier::jsonifier_core<>& parser_new, test_data_type& test_datas, size_t& current_index,
					std::vector<std::vector<std::string>>& new_strings) {
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
			auto read_result = bnch_swt::benchmark_stage<test_name_read, iterations, measured_iterations>::template run_benchmark<jsonifier_library_name, parse_test_struct>(parser,
				test_datas, current_index, new_strings);
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
				JSONIFIER_INLINE static size_t impl(jsonifier::jsonifier_core<>& parser_new, test_data_type& test_datas, size_t& current_index,
					std::vector<std::vector<std::string>>& new_strings) {
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
			auto write_result = bnch_swt::benchmark_stage<test_name_write, iterations, measured_iterations>::template run_benchmark<jsonifier_library_name, serialize_test_struct>(
				parser, test_datas, current_index, new_strings);
			r.read_result  = result<result_type::read>{ "teal", read_result };
			r.write_result = result<result_type::write>{ "steelblue", write_result };
			bnch_swt::file_loader::save_file(static_cast<std::string>(parser.serializeJson(test_datas)),
				json_out_path.operator std::string() + "/" + test_name.operator std::string() + "-jsonifier.json");
			return r;
		}
	};

	template<iterative_test_type test_data_type, bool minified, size_t iterations, const bnch_swt::string_literal test_name_new>
	struct json_test_helper<json_library::jsonifier, test_type::parse_and_serialize, test_data_type, minified, iterations, test_name_new> {
		JSONIFIER_INLINE static auto run(const std::vector<test_data_type>& test_data_new) {
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
				JSONIFIER_INLINE static size_t impl(jsonifier::jsonifier_core<>& parser_new, std::vector<test_data_type>& test_datas, size_t& current_index,
					std::vector<std::string>& new_strings) {
					parser_new.parseJson<jsonifier::parse_options{ .partialRead = partial_read, .knownOrder = known_order, .minified = minified }>(test_datas[current_index],
						new_strings[current_index]);
					bnch_swt::do_not_optimize_away(test_datas[current_index]);
					auto new_size = new_strings[current_index].size();
					++current_index;
					return new_size;
				}
			};
			auto read_result = bnch_swt::benchmark_stage<test_name_read, iterations, measured_iterations>::template run_benchmark<jsonifier_library_name, parse_test_struct>(parser,
				test_datas, current_index, new_strings);
			for (auto& value: parser.getErrors()) {
				std::cout << "Jsonifier Error: " << value << std::endl;
			}
			for (size_t x = 0; x < iterations; ++x) {
				new_strings[x] = std::string{};
			}
			current_index = 0;
			struct serialize_test_struct {
				JSONIFIER_INLINE static size_t impl(jsonifier::jsonifier_core<>& parser_new, std::vector<test_data_type>& test_datas, size_t& current_index,
					std::vector<std::string>& new_strings) {
					parser_new.serializeJson<jsonifier::serialize_options{ .prettify = !minified }>(test_datas[current_index], new_strings[current_index]);
					bnch_swt::do_not_optimize_away(new_strings[current_index]);
					auto new_size = new_strings[current_index].size();
					++current_index;
					return new_size;
				}
			};
			auto write_result = bnch_swt::benchmark_stage<test_name_write, iterations, measured_iterations>::template run_benchmark<jsonifier_library_name, serialize_test_struct>(
				parser, test_datas, current_index, new_strings);
			r.read_result  = result<result_type::read>{ "teal", read_result };
			r.write_result = result<result_type::write>{ "steelblue", write_result };
			bnch_swt::file_loader::save_file(static_cast<std::string>(new_strings[0]),
				json_out_path.operator std::string() + "/" + test_name.operator std::string() + "-jsonifier.json");
			return r;
		}
	};

	template<typename test_data_type, bool minified, size_t iterations, const bnch_swt::string_literal test_name_new>
	struct json_test_helper<json_library::jsonifier, test_type::parse_and_serialize, test_data_type, minified, iterations, test_name_new> {
		JSONIFIER_INLINE static auto run(const test_data_type& test_data_new) {
			static constexpr bnch_swt::string_literal test_name{ test_name_new };
			static constexpr bnch_swt::string_literal test_name_read{ test_name + "-Read" };
			static constexpr bnch_swt::string_literal test_name_write{ test_name + "-Write" };
			static constexpr bool partial_read{ std::is_same_v<test_data_type, partial_test<partial_test_struct>> || std::is_same_v<test_data_type, twitter_partial_message> };
			static constexpr bool known_order{ true };
			results_data r{ jsonifier_library_name, test_name, jsonifier_commit_url };
			jsonifier::jsonifier_core parser{};
			std::vector<std::string> new_strings{ iterations };
			std::string new_string{ parser.serializeJson<jsonifier::serialize_options{ .prettify = !minified }>(test_data_new) };
			std::vector<test_data_type> test_datas{ iterations };
			size_t current_index{};
			struct parse_test_struct {
				JSONIFIER_INLINE static size_t impl(jsonifier::jsonifier_core<>& parser_new, std::vector<test_data_type>& test_datas, size_t& current_index,
					std::string& new_strings) {
					parser_new.parseJson<jsonifier::parse_options{ .partialRead = partial_read, .knownOrder = known_order, .minified = minified }>(test_datas[current_index],
						new_strings);
					bnch_swt::do_not_optimize_away(test_datas[current_index]);
					auto new_size = new_strings.size();
					++current_index;
					return new_size;
				}
			};
			auto read_result = bnch_swt::benchmark_stage<test_name_read, iterations, measured_iterations>::template run_benchmark<jsonifier_library_name, parse_test_struct>(parser,
				test_datas, current_index, new_string);
			for (auto& value: parser.getErrors()) {
				std::cout << "Jsonifier Error: " << value << std::endl;
			}
			for (size_t x = 0; x < iterations; ++x) {
				new_strings[x] = std::string{};
			}
			current_index = 0;
			struct serialize_test_struct {
				JSONIFIER_INLINE static size_t impl(jsonifier::jsonifier_core<>& parser_new, std::vector<test_data_type>& test_datas, size_t& current_index,
					std::vector<std::string>& new_strings) {
					parser_new.serializeJson<jsonifier::serialize_options{ .prettify = !minified }>(test_datas[current_index], new_strings[current_index]);
					bnch_swt::do_not_optimize_away(new_strings[current_index]);
					auto new_size = new_strings[current_index].size();
					++current_index;
					return new_size;
				}
			};
			auto write_result = bnch_swt::benchmark_stage<test_name_write, iterations, measured_iterations>::template run_benchmark<jsonifier_library_name, serialize_test_struct>(
				parser, test_datas, current_index, new_strings);
			r.read_result  = result<result_type::read>{ "teal", read_result };
			r.write_result = result<result_type::write>{ "steelblue", write_result };
			bnch_swt::file_loader::save_file(static_cast<std::string>(new_strings[0]),
				json_out_path.operator std::string() + "/" + test_name.operator std::string() + "-jsonifier.json");
			return r;
		}
	};

	template<size_t iterations, bnch_swt::string_literal test_name_new>
	struct json_test_helper<json_library::jsonifier, test_type::prettify, std::string, false, iterations, test_name_new> {
		JSONIFIER_INLINE static auto run(std::string& new_buffer) {
			static constexpr bnch_swt::string_literal test_name{ test_name_new };
			results_data r{ jsonifier_library_name, test_name, jsonifier_commit_url };
			jsonifier::jsonifier_core parser{};
			std::vector<std::string> new_strings{ iterations };
			size_t current_index{};
			struct prettify_test_struct {
				JSONIFIER_INLINE static size_t impl(jsonifier::jsonifier_core<>& parser_new, size_t& current_index, std::vector<std::string>& new_strings,
					std::string& newer_buffer) {
					parser_new.prettifyJson(newer_buffer, new_strings[current_index]);
					bnch_swt::do_not_optimize_away(new_strings[current_index]);
					auto new_size = new_strings[current_index].size();
					++current_index;
					return new_size;
				}
			};
			auto write_result = bnch_swt::benchmark_stage<test_name, iterations, measured_iterations>::template run_benchmark<jsonifier_library_name, prettify_test_struct>(parser,
				current_index, new_strings, new_buffer);

			for (auto& value: parser.getErrors()) {
				std::cout << "Jsonifier Error: " << value << std::endl;
			}
			bnch_swt::file_loader::save_file(new_strings[0], json_out_path.operator std::string() + "/" + test_name.operator std::string() + "-jsonifier.json");
			r.write_result = result<result_type::write>{ "steelblue", write_result };
			return r;
		}
	};

	template<size_t iterations, bnch_swt::string_literal test_name_new>
	struct json_test_helper<json_library::jsonifier, test_type::minify, std::string, false, iterations, test_name_new> {
		JSONIFIER_INLINE static auto run(std::string& new_buffer) {
			static constexpr bnch_swt::string_literal test_name{ test_name_new };
			std::vector<std::string> new_strings{ iterations };
			results_data r{ jsonifier_library_name, test_name, jsonifier_commit_url };
			jsonifier::jsonifier_core parser{};
			size_t current_index{};
			struct minify_test_struct {
				JSONIFIER_INLINE static size_t impl(jsonifier::jsonifier_core<>& parser_new, size_t& current_index, std::vector<std::string>& new_strings,
					std::string& newer_buffer) {
					parser_new.minifyJson(newer_buffer, new_strings[current_index]);
					bnch_swt::do_not_optimize_away(new_strings[current_index]);
					auto new_size = new_strings[current_index].size();
					++current_index;
					return new_size;
				}
			};
			auto write_result = bnch_swt::benchmark_stage<test_name, iterations, measured_iterations>::template run_benchmark<jsonifier_library_name, minify_test_struct>(parser,
				current_index, new_strings, new_buffer);
			for (auto& value: parser.getErrors()) {
				std::cout << "Jsonifier Error: " << value << std::endl;
			}
			bnch_swt::file_loader::save_file(new_strings[0], json_out_path.operator std::string() + "/" + test_name.operator std::string() + "-jsonifier.json");
			r.write_result = result<result_type::write>{ "steelblue", write_result };
			return r;
		}
	};

	template<size_t iterations, bnch_swt::string_literal test_name_new>
	struct json_test_helper<json_library::jsonifier, test_type::validate, std::string, false, iterations, test_name_new> {
		JSONIFIER_INLINE static auto run(std::string& new_buffer) {
			static constexpr bnch_swt::string_literal test_name{ test_name_new };
			results_data r{ jsonifier_library_name, test_name, jsonifier_commit_url };
			struct validate_test_struct {
				JSONIFIER_INLINE static size_t impl(jsonifier::jsonifier_core<>& parser_new, std::string& newer_buffer) {
					if (auto result = parser_new.validateJson(newer_buffer); !result) {
						bnch_swt::do_not_optimize_away(result);
						return uint64_t{};
					}
					return static_cast<uint64_t>(newer_buffer.size());
				}
			};
			jsonifier::jsonifier_core parser{};
			auto read_result =
				bnch_swt::benchmark_stage<test_name, iterations, measured_iterations>::template run_benchmark<jsonifier_library_name, validate_test_struct>(parser, new_buffer);

			for (auto& value: parser.getErrors()) {
				std::cout << "Jsonifier Error: " << value << std::endl;
			}
			bnch_swt::file_loader::save_file(new_buffer, json_out_path.operator std::string() + "/" + test_name.operator std::string() + "-jsonifier.json");
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

	template<pod_type test_data_type, bool minified, size_t iterations, const bnch_swt::string_literal test_name_new>
	struct json_test_helper<json_library::glaze, test_type::parse_and_serialize, test_data_type, minified, iterations, test_name_new> {
		JSONIFIER_INLINE static auto run(const test_data_type& test_data_new) {
			static constexpr bnch_swt::string_literal test_name{ test_name_new };
			static constexpr bnch_swt::string_literal test_name_read{ test_name + "-Read" };
			static constexpr bnch_swt::string_literal test_name_write{ test_name + "-Write" };
			static constexpr bool partial_read{ std::is_same_v<test_data_type, partial_test<partial_test_struct>> || std::is_same_v<test_data_type, twitter_partial_message> };
			results_data r{ glaze_library_name, test_name, glaze_commit_url };
			std::vector<std::vector<std::string>> new_strings{ iterations };
			for (size_t x = 0; x < iterations; ++x) {
				new_strings[x].resize(test_data_new[x].size());
			}
			for (size_t x = 0; x < iterations; ++x) {
				for (size_t y = 0; y < test_data_new[x].size(); ++y) {
					auto new_result = glz::write<glz::opts{ .skip_null_members = false, .prettify = !minified, .minified = minified }>(test_data_new[x][y], new_strings[x][y]);
					( void )new_result;
				}
			}
			test_data_type test_datas{ iterations };
			for (size_t x = 0; x < iterations; ++x) {
				test_datas[x].resize(test_data_new[x].size());
			}
			size_t current_index{};
			struct parse_test_struct {
				JSONIFIER_INLINE static size_t impl(test_data_type& test_datas, size_t& current_index, std::vector<std::vector<std::string>>& new_strings) {
					size_t new_size{};
					for (size_t x = 0; x < test_datas[current_index].size(); ++x) {
						get_ref_t<decltype(test_datas[current_index][x])> reference = test_datas[current_index][x];
						if (auto error = glz::read<glz::opts{ .error_on_unknown_keys = !partial_read,
								.skip_null_members									 = false,
								.prettify											 = !minified,
								.minified											 = minified,
								.partial_read										 = partial_read }>(reference, new_strings[current_index][x]);
							error) {
							std::cout << "Glaze Error: " << glz::format_error(error, new_strings[current_index][x]) << std::endl;
						}
						bnch_swt::do_not_optimize_away(test_datas[current_index]);
						new_size += new_strings[current_index][x].size();
					}
					++current_index;
					return new_size;
				}
			};
			auto read_result = bnch_swt::benchmark_stage<test_name_read, iterations, measured_iterations>::template run_benchmark<glaze_library_name, parse_test_struct>(test_datas,
				current_index, new_strings);
			for (size_t x = 0; x < iterations; ++x) {
				for (size_t y = 0; y < test_data_new[x].size(); ++y) {
					new_strings[x][y] = std::string{};
				}
			}
			current_index = 0;
			struct serialize_test_struct {
				JSONIFIER_INLINE static size_t impl(test_data_type& test_datas, size_t& current_index, std::vector<std::vector<std::string>>& new_strings) {
					size_t new_size{};
					for (size_t x = 0; x < test_datas[current_index].size(); ++x) {
						auto new_result = glz::write<glz::opts{ .skip_null_members = false, .prettify = !minified, .minified = minified }>(test_datas[current_index][x],
							new_strings[current_index][x]);
						( void )new_result;
						bnch_swt::do_not_optimize_away(new_strings[current_index][x]);
						new_size += new_strings[current_index][x].size();
					}
					++current_index;
					return new_size;
				}
			};
			auto write_result = bnch_swt::benchmark_stage<test_name_write, iterations, measured_iterations>::template run_benchmark<glaze_library_name, serialize_test_struct>(
				test_datas, current_index, new_strings);
			r.read_result  = result<result_type::read>{ "dodgerblue", read_result };
			r.write_result = result<result_type::write>{ "skyblue", write_result };
			std::string new_string{};
			auto new_result = glz::write_json(test_datas, new_string);
			( void )new_result;
			bnch_swt::file_loader::save_file(new_string, json_out_path.operator std::string() + "/" + test_name.operator std::string() + "-glaze.json");
			return r;
		}
	};

	template<iterative_test_type test_data_type, bool minified, size_t iterations, const bnch_swt::string_literal test_name_new>
	struct json_test_helper<json_library::glaze, test_type::parse_and_serialize, test_data_type, minified, iterations, test_name_new> {
		JSONIFIER_INLINE static auto run(const std::vector<test_data_type>& test_data_new) {
			static constexpr bnch_swt::string_literal test_name{ test_name_new };
			static constexpr bnch_swt::string_literal test_name_read{ test_name + "-Read" };
			static constexpr bnch_swt::string_literal test_name_write{ test_name + "-Write" };
			static constexpr bool partial_read{ std::is_same_v<test_data_type, partial_test<partial_test_struct>> || std::is_same_v<test_data_type, twitter_partial_message> };
			results_data r{ glaze_library_name, test_name, glaze_commit_url };
			std::vector<std::string> new_strings{ iterations };
			std::vector<test_data_type> test_datas{ iterations };
			for (size_t x = 0; x < iterations; ++x) {
				auto new_result = glz::write<glz::opts{ .skip_null_members = false, .prettify = !minified, .minified = minified }>(test_data_new[x], new_strings[x]);
				( void )new_result;
			}
			size_t current_index{};
			struct parse_test_struct {
				JSONIFIER_INLINE static size_t impl(std::vector<test_data_type>& test_datas, size_t& current_index, std::vector<std::string>& new_strings) {
					if (auto error = glz::read<glz::opts{ .error_on_unknown_keys = !partial_read,
							.skip_null_members									 = false,
							.prettify											 = !minified,
							.minified											 = minified,
							.partial_read										 = partial_read }>(test_datas[current_index], new_strings[current_index]);
						error) {
						std::cout << "Glaze Error: " << glz::format_error(error, new_strings[current_index]) << std::endl;
					}
					bnch_swt::do_not_optimize_away(test_datas[current_index]);
					auto new_size = new_strings[current_index].size();
					++current_index;
					return new_size;
				}
			};
			auto read_result = bnch_swt::benchmark_stage<test_name_read, iterations, measured_iterations>::template run_benchmark<glaze_library_name, parse_test_struct>(test_datas,
				current_index, new_strings);
			for (size_t x = 0; x < iterations; ++x) {
				new_strings[x] = std::string{};
			}
			current_index = 0;
			struct serialize_test_struct {
				JSONIFIER_INLINE static size_t impl(std::vector<test_data_type>& test_datas, size_t& current_index, std::vector<std::string>& new_strings) {
					auto new_result =
						glz::write<glz::opts{ .skip_null_members = false, .prettify = !minified, .minified = minified }>(test_datas[current_index], new_strings[current_index]);
					bnch_swt::do_not_optimize_away(new_strings[current_index]);
					( void )new_result;
					auto new_size = new_strings[current_index].size();
					++current_index;
					return new_size;
				}
			};
			auto write_result = bnch_swt::benchmark_stage<test_name_write, iterations, measured_iterations>::template run_benchmark<glaze_library_name, serialize_test_struct>(
				test_datas, current_index, new_strings);
			r.read_result  = result<result_type::read>{ "dodgerblue", read_result };
			r.write_result = result<result_type::write>{ "skyblue", write_result };
			bnch_swt::file_loader::save_file(static_cast<std::string>(new_strings[0]),
				json_out_path.operator std::string() + "/" + test_name.operator std::string() + "-glaze.json");
			return r;
		}
	};

	template<typename test_data_type, bool minified, size_t iterations, const bnch_swt::string_literal test_name_new>
	struct json_test_helper<json_library::glaze, test_type::parse_and_serialize, test_data_type, minified, iterations, test_name_new> {
		JSONIFIER_INLINE static auto run(const test_data_type& test_data_new) {
			static constexpr bnch_swt::string_literal test_name{ test_name_new };
			static constexpr bnch_swt::string_literal test_name_read{ test_name + "-Read" };
			static constexpr bnch_swt::string_literal test_name_write{ test_name + "-Write" };
			static constexpr bool partial_read{ std::is_same_v<test_data_type, partial_test<partial_test_struct>> || std::is_same_v<test_data_type, twitter_partial_message> };
			results_data r{ glaze_library_name, test_name, glaze_commit_url };
			std::vector<std::string> new_strings{ iterations };
			std::string new_string{};
			std::vector<test_data_type> test_datas{ iterations };
			auto new_result = glz::write<glz::opts{ .skip_null_members = false, .prettify = !minified, .minified = minified }>(test_data_new, new_string);
			( void )new_result;
			size_t current_index{};
			struct parse_test_struct {
				JSONIFIER_INLINE static size_t impl(std::vector<test_data_type>& test_datas, size_t& current_index, std::string& new_string,
					std::vector<std::string>& new_strings) {
					if (auto error = glz::read<glz::opts{ .error_on_unknown_keys = !partial_read,
							.skip_null_members									 = false,
							.prettify											 = !minified,
							.minified											 = minified,
							.partial_read										 = partial_read }>(test_datas[current_index], new_string);
						error) {
						std::cout << "Glaze Error: " << glz::format_error(error, new_strings[current_index]) << std::endl;
					}
					bnch_swt::do_not_optimize_away(test_datas[current_index]);
					auto new_size = new_string.size();
					++current_index;
					return new_size;
				}
			};
			auto read_result = bnch_swt::benchmark_stage<test_name_read, iterations, measured_iterations>::template run_benchmark<glaze_library_name, parse_test_struct>(test_datas,
				current_index, new_string, new_strings);
			for (size_t x = 0; x < iterations; ++x) {
				test_datas[x] = test_datas[0];
			}
			current_index = 0;
			struct serialize_test_struct {
				JSONIFIER_INLINE static size_t impl(std::vector<test_data_type>& test_datas, size_t& current_index, std::vector<std::string>& new_strings) {
					auto new_result_new =
						glz::write<glz::opts{ .skip_null_members = false, .prettify = !minified, .minified = minified }>(test_datas[current_index], new_strings[current_index]);
					bnch_swt::do_not_optimize_away(new_strings[current_index]);
					( void )new_result_new;
					auto new_size = new_strings[current_index].size();
					++current_index;
					return new_size;
				}
			};
			auto write_result = bnch_swt::benchmark_stage<test_name_write, iterations, measured_iterations>::template run_benchmark<glaze_library_name, serialize_test_struct>(
				test_datas, current_index, new_strings);
			r.read_result  = result<result_type::read>{ "dodgerblue", read_result };
			r.write_result = result<result_type::write>{ "skyblue", write_result };
			bnch_swt::file_loader::save_file(static_cast<std::string>(new_strings[0]),
				json_out_path.operator std::string() + "/" + test_name.operator std::string() + "-glaze.json");
			return r;
		}
	};

	template<size_t iterations, bnch_swt::string_literal test_name_new>
	struct json_test_helper<json_library::glaze, test_type::prettify, std::string, false, iterations, test_name_new> {
		JSONIFIER_INLINE static auto run(std::string& new_buffer) {
			static constexpr bnch_swt::string_literal test_name{ test_name_new };
			std::vector<std::string> new_strings{ iterations };
			results_data r{ glaze_library_name, test_name, glaze_commit_url };
			size_t current_index{};

			struct prettify_test_struct {
				JSONIFIER_INLINE static size_t impl(std::string& new_buffer, std::vector<std::string>& new_strings, size_t& current_index) {
					glz::prettify_json(new_buffer, new_strings[current_index]);
					bnch_swt::do_not_optimize_away(new_strings[current_index]);
					auto new_size = new_strings[current_index].size();
					++current_index;
					return new_size;
				}
			};

			auto write_result = bnch_swt::benchmark_stage<test_name, iterations, measured_iterations>::template run_benchmark<glaze_library_name, prettify_test_struct>(new_buffer,
				new_strings, current_index);

			bnch_swt::file_loader::save_file(new_strings[0], json_out_path.operator std::string() + "/" + test_name.operator std::string() + "-glaze.json");
			r.write_result = result<result_type::write>{ "skyblue", write_result };

			return r;
		}
	};

	template<size_t iterations, bnch_swt::string_literal test_name_new>
	struct json_test_helper<json_library::glaze, test_type::minify, std::string, false, iterations, test_name_new> {
		JSONIFIER_INLINE static auto run(std::string& new_buffer) {
			static constexpr bnch_swt::string_literal test_name{ test_name_new };

			std::string newest_buffer{ new_buffer };
			std::vector<std::string> new_strings{ iterations };
			size_t current_index{};
			results_data r{ glaze_library_name, test_name, glaze_commit_url };

			struct minify_test_struct {
				JSONIFIER_INLINE static size_t impl(std::string& newest_buffer, std::vector<std::string>& new_strings, size_t& current_index) {
					glz::minify_json(newest_buffer, new_strings[current_index]);
					bnch_swt::do_not_optimize_away(new_strings[current_index]);
					auto new_size = new_strings[current_index].size();
					++current_index;
					return new_size;
				}
			};

			auto write_result = bnch_swt::benchmark_stage<test_name, iterations, measured_iterations>::template run_benchmark<glaze_library_name, minify_test_struct>(newest_buffer,
				new_strings, current_index);

			bnch_swt::file_loader::save_file(new_strings[0], json_out_path.operator std::string() + "/" + test_name.operator std::string() + "-glaze.json");
			r.write_result = result<result_type::write>{ "skyblue", write_result };

			return r;
		}
	};

	template<size_t iterations, bnch_swt::string_literal test_name_new>
	struct json_test_helper<json_library::glaze, test_type::validate, std::string, false, iterations, test_name_new> {
		JSONIFIER_INLINE static auto run(std::string& new_buffer) {
			static constexpr bnch_swt::string_literal test_name{ test_name_new };
			results_data r{ glaze_library_name, test_name, glaze_commit_url };

			struct validate_test_struct {
				JSONIFIER_INLINE static size_t impl(std::string& new_buffer) {
					if (auto result = glz::validate_json(new_buffer); result) {
						bnch_swt::do_not_optimize_away(result);
						return size_t{};
					}
					return new_buffer.size();
				}
			};

			auto read_result = bnch_swt::benchmark_stage<test_name, iterations, measured_iterations>::template run_benchmark<glaze_library_name, validate_test_struct>(new_buffer);

			bnch_swt::file_loader::save_file(new_buffer, json_out_path.operator std::string() + "/" + test_name.operator std::string() + "-glaze.json");
			r.read_result = result<result_type::read>{ "dodgerblue", read_result };

			return r;
		}
	};

	template<pod_type test_data_type, bool minified, size_t iterations, const bnch_swt::string_literal test_name_new>
	struct json_test_helper<json_library::simdjson, test_type::parse_and_serialize, test_data_type, minified, iterations, test_name_new> {
		JSONIFIER_INLINE static auto run(const test_data_type& test_data_new) {
			static constexpr bnch_swt::string_literal test_name{ test_name_new };
			static constexpr bnch_swt::string_literal test_name_read{ test_name + "-Read" };
			static constexpr bnch_swt::string_literal test_name_write{ test_name + "-Write" };
			results_data r{ simdjson_library_name, test_name, simdjson_commit_url };
			std::vector<std::vector<std::string>> new_strings{ iterations };
			simdjson::ondemand::parser parser{};
			for (size_t x = 0; x < iterations; ++x) {
				new_strings[x].resize(test_data_new[x].size());
			}
			for (size_t x = 0; x < iterations; ++x) {
				for (size_t y = 0; y < test_data_new[x].size(); ++y) {
					auto new_result = glz::write<glz::opts{ .skip_null_members = false, .prettify = !minified, .minified = minified }>(test_data_new[x][y], new_strings[x][y]);
					( void )new_result;
				}
			}
			test_data_type test_datas{ iterations };
			for (size_t x = 0; x < iterations; ++x) {
				test_datas[x].resize(test_data_new[x].size());
			}
			size_t current_index{};

			struct parse_test_struct {
				JSONIFIER_INLINE static size_t impl(simdjson::ondemand::parser& parser, test_data_type& test_datas, size_t& current_index,
					std::vector<std::vector<std::string>>& new_strings) {
					size_t new_size{};
					for (size_t x = 0; x < test_datas[current_index].size(); ++x) {
						get_value(test_datas[current_index][x], static_cast<simdjson::ondemand::document>(parser.iterate(new_strings[current_index][x])));
						bnch_swt::do_not_optimize_away(test_datas[current_index]);
						new_size += new_strings[current_index][x].size();
					}
					++current_index;
					return new_size;
				}
			};

			auto read_result = bnch_swt::benchmark_stage<test_name_read, iterations, measured_iterations>::template run_benchmark<simdjson_library_name, parse_test_struct>(parser,
				test_datas, current_index, new_strings);
			r.read_result	 = result<result_type::read>{ "cadetblue", read_result };
			std::string new_string{};
			auto new_result = glz::write_json(test_datas, new_string);
			( void )new_result;
			bnch_swt::file_loader::save_file(new_string, json_out_path.operator std::string() + "/" + test_name.operator std::string() + "-simdjson.json");
			return r;
		}
	};

	template<iterative_test_type test_data_type, bool minified, size_t iterations, const bnch_swt::string_literal test_name_new>
	struct json_test_helper<json_library::simdjson, test_type::parse_and_serialize, test_data_type, minified, iterations, test_name_new> {
		JSONIFIER_INLINE static auto run(const std::vector<test_data_type>& test_data_new) {
			static constexpr bnch_swt::string_literal test_name{ test_name_new };
			static constexpr bnch_swt::string_literal test_name_read{ test_name + "-Read" };
			static constexpr bnch_swt::string_literal test_name_write{ test_name + "-Write" };
			results_data r{ simdjson_library_name, test_name, simdjson_commit_url };
			jsonifier::jsonifier_core parser_new{};
			std::vector<std::string> new_strings{ iterations };
			simdjson::ondemand::parser parser{};
			std::vector<test_data_type> test_datas{ iterations };
			for (size_t x = 0; x < iterations; ++x) {
				parser_new.serializeJson<jsonifier::serialize_options{ .prettify = !minified }>(test_data_new[x], new_strings[x]);
			}
			size_t current_index{};

			struct parse_test_struct {
				JSONIFIER_INLINE static size_t impl(simdjson::ondemand::parser& parser, std::vector<test_data_type>& test_datas, size_t& current_index,
					std::vector<std::string>& new_strings) {
					try {
						get_value(test_datas[current_index], parser.iterate(new_strings[current_index]).value().get_value());
						bnch_swt::do_not_optimize_away(test_datas[current_index]);
						auto new_size = new_strings[current_index].size();
						++current_index;
						return new_size;

					} catch (const std::exception& error) {
						std::cout << "Simdjson Error: " << error.what() << std::endl;
					}
					++current_index;
					return size_t{};
				}
			};

			auto read_result = bnch_swt::benchmark_stage<test_name_read, iterations, measured_iterations>::template run_benchmark<simdjson_library_name, parse_test_struct>(parser,
				test_datas, current_index, new_strings);
			r.read_result	 = result<result_type::read>{ "cadetblue", read_result };
			bnch_swt::file_loader::save_file(static_cast<std::string>(new_strings[0]),
				json_out_path.operator std::string() + "/" + test_name.operator std::string() + "-simdjson.json");
			return r;
		}
	};

	template<typename test_data_type, bool minified, size_t iterations, const bnch_swt::string_literal test_name_new>
	struct json_test_helper<json_library::simdjson, test_type::parse_and_serialize, test_data_type, minified, iterations, test_name_new> {
		JSONIFIER_INLINE static auto run(const test_data_type& test_data_new) {
			static constexpr bnch_swt::string_literal test_name{ test_name_new };
			static constexpr bnch_swt::string_literal test_name_read{ test_name + "-Read" };
			static constexpr bnch_swt::string_literal test_name_write{ test_name + "-Write" };
			results_data r{ simdjson_library_name, test_name, simdjson_commit_url };
			jsonifier::jsonifier_core parser_new{};
			simdjson::ondemand::parser parser{};
			std::string new_string{};
			test_data_type test_data_newer{};
			std::vector<test_data_type> test_datas{ iterations };
			parser_new.serializeJson<jsonifier::serialize_options{ .prettify = !minified }>(test_data_new, new_string);
			get_value(test_data_newer, parser.iterate(new_string).value().get_value());
			new_string.clear();
			parser_new.serializeJson<jsonifier::serialize_options{ .prettify = !minified }>(test_data_newer, new_string);
			size_t current_index{};

			struct parse_test_struct {
				JSONIFIER_INLINE static size_t impl(simdjson::ondemand::parser& parser, std::vector<test_data_type>& test_datas, size_t& current_index, std::string& new_string) {
					try {
						get_value(test_datas[current_index], parser.iterate(new_string).value().get_value());
						bnch_swt::do_not_optimize_away(test_datas[current_index]);
						auto new_size = new_string.size();
						++current_index;
						return new_size;

					} catch (const std::exception& error) {
						std::cout << "Simdjson Error: " << error.what() << std::endl;
					}
					++current_index;
					return new_string.size();
				}
			};

			auto read_result = bnch_swt::benchmark_stage<test_name_read, iterations, measured_iterations>::template run_benchmark<simdjson_library_name, parse_test_struct>(parser,
				test_datas, current_index, new_string);
			r.read_result	 = result<result_type::read>{ "cadetblue", read_result };
			bnch_swt::file_loader::save_file(static_cast<std::string>(new_string),
				json_out_path.operator std::string() + "/" + test_name.operator std::string() + "-simdjson.json");
			return r;
		}
	};

	template<size_t iterations, bnch_swt::string_literal test_name_new>
	struct json_test_helper<json_library::simdjson, test_type::minify, std::string, false, iterations, test_name_new> {
		static constexpr bnch_swt::string_literal test_name{ test_name_new };
		JSONIFIER_INLINE static auto run(std::string& new_buffer) {
			results_data r{ simdjson_library_name, test_name, simdjson_commit_url };

			simdjson::dom::parser parser{};
			std::vector<std::string> new_strings{ iterations };
			size_t current_index{};

			struct minify_test_struct {
				JSONIFIER_INLINE static size_t impl(simdjson::dom::parser& parser, std::string& new_buffer, std::vector<std::string>& new_strings, size_t& current_index) {
					try {
						new_strings[current_index] = simdjson::minify(parser.parse(new_buffer));
						bnch_swt::do_not_optimize_away(new_strings[current_index]);
						auto new_size = new_strings[current_index].size();
						++current_index;
						return new_size;
					} catch (std::exception& error) {
						std::cout << "Simdjson Error: " << error.what() << std::endl;
						return new_strings[current_index].size();
					}
				}
			};

			auto write_result = bnch_swt::benchmark_stage<test_name, iterations, measured_iterations>::template run_benchmark<simdjson_library_name, minify_test_struct>(parser,
				new_buffer, new_strings, current_index);

			bnch_swt::file_loader::save_file(new_strings[0], json_out_path.operator std::string() + "/" + test_name.operator std::string() + "-simdjson.json");
			r.write_result = result<result_type::write>{ "cornflowerblue", write_result };

			return r;
		}
	};

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

	static std::string section001{ R"(
 > )" + jsonifier::toString(max_iterations) +
		R"( iterations on a ()" + get_cpu_info() + R"(), where the most stable 20 subsequent iterations are sampled.

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

	std::string generate_section(const std::string& test_name, const std::string& current_path_new) {
		return R"(

### )" + test_name +
			R"( Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/)" + current_path_new + R"(/)" +
			url_encode(test_name) +
			R"(.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/)" +
			current_path_new + R"(/)" + url_encode(test_name) + R"(_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/)" +
			current_path_new + R"(/)" + url_encode(test_name) + R"(_Results.png?raw=true" 
alt="" width="400"/></p>
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/)" +
			current_path_new + R"(/)" + url_encode(test_name) +
			R"(_Cumulative_Speedup.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/)" + current_path_new + R"(/)" + test_name +
			R"(_Cumulative_Speedup.png?raw=true" 
alt="" width="400"/></p>

)" + (test_name.find("Abc (Out of Order) Test (Prettified)") == std::string::npos ? "" : "The JSON documents in the previous tests featured keys ranging from \"a\" to \"z\",\
 where each key corresponds to an array of values. Notably, the documents in this test arrange these keys in reverse order, deviating from the typical \"a\" to \"z\" arrangement.\n\n\
This test effectively demonstrates the challenges encountered when utilizing simdjson and iterative parsers that lack the ability to efficiently allocate memory locations through\
hashing. In cases where the keys are not in the expected sequence, performance is significantly compromised, with the severity escalating as the document size increases.\n\n\
In contrast, hash-based solutions offer a viable alternative by circumventing these issues and maintaining optimal performance regardless of the JSON document's scale, or ordering of the keys being parsed.\n");
	}

	template<test_type type, typename test_data_type, bool minified, uint64_t iterations, bnch_swt::string_literal test_name> struct json_tests_helper;

	template<test_type type, typename test_data_type, bool minified, uint64_t iterations, bnch_swt::string_literal test_name_new> struct json_tests_helper {
		template<typename test_data_type_new> JSONIFIER_INLINE static test_results run(const test_data_type_new& json_data_new) {
			static constexpr bnch_swt::string_literal test_name{ test_name_new };
			test_results json_results{};
			json_results.test_name = test_name.operator std::string();
			results_data jsonifier_results{};
			results_data simdjson_results{};
			results_data glaze_results{};

#if !defined(ASAN_ENABLED)
			simdjson_results = json_test_helper<json_library::simdjson, type, test_data_type, minified, iterations, test_name>::run(json_data_new);
			if constexpr (!std::is_same_v<test_data_type, partial_test<partial_test_struct>> && !std::is_same_v<test_data_type, twitter_partial_message>) {
				glaze_results = json_test_helper<json_library::glaze, type, test_data_type, minified, iterations, test_name>::run(json_data_new);
				json_results.results.emplace_back(glaze_results);
			}
#endif
			jsonifier_results = json_test_helper<json_library::jsonifier, type, test_data_type, minified, iterations, test_name>::run(json_data_new);
#if !defined(ASAN_ENABLED)
			json_results.results.emplace_back(simdjson_results);
#endif
			json_results.results.emplace_back(jsonifier_results);
			json_results.markdown_results += generate_section(test_name, current_path);
			json_results.markdown_results += table_header.operator std::string() + "\n";
			std::sort(json_results.results.begin(), json_results.results.end(), std::greater<results_data>());
			for (auto iter = json_results.results.begin(); iter != json_results.results.end();) {
				json_results.markdown_results += iter->json_stats();
				if (static_cast<size_t>(iter - json_results.results.begin()) != json_results.results.size() - 1) {
					json_results.markdown_results += "\n";
				}
				++iter;
			}
			bnch_swt::benchmark_stage<test_name_new + "-Read", iterations, measured_iterations>::print_results();
			bnch_swt::benchmark_stage<test_name_new + "-Write", iterations, measured_iterations>::print_results();
			return json_results;
		}
	};

	template<uint64_t iterations, bnch_swt::string_literal test_name_new> struct json_tests_helper<test_type::prettify, std::string, false, iterations, test_name_new> {
		JSONIFIER_INLINE static test_results run(std::string& json_data_new) {
			static constexpr bnch_swt::string_literal test_name{ test_name_new };
			test_results json_results{};
			json_results.test_name = test_name.operator std::string();
			results_data jsonifier_results{};
			results_data glaze_results{};
#if !defined(ASAN_ENABLED)
			glaze_results = json_test_helper<json_library::glaze, test_type::prettify, std::string, false, iterations, test_name>::run(json_data_new);
#endif
			jsonifier_results = json_test_helper<json_library::jsonifier, test_type::prettify, std::string, false, iterations, test_name>::run(json_data_new);
#if !defined(ASAN_ENABLED)
			json_results.results.emplace_back(glaze_results);
#endif
			json_results.results.emplace_back(jsonifier_results);
			json_results.markdown_results += generate_section(test_name, current_path);
			json_results.markdown_results += write_table_header.operator std::string() + "\n";
			std::sort(json_results.results.begin(), json_results.results.end(), std::greater<results_data>());
			for (auto iter = json_results.results.begin(); iter != json_results.results.end();) {
				json_results.markdown_results += iter->json_stats();
				if (static_cast<size_t>(iter - json_results.results.begin()) != json_results.results.size() - 1) {
					json_results.markdown_results += "\n";
				}
				++iter;
			}
			bnch_swt::benchmark_stage<test_name_new, iterations, measured_iterations>::print_results();
			return json_results;
		}
	};

	template<uint64_t iterations, bnch_swt::string_literal test_name_new> struct json_tests_helper<test_type::minify, std::string, false, iterations, test_name_new> {
		JSONIFIER_INLINE static test_results run(std::string& json_data_new) {
			static constexpr bnch_swt::string_literal test_name{ test_name_new };
			test_results json_results{};
			json_results.test_name = test_name.operator std::string();
			results_data jsonifier_results{};
			results_data simdjson_results{};
			results_data glaze_results{};
#if !defined(ASAN_ENABLED)
			simdjson_results = json_test_helper<json_library::simdjson, test_type::minify, std::string, false, iterations, test_name>::run(json_data_new);
			glaze_results	 = json_test_helper<json_library::glaze, test_type::minify, std::string, false, iterations, test_name>::run(json_data_new);
#endif
			jsonifier_results = json_test_helper<json_library::jsonifier, test_type::minify, std::string, false, iterations, test_name>::run(json_data_new);
#if !defined(ASAN_ENABLED)
			json_results.results.emplace_back(simdjson_results);
			json_results.results.emplace_back(glaze_results);
#endif
			json_results.results.emplace_back(jsonifier_results);
			json_results.markdown_results += generate_section(test_name, current_path);
			json_results.markdown_results += write_table_header.operator std::string() + "\n";
			std::sort(json_results.results.begin(), json_results.results.end(), std::greater<results_data>());
			for (auto iter = json_results.results.begin(); iter != json_results.results.end();) {
				json_results.markdown_results += iter->json_stats();
				if (static_cast<size_t>(iter - json_results.results.begin()) != json_results.results.size() - 1) {
					json_results.markdown_results += "\n";
				}
				++iter;
			}
			bnch_swt::benchmark_stage<test_name_new, iterations, measured_iterations>::print_results();
			return json_results;
		}
	};

	template<uint64_t iterations, bnch_swt::string_literal test_name_new> struct json_tests_helper<test_type::validate, std::string, false, iterations, test_name_new> {
		JSONIFIER_INLINE static test_results run(std::string& json_data_new) {
			static constexpr bnch_swt::string_literal test_name{ test_name_new };
			test_results json_results{};
			json_results.test_name = test_name.operator std::string();
			results_data jsonifier_results{};
			results_data glaze_results{};
#if !defined(ASAN_ENABLED)
			glaze_results = json_test_helper<json_library::glaze, test_type::validate, std::string, false, iterations, test_name>::run(json_data_new);
#endif
			jsonifier_results = json_test_helper<json_library::jsonifier, test_type::validate, std::string, false, iterations, test_name>::run(json_data_new);
			json_results.results.emplace_back(jsonifier_results);
#if !defined(ASAN_ENABLED)
			json_results.results.emplace_back(glaze_results);
#endif
			json_results.markdown_results += generate_section(test_name.operator std::string(), current_path.operator std::string());
			json_results.markdown_results += read_table_header.operator std::string() + "\n";
			std::sort(json_results.results.begin(), json_results.results.end(), std::greater<results_data>());
			for (auto iter = json_results.results.begin(); iter != json_results.results.end();) {
				json_results.markdown_results += iter->json_stats();
				if (static_cast<size_t>(iter - json_results.results.begin()) != json_results.results.size() - 1) {
					json_results.markdown_results += "\n";
				}
				++iter;
			}
			bnch_swt::benchmark_stage<test_name_new, iterations, measured_iterations>::print_results();
			return json_results;
		}
	};

	JSONIFIER_INLINE void test_function() {
		std::string json_data_new{};
		jsonifier::jsonifier_core parser{};
		std::vector<test<test_struct>> json_data_newer{ max_iterations };
		for (size_t x = 0; x < max_iterations; ++x) {
			json_data_newer[x] = test_generator::generate_test();
		}
		parser.serializeJson<jsonifier::serialize_options{ .prettify = true }>(json_data_newer[0], json_data_new);
		bnch_swt::file_loader::save_file(json_data_new, json_out_path.operator std::string() + "/Json Test (Prettified).json");
		std::string json_minified_data = parser.minifyJson(json_data_new);
		bnch_swt::file_loader::save_file(json_minified_data, json_out_path.operator std::string() + "/Json Test (Minified).json");
		bnch_swt::file_loader::save_file(json_data_new, json_out_path.operator std::string() + "/Partial Test (Prettified).json");
		bnch_swt::file_loader::save_file(json_minified_data, json_out_path.operator std::string() + "/Partial Test (Minified).json");
		bnch_swt::file_loader::save_file(json_data_new, json_out_path.operator std::string() + "/Abc (Out of Order) Test (Prettified).json");
		bnch_swt::file_loader::save_file(json_minified_data, json_out_path.operator std::string() + "/Abc (Out of Order) Test (Minified).json");
		std::string new_time_string{};
		new_time_string.resize(1024);
		std::tm result_two{ get_time() };
		std::vector<test_results> benchmark_data{};
		new_time_string.resize(strftime(new_time_string.data(), 1024, "%b %d, %Y", &result_two));
		std::string newer_string{ section00.operator std::string() + new_time_string + ")\n" + static_cast<std::string>(section002.operator std::string()) +
			static_cast<std::string>(section001) };
		test_results test_results{};
		{
			std::vector<std::vector<double>> double_data{ test_generator::generate_values<double>(max_iterations, 100) };
			parser.serializeJson<jsonifier::serialize_options{ .prettify = true }>(double_data, json_data_new);
			bnch_swt::file_loader::save_file(json_data_new, json_out_path.operator std::string() + "/Double Test.json");
			test_results = json_tests_helper<test_type::parse_and_serialize, std::vector<std::vector<double>>, false, max_iterations, "Double Test">::run(double_data);
			newer_string += test_results.markdown_results;
			benchmark_data.emplace_back(test_results);
		}
		{
			std::vector<std::vector<std::string>> string_data{ test_generator::generate_values<std::string>(max_iterations, 100) };
			parser.serializeJson<jsonifier::serialize_options{ .prettify = true }>(string_data, json_data_new);
			bnch_swt::file_loader::save_file(json_data_new, json_out_path.operator std::string() + "/String Test.json");
			test_results = json_tests_helper<test_type::parse_and_serialize, std::vector<std::vector<std::string>>, false, max_iterations, "String Test">::run(string_data);
			newer_string += test_results.markdown_results;
			benchmark_data.emplace_back(test_results);
		}
		{
			std::vector<std::vector<uint64_t>> uint_data{ test_generator::generate_values<uint64_t>(max_iterations, 100) };
			parser.serializeJson<jsonifier::serialize_options{ .prettify = true }>(uint_data, json_data_new);
			bnch_swt::file_loader::save_file(json_data_new, json_out_path.operator std::string() + "/Uint Test.json");
			test_results = json_tests_helper<test_type::parse_and_serialize, std::vector<std::vector<uint64_t>>, false, max_iterations, "Uint Test">::run(uint_data);
			newer_string += test_results.markdown_results;
			benchmark_data.emplace_back(test_results);
		}
		{
			std::vector<std::vector<int64_t>> int_data{ test_generator::generate_values<int64_t>(max_iterations, 100) };
			parser.serializeJson<jsonifier::serialize_options{ .prettify = true }>(int_data, json_data_new);
			bnch_swt::file_loader::save_file(json_data_new, json_out_path.operator std::string() + "/Int Test.json");
			test_results = json_tests_helper<test_type::parse_and_serialize, std::vector<std::vector<int64_t>>, false, max_iterations, "Int Test">::run(int_data);
			newer_string += test_results.markdown_results;
			benchmark_data.emplace_back(test_results);
		}
		{
			std::vector<std::vector<bool>> bool_data{ test_generator::generate_values<bool>(max_iterations, 100) };
			parser.serializeJson<jsonifier::serialize_options{ .prettify = true }>(bool_data, json_data_new);
			bnch_swt::file_loader::save_file(json_data_new, json_out_path.operator std::string() + "/Bool Test.json");
			test_results = json_tests_helper<test_type::parse_and_serialize, std::vector<std::vector<bool>>, false, max_iterations, "Bool Test">::run(bool_data);
			newer_string += test_results.markdown_results;
			benchmark_data.emplace_back(test_results);
		}
		test_results = json_tests_helper<test_type::parse_and_serialize, test<test_struct>, false, max_iterations, "Json Test (Prettified)">::run(json_data_newer);
		newer_string += test_results.markdown_results;
		benchmark_data.emplace_back(test_results);
		test_results = json_tests_helper<test_type::parse_and_serialize, test<test_struct>, true, max_iterations, "Json Test (Minified)">::run(json_data_newer);
		newer_string += test_results.markdown_results;
		benchmark_data.emplace_back(test_results);
		std::vector<partial_test<partial_test_struct>> json_partial_data_newer{ max_iterations };
		std::vector<std::string> new_strings{ max_iterations };
		for (size_t x = 0; x < max_iterations; ++x) {
			parser.serializeJson(json_data_newer[x], new_strings[x]);
			parser.parseJson(json_partial_data_newer[x], new_strings[x]);
		}
		test_results =
			json_tests_helper<test_type::parse_and_serialize, partial_test<partial_test_struct>, false, max_iterations, "Partial Test (Prettified)">::run(json_partial_data_newer);
		newer_string += test_results.markdown_results;
		benchmark_data.emplace_back(test_results);
		test_results =
			json_tests_helper<test_type::parse_and_serialize, partial_test<partial_test_struct>, true, max_iterations, "Partial Test (Minified)">::run(json_partial_data_newer);
		newer_string += test_results.markdown_results;
		benchmark_data.emplace_back(test_results);
		std::vector<abc_test<abc_test_struct>> abc_data_new{ max_iterations };
		for (size_t x = 0; x < max_iterations; ++x) {
			parser.serializeJson(json_data_newer[x], new_strings[x]);
			parser.parseJson(abc_data_new[x], new_strings[x]);
		}
		test_results =
			json_tests_helper<test_type::parse_and_serialize, abc_test<abc_test_struct>, false, max_iterations, "Abc (Out of Order) Test (Prettified)">::run(abc_data_new);
		newer_string += test_results.markdown_results;
		benchmark_data.emplace_back(test_results);
		test_results = json_tests_helper<test_type::parse_and_serialize, abc_test<abc_test_struct>, true, max_iterations, "Abc (Out of Order) Test (Minified)">::run(abc_data_new);
		newer_string += test_results.markdown_results;
		benchmark_data.emplace_back(test_results);
		{
			std::string discord_data{ bnch_swt::file_loader::load_file(json_path.operator std::string() + "/" + "/Discord Test (Prettified).json") };
			discord_message discord_data_new{};
			parser.parseJson<jsonifier::parse_options{ .knownOrder = true }>(discord_data_new, discord_data);
			for (auto& value: parser.getErrors()) {
				std::cout << "Error: " << value << std::endl;
			}
			bnch_swt::file_loader::save_file(discord_data, json_out_path.operator std::string() + "/Discord Test (Prettified).json");
			std::string discord_minified_data{ bnch_swt::file_loader::load_file(json_path.operator std::string() + "/" + "/Discord Test (Minified).json") };
			bnch_swt::file_loader::save_file(discord_minified_data, json_out_path.operator std::string() + "/Discord Test (Minified).json");
			test_results = json_tests_helper<test_type::parse_and_serialize, discord_message, false, max_iterations, "Discord Test (Prettified)">::run(discord_data_new);
			newer_string += test_results.markdown_results;
			benchmark_data.emplace_back(test_results);
			test_results = json_tests_helper<test_type::parse_and_serialize, discord_message, true, max_iterations, "Discord Test (Minified)">::run(discord_data_new);
			newer_string += test_results.markdown_results;
			benchmark_data.emplace_back(test_results);
		}
		{
			std::string canada_data{ bnch_swt::file_loader::load_file(json_path.operator std::string() + "/" + "/Canada Test (Prettified).json") };
			canada_message canada_data_new{};
			parser.parseJson<jsonifier::parse_options{ .knownOrder = true }>(canada_data_new, canada_data);
			for (auto& value: parser.getErrors()) {
				std::cout << "Error: " << value << std::endl;
			}
			bnch_swt::file_loader::save_file(canada_data, json_out_path.operator std::string() + "/Canada Test (Prettified).json");
			std::string canada_minified_data{ bnch_swt::file_loader::load_file(json_path.operator std::string() + "/" + "/Canada Test (Minified).json") };
			bnch_swt::file_loader::save_file(canada_minified_data, json_out_path.operator std::string() + "/Canada Test (Minified).json");
			test_results = json_tests_helper<test_type::parse_and_serialize, canada_message, false, max_iterations, "Canada Test (Prettified)">::run(canada_data_new);
			newer_string += test_results.markdown_results;
			benchmark_data.emplace_back(test_results);
			test_results = json_tests_helper<test_type::parse_and_serialize, canada_message, true, max_iterations, "Canada Test (Minified)">::run(canada_data_new);
			newer_string += test_results.markdown_results;
			benchmark_data.emplace_back(test_results);
		}
		{
			std::string citm_catalog_data{ bnch_swt::file_loader::load_file(json_path.operator std::string() + "/" + "/CitmCatalog Test (Prettified).json") };
			citm_catalog_message citm_catalog_data_new{};
			parser.parseJson<jsonifier::parse_options{ .knownOrder = true }>(citm_catalog_data_new, citm_catalog_data);
			for (auto& value: parser.getErrors()) {
				std::cout << "Error: " << value << std::endl;
			}
			bnch_swt::file_loader::save_file(citm_catalog_data, json_out_path.operator std::string() + "/CitmCatalog Test (Prettified).json");
			std::string citm_catalog_minified_data{ bnch_swt::file_loader::load_file(json_path.operator std::string() + "/" + "/CitmCatalog Test (Minified).json") };
			bnch_swt::file_loader::save_file(citm_catalog_minified_data, json_out_path.operator std::string() + "/CitmCatalog Test (Minified).json");
			test_results =
				json_tests_helper<test_type::parse_and_serialize, citm_catalog_message, false, max_iterations, "CitmCatalog Test (Prettified)">::run(citm_catalog_data_new);
			newer_string += test_results.markdown_results;
			benchmark_data.emplace_back(test_results);
			test_results = json_tests_helper<test_type::parse_and_serialize, citm_catalog_message, true, max_iterations, "CitmCatalog Test (Minified)">::run(citm_catalog_data_new);
			newer_string += test_results.markdown_results;
			benchmark_data.emplace_back(test_results);
		}
		{
			std::string twitter_data{ bnch_swt::file_loader::load_file(json_path.operator std::string() + "/" + "/Twitter Test (Prettified).json") };
			twitter_message twitter_data_new{};
			parser.parseJson<jsonifier::parse_options{ .knownOrder = true }>(twitter_data_new, twitter_data);
			for (auto& value: parser.getErrors()) {
				std::cout << "Error: " << value << std::endl;
			}
			bnch_swt::file_loader::save_file(twitter_data, json_out_path.operator std::string() + "/Twitter Test (Prettified).json");
			std::string twitter_minified_data{ bnch_swt::file_loader::load_file(json_path.operator std::string() + "/" + "/Twitter Test (Minified).json") };
			bnch_swt::file_loader::save_file(twitter_minified_data, json_out_path.operator std::string() + "/Twitter Test (Minified).json");
			bnch_swt::file_loader::save_file(twitter_data, json_out_path.operator std::string() + "/Minify Test.json");
			bnch_swt::file_loader::save_file(twitter_minified_data, json_out_path.operator std::string() + "/Prettify Test.json");
			bnch_swt::file_loader::save_file(twitter_data, json_out_path.operator std::string() + "/Validate Test.json");
			twitter_partial_message twitter_partial_data_new{};
			parser.parseJson<jsonifier::parse_options{}>(twitter_partial_data_new, twitter_data);
			test_results = json_tests_helper<test_type::parse_and_serialize, twitter_partial_message, false, max_iterations, "Twitter Partial Test (Prettified)">::run(
				twitter_partial_data_new);
			newer_string += test_results.markdown_results;
			benchmark_data.emplace_back(test_results);
			test_results =
				json_tests_helper<test_type::parse_and_serialize, twitter_partial_message, true, max_iterations, "Twitter Partial Test (Minified)">::run(twitter_partial_data_new);
			newer_string += test_results.markdown_results;
			benchmark_data.emplace_back(test_results);

			test_results = json_tests_helper<test_type::parse_and_serialize, twitter_message, false, max_iterations, "Twitter Test (Prettified)">::run(twitter_data_new);
			newer_string += test_results.markdown_results;
			benchmark_data.emplace_back(test_results);
			test_results = json_tests_helper<test_type::parse_and_serialize, twitter_message, true, max_iterations, "Twitter Test (Minified)">::run(twitter_data_new);
			newer_string += test_results.markdown_results;
			benchmark_data.emplace_back(test_results);
			test_results = json_tests_helper<test_type::minify, std::string, false, max_iterations, "Minify Test">::run(twitter_data);
			newer_string += test_results.markdown_results;
			benchmark_data.emplace_back(test_results);
			test_results = json_tests_helper<test_type::prettify, std::string, false, max_iterations, "Prettify Test">::run(twitter_minified_data);
			newer_string += test_results.markdown_results;
			benchmark_data.emplace_back(test_results);
			test_results = json_tests_helper<test_type::validate, std::string, false, max_iterations, "Validate Test">::run(twitter_data);
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
		bnch_swt::file_loader::save_file(results_string_json, json_out_path.operator std::string() + "/Results.json");
		bnch_swt::file_loader::save_file(static_cast<std::string>(newer_string), read_me_path.operator std::string() + "/" + current_path.operator std::string() + ".md");
		std::cout << "Md Data: " << newer_string << std::endl;
		execute_python_script(base_path.operator std::string() + "/GenerateGraphs.py", json_out_path.operator std::string() + "/Results.json", graphs_path.operator std::string());
#if !defined(NDEBUG)
		for (auto& value: jsonifier::internal::types) {
			std::cout << "TYPE: " << value.first << ", HASH-TYPE: " << value.second << std::endl;
		}
#endif
	};

}