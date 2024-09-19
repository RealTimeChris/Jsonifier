#if defined(JSONIFIER_CPU_INSTRUCTIONS)
//#undef JSONIFIER_CPU_INSTRUCTIONS
//#define JSONIFIER_CPU_INSTRUCTIONS (JSONIFIER_AVX2 | JSONIFIER_POPCNT)
#endif
#include "UnicodeEmoji.hpp"
#include <BnchSwt/BenchmarkSuite.hpp>
#include "Simdjson.hpp"
#include "Jsonifier.hpp"
#include "Glaze.hpp"
#include <unordered_set>
#include <unordered_map>
#include <filesystem>
#include <algorithm>
#include <iostream>
#include <random>
#include <thread>
#include <chrono>

constexpr jsonifier_internal::string_literal jsonifierLibraryName{ "jsonifier" };
constexpr jsonifier_internal::string_literal simdjsonLibraryName{ "simdjson" };
constexpr jsonifier_internal::string_literal glazeLibraryName{ "glaze" };
constexpr jsonifier_internal::string_literal jsonifierCommitUrl{ jsonifier_internal::string_literal{ "https://github.com/realtimechris/jsonifier/commit/" } + JSONIFIER_COMMIT };
constexpr jsonifier_internal::string_literal glazeCommitUrl{ jsonifier_internal::string_literal{ "https://github.com/stephenberry/glaze/commit/" } + GLAZE_COMMIT };
constexpr jsonifier_internal::string_literal simdjsonCommitUrl{ jsonifier_internal::string_literal{ "https://github.com/simdjson/simdjson/commit/" } + SIMDJSON_COMMIT };

#if defined(NDEBUG)
constexpr size_t totalIterationCountCap{ 500 };
#else
constexpr size_t totalIterationCountCap{ 101 };
#endif

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

constexpr jsonifier_internal::string_literal basePath{ JSON_PATH };

template<json_library lib, test_type type, typename test_data_type, bool minified, size_t iterations, jsonifier_internal::string_literal testName> struct json_test_helper {};

std::string table_header = R"(
| Library | Read (MB/s) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/s) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | ------------------- | -------------- | -------------------- | ------------ | -------------------- | --------------- | --------------------- |   )";

std::string read_table_header = R"(
| Library | Read (MB/s) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count |
| ------- | ----------- | ------------------- | -------------- | -------------------- |   )";

std::string write_table_header = R"(
| Library | Write (MB/s) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ------------ | -------------------- | --------------- | --------------------- |   )";

#if defined(__x86_64__) || defined(_M_AMD64)
static inline void cpuid(uint32_t* eax, uint32_t* ebx, uint32_t* ecx, uint32_t* edx);
inline static uint64_t xgetbv();
#endif

std::string getCPUInfo() {
	char brand[49] = { 0 };
	uint32_t regs[12]{};
#if defined(__x86_64__) || defined(_M_AMD64)
	regs[0] = 0x80000000;
	cpuid(regs, regs + 1, regs + 2, regs + 3);
	if (regs[0] < 0x80000004) {
		return {};
	}
	regs[0] = 0x80000002;
	cpuid(regs, regs + 1, regs + 2, regs + 3);
	regs[4] = 0x80000003;
	cpuid(regs + 4, regs + 5, regs + 6, regs + 7);
	regs[8] = 0x80000004;
	cpuid(regs + 8, regs + 9, regs + 10, regs + 11);

	memcpy(brand, regs, sizeof(regs));
#endif
	return { brand, sizeof(regs) };
}

#if defined(__x86_64__) || defined(_M_AMD64)

static inline void cpuid(uint32_t* eax, uint32_t* ebx, uint32_t* ecx, uint32_t* edx) {
	#if defined(_MSC_VER)
	int32_t cpuInfo[4];
	__cpuidex(cpuInfo, *eax, *ecx);
	*eax = cpuInfo[0];
	*ebx = cpuInfo[1];
	*ecx = cpuInfo[2];
	*edx = cpuInfo[3];
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
}
#endif

static const std::string section001{ R"(
 > At least )" +
	jsonifier::toString(30) + R"( iterations on a ()" + getCPUInfo() + R"(), until coefficient of variance is at or below 1%.
)" };

constexpr auto newString02{ jsonifier_internal::string_literal{ R"(#### Using the following commits:
----
| Jsonifier: [)" } +
	jsonifier_internal::string_literal{ JSONIFIER_COMMIT } + jsonifier_internal::string_literal{ R"(](https://github.com/RealTimeChris/Jsonifier/commit/)" } +
	jsonifier_internal::string_literal{ JSONIFIER_COMMIT } + jsonifier_internal::string_literal{ ")  \n" } + jsonifier_internal::string_literal{ R"(| Glaze: [)" } +
	jsonifier_internal::string_literal{ GLAZE_COMMIT } + jsonifier_internal::string_literal{ R"(](https://github.com/stephenberry/glaze/commit/)" } +
	jsonifier_internal::string_literal{ GLAZE_COMMIT } + jsonifier_internal::string_literal{ ")  \n" } + jsonifier_internal::string_literal{ R"(| Simdjson: [)" } +
	jsonifier_internal::string_literal{ SIMDJSON_COMMIT } + jsonifier_internal::string_literal{ R"(](https://github.com/simdjson/simdjson/commit/)" } +
	jsonifier_internal::string_literal{ SIMDJSON_COMMIT } + jsonifier_internal::string_literal{ ")  \n" } };

constexpr jsonifier::string_view section002{ newString02.view() };

constexpr jsonifier::string_view section00{ R"(# Json-Performance
Performance profiling of JSON libraries (Compiled and run on Ubuntu-22.04 using the Clang++19 compiler)

Latest Results: ()" };

constexpr jsonifier::string_view section01{
	R"(

### Json Test (Prettified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/JsonData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Json%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Json%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>

)"
};

constexpr jsonifier::string_view section02{
	R"(

### Json Test (Minified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/JsonData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Json%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Json%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>

)"
};

constexpr jsonifier::string_view section03{
	R"(

### ABC Test (Out of Sequence Performance - Prettified) [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/JsonData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Abc%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Abc%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>

The JSON documents in the previous tests featured keys ranging from "a" to "z," where each key corresponds to an array of values. Notably, the documents in this test arrange these keys in reverse order, deviating from the typical "a" to "z" arrangement.

This test effectively demonstrates the challenges encountered when utilizing simdjson and iterative parsers that lack the ability to efficiently allocate memory locations through hashing. In cases where the keys are not in the expected sequence, performance is significantly compromised, with the severity escalating as the document size increases.

In contrast, hash-based solutions offer a viable alternative by circumventing these issues and maintaining optimal performance regardless of the JSON document's scale, or ordering of the keys being parsed.
)"
};

constexpr jsonifier::string_view section04{ R"(

### ABC Test (Out of Sequence Performance - Minified) [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/JsonData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Abc%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Abc%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>

)" };

constexpr jsonifier::string_view section05{ R"(

### Discord Test (Prettified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Discord%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Discord%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>

)" };

constexpr jsonifier::string_view section06{ R"(

### Discord Test (Minified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Discord%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Discord%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>

)" };

constexpr jsonifier::string_view section07{ R"(

### Canada Test (Prettified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/CanadaData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Canada%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Canada%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>

)" };

constexpr jsonifier::string_view section08{ R"(

### Canada Test (Minified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/CanadaData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Canada%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Canada%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>

)" };

constexpr jsonifier::string_view section09{ R"(

### CitmCatalog Test (Prettified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/CitmCatalogData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/CitmCatalog%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/CitmCatalog%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>

)" };

constexpr jsonifier::string_view section10{ R"(

### CitmCatalog Test (Minified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/CitmCatalogData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/CitmCatalog%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/CitmCatalog%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>

)" };

constexpr jsonifier::string_view section11{ R"(

### Twitter Test (Prettified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/TwitterData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Twitter%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Twitter%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>

)" };

constexpr jsonifier::string_view section12{ R"(

### Twitter Test (Minified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/TwitterData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Twitter%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Twitter%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>

)" };

constexpr jsonifier::string_view section13{
	R"(

### Minify Test Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Minify%20Test_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Minify%20Test_Results.png?raw=true" 
alt="" width="400"/></p>

)"
};

constexpr jsonifier::string_view section14{
	R"(

### Prettify Test Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Prettify%20Test_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Prettify%20Test_Results.png?raw=true" 
alt="" width="400"/></p>

)"
};

constexpr jsonifier::string_view section15{
	R"(

### Validation Test Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Validate%20Test_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Validate%20Test_Results.png?raw=true" 
alt="" width="400"/></p>

)"
};

std::string getCurrentWorkingDirectory() {
	try {
		return std::filesystem::current_path().string();
	} catch (const std::filesystem::filesystem_error& e) {
		std::cout << "Error: " << e.what() << std::endl;
		return "";
	}
}

void executePythonScript(const jsonifier::string& scriptPath, const jsonifier::string& argument01, const jsonifier::string& argument02) {
#if defined(JSONIFIER_WIN)
	constexpr std::string_view pythonName{ "python " };
#else
	constexpr std::string_view pythonName{ "python3 " };
#endif
	jsonifier::string command = static_cast<jsonifier::string>(pythonName) + scriptPath + " " + argument01 + " " + argument02;
	int32_t result			  = system(command.data());
	if (result != 0) {
		std::cout << "Error: Failed to execute Python script. Command exited with code " << result << std::endl;
	}
}

template<test_type type, typename test_data_type, bool minified, jsonifier_internal::string_literal testName> struct json_tests_helper;

template<jsonifier_internal::string_literal testName> struct json_tests_helper<test_type::prettify, std::string, false, testName> {
	JSONIFIER_INLINE static test_results run(const std::string& jsonData) {
		jsonifier::vector<results_data> resultsNew{};
		test_results jsonResults{};
		jsonResults.testName = static_cast<jsonifier::string>(testName);
#if !defined(ASAN_ENABLED)
		std::string glazeBuffer{ jsonData };
		std::string glazeNewerBuffer{};
		auto glazeWriteResult = bnch_swt::benchmark_stage<"Json-Performance",
			bnch_swt::bench_options{ .type = bnch_swt::result_type::time, .totalIterationCountCap = totalIterationCountCap }>::runBenchmark<testName + "-Write", glazeLibraryName,
			"steelblue">([&]() {
			glz::prettify_json(glazeBuffer, glazeNewerBuffer);
			bnch_swt::doNotOptimizeAway(glazeNewerBuffer);
		});

		results_data glazeResults{ static_cast<jsonifier::string>(glazeLibraryName), static_cast<jsonifier::string>(testName), static_cast<jsonifier::string>(glazeCommitUrl),
			glazeWriteResult.iterationCount };

		bnch_swt::file_loader<basePath + "/" + testName + "-glaze.json">::saveFile(glazeNewerBuffer);
		glazeResults.writeResult = result<result_type::write>{ "skyblue", glazeNewerBuffer.size(), glazeWriteResult };
		glazeResults.print();

		resultsNew.emplace_back(glazeResults);
#endif
		std::string jsonifierBuffer{ jsonData };

		jsonifier::jsonifier_core parser{};
		std::string jsonifierNewerBuffer{};
		auto jsonifierWriteResult = bnch_swt::benchmark_stage<"Json-Performance",
			bnch_swt::bench_options{ .type = bnch_swt::result_type::time, .totalIterationCountCap = totalIterationCountCap }>::runBenchmark<testName + "-Write",
			jsonifierLibraryName, "steelblue">([&]() {
			parser.prettifyJson(jsonifierBuffer, jsonifierNewerBuffer);
			bnch_swt::doNotOptimizeAway(jsonifierNewerBuffer);
		});

		results_data jsonifierResults{ static_cast<jsonifier::string>(jsonifierLibraryName), static_cast<jsonifier::string>(testName),
			static_cast<jsonifier::string>(jsonifierCommitUrl), jsonifierWriteResult.iterationCount };

		for (auto& value: parser.getErrors()) {
			std::cout << "Jsonifier Error: " << value << std::endl;
		}
		bnch_swt::file_loader<basePath + "/" + testName + "-jsonifier.json">::saveFile(jsonifierNewerBuffer);

		jsonifierResults.writeResult = result<result_type::write>{ "steelblue", jsonifierNewerBuffer.size(), jsonifierWriteResult };

		jsonifierResults.print();

		resultsNew.emplace_back(jsonifierResults);

		std::string table{};
		const auto n = resultsNew.size();
		table += write_table_header + "\n";
		std::sort(resultsNew.begin(), resultsNew.end(), std::greater<results_data>());
		for (size_t x = 0; x < n; ++x) {
			jsonResults.results.emplace_back(resultsNew[x]);
			table += resultsNew[x].jsonStats();
			if (x != n - 1) {
				table += "\n";
			}
		}
		jsonResults.markdownResults = table;
		return jsonResults;
	}
};

template<jsonifier_internal::string_literal testName> struct json_tests_helper<test_type::minify, std::string, false, testName> {
	JSONIFIER_INLINE static test_results run(const std::string& jsonData) {
		jsonifier::vector<results_data> resultsNew{};
		test_results jsonResults{};
		jsonResults.testName = static_cast<jsonifier::string>(testName);
#if !defined(ASAN_ENABLED)

		std::string simdjsonBuffer{ jsonData };

		simdjson::dom::parser simdjsonParser{};
		std::string simdjsonNewerBuffer{};

		auto simdjsonWriteResult = bnch_swt::benchmark_stage<"Json-Performance",
			bnch_swt::bench_options{ .type = bnch_swt::result_type::time, .totalIterationCountCap = totalIterationCountCap }>::runBenchmark<testName + "-Write", glazeLibraryName,
			"cornflowerblue">([&]() {
			try {
				simdjsonNewerBuffer = simdjson::minify(simdjsonParser.parse(simdjsonBuffer));
				bnch_swt::doNotOptimizeAway(simdjsonNewerBuffer);
			} catch (std::exception& error) {
				std::cout << "Simdjson Error: " << error.what() << std::endl;
			}
			return;
		});

		results_data simdjsonResults{ static_cast<jsonifier::string>(simdjsonLibraryName), static_cast<jsonifier::string>(testName),
			static_cast<jsonifier::string>(simdjsonCommitUrl), simdjsonWriteResult.iterationCount };

		bnch_swt::file_loader<basePath + "/" + testName + "-simdjson.json">::saveFile(simdjsonNewerBuffer);
		simdjsonResults.writeResult = result<result_type::write>{ "cornflowerblue", simdjsonNewerBuffer.size(), simdjsonWriteResult };

		simdjsonResults.print();

		resultsNew.emplace_back(simdjsonResults);
		std::string glazeBuffer{ jsonData };


		std::string glazeNewerBuffer{};
		auto glazeWriteResult = bnch_swt::benchmark_stage<"Json-Performance",
			bnch_swt::bench_options{ .type = bnch_swt::result_type::time, .totalIterationCountCap = totalIterationCountCap }>::runBenchmark<testName + "-Write", glazeLibraryName,
			"steelblue">([&]() {
			glazeNewerBuffer = glz::minify_json(glazeBuffer);
			bnch_swt::doNotOptimizeAway(glazeNewerBuffer);
		});
		results_data glazeResults{ static_cast<jsonifier::string>(glazeLibraryName), static_cast<jsonifier::string>(testName), static_cast<jsonifier::string>(glazeCommitUrl),
			glazeWriteResult.iterationCount };

		bnch_swt::file_loader<basePath + "/" + testName + "-glaze.json">::saveFile(glazeNewerBuffer);
		glazeResults.writeResult = result<result_type::write>{ "skyblue", glazeNewerBuffer.size(), glazeWriteResult };
		glazeResults.print();
		resultsNew.emplace_back(glazeResults);
#endif
		std::string jsonifierBuffer{ jsonData };
		std::string jsonifierNewerBuffer{};
		jsonifier::jsonifier_core parser{};
		auto jsonifierWriteResult = bnch_swt::benchmark_stage<"Json-Performance",
			bnch_swt::bench_options{ .type = bnch_swt::result_type::time, .totalIterationCountCap = totalIterationCountCap }>::runBenchmark<testName + "-Read",
			jsonifierLibraryName, "steelblue">([&]() {
			parser.minifyJson(jsonifierBuffer, jsonifierNewerBuffer);
			bnch_swt::doNotOptimizeAway(jsonifierNewerBuffer);
		});
		results_data jsonifierResults{ static_cast<jsonifier::string>(jsonifierLibraryName), static_cast<jsonifier::string>(testName),
			static_cast<jsonifier::string>(jsonifierCommitUrl), jsonifierWriteResult.iterationCount };
		for (auto& value: parser.getErrors()) {
			std::cout << "Jsonifier Error: " << value << std::endl;
		}
		bnch_swt::file_loader<basePath + "/" + testName + "-jsonifier.json">::saveFile(jsonifierNewerBuffer);

		jsonifierResults.writeResult = result<result_type::write>{ "steelblue", jsonifierNewerBuffer.size(), jsonifierWriteResult };
		jsonifierResults.print();
		resultsNew.emplace_back(jsonifierResults);
		std::string table{};
		const auto n = resultsNew.size();
		table += write_table_header + "\n";
		std::sort(resultsNew.begin(), resultsNew.end(), std::greater<results_data>());
		for (size_t x = 0; x < n; ++x) {
			jsonResults.results.emplace_back(resultsNew[x]);
			table += resultsNew[x].jsonStats();
			if (x != n - 1) {
				table += "\n";
			}
		}
		jsonResults.markdownResults = table;
		return jsonResults;
	}
};

template<jsonifier_internal::string_literal testName> struct json_tests_helper<test_type::validate, std::string, false, testName> {
	JSONIFIER_INLINE static test_results run(const std::string& jsonData) {
		jsonifier::vector<results_data> resultsNew{};
		test_results jsonResults{};
		jsonResults.testName = static_cast<jsonifier::string>(testName);
#if !defined(ASAN_ENABLED)

		std::string glazeBuffer{ jsonData };
		auto glazeReadResult = bnch_swt::benchmark_stage<"Json-Performance",
			bnch_swt::bench_options{ .type = bnch_swt::result_type::time, .totalIterationCountCap = totalIterationCountCap }>::runBenchmark<testName + "-Read", glazeLibraryName,
			"skyblue">([&]() {
			auto result = glz::validate_json(glazeBuffer);
			bnch_swt::doNotOptimizeAway(result);
		});

		results_data glazeResults{ static_cast<jsonifier::string>(glazeLibraryName), static_cast<jsonifier::string>(testName), static_cast<jsonifier::string>(glazeCommitUrl),
			glazeReadResult.iterationCount };

		bnch_swt::file_loader<basePath + "/" + testName + "-glaze.json">::saveFile(glazeBuffer);
		glazeResults.readResult = result<result_type::read>{ "skyblue", glazeBuffer.size(), glazeReadResult };
		glazeResults.print();
		resultsNew.emplace_back(glazeResults);
#endif
		std::string jsonifierBuffer{ jsonData };
		jsonifier::jsonifier_core parser{};
		auto jsonifierReadResult = bnch_swt::benchmark_stage<"Json-Performance",
			bnch_swt::bench_options{ .type = bnch_swt::result_type::time, .totalIterationCountCap = totalIterationCountCap }>::runBenchmark<testName + "-Read",
			jsonifierLibraryName, "steelblue">([&]() {
			parser.validateJson(jsonifierBuffer);
			bnch_swt::doNotOptimizeAway(jsonifierBuffer);
		});

		results_data jsonifierResults{ static_cast<jsonifier::string>(jsonifierLibraryName), static_cast<jsonifier::string>(testName),
			static_cast<jsonifier::string>(jsonifierCommitUrl), jsonifierReadResult.iterationCount };

		for (auto& value: parser.getErrors()) {
			std::cout << "Jsonifier Error: " << value << std::endl;
		}

		jsonifierResults.readResult = result<result_type::read>{ "teal", jsonifierBuffer.size(), jsonifierReadResult };

		jsonifierResults.print();

		resultsNew.emplace_back(jsonifierResults);

		std::string table{};
		const auto n = resultsNew.size();
		table += read_table_header + "\n";
		std::sort(resultsNew.begin(), resultsNew.end(), std::greater<results_data>());
		for (size_t x = 0; x < n; ++x) {
			jsonResults.results.emplace_back(resultsNew[x]);
			table += resultsNew[x].jsonStats();
			if (x != n - 1) {
				table += "\n";
			}
		}
		jsonResults.markdownResults = table;
		return jsonResults;
	}
};

#include "ConformanceTests.hpp"
#include "FailTests.hpp"

int32_t main() {
	try {
		std::bitset<2048> bits{};
		bits.operator[](2034);
		conformanceTests();
		failTests();
		test_generator<test_struct> testJsonData{};
		std::string jsonData{};
		jsonifier::jsonifier_core parser{};
		auto fileLoader02 = bnch_swt::file_loader<basePath + "/JsonData-Prettified.json">::loadFile();
		parser.serializeJson<jsonifier::serialize_options{ .prettify = true }>(testJsonData, jsonData);
		bnch_swt::file_loader<basePath + "/JsonData-Prettified.json">::saveFile(jsonData);
		auto fileLoader03 = bnch_swt::file_loader<basePath + "/JsonData-Minified.json">::loadFile();
		std::string jsonMinifiedData{ parser.minifyJson(jsonData) };
		bnch_swt::file_loader<basePath + "/JsonData-Minified.json">::saveFile(jsonMinifiedData);
		auto fileLoader04 = bnch_swt::file_loader<basePath + "/Results.json">::loadFile();
		auto fileLoader05 = bnch_swt::file_loader<basePath + "/DiscordData-Prettified.json">::loadFile();
		std::string discordData{ fileLoader05 };
		discord_message discordMessage{};
		//std::cout << "WERE HERE THIS IS IT 0101: " << jsonMinifiedData << std::endl;
		parser.parseJson(discordMessage, discordData);
		for (auto& value: parser.getErrors()) {
			std::cout << "PARSER ERROR: " << value << std::endl;
		}
		//std::cout << "WERE HERE THIS IS IT 0101: " << discordData << std::endl;
		bnch_swt::file_loader<basePath + "/DiscordData-Prettified.json">::saveFile(discordData);
		auto fileLoader06 = bnch_swt::file_loader<basePath + "/DiscordData-Minified.json">::loadFile();
		std::string discordMinifiedData{ fileLoader06 };
		discordMinifiedData = parser.minifyJson(discordData);
		bnch_swt::file_loader<basePath + "/DiscordData-Minified.json">::saveFile(discordMinifiedData);
		auto fileLoader07 = bnch_swt::file_loader<basePath + "/CanadaData-Prettified.json">::loadFile();
		std::string canadaData{ fileLoader07 };
		canada_message canadaMessage{};
		parser.parseJson(canadaMessage, canadaData);
		for (auto& value: parser.getErrors()) {
			std::cout << "PARSER ERROR: " << value << std::endl;
		}
		//std::cout << "WERE HERE THIS IS IT 0202: " << canadaData << std::endl;
		bnch_swt::file_loader<basePath + "/CanadaData-Prettified.json">::saveFile(canadaData);
		auto fileLoader08 = bnch_swt::file_loader<basePath + "/CanadaData-Minified.json">::loadFile();
		std::string canadaMinifiedData{ fileLoader08 };
		canadaMinifiedData = parser.minifyJson(canadaData);
		bnch_swt::file_loader<basePath + "/CanadaData-Minified.json">::saveFile(canadaMinifiedData);
		auto fileLoader09 = bnch_swt::file_loader<basePath + "/TwitterData-Prettified.json">::loadFile();
		std::string twitterData{ fileLoader09 };
		twitter_message twitterMessage{};
		parser.parseJson(twitterMessage, twitterData);
		for (auto& value: parser.getErrors()) {
			std::cout << "PARSER ERROR: " << value << std::endl;
		}
		//std::cout << "WERE HERE THIS IS IT 0303: " << twitterData << std::endl;
		bnch_swt::file_loader<basePath + "/TwitterData-Prettified.json">::saveFile(twitterData);
		auto fileLoader10 = bnch_swt::file_loader<basePath + "/TwitterData-Minified.json">::loadFile();
		std::string twitterMinifiedData{ fileLoader10 };
		twitterMinifiedData = parser.minifyJson(twitterData);
		bnch_swt::file_loader<basePath + "/TwitterData-Minified.json">::saveFile(twitterMinifiedData);
		auto fileLoader11 = bnch_swt::file_loader<basePath + "/CitmCatalogData-Prettified.json">::loadFile();
		std::string citmCatalogData{ fileLoader11 };
		citm_catalog_message citmCatalogMessage{};
		parser.parseJson(citmCatalogMessage, citmCatalogData);
		for (auto& value: parser.getErrors()) {
			std::cout << "PARSER ERROR: " << value << std::endl;
		}
		//std::cout << "WERE HERE THIS IS IT 0303: " << citmCatalogData << std::endl;
		bnch_swt::file_loader<basePath + "/CitmCatalogData-Prettified.json">::saveFile(citmCatalogData);
		auto fileLoader12 = bnch_swt::file_loader<basePath + "/CitmCatalogData-Minified.json">::loadFile();
		std::string citmCatalogMinifiedData{ fileLoader12 };
		citmCatalogMinifiedData = parser.minifyJson(citmCatalogData);
		bnch_swt::file_loader<basePath + "/CitmCatalogData-Minified.json">::saveFile(citmCatalogMinifiedData);
		for (auto& value: parser.getErrors()) {
			std::cout << "PARSER ERROR: " << value << std::endl;
		}
		std::string newTimeString{};
		newTimeString.resize(1024);
		std::tm resultTwo{};
		std::time_t timeResult = std::time(nullptr);
		resultTwo			   = *localtime(&timeResult);
		std::vector<test_results> benchmark_data{};
		newTimeString.resize(strftime(newTimeString.data(), 1024, "%b %d, %Y", &resultTwo));
		jsonifier::string newerString{ section00 + newTimeString + ")\n" + section002 + section001 + section01 };

		std::string jsonifierBuffer{ jsonData };
		std::string simdjsonBuffer{ jsonData };
		std::string glazeBuffer{ jsonData };
#if !defined(ASAN_ENABLED)
		simdjson::ondemand::parser simdjsonParser{};
#endif
		{
			jsonifier::vector<results_data> resultsNew{};
			test_results jsonResults{};
			jsonResults.testName = static_cast<jsonifier::string>("Json Test (Prettified)");
#if !defined(ASAN_ENABLED)
			test<test_struct> simdjsonTestData{};
			auto simdjsonReadResult = bnch_swt::benchmark_stage<jsonifier_internal::string_literal{ "Json-Performance" },
				bnch_swt::bench_options{ .type = bnch_swt::result_type::time, .totalIterationCountCap = totalIterationCountCap }>::
				runBenchmark<jsonifier_internal::string_literal{ "Json Test (Prettified)" } + "-Read", simdjsonLibraryName, "cadetblue">([&]() {
					try {
						getValue(simdjsonTestData, simdjsonParser.iterate(simdjsonBuffer).value());
						bnch_swt::doNotOptimizeAway(simdjsonTestData);
					} catch (std::exception& error) {
						std::cout << "Simdjson Error: " << error.what() << std::endl;
					}
				});
			parser.serializeJson<jsonifier::serialize_options{ .prettify = true }>(simdjsonTestData, simdjsonBuffer);
			auto simdjsonReadSize = simdjsonBuffer.size();

			results_data simdjsonResults{ static_cast<jsonifier::string>(simdjsonLibraryName), static_cast<jsonifier::string>("Json Test (Prettified)"),
				static_cast<jsonifier::string>(simdjsonCommitUrl), simdjsonReadResult.iterationCount };
			simdjsonResults.readResult = result<result_type::read>{ "cadetblue", simdjsonReadSize, simdjsonReadResult };
			bnch_swt::file_loader<basePath + "/" + "Json Test (Prettified)" + "-simdjson.json">::saveFile(simdjsonBuffer);
			simdjsonResults.print();
			resultsNew.emplace_back(simdjsonResults);

			test<test_struct> glazeTestData{};
			auto glazeReadResult =
				bnch_swt::benchmark_stage<"Json-Performance", bnch_swt::bench_options{ .type = bnch_swt::result_type::time, .totalIterationCountCap = totalIterationCountCap }>::
					runBenchmark<jsonifier_internal::string_literal{ "Json Test (Prettified)" } + "-Read", glazeLibraryName, "dodgerblue">([&]() {
						if (auto error = glz::read<glz::opts{ .skip_null_members = false, .minified = false }>(glazeTestData, glazeBuffer); error) {
							std::cout << "Glaze Error: " << glz::format_error(error, glazeBuffer) << std::endl;
						}
						auto* newPtr = &glazeTestData;
						bnch_swt::doNotOptimizeAway(newPtr);
					});

			results_data glazeResults{ static_cast<jsonifier::string>(glazeLibraryName), static_cast<jsonifier::string>("Json Test (Prettified)"),
				static_cast<jsonifier::string>(glazeCommitUrl), glazeReadResult.iterationCount };
			std::string glazeNewerBuffer{};
			auto glazeWriteResult =
				bnch_swt::benchmark_stage<"Json-Performance", bnch_swt::bench_options{ .type = bnch_swt::result_type::time, .totalIterationCountCap = totalIterationCountCap }>::
					runBenchmark<jsonifier_internal::string_literal{ "Json Test (Prettified)" } + "-Write", glazeLibraryName, "steelblue">([&]() {
						bnch_swt::doNotOptimizeAway(glz::write<glz::opts{ .skip_null_members = false, .prettify = true, .minified = false }>(glazeTestData, glazeNewerBuffer));
						auto* newPtr = &glazeNewerBuffer;
						bnch_swt::doNotOptimizeAway(newPtr);
					});

			auto glazeReadSize		 = glazeNewerBuffer.size();
			auto glazeWrittenSize	 = glazeNewerBuffer.size();
			glazeResults.readResult	 = result<result_type::read>{ "dodgerblue", glazeReadSize, glazeReadResult };
			glazeResults.writeResult = result<result_type::write>{ "skyblue", glazeWrittenSize, glazeWriteResult };
			bnch_swt::file_loader<basePath + "/" + "Json Test (Prettified)" + "-glaze.json">::saveFile(glazeBuffer);
			glazeResults.print();
			resultsNew.emplace_back(glazeResults);
#endif
			test<test_struct> jsonifierTestData{};
			auto jsonifierReadResult =
				bnch_swt::benchmark_stage<"Json-Performance", bnch_swt::bench_options{ .type = bnch_swt::result_type::time, .totalIterationCountCap = totalIterationCountCap }>::
					runBenchmark<jsonifier_internal::string_literal{ "Json Test (Prettified)" } + "-Read", jsonifierLibraryName, "teal">([&]() {
						parser.parseJson<jsonifier::parse_options{ .minified = false }>(jsonifierTestData, jsonifierBuffer);
						auto* newPtr = &jsonifierTestData;
						bnch_swt::doNotOptimizeAway(newPtr);
					});
			results_data jsonifierResesults{ static_cast<jsonifier::string>(jsonifierLibraryName), static_cast<jsonifier::string>("Json Test (Prettified)"),
				static_cast<jsonifier::string>(jsonifierCommitUrl), jsonifierReadResult.iterationCount };
			for (auto& value: parser.getErrors()) {
				std::cout << "Jsonifier Error: " << value << std::endl;
			}
			std::string jsonifierNewerBuffer{};
			auto jsonifierWriteResult =
				bnch_swt::benchmark_stage<"Json-Performance", bnch_swt::bench_options{ .type = bnch_swt::result_type::time, .totalIterationCountCap = totalIterationCountCap }>::
					runBenchmark<jsonifier_internal::string_literal{ "Json Test (Prettified)" } + "-Write", jsonifierLibraryName, "steelblue">([&]() {
						parser.serializeJson<jsonifier::serialize_options{ .prettify = true }>(jsonifierTestData, jsonifierNewerBuffer);
						auto* newPtr = &jsonifierNewerBuffer;
						bnch_swt::doNotOptimizeAway(newPtr);
					});

			for (auto& value: parser.getErrors()) {
				std::cout << "Jsonifier Error: " << value << std::endl;
			}

			auto jsonifierReadSize		   = jsonifierNewerBuffer.size();
			auto jsonifierWrittenSize	   = jsonifierNewerBuffer.size();
			jsonifierResesults.readResult  = result<result_type::read>{ "teal", jsonifierReadSize, jsonifierReadResult };
			jsonifierResesults.writeResult = result<result_type::write>{ "steelblue", jsonifierWrittenSize, jsonifierWriteResult };
			bnch_swt::file_loader<basePath + "/" + "Json Test (Prettified)" + "-jsonifier.json">::saveFile(jsonifierBuffer);
			jsonifierResesults.print();
			resultsNew.emplace_back(jsonifierResesults);

			std::string table{};
			auto n = resultsNew.size();
			table += table_header + "\n";
			std::sort(resultsNew.begin(), resultsNew.end(), std::greater<results_data>());
			for (size_t x = 0; x < n; ++x) {
				jsonResults.results.emplace_back(resultsNew[x]);
				table += resultsNew[x].jsonStats();
				if (x != n - 1) {
					table += "\n";
				}
			}
			jsonResults.markdownResults = table;
			auto testResults			= jsonResults;
			newerString += testResults.markdownResults;
			benchmark_data.emplace_back(testResults);
		}
		{
			jsonifier::vector<results_data> resultsNew{};
			test_results jsonResults{};
			jsonResults.testName = static_cast<jsonifier::string>("Json Test (Minified)");

#if !defined(ASAN_ENABLED)
			test<test_struct> simdjsonTestData{};
			simdjsonBuffer = jsonMinifiedData;
			auto simdjsonReadResult =
				bnch_swt::benchmark_stage<"Json-Performance", bnch_swt::bench_options{ .type = bnch_swt::result_type::time, .totalIterationCountCap = totalIterationCountCap }>::
					runBenchmark<jsonifier_internal::string_literal{ "Json Test (Minified)" } + "-Read", simdjsonLibraryName, "cadetblue">([&]() {
						try {
							getValue(simdjsonTestData, simdjsonParser.iterate(simdjsonBuffer).value());
							bnch_swt::doNotOptimizeAway(simdjsonTestData);
						} catch (std::exception& error) {
							std::cout << "Simdjson Error: " << error.what() << std::endl;
						}
					});
			parser.serializeJson(simdjsonTestData, simdjsonBuffer);
			auto simdjsonReadSize = simdjsonBuffer.size();
			results_data simdjsonResults{ static_cast<jsonifier::string>(simdjsonLibraryName), static_cast<jsonifier::string>("Json Test (Minified)"),
				static_cast<jsonifier::string>(simdjsonCommitUrl), simdjsonReadResult.iterationCount };

			simdjsonResults.readResult = result<result_type::read>{ "cadetblue", simdjsonReadSize, simdjsonReadResult };
			bnch_swt::file_loader<basePath + "/" + "Json Test (Minified)" + "-simdjson.json">::saveFile(simdjsonBuffer);
			simdjsonResults.print();
			resultsNew.emplace_back(simdjsonResults);

			glazeBuffer = jsonMinifiedData;

			test<test_struct> glazeTestData{};
			auto glazeReadResult =
				bnch_swt::benchmark_stage<"Json-Performance", bnch_swt::bench_options{ .type = bnch_swt::result_type::time, .totalIterationCountCap = totalIterationCountCap }>::
					runBenchmark<jsonifier_internal::string_literal{ "Json Test (Minified)" } + "-Read", glazeLibraryName, "dodgerblue">([&]() {
						if (auto error = glz::read<glz::opts{ .skip_null_members = false, .minified = true }>(glazeTestData, glazeBuffer); error) {
							std::cout << "Glaze Error: " << glz::format_error(error, glazeBuffer) << std::endl;
						}
						auto* newPtr = &glazeTestData;
						bnch_swt::doNotOptimizeAway(newPtr);
					});

			results_data glazeResults{ static_cast<jsonifier::string>(glazeLibraryName), static_cast<jsonifier::string>("Json Test (Minified)"),
				static_cast<jsonifier::string>(glazeCommitUrl), glazeReadResult.iterationCount };
			std::string glazeNewerBuffer{};
			auto glazeWriteResult =
				bnch_swt::benchmark_stage<"Json-Performance", bnch_swt::bench_options{ .type = bnch_swt::result_type::time, .totalIterationCountCap = totalIterationCountCap }>::
					runBenchmark<jsonifier_internal::string_literal{ "Json Test (Minified)" } + "-Write", glazeLibraryName, "steelblue">([&]() {
						bnch_swt::doNotOptimizeAway(glz::write<glz::opts{ .skip_null_members = false, .prettify = false, .minified = true }>(glazeTestData, glazeNewerBuffer));
						auto* newPtr = &glazeNewerBuffer;
						bnch_swt::doNotOptimizeAway(newPtr);
					});

			auto glazeReadSize		 = glazeNewerBuffer.size();
			auto glazeWrittenSize	 = glazeNewerBuffer.size();
			glazeResults.readResult	 = result<result_type::read>{ "dodgerblue", glazeReadSize, glazeReadResult };
			glazeResults.writeResult = result<result_type::write>{ "skyblue", glazeWrittenSize, glazeWriteResult };
			bnch_swt::file_loader<basePath + "/" + "Json Test (Minified)" + "-glaze.json">::saveFile(glazeBuffer);
			glazeResults.print();
			resultsNew.emplace_back(glazeResults);
#endif
			jsonifierBuffer = jsonMinifiedData;
			test<test_struct> jsonifierTestData{};
			auto jsonifierReadResult =
				bnch_swt::benchmark_stage<"Json-Performance", bnch_swt::bench_options{ .type = bnch_swt::result_type::time, .totalIterationCountCap = totalIterationCountCap }>::
					runBenchmark<jsonifier_internal::string_literal{ "Json Test (Minified)" } + "-Read", jsonifierLibraryName, "teal">([&]() {
						parser.parseJson<jsonifier::parse_options{ .minified = true }>(jsonifierTestData, jsonifierBuffer);
						auto* newPtr = &jsonifierTestData;
						bnch_swt::doNotOptimizeAway(newPtr);
					});
			results_data jsonifierResesults{ static_cast<jsonifier::string>(jsonifierLibraryName), static_cast<jsonifier::string>("Json Test (Minified)"),
				static_cast<jsonifier::string>(jsonifierCommitUrl), jsonifierReadResult.iterationCount };
			for (auto& value: parser.getErrors()) {
				std::cout << "Jsonifier Error: " << value << std::endl;
			}
			std::string jsonifierNewerBuffer{};
			auto jsonifierWriteResult =
				bnch_swt::benchmark_stage<"Json-Performance", bnch_swt::bench_options{ .type = bnch_swt::result_type::time, .totalIterationCountCap = totalIterationCountCap }>::
					runBenchmark<jsonifier_internal::string_literal{ "Json Test (Minified)" } + "-Write", jsonifierLibraryName, "steelblue">([&]() {
						parser.serializeJson<jsonifier::serialize_options{ .prettify = false }>(jsonifierTestData, jsonifierNewerBuffer);
						auto* newPtr = &jsonifierNewerBuffer;
						bnch_swt::doNotOptimizeAway(newPtr);
					});

			for (auto& value: parser.getErrors()) {
				std::cout << "Jsonifier Error: " << value << std::endl;
			}

			auto jsonifierReadSize		   = jsonifierNewerBuffer.size();
			auto jsonifierWrittenSize	   = jsonifierNewerBuffer.size();
			jsonifierResesults.readResult  = result<result_type::read>{ "teal", jsonifierReadSize, jsonifierReadResult };
			jsonifierResesults.writeResult = result<result_type::write>{ "steelblue", jsonifierWrittenSize, jsonifierWriteResult };
			bnch_swt::file_loader<basePath + "/" + "Json Test (Minified)" + "-jsonifier.json">::saveFile(jsonifierBuffer);
			jsonifierResesults.print();
			resultsNew.emplace_back(jsonifierResesults);

			std::string table{};
			const auto n = resultsNew.size();
			table += table_header + "\n";
			std::sort(resultsNew.begin(), resultsNew.end(), std::greater<results_data>());
			for (size_t x = 0; x < n; ++x) {
				jsonResults.results.emplace_back(resultsNew[x]);
				table += resultsNew[x].jsonStats();
				if (x != n - 1) {
					table += "\n";
				}
			}
			jsonResults.markdownResults = table;
			auto testResults			= jsonResults;
			newerString += static_cast<jsonifier::string>(section02);
			newerString += testResults.markdownResults;
			benchmark_data.emplace_back(testResults);
		}
		{
			jsonifier::vector<results_data> resultsNew{};
			test_results jsonResults{};
			simdjsonBuffer		 = jsonData;
			jsonResults.testName = static_cast<jsonifier::string>("Abc Test (Prettified)");
#if !defined(ASAN_ENABLED)
			abc_test<test_struct> simdjsonTestData{};
			auto simdjsonReadResult =
				bnch_swt::benchmark_stage<"Abc-Performance", bnch_swt::bench_options{ .type = bnch_swt::result_type::time, .totalIterationCountCap = totalIterationCountCap }>::
					runBenchmark<jsonifier_internal::string_literal{ "Abc Test (Prettified)" } + "-Read", simdjsonLibraryName, "cadetblue">([&]() {
						try {
							getValue(simdjsonTestData, simdjsonParser.iterate(simdjsonBuffer).value());
							bnch_swt::doNotOptimizeAway(simdjsonTestData);
						} catch (std::exception& error) {
							std::cout << "Simdjson Error: " << error.what() << std::endl;
						}
					});
			parser.serializeJson<jsonifier::serialize_options{ .prettify = true }>(simdjsonTestData, simdjsonBuffer);
			auto simdjsonReadSize = simdjsonBuffer.size();
			results_data simdjsonResults{ static_cast<jsonifier::string>(simdjsonLibraryName), static_cast<jsonifier::string>("Abc Test (Prettified)"),
				static_cast<jsonifier::string>(simdjsonCommitUrl), simdjsonReadResult.iterationCount };

			simdjsonResults.readResult = result<result_type::read>{ "cadetblue", simdjsonReadSize, simdjsonReadResult };
			bnch_swt::file_loader<basePath + "/"  + "Abc Test (Prettified)" + "-simdjson.json">::saveFile(simdjsonBuffer);
			simdjsonResults.print();
			resultsNew.emplace_back(simdjsonResults);

			glazeBuffer = jsonData;

			abc_test<test_struct> glazeTestData{};
			auto glazeReadResult =
				bnch_swt::benchmark_stage<"Abc-Performance", bnch_swt::bench_options{ .type = bnch_swt::result_type::time, .totalIterationCountCap = totalIterationCountCap }>::
					runBenchmark<jsonifier_internal::string_literal{ "Abc Test (Prettified)" } + "-Read", glazeLibraryName, "dodgerblue">([&]() {
						if (auto error = glz::read<glz::opts{ .skip_null_members = false, .minified = false }>(glazeTestData, glazeBuffer); error) {
							std::cout << "Glaze Error: " << glz::format_error(error, glazeBuffer) << std::endl;
						}
						auto* newPtr = &glazeTestData;
						bnch_swt::doNotOptimizeAway(newPtr);
					});

			results_data glazeResults{ static_cast<jsonifier::string>(glazeLibraryName), static_cast<jsonifier::string>("Abc Test (Prettified)"),
				static_cast<jsonifier::string>(glazeCommitUrl), glazeReadResult.iterationCount };
			std::string glazeNewerBuffer{};
			auto glazeWriteResult =
				bnch_swt::benchmark_stage<"Abc-Performance", bnch_swt::bench_options{ .type = bnch_swt::result_type::time, .totalIterationCountCap = totalIterationCountCap }>::
					runBenchmark<jsonifier_internal::string_literal{ "Abc Test (Prettified)" } + "-Write", glazeLibraryName, "steelblue">([&]() {
						bnch_swt::doNotOptimizeAway(glz::write<glz::opts{ .skip_null_members = false, .prettify = true, .minified = false }>(glazeTestData, glazeNewerBuffer));
						auto* newPtr = &glazeNewerBuffer;
						bnch_swt::doNotOptimizeAway(newPtr);
					});

			auto glazeReadSize		 = glazeNewerBuffer.size();
			auto glazeWrittenSize	 = glazeNewerBuffer.size();
			glazeResults.readResult	 = result<result_type::read>{ "dodgerblue", glazeReadSize, glazeReadResult };
			glazeResults.writeResult = result<result_type::write>{ "skyblue", glazeWrittenSize, glazeWriteResult };
			bnch_swt::file_loader<basePath + "/"  + "Abc Test (Prettified)" + "-glaze.json">::saveFile(glazeBuffer);
			glazeResults.print();
			resultsNew.emplace_back(glazeResults);
#endif
			jsonifierBuffer = jsonData;
			abc_test<test_struct> jsonifierTestData{};
			auto jsonifierReadResult =
				bnch_swt::benchmark_stage<"Abc-Performance", bnch_swt::bench_options{ .type = bnch_swt::result_type::time, .totalIterationCountCap = totalIterationCountCap }>::
					runBenchmark<jsonifier_internal::string_literal{ "Abc Test (Prettified)" } + "-Read", jsonifierLibraryName, "teal">([&]() {
						parser.parseJson<jsonifier::parse_options{ .minified = false }>(jsonifierTestData, jsonifierBuffer);
						auto* newPtr = &jsonifierTestData;
						bnch_swt::doNotOptimizeAway(newPtr);
					});
			results_data jsonifierResesults{ static_cast<jsonifier::string>(jsonifierLibraryName), static_cast<jsonifier::string>("Abc Test (Prettified)"),
				static_cast<jsonifier::string>(jsonifierCommitUrl), jsonifierReadResult.iterationCount };
			for (auto& value: parser.getErrors()) {
				std::cout << "Abcifier Error: " << value << std::endl;
			}
			std::string jsonifierNewerBuffer{};
			auto jsonifierWriteResult =
				bnch_swt::benchmark_stage<"Abc-Performance", bnch_swt::bench_options{ .type = bnch_swt::result_type::time, .totalIterationCountCap = totalIterationCountCap }>::
					runBenchmark<jsonifier_internal::string_literal{ "Abc Test (Prettified)" } + "-Write", jsonifierLibraryName, "steelblue">([&]() {
						parser.serializeJson<jsonifier::serialize_options{ .prettify = true }>(jsonifierTestData, jsonifierNewerBuffer);
						auto* newPtr = &jsonifierNewerBuffer;
						bnch_swt::doNotOptimizeAway(newPtr);
					});

			for (auto& value: parser.getErrors()) {
				std::cout << "Abcifier Error: " << value << std::endl;
			}

			auto jsonifierReadSize		   = jsonifierNewerBuffer.size();
			auto jsonifierWrittenSize	   = jsonifierNewerBuffer.size();
			jsonifierResesults.readResult  = result<result_type::read>{ "teal", jsonifierReadSize, jsonifierReadResult };
			jsonifierResesults.writeResult = result<result_type::write>{ "steelblue", jsonifierWrittenSize, jsonifierWriteResult };
			bnch_swt::file_loader<basePath + "/"  + "Abc Test (Prettified)" + "-jsonifier.json">::saveFile(jsonifierBuffer);
			jsonifierResesults.print();
			resultsNew.emplace_back(jsonifierResesults);

			std::string table{};
			const auto n = resultsNew.size();
			table += table_header + "\n";
			std::sort(resultsNew.begin(), resultsNew.end(), std::greater<results_data>());
			for (size_t x = 0; x < n; ++x) {
				jsonResults.results.emplace_back(resultsNew[x]);
				table += resultsNew[x].jsonStats();
				if (x != n - 1) {
					table += "\n";
				}
			}
			jsonResults.markdownResults = table;
			auto testResults			= jsonResults;
			newerString += static_cast<jsonifier::string>(section03);
			newerString += testResults.markdownResults;
			benchmark_data.emplace_back(testResults);
		}
		{
			jsonifier::vector<results_data> resultsNew{};
			test_results jsonResults{};
			simdjsonBuffer		 = jsonMinifiedData;
			jsonResults.testName = static_cast<jsonifier::string>("Abc Test (Minified)");
#if !defined(ASAN_ENABLED)
			abc_test<test_struct> simdjsonTestData{};
			auto simdjsonReadResult =
				bnch_swt::benchmark_stage<"Abc-Performance", bnch_swt::bench_options{ .type = bnch_swt::result_type::time, .totalIterationCountCap = totalIterationCountCap }>::
					runBenchmark<jsonifier_internal::string_literal{ "Abc Test (Minified)" } + "-Read", simdjsonLibraryName, "cadetblue">([&]() {
						try {
							getValue(simdjsonTestData, simdjsonParser.iterate(simdjsonBuffer).value());
							bnch_swt::doNotOptimizeAway(simdjsonTestData);
						} catch (std::exception& error) {
							std::cout << "Simdjson Error: " << error.what() << std::endl;
						}
					});
			parser.serializeJson(simdjsonTestData, simdjsonBuffer);
			auto simdjsonReadSize = simdjsonBuffer.size();
			results_data simdjsonResults{ static_cast<jsonifier::string>(simdjsonLibraryName), static_cast<jsonifier::string>("Abc Test (Minified)"),
				static_cast<jsonifier::string>(simdjsonCommitUrl), simdjsonReadResult.iterationCount };

			simdjsonResults.readResult = result<result_type::read>{ "cadetblue", simdjsonReadSize, simdjsonReadResult };
			bnch_swt::file_loader<basePath + "/"  + "Abc Test (Minified)" + "-simdjson.json">::saveFile(simdjsonBuffer);
			simdjsonResults.print();
			resultsNew.emplace_back(simdjsonResults);

			glazeBuffer = jsonMinifiedData;

			abc_test<test_struct> glazeTestData{};
			auto glazeReadResult =
				bnch_swt::benchmark_stage<"Abc-Performance", bnch_swt::bench_options{ .type = bnch_swt::result_type::time, .totalIterationCountCap = totalIterationCountCap }>::
					runBenchmark<jsonifier_internal::string_literal{ "Abc Test (Minified)" } + "-Read", glazeLibraryName, "dodgerblue">([&]() {
						if (auto error = glz::read<glz::opts{ .skip_null_members = false, .minified = true }>(glazeTestData, glazeBuffer); error) {
							std::cout << "Glaze Error: " << glz::format_error(error, glazeBuffer) << std::endl;
						}
						auto* newPtr = &glazeTestData;
						bnch_swt::doNotOptimizeAway(newPtr);
					});

			results_data glazeResults{ static_cast<jsonifier::string>(glazeLibraryName), static_cast<jsonifier::string>("Abc Test (Minified)"),
				static_cast<jsonifier::string>(glazeCommitUrl), glazeReadResult.iterationCount };
			std::string glazeNewerBuffer{};
			auto glazeWriteResult =
				bnch_swt::benchmark_stage<"Abc-Performance", bnch_swt::bench_options{ .type = bnch_swt::result_type::time, .totalIterationCountCap = totalIterationCountCap }>::
					runBenchmark<jsonifier_internal::string_literal{ "Abc Test (Minified)" } + "-Write", glazeLibraryName, "steelblue">([&]() {
						bnch_swt::doNotOptimizeAway(glz::write<glz::opts{ .skip_null_members = false, .prettify = false, .minified = true }>(glazeTestData, glazeNewerBuffer));
						auto* newPtr = &glazeNewerBuffer;
						bnch_swt::doNotOptimizeAway(newPtr);
					});

			auto glazeReadSize		 = glazeNewerBuffer.size();
			auto glazeWrittenSize	 = glazeNewerBuffer.size();
			glazeResults.readResult	 = result<result_type::read>{ "dodgerblue", glazeReadSize, glazeReadResult };
			glazeResults.writeResult = result<result_type::write>{ "skyblue", glazeWrittenSize, glazeWriteResult };
			bnch_swt::file_loader<basePath + "/"  + "Abc Test (Minified)" + "-glaze.json">::saveFile(glazeBuffer);
			glazeResults.print();
			resultsNew.emplace_back(glazeResults);
#endif
			jsonifierBuffer = jsonMinifiedData;
			abc_test<test_struct> jsonifierTestData{};
			auto jsonifierReadResult =
				bnch_swt::benchmark_stage<"Abc-Performance", bnch_swt::bench_options{ .type = bnch_swt::result_type::time, .totalIterationCountCap = totalIterationCountCap }>::
					runBenchmark<jsonifier_internal::string_literal{ "Abc Test (Minified)" } + "-Read", jsonifierLibraryName, "teal">([&]() {
						parser.parseJson<jsonifier::parse_options{ .minified = true }>(jsonifierTestData, jsonifierBuffer);
						auto* newPtr = &jsonifierTestData;
						bnch_swt::doNotOptimizeAway(newPtr);
					});
			results_data jsonifierResesults{ static_cast<jsonifier::string>(jsonifierLibraryName), static_cast<jsonifier::string>("Abc Test (Minified)"),
				static_cast<jsonifier::string>(jsonifierCommitUrl), jsonifierReadResult.iterationCount };
			for (auto& value: parser.getErrors()) {
				std::cout << "Abcifier Error: " << value << std::endl;
			}
			std::string jsonifierNewerBuffer{};
			auto jsonifierWriteResult =
				bnch_swt::benchmark_stage<"Abc-Performance", bnch_swt::bench_options{ .type = bnch_swt::result_type::time, .totalIterationCountCap = totalIterationCountCap }>::
					runBenchmark<jsonifier_internal::string_literal{ "Abc Test (Minified)" } + "-Write", jsonifierLibraryName, "steelblue">([&]() {
						parser.serializeJson<jsonifier::serialize_options{ .prettify = false }>(jsonifierTestData, jsonifierNewerBuffer);
						auto* newPtr = &jsonifierNewerBuffer;
						bnch_swt::doNotOptimizeAway(newPtr);
					});

			for (auto& value: parser.getErrors()) {
				std::cout << "Abcifier Error: " << value << std::endl;
			}

			auto jsonifierReadSize		   = jsonifierNewerBuffer.size();
			auto jsonifierWrittenSize	   = jsonifierNewerBuffer.size();
			jsonifierResesults.readResult  = result<result_type::read>{ "teal", jsonifierReadSize, jsonifierReadResult };
			jsonifierResesults.writeResult = result<result_type::write>{ "steelblue", jsonifierWrittenSize, jsonifierWriteResult };
			bnch_swt::file_loader<basePath + "/"  + "Abc Test (Minified)" + "-jsonifier.json">::saveFile(jsonifierBuffer);
			jsonifierResesults.print();
			resultsNew.emplace_back(jsonifierResesults);

			std::string table{};
			const auto n = resultsNew.size();
			table += table_header + "\n";
			std::sort(resultsNew.begin(), resultsNew.end(), std::greater<results_data>());
			for (size_t x = 0; x < n; ++x) {
				jsonResults.results.emplace_back(resultsNew[x]);
				table += resultsNew[x].jsonStats();
				if (x != n - 1) {
					table += "\n";
				}
			}
			jsonResults.markdownResults = table;
			auto testResults			= jsonResults;
			newerString += static_cast<jsonifier::string>(section04);
			newerString += testResults.markdownResults;
			benchmark_data.emplace_back(testResults);
		}
		{
			jsonifier::vector<results_data> resultsNew{};
			test_results jsonResults{};
			simdjsonBuffer		 = discordData;
			jsonResults.testName = static_cast<jsonifier::string>("Discord Test (Prettified)");
#if !defined(ASAN_ENABLED)
			discord_message simdjsonTestData{};
			auto simdjsonReadResult =
				bnch_swt::benchmark_stage<"Discord-Performance", bnch_swt::bench_options{ .type = bnch_swt::result_type::time, .totalIterationCountCap = totalIterationCountCap }>::
					runBenchmark<jsonifier_internal::string_literal{ "Discord Test (Prettified)" } + "-Read", simdjsonLibraryName, "cadetblue">([&]() {
						try {
							getValue(simdjsonTestData, simdjsonParser.iterate(simdjsonBuffer).value());
							bnch_swt::doNotOptimizeAway(simdjsonTestData);
						} catch (std::exception& error) {
							std::cout << "Simdjson Error: " << error.what() << std::endl;
						}
					});
			parser.serializeJson<jsonifier::serialize_options{ .prettify = true }>(simdjsonTestData, simdjsonBuffer);
			auto simdjsonReadSize = simdjsonBuffer.size();
			results_data simdjsonResults{ static_cast<jsonifier::string>(simdjsonLibraryName), static_cast<jsonifier::string>("Discord Test (Prettified)"),
				static_cast<jsonifier::string>(simdjsonCommitUrl), simdjsonReadResult.iterationCount };

			simdjsonResults.readResult = result<result_type::read>{ "cadetblue", simdjsonReadSize, simdjsonReadResult };
			bnch_swt::file_loader<basePath + "/"  + "Discord Test (Prettified)" + "-simdjson.json">::saveFile(simdjsonBuffer);
			simdjsonResults.print();
			resultsNew.emplace_back(simdjsonResults);

			glazeBuffer = discordData;

			discord_message glazeTestData{};
			auto glazeReadResult =
				bnch_swt::benchmark_stage<"Discord-Performance", bnch_swt::bench_options{ .type = bnch_swt::result_type::time, .totalIterationCountCap = totalIterationCountCap }>::
					runBenchmark<jsonifier_internal::string_literal{ "Discord Test (Prettified)" } + "-Read", glazeLibraryName, "dodgerblue">([&]() {
						if (auto error = glz::read<glz::opts{ .skip_null_members = false, .minified = false }>(glazeTestData, glazeBuffer); error) {
							std::cout << "Glaze Error: " << glz::format_error(error, glazeBuffer) << std::endl;
						}
						auto* newPtr = &glazeTestData;
						bnch_swt::doNotOptimizeAway(newPtr);
					});

			results_data glazeResults{ static_cast<jsonifier::string>(glazeLibraryName), static_cast<jsonifier::string>("Discord Test (Prettified)"),
				static_cast<jsonifier::string>(glazeCommitUrl), glazeReadResult.iterationCount };
			std::string glazeNewerBuffer{};
			auto glazeWriteResult =
				bnch_swt::benchmark_stage<"Discord-Performance", bnch_swt::bench_options{ .type = bnch_swt::result_type::time, .totalIterationCountCap = totalIterationCountCap }>::
					runBenchmark<jsonifier_internal::string_literal{ "Discord Test (Prettified)" } + "-Write", glazeLibraryName, "steelblue">([&]() {
						bnch_swt::doNotOptimizeAway(glz::write<glz::opts{ .skip_null_members = false, .prettify = true, .minified = false }>(glazeTestData, glazeNewerBuffer));
						auto* newPtr = &glazeNewerBuffer;
						bnch_swt::doNotOptimizeAway(newPtr);
					});

			auto glazeReadSize		 = glazeNewerBuffer.size();
			auto glazeWrittenSize	 = glazeNewerBuffer.size();
			glazeResults.readResult	 = result<result_type::read>{ "dodgerblue", glazeReadSize, glazeReadResult };
			glazeResults.writeResult = result<result_type::write>{ "skyblue", glazeWrittenSize, glazeWriteResult };
			bnch_swt::file_loader<basePath + "/"  + "Discord Test (Prettified)" + "-glaze.json">::saveFile(glazeBuffer);
			glazeResults.print();
			resultsNew.emplace_back(glazeResults);
#endif
			jsonifierBuffer = discordData;
			discord_message jsonifierTestData{};
			auto jsonifierReadResult =
				bnch_swt::benchmark_stage<"Discord-Performance", bnch_swt::bench_options{ .type = bnch_swt::result_type::time, .totalIterationCountCap = totalIterationCountCap }>::
					runBenchmark<jsonifier_internal::string_literal{ "Discord Test (Prettified)" } + "-Read", jsonifierLibraryName, "teal">([&]() {
						parser.parseJson<jsonifier::parse_options{ .minified = false }>(jsonifierTestData, jsonifierBuffer);
						auto* newPtr = &jsonifierTestData;
						bnch_swt::doNotOptimizeAway(newPtr);
					});
			results_data jsonifierResesults{ static_cast<jsonifier::string>(jsonifierLibraryName), static_cast<jsonifier::string>("Discord Test (Prettified)"),
				static_cast<jsonifier::string>(jsonifierCommitUrl), jsonifierReadResult.iterationCount };
			for (auto& value: parser.getErrors()) {
				std::cout << "Discordifier Error: " << value << std::endl;
			}
			std::string jsonifierNewerBuffer{};
			auto jsonifierWriteResult =
				bnch_swt::benchmark_stage<"Discord-Performance", bnch_swt::bench_options{ .type = bnch_swt::result_type::time, .totalIterationCountCap = totalIterationCountCap }>::
					runBenchmark<jsonifier_internal::string_literal{ "Discord Test (Prettified)" } + "-Write", jsonifierLibraryName, "steelblue">([&]() {
						parser.serializeJson<jsonifier::serialize_options{ .prettify = true }>(jsonifierTestData, jsonifierNewerBuffer);
						auto* newPtr = &jsonifierNewerBuffer;
						bnch_swt::doNotOptimizeAway(newPtr);
					});

			for (auto& value: parser.getErrors()) {
				std::cout << "Discordifier Error: " << value << std::endl;
			}

			auto jsonifierReadSize		   = jsonifierNewerBuffer.size();
			auto jsonifierWrittenSize	   = jsonifierNewerBuffer.size();
			jsonifierResesults.readResult  = result<result_type::read>{ "teal", jsonifierReadSize, jsonifierReadResult };
			jsonifierResesults.writeResult = result<result_type::write>{ "steelblue", jsonifierWrittenSize, jsonifierWriteResult };
			bnch_swt::file_loader<basePath + "/"  + "Discord Test (Prettified)" + "-jsonifier.json">::saveFile(jsonifierBuffer);
			jsonifierResesults.print();
			resultsNew.emplace_back(jsonifierResesults);

			std::string table{};
			const auto n = resultsNew.size();
			table += table_header + "\n";
			std::sort(resultsNew.begin(), resultsNew.end(), std::greater<results_data>());
			for (size_t x = 0; x < n; ++x) {
				jsonResults.results.emplace_back(resultsNew[x]);
				table += resultsNew[x].jsonStats();
				if (x != n - 1) {
					table += "\n";
				}
			}
			jsonResults.markdownResults = table;
			auto testResults			= jsonResults;
			newerString += static_cast<jsonifier::string>(section05);
			newerString += testResults.markdownResults;
			benchmark_data.emplace_back(testResults);
		}
		{
			jsonifier::vector<results_data> resultsNew{};
			test_results jsonResults{};
			jsonResults.testName = static_cast<jsonifier::string>("Discord Test (Minified)");
#if !defined(ASAN_ENABLED)
			discord_message simdjsonTestData{};
			simdjsonBuffer = discordMinifiedData;
			auto simdjsonReadResult =
				bnch_swt::benchmark_stage<"Discord-Performance", bnch_swt::bench_options{ .type = bnch_swt::result_type::time, .totalIterationCountCap = totalIterationCountCap }>::
					runBenchmark<jsonifier_internal::string_literal{ "Discord Test (Minified)" } + "-Read", simdjsonLibraryName, "cadetblue">([&]() {
						try {
							getValue(simdjsonTestData, simdjsonParser.iterate(simdjsonBuffer).value());
							bnch_swt::doNotOptimizeAway(simdjsonTestData);
						} catch (std::exception& error) {
							std::cout << "Simdjson Error: " << error.what() << std::endl;
						}
					});
			parser.serializeJson(simdjsonTestData, simdjsonBuffer);
			auto simdjsonReadSize = simdjsonBuffer.size();
			results_data simdjsonResults{ static_cast<jsonifier::string>(simdjsonLibraryName), static_cast<jsonifier::string>("Discord Test (Minified)"),
				static_cast<jsonifier::string>(simdjsonCommitUrl), simdjsonReadResult.iterationCount };

			simdjsonResults.readResult = result<result_type::read>{ "cadetblue", simdjsonReadSize, simdjsonReadResult };
			bnch_swt::file_loader<basePath + "/"  + "Discord Test (Minified)" + "-simdjson.json">::saveFile(simdjsonBuffer);
			simdjsonResults.print();
			resultsNew.emplace_back(simdjsonResults);

			glazeBuffer = discordMinifiedData;

			discord_message glazeTestData{};
			auto glazeReadResult =
				bnch_swt::benchmark_stage<"Discord-Performance", bnch_swt::bench_options{ .type = bnch_swt::result_type::time, .totalIterationCountCap = totalIterationCountCap }>::
					runBenchmark<jsonifier_internal::string_literal{ "Discord Test (Minified)" } + "-Read", glazeLibraryName, "dodgerblue">([&]() {
						if (auto error = glz::read<glz::opts{ .skip_null_members = false, .minified = true }>(glazeTestData, glazeBuffer); error) {
							std::cout << "Glaze Error: " << glz::format_error(error, glazeBuffer) << std::endl;
						}
						auto* newPtr = &glazeTestData;
						bnch_swt::doNotOptimizeAway(newPtr);
					});

			results_data glazeResults{ static_cast<jsonifier::string>(glazeLibraryName), static_cast<jsonifier::string>("Discord Test (Minified)"),
				static_cast<jsonifier::string>(glazeCommitUrl), glazeReadResult.iterationCount };
			std::string glazeNewerBuffer{};
			auto glazeWriteResult =
				bnch_swt::benchmark_stage<"Discord-Performance", bnch_swt::bench_options{ .type = bnch_swt::result_type::time, .totalIterationCountCap = totalIterationCountCap }>::
					runBenchmark<jsonifier_internal::string_literal{ "Discord Test (Minified)" } + "-Write", glazeLibraryName, "steelblue">([&]() {
						bnch_swt::doNotOptimizeAway(glz::write<glz::opts{ .skip_null_members = false, .prettify = false, .minified = true }>(glazeTestData, glazeNewerBuffer));
						auto* newPtr = &glazeNewerBuffer;
						bnch_swt::doNotOptimizeAway(newPtr);
					});

			auto glazeReadSize		 = glazeNewerBuffer.size();
			auto glazeWrittenSize	 = glazeNewerBuffer.size();
			glazeResults.readResult	 = result<result_type::read>{ "dodgerblue", glazeReadSize, glazeReadResult };
			glazeResults.writeResult = result<result_type::write>{ "skyblue", glazeWrittenSize, glazeWriteResult };
			bnch_swt::file_loader<basePath + "/"  + "Discord Test (Minified)" + "-glaze.json">::saveFile(glazeBuffer);
			glazeResults.print();
			resultsNew.emplace_back(glazeResults);
#endif
			jsonifierBuffer = discordMinifiedData;
			discord_message jsonifierTestData{};
			auto jsonifierReadResult =
				bnch_swt::benchmark_stage<"Discord-Performance", bnch_swt::bench_options{ .type = bnch_swt::result_type::time, .totalIterationCountCap = totalIterationCountCap }>::
					runBenchmark<jsonifier_internal::string_literal{ "Discord Test (Minified)" } + "-Read", jsonifierLibraryName, "teal">([&]() {
						parser.parseJson<jsonifier::parse_options{ .minified = true }>(jsonifierTestData, jsonifierBuffer);
						auto* newPtr = &jsonifierTestData;
						bnch_swt::doNotOptimizeAway(newPtr);
					});
			results_data jsonifierResesults{ static_cast<jsonifier::string>(jsonifierLibraryName), static_cast<jsonifier::string>("Discord Test (Minified)"),
				static_cast<jsonifier::string>(jsonifierCommitUrl), jsonifierReadResult.iterationCount };
			for (auto& value: parser.getErrors()) {
				std::cout << "Discordifier Error: " << value << std::endl;
			}
			std::string jsonifierNewerBuffer{};
			auto jsonifierWriteResult =
				bnch_swt::benchmark_stage<"Discord-Performance", bnch_swt::bench_options{ .type = bnch_swt::result_type::time, .totalIterationCountCap = totalIterationCountCap }>::
					runBenchmark<jsonifier_internal::string_literal{ "Discord Test (Minified)" } + "-Write", jsonifierLibraryName, "steelblue">([&]() {
						parser.serializeJson<jsonifier::serialize_options{ .prettify = false }>(jsonifierTestData, jsonifierNewerBuffer);
						auto* newPtr = &jsonifierNewerBuffer;
						bnch_swt::doNotOptimizeAway(newPtr);
					});

			for (auto& value: parser.getErrors()) {
				std::cout << "Discordifier Error: " << value << std::endl;
			}

			auto jsonifierReadSize		   = jsonifierNewerBuffer.size();
			auto jsonifierWrittenSize	   = jsonifierNewerBuffer.size();
			jsonifierResesults.readResult  = result<result_type::read>{ "teal", jsonifierReadSize, jsonifierReadResult };
			jsonifierResesults.writeResult = result<result_type::write>{ "steelblue", jsonifierWrittenSize, jsonifierWriteResult };
			bnch_swt::file_loader<basePath + "/"  + "Discord Test (Minified)" + "-jsonifier.json">::saveFile(jsonifierBuffer);
			jsonifierResesults.print();
			resultsNew.emplace_back(jsonifierResesults);

			std::string table{};
			const auto n = resultsNew.size();
			table += table_header + "\n";
			std::sort(resultsNew.begin(), resultsNew.end(), std::greater<results_data>());
			for (size_t x = 0; x < n; ++x) {
				jsonResults.results.emplace_back(resultsNew[x]);
				table += resultsNew[x].jsonStats();
				if (x != n - 1) {
					table += "\n";
				}
			}
			jsonResults.markdownResults = table;
			auto testResults			= jsonResults;
			newerString += static_cast<jsonifier::string>(section06);
			newerString += testResults.markdownResults;
			benchmark_data.emplace_back(testResults);
		}
		{
			jsonifier::vector<results_data> resultsNew{};
			test_results jsonResults{};
			jsonResults.testName = static_cast<jsonifier::string>("Canada Test (Prettified)");
#if !defined(ASAN_ENABLED)
			canada_message simdjsonTestData{};
			resultsNew.clear();
			simdjsonBuffer = canadaData;
			auto simdjsonReadResult =
				bnch_swt::benchmark_stage<"Json-Performance", bnch_swt::bench_options{ .type = bnch_swt::result_type::time, .totalIterationCountCap = totalIterationCountCap }>::
					runBenchmark<jsonifier_internal::string_literal{ "Canada Test (Prettified)" } + "-Read", simdjsonLibraryName, "cadetblue">([&]() {
						try {
							getValue(simdjsonTestData, simdjsonParser.iterate(simdjsonBuffer).value());
							bnch_swt::doNotOptimizeAway(simdjsonTestData);
						} catch (std::exception& error) {
							std::cout << "Simdjson Error: " << error.what() << std::endl;
						}
					});
			parser.serializeJson<jsonifier::serialize_options{ .prettify = true }>(simdjsonTestData, simdjsonBuffer);
			auto simdjsonReadSize = simdjsonBuffer.size();
			results_data simdjsonResults{ static_cast<jsonifier::string>(simdjsonLibraryName), static_cast<jsonifier::string>("Canada Test (Prettified)"),
				static_cast<jsonifier::string>(simdjsonCommitUrl), simdjsonReadResult.iterationCount };

			simdjsonResults.readResult = result<result_type::read>{ "cadetblue", simdjsonReadSize, simdjsonReadResult };
			bnch_swt::file_loader<basePath + "/"  + "Canada Test (Prettified)" + "-simdjson.json">::saveFile(simdjsonBuffer);
			simdjsonResults.print();
			resultsNew.emplace_back(simdjsonResults);

			glazeBuffer = canadaData;

			canada_message glazeTestData{};
			auto glazeReadResult =
				bnch_swt::benchmark_stage<"Json-Performance", bnch_swt::bench_options{ .type = bnch_swt::result_type::time, .totalIterationCountCap = totalIterationCountCap }>::
					runBenchmark<jsonifier_internal::string_literal{ "Canada Test (Prettified)" } + "-Read", glazeLibraryName, "dodgerblue">([&]() {
						if (auto error = glz::read<glz::opts{ .skip_null_members = false, .minified = false }>(glazeTestData, glazeBuffer); error) {
							std::cout << "Glaze Error: " << glz::format_error(error, glazeBuffer) << std::endl;
						}
						auto* newPtr = &glazeTestData;
						bnch_swt::doNotOptimizeAway(newPtr);
					});

			results_data glazeResults{ static_cast<jsonifier::string>(glazeLibraryName), static_cast<jsonifier::string>("Canada Test (Prettified)"),
				static_cast<jsonifier::string>(glazeCommitUrl), glazeReadResult.iterationCount };
			std::string glazeNewerBuffer{};
			auto glazeWriteResult =
				bnch_swt::benchmark_stage<"Json-Performance", bnch_swt::bench_options{ .type = bnch_swt::result_type::time, .totalIterationCountCap = totalIterationCountCap }>::
					runBenchmark<jsonifier_internal::string_literal{ "Canada Test (Prettified)" } + "-Write", glazeLibraryName, "steelblue">([&]() {
						bnch_swt::doNotOptimizeAway(glz::write<glz::opts{ .skip_null_members = false, .prettify = true, .minified = false }>(glazeTestData, glazeNewerBuffer));
						auto* newPtr = &glazeNewerBuffer;
						bnch_swt::doNotOptimizeAway(newPtr);
					});

			auto glazeReadSize		 = glazeNewerBuffer.size();
			auto glazeWrittenSize	 = glazeNewerBuffer.size();
			glazeResults.readResult	 = result<result_type::read>{ "dodgerblue", glazeReadSize, glazeReadResult };
			glazeResults.writeResult = result<result_type::write>{ "skyblue", glazeWrittenSize, glazeWriteResult };
			bnch_swt::file_loader<basePath + "/"  + "Canada Test (Prettified)" + "-glaze.json">::saveFile(glazeBuffer);
			glazeResults.print();
			resultsNew.emplace_back(glazeResults);
#endif
			jsonifierBuffer = canadaData;
			canada_message jsonifierTestData{};
			auto jsonifierReadResult =
				bnch_swt::benchmark_stage<"Json-Performance", bnch_swt::bench_options{ .type = bnch_swt::result_type::time, .totalIterationCountCap = totalIterationCountCap }>::
					runBenchmark<jsonifier_internal::string_literal{ "Canada Test (Prettified)" } + "-Read", jsonifierLibraryName, "teal">([&]() {
						parser.parseJson<jsonifier::parse_options{ .minified = false }>(jsonifierTestData, jsonifierBuffer);
						auto* newPtr = &jsonifierTestData;
						bnch_swt::doNotOptimizeAway(newPtr);
					});
			results_data jsonifierResesults{ static_cast<jsonifier::string>(jsonifierLibraryName), static_cast<jsonifier::string>("Canada Test (Prettified)"),
				static_cast<jsonifier::string>(jsonifierCommitUrl), jsonifierReadResult.iterationCount };
			for (auto& value: parser.getErrors()) {
				std::cout << "Jsonifier Error: " << value << std::endl;
			}
			std::string jsonifierNewerBuffer{};
			auto jsonifierWriteResult =
				bnch_swt::benchmark_stage<"Json-Performance", bnch_swt::bench_options{ .type = bnch_swt::result_type::time, .totalIterationCountCap = totalIterationCountCap }>::
					runBenchmark<jsonifier_internal::string_literal{ "Canada Test (Prettified)" } + "-Write", jsonifierLibraryName, "steelblue">([&]() {
						parser.serializeJson<jsonifier::serialize_options{ .prettify = true }>(jsonifierTestData, jsonifierNewerBuffer);
						auto* newPtr = &jsonifierNewerBuffer;
						bnch_swt::doNotOptimizeAway(newPtr);
					});

			for (auto& value: parser.getErrors()) {
				std::cout << "Jsonifier Error: " << value << std::endl;
			}

			auto jsonifierReadSize		   = jsonifierNewerBuffer.size();
			auto jsonifierWrittenSize	   = jsonifierNewerBuffer.size();
			jsonifierResesults.readResult  = result<result_type::read>{ "teal", jsonifierReadSize, jsonifierReadResult };
			jsonifierResesults.writeResult = result<result_type::write>{ "steelblue", jsonifierWrittenSize, jsonifierWriteResult };
			bnch_swt::file_loader<basePath + "/"  + "Canada Test (Prettified)" + "-jsonifier.json">::saveFile(jsonifierBuffer);
			jsonifierResesults.print();
			resultsNew.emplace_back(jsonifierResesults);

			std::string table{};
			const auto n = resultsNew.size();
			table += table_header + "\n";
			std::sort(resultsNew.begin(), resultsNew.end(), std::greater<results_data>());
			for (size_t x = 0; x < n; ++x) {
				jsonResults.results.emplace_back(resultsNew[x]);
				table += resultsNew[x].jsonStats();
				if (x != n - 1) {
					table += "\n";
				}
			}
			jsonResults.markdownResults = table;
			auto testResults			= jsonResults;
			newerString += static_cast<jsonifier::string>(section07);
			newerString += testResults.markdownResults;
			benchmark_data.emplace_back(testResults);
		}
		{
			jsonifier::vector<results_data> resultsNew{};
			test_results jsonResults{};
#if !defined(ASAN_ENABLED)
			canada_message simdjsonTestData{};
			simdjsonBuffer		 = canadaMinifiedData;
			jsonResults.testName = static_cast<jsonifier::string>("Canada Test (Minified)");
			auto simdjsonReadResult =
				bnch_swt::benchmark_stage<"Json-Performance", bnch_swt::bench_options{ .type = bnch_swt::result_type::time, .totalIterationCountCap = totalIterationCountCap }>::
					runBenchmark<jsonifier_internal::string_literal{ "Canada Test (Minified)" } + "-Read", simdjsonLibraryName, "cadetblue">([&]() {
						try {
							getValue(simdjsonTestData, simdjsonParser.iterate(simdjsonBuffer).value());
							bnch_swt::doNotOptimizeAway(simdjsonTestData);
						} catch (std::exception& error) {
							std::cout << "Simdjson Error: " << error.what() << std::endl;
						}
					});
			parser.serializeJson(simdjsonTestData, simdjsonBuffer);
			auto simdjsonReadSize = simdjsonBuffer.size();
			results_data simdjsonResults{ static_cast<jsonifier::string>(simdjsonLibraryName), static_cast<jsonifier::string>("Canada Test (Minified)"),
				static_cast<jsonifier::string>(simdjsonCommitUrl), simdjsonReadResult.iterationCount };

			simdjsonResults.readResult = result<result_type::read>{ "cadetblue", simdjsonReadSize, simdjsonReadResult };
			bnch_swt::file_loader<basePath + "/"  + "Canada Test (Minified)" + "-simdjson.json">::saveFile(simdjsonBuffer);
			simdjsonResults.print();
			resultsNew.emplace_back(simdjsonResults);

			glazeBuffer = canadaMinifiedData;

			canada_message glazeTestData{};
			auto glazeReadResult =
				bnch_swt::benchmark_stage<"Json-Performance", bnch_swt::bench_options{ .type = bnch_swt::result_type::time, .totalIterationCountCap = totalIterationCountCap }>::
					runBenchmark<jsonifier_internal::string_literal{ "Canada Test (Minified)" } + "-Read", glazeLibraryName, "dodgerblue">([&]() {
						if (auto error = glz::read<glz::opts{ .skip_null_members = false, .minified = true }>(glazeTestData, glazeBuffer); error) {
							std::cout << "Glaze Error: " << glz::format_error(error, glazeBuffer) << std::endl;
						}
						auto* newPtr = &glazeTestData;
						bnch_swt::doNotOptimizeAway(newPtr);
					});

			results_data glazeResults{ static_cast<jsonifier::string>(glazeLibraryName), static_cast<jsonifier::string>("Canada Test (Minified)"),
				static_cast<jsonifier::string>(glazeCommitUrl), glazeReadResult.iterationCount };
			std::string glazeNewerBuffer{};
			auto glazeWriteResult =
				bnch_swt::benchmark_stage<"Json-Performance", bnch_swt::bench_options{ .type = bnch_swt::result_type::time, .totalIterationCountCap = totalIterationCountCap }>::
					runBenchmark<jsonifier_internal::string_literal{ "Canada Test (Minified)" } + "-Write", glazeLibraryName, "steelblue">([&]() {
						bnch_swt::doNotOptimizeAway(glz::write<glz::opts{ .skip_null_members = false, .prettify = false, .minified = true }>(glazeTestData, glazeNewerBuffer));
						auto* newPtr = &glazeNewerBuffer;
						bnch_swt::doNotOptimizeAway(newPtr);
					});

			auto glazeReadSize		 = glazeNewerBuffer.size();
			auto glazeWrittenSize	 = glazeNewerBuffer.size();
			glazeResults.readResult	 = result<result_type::read>{ "dodgerblue", glazeReadSize, glazeReadResult };
			glazeResults.writeResult = result<result_type::write>{ "skyblue", glazeWrittenSize, glazeWriteResult };
			bnch_swt::file_loader<basePath + "/"  + "Canada Test (Minified)" + "-glaze.json">::saveFile(glazeBuffer);
			glazeResults.print();
			resultsNew.emplace_back(glazeResults);
#endif
			jsonifierBuffer = canadaMinifiedData;
			canada_message jsonifierTestData{};
			auto jsonifierReadResult =
				bnch_swt::benchmark_stage<"Json-Performance", bnch_swt::bench_options{ .type = bnch_swt::result_type::time, .totalIterationCountCap = totalIterationCountCap }>::
					runBenchmark<jsonifier_internal::string_literal{ "Canada Test (Minified)" } + "-Read", jsonifierLibraryName, "teal">([&]() {
						parser.parseJson<jsonifier::parse_options{ .minified = true }>(jsonifierTestData, jsonifierBuffer);
						auto* newPtr = &jsonifierTestData;
						bnch_swt::doNotOptimizeAway(newPtr);
					});
			results_data jsonifierResesults{ static_cast<jsonifier::string>(jsonifierLibraryName), static_cast<jsonifier::string>("Canada Test (Minified)"),
				static_cast<jsonifier::string>(jsonifierCommitUrl), jsonifierReadResult.iterationCount };
			for (auto& value: parser.getErrors()) {
				std::cout << "Jsonifier Error: " << value << std::endl;
			}
			std::string jsonifierNewerBuffer{};
			auto jsonifierWriteResult =
				bnch_swt::benchmark_stage<"Json-Performance", bnch_swt::bench_options{ .type = bnch_swt::result_type::time, .totalIterationCountCap = totalIterationCountCap }>::
					runBenchmark<jsonifier_internal::string_literal{ "Canada Test (Minified)" } + "-Write", jsonifierLibraryName, "steelblue">([&]() {
						parser.serializeJson<jsonifier::serialize_options{ .prettify = false }>(jsonifierTestData, jsonifierNewerBuffer);
						auto* newPtr = &jsonifierNewerBuffer;
						bnch_swt::doNotOptimizeAway(newPtr);
					});

			for (auto& value: parser.getErrors()) {
				std::cout << "Jsonifier Error: " << value << std::endl;
			}

			auto jsonifierReadSize		   = jsonifierNewerBuffer.size();
			auto jsonifierWrittenSize	   = jsonifierNewerBuffer.size();
			jsonifierResesults.readResult  = result<result_type::read>{ "teal", jsonifierReadSize, jsonifierReadResult };
			jsonifierResesults.writeResult = result<result_type::write>{ "steelblue", jsonifierWrittenSize, jsonifierWriteResult };
			bnch_swt::file_loader<basePath + "/"  + "Canada Test (Minified)" + "-jsonifier.json">::saveFile(jsonifierBuffer);
			jsonifierResesults.print();
			resultsNew.emplace_back(jsonifierResesults);

			std::string table{};
			const auto n = resultsNew.size();
			table += table_header + "\n";
			std::sort(resultsNew.begin(), resultsNew.end(), std::greater<results_data>());
			for (size_t x = 0; x < n; ++x) {
				jsonResults.results.emplace_back(resultsNew[x]);
				table += resultsNew[x].jsonStats();
				if (x != n - 1) {
					table += "\n";
				}
			}
			jsonResults.markdownResults = table;
			auto testResults			= jsonResults;
			newerString += static_cast<jsonifier::string>(section08);
			newerString += testResults.markdownResults;
			benchmark_data.emplace_back(testResults);
		}
		{
			jsonifier::vector<results_data> resultsNew{};
			test_results jsonResults{};
#if !defined(ASAN_ENABLED)
			citm_catalog_message simdjsonTestData{};
			simdjsonBuffer		 = citmCatalogData;
			jsonResults.testName = static_cast<jsonifier::string>("CitmCatalog Test (Prettified)");
			auto simdjsonReadResult =
				bnch_swt::benchmark_stage<"Json-Performance", bnch_swt::bench_options{ .type = bnch_swt::result_type::time, .totalIterationCountCap = totalIterationCountCap }>::
					runBenchmark<jsonifier_internal::string_literal{ "CitmCatalog Test (Prettified)" } + "-Read", simdjsonLibraryName, "cadetblue">([&]() {
						try {
							getValue(simdjsonTestData, simdjsonParser.iterate(simdjsonBuffer).value());
							bnch_swt::doNotOptimizeAway(simdjsonTestData);
						} catch (std::exception& error) {
							std::cout << "Simdjson Error: " << error.what() << std::endl;
						}
					});
			parser.serializeJson(simdjsonTestData, simdjsonBuffer);
			auto simdjsonReadSize = simdjsonBuffer.size();
			results_data simdjsonResults{ static_cast<jsonifier::string>(simdjsonLibraryName), static_cast<jsonifier::string>("CitmCatalog Test (Prettified)"),
				static_cast<jsonifier::string>(simdjsonCommitUrl), simdjsonReadResult.iterationCount };

			simdjsonResults.readResult = result<result_type::read>{ "cadetblue", simdjsonReadSize, simdjsonReadResult };
			bnch_swt::file_loader<basePath + "/"  + "CitmCatalog Test (Prettified)" + "-simdjson.json">::saveFile(simdjsonBuffer);
			simdjsonResults.print();
			resultsNew.emplace_back(simdjsonResults);

			glazeBuffer = citmCatalogData;

			citm_catalog_message glazeTestData{};
			auto glazeReadResult =
				bnch_swt::benchmark_stage<"Json-Performance", bnch_swt::bench_options{ .type = bnch_swt::result_type::time, .totalIterationCountCap = totalIterationCountCap }>::
					runBenchmark<jsonifier_internal::string_literal{ "CitmCatalog Test (Prettified)" } + "-Read", glazeLibraryName, "dodgerblue">([&]() {
						if (auto error = glz::read<glz::opts{ .skip_null_members = false, .minified = false }>(glazeTestData, glazeBuffer); error) {
							std::cout << "Glaze Error: " << glz::format_error(error, glazeBuffer) << std::endl;
						}
						auto* newPtr = &glazeTestData;
						bnch_swt::doNotOptimizeAway(newPtr);
					});

			results_data glazeResults{ static_cast<jsonifier::string>(glazeLibraryName), static_cast<jsonifier::string>("CitmCatalog Test (Prettified)"),
				static_cast<jsonifier::string>(glazeCommitUrl), glazeReadResult.iterationCount };
			std::string glazeNewerBuffer{};
			auto glazeWriteResult =
				bnch_swt::benchmark_stage<"Json-Performance", bnch_swt::bench_options{ .type = bnch_swt::result_type::time, .totalIterationCountCap = totalIterationCountCap }>::
					runBenchmark<jsonifier_internal::string_literal{ "CitmCatalog Test (Prettified)" } + "-Write", glazeLibraryName, "steelblue">([&]() {
						bnch_swt::doNotOptimizeAway(glz::write<glz::opts{ .skip_null_members = false, .prettify = true, .minified = false }>(glazeTestData, glazeNewerBuffer));
						auto* newPtr = &glazeNewerBuffer;
						bnch_swt::doNotOptimizeAway(newPtr);
					});

			auto glazeReadSize		 = glazeNewerBuffer.size();
			auto glazeWrittenSize	 = glazeNewerBuffer.size();
			glazeResults.readResult	 = result<result_type::read>{ "dodgerblue", glazeReadSize, glazeReadResult };
			glazeResults.writeResult = result<result_type::write>{ "skyblue", glazeWrittenSize, glazeWriteResult };
			bnch_swt::file_loader<basePath + "/"  + "CitmCatalog Test (Prettified)" + "-glaze.json">::saveFile(glazeBuffer);
			glazeResults.print();
			resultsNew.emplace_back(glazeResults);
#endif
			jsonifierBuffer = citmCatalogData;
			citm_catalog_message jsonifierTestData{};
			auto jsonifierReadResult =
				bnch_swt::benchmark_stage<"Json-Performance", bnch_swt::bench_options{ .type = bnch_swt::result_type::time, .totalIterationCountCap = totalIterationCountCap }>::
					runBenchmark<jsonifier_internal::string_literal{ "CitmCatalog Test (Prettified)" } + "-Read", jsonifierLibraryName, "teal">([&]() {
						parser.parseJson<jsonifier::parse_options{ .minified = false }>(jsonifierTestData, jsonifierBuffer);
						auto* newPtr = &jsonifierTestData;
						bnch_swt::doNotOptimizeAway(newPtr);
					});
			results_data jsonifierResesults{ static_cast<jsonifier::string>(jsonifierLibraryName), static_cast<jsonifier::string>("CitmCatalog Test (Prettified)"),
				static_cast<jsonifier::string>(jsonifierCommitUrl), jsonifierReadResult.iterationCount };
			for (auto& value: parser.getErrors()) {
				std::cout << "Jsonifier Error: " << value << std::endl;
			}
			std::string jsonifierNewerBuffer{};
			auto jsonifierWriteResult =
				bnch_swt::benchmark_stage<"Json-Performance", bnch_swt::bench_options{ .type = bnch_swt::result_type::time, .totalIterationCountCap = totalIterationCountCap }>::
					runBenchmark<jsonifier_internal::string_literal{ "CitmCatalog Test (Prettified)" } + "-Write", jsonifierLibraryName, "steelblue">([&]() {
						parser.serializeJson<jsonifier::serialize_options{ .prettify = true }>(jsonifierTestData, jsonifierNewerBuffer);
						auto* newPtr = &jsonifierNewerBuffer;
						bnch_swt::doNotOptimizeAway(newPtr);
					});

			for (auto& value: parser.getErrors()) {
				std::cout << "Jsonifier Error: " << value << std::endl;
			}

			auto jsonifierReadSize		   = jsonifierNewerBuffer.size();
			auto jsonifierWrittenSize	   = jsonifierNewerBuffer.size();
			jsonifierResesults.readResult  = result<result_type::read>{ "teal", jsonifierReadSize, jsonifierReadResult };
			jsonifierResesults.writeResult = result<result_type::write>{ "steelblue", jsonifierWrittenSize, jsonifierWriteResult };
			bnch_swt::file_loader<basePath + "/"  + "CitmCatalog Test (Prettified)" + "-jsonifier.json">::saveFile(jsonifierBuffer);
			jsonifierResesults.print();
			resultsNew.emplace_back(jsonifierResesults);

			std::string table{};
			const auto n = resultsNew.size();
			table += table_header + "\n";
			std::sort(resultsNew.begin(), resultsNew.end(), std::greater<results_data>());
			for (size_t x = 0; x < n; ++x) {
				jsonResults.results.emplace_back(resultsNew[x]);
				table += resultsNew[x].jsonStats();
				if (x != n - 1) {
					table += "\n";
				}
			}
			jsonResults.markdownResults = table;
			auto testResults			= jsonResults;
			newerString += static_cast<jsonifier::string>(section09);
			newerString += testResults.markdownResults;
			benchmark_data.emplace_back(testResults);
		}
		{
			jsonifier::vector<results_data> resultsNew{};
			test_results jsonResults{};
#if !defined(ASAN_ENABLED)
			citm_catalog_message simdjsonTestData{};
			simdjsonBuffer		 = citmCatalogMinifiedData;
			jsonResults.testName = static_cast<jsonifier::string>("CitmCatalog Test (Minified)");
			auto simdjsonReadResult =
				bnch_swt::benchmark_stage<"Json-Performance", bnch_swt::bench_options{ .type = bnch_swt::result_type::time, .totalIterationCountCap = totalIterationCountCap }>::
					runBenchmark<jsonifier_internal::string_literal{ "CitmCatalog Test (Minified)" } + "-Read", simdjsonLibraryName, "cadetblue">([&]() {
						try {
							getValue(simdjsonTestData, simdjsonParser.iterate(simdjsonBuffer).value());
							bnch_swt::doNotOptimizeAway(simdjsonTestData);
						} catch (std::exception& error) {
							std::cout << "Simdjson Error: " << error.what() << std::endl;
						}
					});
			parser.serializeJson(simdjsonTestData, simdjsonBuffer);
			auto simdjsonReadSize = simdjsonBuffer.size();
			results_data simdjsonResults{ static_cast<jsonifier::string>(simdjsonLibraryName), static_cast<jsonifier::string>("CitmCatalog Test (Minified)"),
				static_cast<jsonifier::string>(simdjsonCommitUrl), simdjsonReadResult.iterationCount };

			simdjsonResults.readResult = result<result_type::read>{ "cadetblue", simdjsonReadSize, simdjsonReadResult };
			bnch_swt::file_loader<basePath + "/"  + "CitmCatalog Test (Minified)" + "-simdjson.json">::saveFile(simdjsonBuffer);
			simdjsonResults.print();
			resultsNew.emplace_back(simdjsonResults);

			glazeBuffer = citmCatalogMinifiedData;

			citm_catalog_message glazeTestData{};
			auto glazeReadResult =
				bnch_swt::benchmark_stage<"Json-Performance", bnch_swt::bench_options{ .type = bnch_swt::result_type::time, .totalIterationCountCap = totalIterationCountCap }>::
					runBenchmark<jsonifier_internal::string_literal{ "CitmCatalog Test (Minified)" } + "-Read", glazeLibraryName, "dodgerblue">([&]() {
						if (auto error = glz::read<glz::opts{ .skip_null_members = false, .minified = true }>(glazeTestData, glazeBuffer); error) {
							std::cout << "Glaze Error: " << glz::format_error(error, glazeBuffer) << std::endl;
						}
						auto* newPtr = &glazeTestData;
						bnch_swt::doNotOptimizeAway(newPtr);
					});

			results_data glazeResults{ static_cast<jsonifier::string>(glazeLibraryName), static_cast<jsonifier::string>("CitmCatalog Test (Minified)"),
				static_cast<jsonifier::string>(glazeCommitUrl), glazeReadResult.iterationCount };
			std::string glazeNewerBuffer{};
			auto glazeWriteResult =
				bnch_swt::benchmark_stage<"Json-Performance", bnch_swt::bench_options{ .type = bnch_swt::result_type::time, .totalIterationCountCap = totalIterationCountCap }>::
					runBenchmark<jsonifier_internal::string_literal{ "CitmCatalog Test (Minified)" } + "-Write", glazeLibraryName, "steelblue">([&]() {
						bnch_swt::doNotOptimizeAway(glz::write<glz::opts{ .skip_null_members = false, .prettify = false, .minified = true }>(glazeTestData, glazeNewerBuffer));
						auto* newPtr = &glazeNewerBuffer;
						bnch_swt::doNotOptimizeAway(newPtr);
					});

			auto glazeReadSize		 = glazeNewerBuffer.size();
			auto glazeWrittenSize	 = glazeNewerBuffer.size();
			glazeResults.readResult	 = result<result_type::read>{ "dodgerblue", glazeReadSize, glazeReadResult };
			glazeResults.writeResult = result<result_type::write>{ "skyblue", glazeWrittenSize, glazeWriteResult };
			bnch_swt::file_loader<basePath + "/"  + "CitmCatalog Test (Minified)" + "-glaze.json">::saveFile(glazeBuffer);
			glazeResults.print();
			resultsNew.emplace_back(glazeResults);
#endif
			jsonifierBuffer = citmCatalogMinifiedData;
			citm_catalog_message jsonifierTestData{};
			auto jsonifierReadResult =
				bnch_swt::benchmark_stage<"Json-Performance", bnch_swt::bench_options{ .type = bnch_swt::result_type::time, .totalIterationCountCap = totalIterationCountCap }>::
					runBenchmark<jsonifier_internal::string_literal{ "CitmCatalog Test (Minified)" } + "-Read", jsonifierLibraryName, "teal">([&]() {
						parser.parseJson<jsonifier::parse_options{ .minified = true }>(jsonifierTestData, jsonifierBuffer);
						auto* newPtr = &jsonifierTestData;
						bnch_swt::doNotOptimizeAway(newPtr);
					});
			results_data jsonifierResesults{ static_cast<jsonifier::string>(jsonifierLibraryName), static_cast<jsonifier::string>("CitmCatalog Test (Minified)"),
				static_cast<jsonifier::string>(jsonifierCommitUrl), jsonifierReadResult.iterationCount };
			for (auto& value: parser.getErrors()) {
				std::cout << "Jsonifier Error: " << value << std::endl;
			}
			std::string jsonifierNewerBuffer{};
			auto jsonifierWriteResult =
				bnch_swt::benchmark_stage<"Json-Performance", bnch_swt::bench_options{ .type = bnch_swt::result_type::time, .totalIterationCountCap = totalIterationCountCap }>::
					runBenchmark<jsonifier_internal::string_literal{ "CitmCatalog Test (Minified)" } + "-Write", jsonifierLibraryName, "steelblue">([&]() {
						parser.serializeJson<jsonifier::serialize_options{ .prettify = false }>(jsonifierTestData, jsonifierNewerBuffer);
						auto* newPtr = &jsonifierNewerBuffer;
						bnch_swt::doNotOptimizeAway(newPtr);
					});

			for (auto& value: parser.getErrors()) {
				std::cout << "Jsonifier Error: " << value << std::endl;
			}

			auto jsonifierReadSize		   = jsonifierNewerBuffer.size();
			auto jsonifierWrittenSize	   = jsonifierNewerBuffer.size();
			jsonifierResesults.readResult  = result<result_type::read>{ "teal", jsonifierReadSize, jsonifierReadResult };
			jsonifierResesults.writeResult = result<result_type::write>{ "steelblue", jsonifierWrittenSize, jsonifierWriteResult };
			bnch_swt::file_loader<basePath + "/"  + "CitmCatalog Test (Minified)" + "-jsonifier.json">::saveFile(jsonifierBuffer);
			jsonifierResesults.print();
			resultsNew.emplace_back(jsonifierResesults);

			std::string table{};
			const auto n = resultsNew.size();
			table += table_header + "\n";
			std::sort(resultsNew.begin(), resultsNew.end(), std::greater<results_data>());
			for (size_t x = 0; x < n; ++x) {
				jsonResults.results.emplace_back(resultsNew[x]);
				table += resultsNew[x].jsonStats();
				if (x != n - 1) {
					table += "\n";
				}
			}
			jsonResults.markdownResults = table;
			auto testResults			= jsonResults;
			newerString += static_cast<jsonifier::string>(section10);
			newerString += testResults.markdownResults;
			benchmark_data.emplace_back(testResults);
		}
		{
			jsonifier::vector<results_data> resultsNew{};
			test_results jsonResults{};
#if !defined(ASAN_ENABLED)
			twitter_message simdjsonTestData{};
			simdjsonBuffer		 = twitterData;
			jsonResults.testName = static_cast<jsonifier::string>("Twitter Test (Prettified)");
			auto simdjsonReadResult =
				bnch_swt::benchmark_stage<"Json-Performance", bnch_swt::bench_options{ .type = bnch_swt::result_type::time, .totalIterationCountCap = totalIterationCountCap }>::
					runBenchmark<jsonifier_internal::string_literal{ "Twitter Test (Prettified)" } + "-Read", simdjsonLibraryName, "cadetblue">([&]() {
						try {
							getValue(simdjsonTestData, simdjsonParser.iterate(simdjsonBuffer).value());
							bnch_swt::doNotOptimizeAway(simdjsonTestData);
						} catch (std::exception& error) {
							std::cout << "Simdjson Error: " << error.what() << std::endl;
						}
					});
			parser.serializeJson<jsonifier::serialize_options{ .prettify = true }>(simdjsonTestData, simdjsonBuffer);
			auto simdjsonReadSize = simdjsonBuffer.size();
			results_data simdjsonResults{ static_cast<jsonifier::string>(simdjsonLibraryName), static_cast<jsonifier::string>("Twitter Test (Prettified)"),
				static_cast<jsonifier::string>(simdjsonCommitUrl), simdjsonReadResult.iterationCount };

			simdjsonResults.readResult = result<result_type::read>{ "cadetblue", simdjsonReadSize, simdjsonReadResult };
			bnch_swt::file_loader<basePath + "/"  + "Twitter Test (Prettified)" + "-simdjson.json">::saveFile(simdjsonBuffer);
			simdjsonResults.print();
			resultsNew.emplace_back(simdjsonResults);

			glazeBuffer = twitterData;

			twitter_message glazeTestData{};
			auto glazeReadResult =
				bnch_swt::benchmark_stage<"Json-Performance", bnch_swt::bench_options{ .type = bnch_swt::result_type::time, .totalIterationCountCap = totalIterationCountCap }>::
					runBenchmark<jsonifier_internal::string_literal{ "Twitter Test (Prettified)" } + "-Read", glazeLibraryName, "dodgerblue">([&]() {
						if (auto error = glz::read<glz::opts{ .skip_null_members = false, .minified = false }>(glazeTestData, glazeBuffer); error) {
							std::cout << "Glaze Error: " << glz::format_error(error, glazeBuffer) << std::endl;
						}
						auto* newPtr = &glazeTestData;
						bnch_swt::doNotOptimizeAway(newPtr);
					});

			results_data glazeResults{ static_cast<jsonifier::string>(glazeLibraryName), static_cast<jsonifier::string>("Twitter Test (Prettified)"),
				static_cast<jsonifier::string>(glazeCommitUrl), glazeReadResult.iterationCount };
			std::string glazeNewerBuffer{};
			auto glazeWriteResult =
				bnch_swt::benchmark_stage<"Json-Performance", bnch_swt::bench_options{ .type = bnch_swt::result_type::time, .totalIterationCountCap = totalIterationCountCap }>::
					runBenchmark<jsonifier_internal::string_literal{ "Twitter Test (Prettified)" } + "-Write", glazeLibraryName, "steelblue">([&]() {
						bnch_swt::doNotOptimizeAway(glz::write<glz::opts{ .skip_null_members = false, .prettify = true, .minified = false }>(glazeTestData, glazeNewerBuffer));
						auto* newPtr = &glazeNewerBuffer;
						bnch_swt::doNotOptimizeAway(newPtr);
					});

			auto glazeReadSize		 = glazeNewerBuffer.size();
			auto glazeWrittenSize	 = glazeNewerBuffer.size();
			glazeResults.readResult	 = result<result_type::read>{ "dodgerblue", glazeReadSize, glazeReadResult };
			glazeResults.writeResult = result<result_type::write>{ "skyblue", glazeWrittenSize, glazeWriteResult };
			bnch_swt::file_loader<basePath + "/"  + "Twitter Test (Prettified)" + "-glaze.json">::saveFile(glazeBuffer);
			glazeResults.print();
			resultsNew.emplace_back(glazeResults);
#endif
			jsonifierBuffer = twitterData;
			twitter_message jsonifierTestData{};
			auto jsonifierReadResult =
				bnch_swt::benchmark_stage<"Json-Performance", bnch_swt::bench_options{ .type = bnch_swt::result_type::time, .totalIterationCountCap = totalIterationCountCap }>::
					runBenchmark<jsonifier_internal::string_literal{ "Twitter Test (Prettified)" } + "-Read", jsonifierLibraryName, "teal">([&]() {
						parser.parseJson<jsonifier::parse_options{ .minified = false }>(jsonifierTestData, jsonifierBuffer);
						auto* newPtr = &jsonifierTestData;
						bnch_swt::doNotOptimizeAway(newPtr);
					});
			results_data jsonifierResesults{ static_cast<jsonifier::string>(jsonifierLibraryName), static_cast<jsonifier::string>("Twitter Test (Prettified)"),
				static_cast<jsonifier::string>(jsonifierCommitUrl), jsonifierReadResult.iterationCount };
			for (auto& value: parser.getErrors()) {
				std::cout << "Jsonifier Error: " << value << std::endl;
			}
			std::string jsonifierNewerBuffer{};
			auto jsonifierWriteResult =
				bnch_swt::benchmark_stage<"Json-Performance", bnch_swt::bench_options{ .type = bnch_swt::result_type::time, .totalIterationCountCap = totalIterationCountCap }>::
					runBenchmark<jsonifier_internal::string_literal{ "Twitter Test (Prettified)" } + "-Write", jsonifierLibraryName, "steelblue">([&]() {
						parser.serializeJson<jsonifier::serialize_options{ .prettify = true }>(jsonifierTestData, jsonifierNewerBuffer);
						auto* newPtr = &jsonifierNewerBuffer;
						bnch_swt::doNotOptimizeAway(newPtr);
					});

			for (auto& value: parser.getErrors()) {
				std::cout << "Jsonifier Error: " << value << std::endl;
			}

			auto jsonifierReadSize		   = jsonifierNewerBuffer.size();
			auto jsonifierWrittenSize	   = jsonifierNewerBuffer.size();
			jsonifierResesults.readResult  = result<result_type::read>{ "teal", jsonifierReadSize, jsonifierReadResult };
			jsonifierResesults.writeResult = result<result_type::write>{ "steelblue", jsonifierWrittenSize, jsonifierWriteResult };
			bnch_swt::file_loader<basePath + "/"  + "Twitter Test (Prettified)" + "-jsonifier.json">::saveFile(jsonifierBuffer);
			jsonifierResesults.print();
			resultsNew.emplace_back(jsonifierResesults);

			std::string table{};
			const auto n = resultsNew.size();
			table += table_header + "\n";
			std::sort(resultsNew.begin(), resultsNew.end(), std::greater<results_data>());
			for (size_t x = 0; x < n; ++x) {
				jsonResults.results.emplace_back(resultsNew[x]);
				table += resultsNew[x].jsonStats();
				if (x != n - 1) {
					table += "\n";
				}
			}
			jsonResults.markdownResults = table;
			auto testResults			= jsonResults;
			newerString += static_cast<jsonifier::string>(section11);
			newerString += testResults.markdownResults;
			benchmark_data.emplace_back(testResults);
		}
		{
			jsonifier::vector<results_data> resultsNew{};
			test_results jsonResults{};
#if !defined(ASAN_ENABLED)
			twitter_message simdjsonTestData{};
			simdjsonBuffer		 = twitterMinifiedData;
			jsonResults.testName = static_cast<jsonifier::string>("Twitter Test (Minified)");
			auto simdjsonReadResult =
				bnch_swt::benchmark_stage<"Json-Performance", bnch_swt::bench_options{ .type = bnch_swt::result_type::time, .totalIterationCountCap = totalIterationCountCap }>::
					runBenchmark<jsonifier_internal::string_literal{ "Twitter Test (Minified)" } + "-Read", simdjsonLibraryName, "cadetblue">([&]() {
						try {
							getValue(simdjsonTestData, simdjsonParser.iterate(simdjsonBuffer).value());
							bnch_swt::doNotOptimizeAway(simdjsonTestData);
						} catch (std::exception& error) {
							std::cout << "Simdjson Error: " << error.what() << std::endl;
						}
					});
			parser.serializeJson(simdjsonTestData, simdjsonBuffer);
			auto simdjsonReadSize = simdjsonBuffer.size();
			results_data simdjsonResults{ static_cast<jsonifier::string>(simdjsonLibraryName), static_cast<jsonifier::string>("Twitter Test (Minified)"),
				static_cast<jsonifier::string>(simdjsonCommitUrl), simdjsonReadResult.iterationCount };

			simdjsonResults.readResult = result<result_type::read>{ "cadetblue", simdjsonReadSize, simdjsonReadResult };
			bnch_swt::file_loader<basePath + "/"  + "Twitter Test (Minified)" + "-simdjson.json">::saveFile(simdjsonBuffer);
			simdjsonResults.print();
			resultsNew.emplace_back(simdjsonResults);

			glazeBuffer = twitterMinifiedData;

			twitter_message glazeTestData{};
			auto glazeReadResult =
				bnch_swt::benchmark_stage<"Json-Performance", bnch_swt::bench_options{ .type = bnch_swt::result_type::time, .totalIterationCountCap = totalIterationCountCap }>::
					runBenchmark<jsonifier_internal::string_literal{ "Twitter Test (Minified)" } + "-Read", glazeLibraryName, "dodgerblue">([&]() {
						if (auto error = glz::read<glz::opts{ .skip_null_members = false, .minified = true }>(glazeTestData, glazeBuffer); error) {
							std::cout << "Glaze Error: " << glz::format_error(error, glazeBuffer) << std::endl;
						}
						auto* newPtr = &glazeTestData;
						bnch_swt::doNotOptimizeAway(newPtr);
					});

			results_data glazeResults{ static_cast<jsonifier::string>(glazeLibraryName), static_cast<jsonifier::string>("Twitter Test (Minified)"),
				static_cast<jsonifier::string>(glazeCommitUrl), glazeReadResult.iterationCount };
			std::string glazeNewerBuffer{};
			auto glazeWriteResult =
				bnch_swt::benchmark_stage<"Json-Performance", bnch_swt::bench_options{ .type = bnch_swt::result_type::time, .totalIterationCountCap = totalIterationCountCap }>::
					runBenchmark<jsonifier_internal::string_literal{ "Twitter Test (Minified)" } + "-Write", glazeLibraryName, "steelblue">([&]() {
						bnch_swt::doNotOptimizeAway(glz::write<glz::opts{ .skip_null_members = false, .prettify = false, .minified = true }>(glazeTestData, glazeNewerBuffer));
						auto* newPtr = &glazeNewerBuffer;
						bnch_swt::doNotOptimizeAway(newPtr);
					});

			auto glazeReadSize		 = glazeNewerBuffer.size();
			auto glazeWrittenSize	 = glazeNewerBuffer.size();
			glazeResults.readResult	 = result<result_type::read>{ "dodgerblue", glazeReadSize, glazeReadResult };
			glazeResults.writeResult = result<result_type::write>{ "skyblue", glazeWrittenSize, glazeWriteResult };
			bnch_swt::file_loader<basePath + "/"  + "Twitter Test (Minified)" + "-glaze.json">::saveFile(glazeBuffer);
			glazeResults.print();
			resultsNew.emplace_back(glazeResults);
#endif
			jsonifierBuffer = twitterMinifiedData;
			twitter_message jsonifierTestData{};
			auto jsonifierReadResult =
				bnch_swt::benchmark_stage<"Json-Performance", bnch_swt::bench_options{ .type = bnch_swt::result_type::time, .totalIterationCountCap = totalIterationCountCap }>::
					runBenchmark<jsonifier_internal::string_literal{ "Twitter Test (Minified)" } + "-Read", jsonifierLibraryName, "teal">([&]() {
						parser.parseJson<jsonifier::parse_options{ .minified = true }>(jsonifierTestData, jsonifierBuffer);
						auto* newPtr = &jsonifierTestData;
						bnch_swt::doNotOptimizeAway(newPtr);
					});
			results_data jsonifierResesults{ static_cast<jsonifier::string>(jsonifierLibraryName), static_cast<jsonifier::string>("Twitter Test (Minified)"),
				static_cast<jsonifier::string>(jsonifierCommitUrl), jsonifierReadResult.iterationCount };
			for (auto& value: parser.getErrors()) {
				std::cout << "Jsonifier Error: " << value << std::endl;
			}
			std::string jsonifierNewerBuffer{};
			auto jsonifierWriteResult =
				bnch_swt::benchmark_stage<"Json-Performance", bnch_swt::bench_options{ .type = bnch_swt::result_type::time, .totalIterationCountCap = totalIterationCountCap }>::
					runBenchmark<jsonifier_internal::string_literal{ "Twitter Test (Minified)" } + "-Write", jsonifierLibraryName, "steelblue">([&]() {
						parser.serializeJson<jsonifier::serialize_options{ .prettify = false }>(jsonifierTestData, jsonifierNewerBuffer);
						auto* newPtr = &jsonifierNewerBuffer;
						bnch_swt::doNotOptimizeAway(newPtr);
					});

			for (auto& value: parser.getErrors()) {
				std::cout << "Jsonifier Error: " << value << std::endl;
			}

			auto jsonifierReadSize		   = jsonifierNewerBuffer.size();
			auto jsonifierWrittenSize	   = jsonifierNewerBuffer.size();
			jsonifierResesults.readResult  = result<result_type::read>{ "teal", jsonifierReadSize, jsonifierReadResult };
			jsonifierResesults.writeResult = result<result_type::write>{ "steelblue", jsonifierWrittenSize, jsonifierWriteResult };
			bnch_swt::file_loader<basePath + "/"  + "Twitter Test (Minified)" + "-jsonifier.json">::saveFile(jsonifierBuffer);
			jsonifierResesults.print();
			resultsNew.emplace_back(jsonifierResesults);

			std::string table{};
			const auto n = resultsNew.size();
			table += table_header + "\n";
			std::sort(resultsNew.begin(), resultsNew.end(), std::greater<results_data>());
			for (size_t x = 0; x < n; ++x) {
				jsonResults.results.emplace_back(resultsNew[x]);
				table += resultsNew[x].jsonStats();
				if (x != n - 1) {
					table += "\n";
				}
			}
			jsonResults.markdownResults = table;
			auto testResults			= jsonResults;
			newerString += static_cast<jsonifier::string>(section12);
			newerString += testResults.markdownResults;
			benchmark_data.emplace_back(testResults);
		}
		auto testResults = json_tests_helper<test_type::minify, std::string, false, "Minify Test">::run(discordData);
		newerString += static_cast<jsonifier::string>(section13);
		newerString += testResults.markdownResults;
		benchmark_data.emplace_back(testResults);
		testResults = json_tests_helper<test_type::prettify, std::string, false, "Prettify Test">::run(jsonMinifiedData);
		newerString += static_cast<jsonifier::string>(section14);
		newerString += testResults.markdownResults;
		benchmark_data.emplace_back(testResults);
		testResults = json_tests_helper<test_type::validate, std::string, false, "Validate Test">::run(discordData);
		newerString += static_cast<jsonifier::string>(section15);
		newerString += testResults.markdownResults;
		benchmark_data.emplace_back(testResults);
		bnch_swt::benchmark_stage<"Json-Performance">::printResults();
		std::string resultsStringJson{};
		test_results_final resultsData{};
		for (auto& value: benchmark_data) {
			test_elements_final testElement{};
			testElement.testName = value.testName;
			for (auto& valueNew: value.results) {
				test_element_final resultFinal{};
				if (valueNew.readResult.jsonSpeed.has_value()) {
					resultFinal.libraryName	   = valueNew.name;
					resultFinal.color		   = valueNew.readResult.color;
					resultFinal.resultSpeed	   = valueNew.readResult.jsonSpeed.value();
					resultFinal.iterationCount = valueNew.readResult.iterationCount.value();
					resultFinal.resultType	   = "Read";
					testElement.results.emplace_back(resultFinal);
				}
				if (valueNew.writeResult.jsonSpeed.has_value()) {
					resultFinal.libraryName	   = valueNew.name;
					resultFinal.color		   = valueNew.writeResult.color;
					resultFinal.resultSpeed	   = valueNew.writeResult.jsonSpeed.value();
					resultFinal.iterationCount = valueNew.writeResult.iterationCount.value();
					resultFinal.resultType	   = "Write";
					testElement.results.emplace_back(resultFinal);
				}
			}
			resultsData.emplace_back(testElement);
		}
		parser.serializeJson<jsonifier::serialize_options{ .prettify = true }>(resultsData, resultsStringJson);
		std::cout << "Json Data: " << resultsStringJson << std::endl;
		bnch_swt::file_loader<basePath + "/Results.json">::saveFile(resultsStringJson);
		bnch_swt::file_loader<README_PATH>::saveFile(newerString);
		std::cout << "Md Data: " << newerString << std::endl;
		executePythonScript(static_cast<std::string>(BASE_PATH) + "/GenerateGraphs.py", basePath + "/Results.json", static_cast<std::string>(GRAPHS_PATH));
	} catch (std::runtime_error& e) {
		std::cout << e.what() << std::endl;
	} catch (std::out_of_range& e) {
		std::cout << e.what() << std::endl;
	}
	return 0;
}