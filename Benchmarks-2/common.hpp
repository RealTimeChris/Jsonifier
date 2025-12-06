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

#include <bnch_swt/index.hpp>
#include "citm_catalog.hpp"
#include "twitter.hpp"
#include "discord.hpp"
#include "canada.hpp"
#include <unordered_set>
#include <thread>
#include <random>

#if defined(NDEBUG)
static constexpr auto max_iterations{ 1400 };
static constexpr auto measured_iterations{ 20 };
#else
static constexpr auto max_iterations{ 200 };
static constexpr auto measured_iterations{ 25 };
#endif

constexpr auto get_current_operating_system() {
	constexpr bnch_swt::string_literal os_name{ OPERATING_SYSTEM_NAME };
	constexpr auto os_name_new = bnch_swt::internal::to_lower(os_name);
	if constexpr (os_name_new.operator std::string_view().contains("linux")) {
		return bnch_swt::string_literal{ "Ubuntu" };
	} else if constexpr (os_name_new.operator std::string_view().contains("windows")) {
		return bnch_swt::string_literal{ "Windows" };
	} else if constexpr (os_name_new.operator std::string_view().contains("darwin")) {
		return bnch_swt::string_literal{ "MacOS" };
	} else {
		return bnch_swt::string_literal{ "x" };
	}
}

constexpr auto get_current_compiler_id() {
	constexpr bnch_swt::string_literal compiler_id{ COMPILER_ID };
	constexpr auto os_compiler_id_new = bnch_swt::internal::to_lower(compiler_id);
	if constexpr (os_compiler_id_new.operator std::string_view().contains("gnu") || os_compiler_id_new.operator std::string_view().contains("gcc") ||
		os_compiler_id_new.operator std::string_view().contains("g++")) {
		return bnch_swt::string_literal{ "GCC" };
	} else if constexpr (os_compiler_id_new.operator std::string_view().contains("clang") || os_compiler_id_new.operator std::string_view().contains("appleclang")) {
		return bnch_swt::string_literal{ "CLANG" };
	} else if constexpr (os_compiler_id_new.operator std::string_view().contains("msvc")) {
		return bnch_swt::string_literal{ "MSVC" };
	} else {
		return bnch_swt::string_literal{ "x" };
	}
}

constexpr auto get_current_path_impl() {
	return get_current_operating_system() + "-" + get_current_compiler_id();
}

constexpr bnch_swt::string_literal current_path{ get_current_path_impl() };
constexpr bnch_swt::string_literal base_path{ BASE_PATH };
constexpr bnch_swt::string_literal test_path{ base_path + "/Source" };
constexpr bnch_swt::string_literal read_me_path{ BASE_PATH };
constexpr bnch_swt::string_literal json_path{ base_path + "/Json" };
constexpr bnch_swt::string_literal json_out_path{ json_path + "/" + get_current_path_impl() };
constexpr bnch_swt::string_literal graphs_path{ base_path + "/Graphs/" + get_current_path_impl() + "/" };
constexpr bnch_swt::string_literal jsonifier_library_name{ "jsonifier" };
constexpr bnch_swt::string_literal jsonifier_commit_url_base{ "https://github.com/realtimechris/jsonifier/commit/" };
constexpr bnch_swt::string_literal simdjson_library_name{ "simdjson" };
constexpr bnch_swt::string_literal simdjson_commit_url_base{ "https://github.com/simdjson/simdjson/commit/" };
constexpr bnch_swt::string_literal glaze_library_name{ "glaze" };
constexpr bnch_swt::string_literal glaze_commit_url_base{ "https://github.com/stephenberry/glaze/commit/" };
constexpr bnch_swt::string_literal jsonifier_commit_url{ jsonifier_commit_url_base + JSONIFIER_COMMIT };
constexpr bnch_swt::string_literal simdjson_commit_url{ simdjson_commit_url_base + SIMDJSON_COMMIT };
constexpr bnch_swt::string_literal glaze_commit_url{ glaze_commit_url_base + GLAZE_COMMIT };

std::string get_cpu_info() {
	char brand[49]{};
	int32_t regs[12]{};
	uint64_t length{};
#if defined(__x86_64__) || defined(_M_AMD64)
	static constexpr auto cpuid = [](int32_t* eax, int32_t* ebx, int32_t* ecx, int32_t* edx) {
	#if defined(_MSC_VER)
		int32_t cpu_info[4];
		__cpuidex(cpu_info, *eax, *ecx);
		*eax = cpu_info[0];
		*ebx = cpu_info[1];
		*ecx = cpu_info[2];
		*edx = cpu_info[3];
	#elif defined(HAVE_GCC_GET_CPUID) && defined(USE_GCC_GET_CPUID)
		uint32_t level = *eax;
		__get_cpuid(level, eax, ebx, ecx, edx);
	#else
		uint32_t a = *eax, b, c = *ecx, d;
		asm volatile("cpuid" : "=a"(a), "=b"(b), "=c"(c), "=d"(d) : "a"(a), "c"(c));
		*eax = a;
		*ebx = b;
		*ecx = c;
		*edx = d;
	#endif
	};
	regs[0] = 0x80000000;
	cpuid(regs, regs + 1, regs + 2, regs + 3);
	if (static_cast<uint32_t>(regs[0]) < 0x80000004) {
		return {};
	}
	regs[0] = 0x80000002;
	cpuid(regs, regs + 1, regs + 2, regs + 3);
	regs[4] = 0x80000003;
	cpuid(regs + 4, regs + 5, regs + 6, regs + 7);
	regs[8] = 0x80000004;
	cpuid(regs + 8, regs + 9, regs + 10, regs + 11);
	memcpy(brand, regs, sizeof(regs));
	length = std::strlen(brand) > 0 ? std::strlen(brand) - 1 : 0;
	std::string return_values{};
	return_values.resize(length - 1);
	std::copy(brand, brand + length, return_values.data());
	return return_values.substr(0, return_values.find_last_of("abcdefghijklmnopqrstuvwxyz1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ") + 1);
#else
	char new_buffer[256];
	size_t buffer_size = sizeof(new_buffer);
	if (sysctlbyname("machdep.cpu.brand_string", &new_buffer, &buffer_size, nullptr, 0) == 0) {
		return std::string(new_buffer);
	} else {
		return std::string{ "Unknown CPU" };
	}
#endif
}

void execute_python_script(const std::string& script_path, const std::string& argument_01, const std::string& argument_02) {
#if defined(JSONIFIER_WIN)
	static std::string python_name{ "python " };
#else
	static std::string python_name{ "python3 " };
#endif
	std::string command = python_name + script_path + " " + argument_01 + " " + argument_02;
	int32_t result		= system(command.data());
	if (result != 0) {
		std::cout << "Error: Failed to execute Python script. Command exited with code " << result << std::endl;
	}
}

struct test_struct {
	std::string test_string{};
	uint64_t test_uint{};
	int64_t test_int{};
	double test_double{};
	bool test_bool{};
};

struct partial_test_struct {
	std::string test_string{};
	bool test_bool{};
};

struct abc_test_struct {
	bool test_bool{};
	double test_double{};
	int64_t test_int{};
	uint64_t test_uint{};
	std::string test_string{};
};

template<typename value_type> struct test {
	std::vector<value_type> a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v, w, x, y, z;
};

template<typename value_type> struct partial_test {
	std::vector<value_type> m, s;
};

struct test_generator {
	template<jsonifier::concepts::string_t value_type> static value_type generate_value() {
		return bnch_swt::random_generator<std::string>::impl(bnch_swt::random_generator<uint64_t>::impl(16ull, 64ull));
	}

	template<jsonifier::concepts::float_t value_type> static value_type generate_value() {
		return bnch_swt::random_generator<double>::impl();
	}

	template<jsonifier::concepts::bool_t value_type> static value_type generate_value() {
		return bnch_swt::random_generator<bool>::impl();
	}

	template<jsonifier::concepts::uns64_t value_type> static value_type generate_value() {
		return bnch_swt::random_generator<uint64_t>::impl();
	}

	template<jsonifier::concepts::sig64_t value_type> static value_type generate_value() {
		return bnch_swt::random_generator<int64_t>::impl();
	}

	static test_struct generate_test_struct() {
		test_struct return_values{};
		return_values.test_bool	  = generate_value<bool>();
		return_values.test_double = generate_value<double>();
		return_values.test_int	  = generate_value<int64_t>();
		return_values.test_uint	  = generate_value<uint64_t>();
		return_values.test_string = generate_value<std::string>();
		return return_values;
	}

	template<typename value_type> static std::vector<std::vector<value_type>> generate_values(size_t vec_count, size_t value_count) {
		std::vector<std::vector<value_type>> return_values{};
		return_values.resize(vec_count);
		for (size_t x = 0; x < vec_count; ++x) {
			for (size_t y = 0; y < value_count; ++y) {
				return_values[x].emplace_back(generate_value<value_type>());
			}
		}
		return return_values;
	}

	static test<test_struct> generate_test() {
		test<test_struct> return_values{};
		auto fill = [&](auto& v) {
			const auto array_size_01 = bnch_swt::random_generator<uint64_t>::impl(1ull, 15ull);
			v.resize(array_size_01);
			for (uint64_t x = 0; x < array_size_01; ++x) {
				v[x] = generate_test_struct();
			}
		};

		fill(return_values.a);
		fill(return_values.b);
		fill(return_values.c);
		fill(return_values.d);
		fill(return_values.e);
		fill(return_values.f);
		fill(return_values.g);
		fill(return_values.h);
		fill(return_values.i);
		fill(return_values.j);
		fill(return_values.k);
		fill(return_values.l);
		fill(return_values.m);
		fill(return_values.n);
		fill(return_values.o);
		fill(return_values.p);
		fill(return_values.q);
		fill(return_values.r);
		fill(return_values.s);
		fill(return_values.t);
		fill(return_values.u);
		fill(return_values.v);
		fill(return_values.w);
		fill(return_values.x);
		fill(return_values.y);
		fill(return_values.z);
		return return_values;
	}
};

struct test_element_final {
	std::string library_name{};
	std::string result_type{};
	double result_speed{};
	std::string color{};
	bool operator>(const test_element_final& other) const noexcept {
		return result_speed > other.result_speed;
	}
};

struct test_element_pair {
	test_element_final write_data{};
	test_element_final read_data{};
	bool operator>(const test_element_pair& other) const noexcept {
		return write_data > other.write_data && read_data > other.read_data;
	}
};

struct test_elements_final {
	std::vector<test_element_final> results{};
	std::string test_name{};
};

using test_results_final = std::vector<test_elements_final>;

template<typename value_type> struct abc_test {
	std::vector<value_type> z, y, x, w, v, u, t, s, r, q, p, o, n, m, l, k, j, i, h, g, f, e, d, c, b, a;
};

enum class result_type { read = 0, write = 1 };

template<result_type type> constexpr auto enum_to_string() {
	if constexpr (type == result_type::read) {
		return bnch_swt::string_literal{ "Read" };
	} else {
		return bnch_swt::string_literal{ "Write" };
	}
}

template<result_type type> struct result {
	std::optional<double> json_speed_percentage_deviation{};
	std::optional<uint64_t> byte_length{};
	std::optional<double> json_cycles{};
	std::optional<double> json_speed{};
	std::optional<double> json_time{};
	std::string color{};

	result() noexcept = default;

	result& operator=(result&&) noexcept	  = default;
	result(result&&) noexcept				  = default;
	result& operator=(const result&) noexcept = default;
	result(const result&) noexcept			  = default;

	result(const std::string& color_new, const bnch_swt::performance_metrics<bnch_swt::benchmark_types::cpu>& results) {
		byte_length.emplace(results.bytes_processed);
		json_time.emplace(results.time_in_ns);
		json_speed_percentage_deviation.emplace(results.throughput_percentage_deviation);
		json_speed.emplace(results.throughput_mb_per_sec);
		if (results.cycles_per_byte.has_value()) {
			json_cycles.emplace(results.cycles_per_byte.value() * 1024 * 1024);
		}
		color = color_new;
	}

	operator bool() const noexcept {
		return json_speed.has_value();
	}

	bool operator>(const result& other) const noexcept {
		if (json_speed.has_value() && other.json_speed.has_value()) {
			return json_speed.value() > other.json_speed.value();
		} else if (!json_speed.has_value()) {
			return false;
		} else if (!other.json_speed.has_value()) {
			return true;
		}
		return false;
	}
};

struct results_data {
	std::unordered_set<std::string> jsonifier_excluded_keys{};
	result<result_type::write> write_result{};
	result<result_type::read> read_result{};
	std::string name{};
	std::string test{};
	std::string url{};

	bool operator>(const results_data& other) const noexcept {
		if (read_result && other.read_result) {
			return read_result > other.read_result;
		} else if (write_result && other.write_result) {
			return write_result > other.write_result;
		} else {
			return false;
		}
	}

	results_data() noexcept = default;

	results_data(const std::string& name_new, const std::string& test_new, const std::string& url_new) {
		name = name_new;
		test = test_new;
		url	 = url_new;
	}

	void check_for_missing_keys() {
		if (!write_result.json_speed.has_value()) {
			jsonifier_excluded_keys.emplace("write_result");
		} else if (!read_result.json_speed.has_value()) {
			jsonifier_excluded_keys.emplace("read_result");
		}
	}

	std::string json_stats() const noexcept {
		std::string final_string{ "| [" + name + "](" + url + ") | " };
		if (read_result.json_time.has_value() && read_result.byte_length.has_value()) {
			std::ostringstream final_stream{};
			final_stream << std::setprecision(6) << read_result.json_speed.value() << " | " << std::setprecision(6) << read_result.json_speed_percentage_deviation.value() << " | ";
			if (read_result.json_cycles.has_value()) {
				final_stream << std::setprecision(6) << read_result.json_cycles.value() << " | ";
			}
			final_stream << static_cast<uint64_t>(read_result.byte_length.value()) << " | " << std::setprecision(6) << read_result.json_time.value() << " | ";
			final_string += final_stream.str();
		}
		if (write_result.json_time.has_value() && write_result.byte_length.has_value()) {
			std::ostringstream final_stream{};
			final_stream << std::setprecision(6) << write_result.json_speed.value() << " | " << std::setprecision(6) << write_result.json_speed_percentage_deviation.value()
						 << " | ";
			if (write_result.json_cycles.has_value()) {
				final_stream << std::setprecision(6) << write_result.json_cycles.value() << " | ";
			}
			final_stream << static_cast<uint64_t>(write_result.byte_length.value()) << " | " << std::setprecision(6) << write_result.json_time.value() << " | ";
			final_string += final_stream.str();
		}
		return final_string;
	}
};

struct test_results {
	std::vector<results_data> results{};
	std::string markdown_results{};
	std::string test_name{};
};

std::tm get_time() {
#if defined(JSONIFIER_WIN)
	std::time_t result = std::time(nullptr);
	std::tm result_two{};
	localtime_s(&result_two, &result);
	return result_two;
#else
	std::time_t result = std::time(nullptr);
	return *localtime(&result);
#endif
}

static std::string url_encode(std::string value) {
	std::ostringstream escaped;
	escaped.fill('0');
	escaped << std::hex;

	for (char c: value) {
		if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
			escaped << c;
		} else if (c == ':') {
			escaped << '%' << std::setw(2) << int32_t(( unsigned char )' ');
		} else {
			escaped << '%' << std::setw(2) << int32_t(( unsigned char )c);
		}
	}

	return escaped.str();
}