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
#include "UnicodeEmoji.hpp"
#include "CitmCatalog.hpp"
#include "Twitter.hpp"
#include "Discord.hpp"
#include "Canada.hpp"
#include <thread>

#if defined(NDEBUG)
static constexpr auto maxIterationCount{ 100 };
#else
static constexpr auto maxIterationCount{ 5 };
#endif

constexpr auto getCurrentOperatingSystem() {
	constexpr jsonifier_internal::string_literal osName{ OPERATING_SYSTEM_NAME };
	constexpr auto osNameNew = jsonifier_internal::toLower(osName);
	if constexpr (osNameNew.view().operator std::string_view().contains("linux")) {
		return jsonifier_internal::string_literal{ "Ubuntu" };
	} else if constexpr (osNameNew.view().operator std::string_view().contains("windows")) {
		return jsonifier_internal::string_literal{ "Windows" };
	} else if constexpr (osNameNew.view().operator std::string_view().contains("darwin")) {
		return jsonifier_internal::string_literal{ "MacOS" };
	} else {
		return jsonifier_internal::string_literal{ "" };
	}
}

constexpr auto getCurrentCompilerId() {
	constexpr jsonifier_internal::string_literal compilerId{ COMPILER_ID };
	constexpr auto osCompilerIdNew = jsonifier_internal::toLower(compilerId);
	if constexpr (osCompilerIdNew.view().operator std::string_view().contains("gnu") || osCompilerIdNew.view().operator std::string_view().contains("gcc") ||
		osCompilerIdNew.view().operator std::string_view().contains("g++") || osCompilerIdNew.view().operator std::string_view().contains("apple")) {
		return jsonifier_internal::string_literal{ "GNUCXX" };
	} else if constexpr (osCompilerIdNew.view().operator std::string_view().contains("clang")) {
		return jsonifier_internal::string_literal{ "CLANG" };
	} else if constexpr (osCompilerIdNew.view().operator std::string_view().contains("msvc")) {
		return jsonifier_internal::string_literal{ "MSVC" };
	} else {
		return jsonifier_internal::string_literal{ "" };
	}
}

constexpr auto getCurrentPathImpl() {
	return getCurrentOperatingSystem() + "-" + getCurrentCompilerId();
}

constexpr jsonifier_internal::string_literal basePath{ BASE_PATH };
constexpr jsonifier_internal::string_literal testPath{ basePath + "/Source" };
constexpr jsonifier_internal::string_literal readMePath{ BASE_PATH };
constexpr jsonifier_internal::string_literal jsonPath{ basePath + "/Json" };
constexpr jsonifier_internal::string_literal jsonOutPath{ jsonPath + "/" + getCurrentPathImpl() };
constexpr jsonifier_internal::string_literal graphsPath{ basePath + "/Graphs/" + getCurrentPathImpl() + "/" };
constexpr jsonifier_internal::string_literal jsonifierLibraryName{ "jsonifier" };
constexpr jsonifier_internal::string_literal jsonifierCommitUrlBase{ "https://github.com/realtimechris/jsonifier/commit/" };
constexpr jsonifier_internal::string_literal simdjsonLibraryName{ "simdjson" };
constexpr jsonifier_internal::string_literal simdjsonCommitUrlBase{ "https://github.com/simdjson/simdjson/commit/" };
constexpr jsonifier_internal::string_literal glazeLibraryName{ "glaze" };
constexpr jsonifier_internal::string_literal glazeCommitUrlBase{ "https://github.com/stephenberry/glaze/commit/" };
constexpr jsonifier_internal::string_literal jsonifierCommitUrl{ jsonifierCommitUrlBase + JSONIFIER_COMMIT };
constexpr jsonifier_internal::string_literal simdjsonCommitUrl{ simdjsonCommitUrlBase + SIMDJSON_COMMIT };
constexpr jsonifier_internal::string_literal glazeCommitUrl{ glazeCommitUrlBase + GLAZE_COMMIT };

class test_base {
  public:
	test_base() noexcept = default;

	test_base(const std::string& stringNew, const std::string& fileContentsNew, bool areWeAFailingTestNew)
		: fileContents{ fileContentsNew }, areWeAFailingTest{ areWeAFailingTestNew }, testName{ stringNew } {};
	std::string fileContents{};
	bool areWeAFailingTest{};
	std::string testName{};
};

std::string getCPUInfo() {
	char brand[49] = { 0 };
	int32_t regs[12]{};
	size_t length{};
#if defined(__x86_64__) || defined(_M_AMD64)
	static constexpr auto cpuid = [](int32_t* eax, int32_t* ebx, int32_t* ecx, int32_t* edx) {
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
	std::string returnValues{};
	returnValues.resize(length - 1);
	std::copy(brand, brand + length, returnValues.data());
	return returnValues.substr(0, returnValues.find_last_of("abcdefghijklmnopqrstuvwxyz1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ") + 1);
#else
	char newBuffer[256];
	size_t bufferSize = sizeof(newBuffer);
	if (sysctlbyname("machdep.cpu.brand_string", &newBuffer, &bufferSize, nullptr, 0) == 0) {
		return std::string(newBuffer);
	} else {
		return std::string{ "Unknown CPU" };
	}
#endif
}

std::string getCurrentWorkingDirectory() {
	try {
		return std::filesystem::current_path().string();
	} catch (const std::filesystem::filesystem_error& e) {
		std::cout << "Error: " << e.what() << std::endl;
		return "";
	}
}

void executePythonScript(const std::string& scriptPath, const std::string& argument01, const std::string& argument02) {
#if defined(JSONIFIER_WIN)
	static std::string pythonName{ "python " };
#else
	static std::string pythonName{ "python3 " };
#endif
	std::string command = pythonName + scriptPath + " " + argument01 + " " + argument02;
	int32_t result		= system(command.data());
	if (result != 0) {
		std::cout << "Error: Failed to execute Python script. Command exited with code " << result << std::endl;
	}
}

bool processFilesInFolder(std::unordered_map<std::string, test_base>& resultFileContents, const std::string& testType) noexcept {
	try {
		for (const auto& entry: std::filesystem::directory_iterator(std::string{ testPath } + testType)) {
			if (entry.is_regular_file()) {
				const std::string fileName = entry.path().filename().string();

				if (fileName.size() >= 5 && fileName.substr(fileName.size() - 5) == std::string{ ".json" }) {
					std::ifstream file(entry.path());
					if (file.is_open()) {
						std::string fileContents((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
						bool returnValue					= (fileName.find(".json") != std::string::npos);
						resultFileContents[fileName.data()] = { fileName, fileContents, returnValue };
						file.close();
					} else {
						std::cerr << "Error opening file: " << fileName << std::endl;
						return false;
					}
				}
			}
		}
	} catch (const std::exception& e) {
		std::cerr << "Error while processing files: " << e.what() << std::endl;
		return false;
	}

	return true;
}

struct test_struct {
	std::vector<std::string> testStrings{};
	std::vector<uint64_t> testUints{};
	std::vector<double> testDoubles{};
	std::vector<int64_t> testInts{};
	std::vector<bool> testBools{};
};

struct abc_test_struct {
	std::vector<bool> testBools{};
	std::vector<int64_t> testInts{};
	std::vector<double> testDoubles{};
	std::vector<uint64_t> testUints{};
	std::vector<std::string> testStrings{};
};

template<typename value_type> struct test {
	std::vector<value_type> a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v, w, x, y, z;
};

template<typename value_type> struct partial_test {
	std::vector<value_type> m, s;
};

inline static std::random_device randomEngine{};
inline static std::mt19937_64 gen{ randomEngine() };
static constexpr std::string_view charset{ "!#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[]^_`abcdefghijklmnopqrstuvwxyz{|}~\"\\\r\b\f\t\n" };

template<typename value_type> struct test_generator {
	std::vector<value_type> a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v, w, x, y, z;

	template<typename value_type01, typename value_type02> static value_type01 randomizeNumberUniform(value_type01 start, value_type02 end) {
		std::uniform_real_distribution<value_type01> dis{ start, static_cast<value_type01>(end) };
		return dis(gen);
	}

	template<jsonifier::concepts::integer_t value_type01, jsonifier::concepts::integer_t value_type02>
	static value_type01 randomizeNumberUniform(value_type01 start, value_type02 end) {
		std::uniform_int_distribution<value_type01> dis{ start, static_cast<value_type01>(end) };
		return dis(gen);
	}

	static void insertUnicodeInJSON(std::string& jsonString) {
		auto newStringView = unicode_emoji::unicodeEmoji[randomizeNumberUniform(0ull, std::size(unicode_emoji::unicodeEmoji) - 1)];
		jsonString += static_cast<std::string>(newStringView);
	}

	static std::string generateString() {
		auto length{ randomizeNumberUniform(32ull, 64ull) };
		constexpr size_t charsetSize = charset.size();
		auto unicodeCount			 = length / 4ull;
		std::vector<size_t> unicodeIndices{};
		static constexpr auto checkForPresenceOfIndex = [](auto& indices, auto index, auto length, auto&& checkForPresenceOfIndexNew) -> void {
			if (std::find(indices.begin(), indices.end(), index) != indices.end()) {
				index = randomizeNumberUniform(0ull, length);
				checkForPresenceOfIndexNew(indices, index, length, checkForPresenceOfIndexNew);
			} else {
				indices.emplace_back(index);
			}
		};
		for (size_t x = 0; x < unicodeCount; ++x) {
			auto newValue = randomizeNumberUniform(0ull, length);
			checkForPresenceOfIndex(unicodeIndices, newValue, length, checkForPresenceOfIndex);
		}
		std::sort(unicodeIndices.begin(), unicodeIndices.end(), std::less<size_t>{});

		std::string result{};
		size_t insertedUnicode = 0;
		auto iter			   = unicodeIndices.begin();
		for (size_t x = 0; x < length; ++x) {
			if (iter < unicodeIndices.end() && x == *iter) [[unlikely]] {
				insertUnicodeInJSON(result);
				insertedUnicode++;
				++iter;
			} else {
				result += charset[randomizeNumberUniform(0ull, charsetSize - 1)];
			}
		}

		return result;
	}

	static double generateDouble() {
		double min = std::numeric_limits<double>::min();
		double max = std::numeric_limits<double>::max();
		std::uniform_real_distribution<double> dis(log(min), log(max));
		double logValue = dis(gen);
		bool negative{ generateBool() };
		return negative ? -std::exp(logValue) : std::exp(logValue);
	}

	static bool generateBool() {
		return static_cast<bool>(randomizeNumberUniform(0, 100) >= 50);
	};

	static size_t generateUint() {
		return randomizeNumberUniform(std::numeric_limits<size_t>::min(), std::numeric_limits<size_t>::max());
	};

	static int64_t generateInt() {
		return randomizeNumberUniform(std::numeric_limits<int64_t>::min(), std::numeric_limits<int64_t>::max());
	};

	test_generator() {
		auto fill = [&](auto& v) {
			auto arraySize01 = randomizeNumberUniform(15ull, 25ull);
			v.resize(arraySize01);
			for (size_t x = 0; x < arraySize01; ++x) {
				auto arraySize02 = randomizeNumberUniform(15ull, 35ull);
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

template<result_type type> constexpr auto enumToString() {
	if constexpr (type == result_type::read) {
		return jsonifier_internal::string_literal{ "Read" };
	} else {
		return jsonifier_internal::string_literal{ "Write" };
	}
}

template<result_type type> struct result {
	std::optional<double> jsonSpeedVariation{};
	std::optional<size_t> byteLength{};
	std::optional<double> jsonCycles{};
	std::optional<double> jsonSpeed{};
	std::optional<double> jsonTime{};
	std::string color{};

	result() noexcept = default;

	result& operator=(result&&) noexcept	  = default;
	result(result&&) noexcept				  = default;
	result& operator=(const result&) noexcept = default;
	result(const result&) noexcept			  = default;

	result(const std::string& colorNew, size_t byteLengthNew, const bnch_swt::performance_metrics& results) {
		byteLength.emplace(byteLengthNew);
		jsonTime.emplace(results.timeInns);
		jsonSpeedVariation.emplace(results.throughputVariation.value());
		jsonSpeed.emplace(results.throughputMbPerSec.value());
		if (results.cyclesPerByte.has_value()) {
			jsonCycles.emplace(results.cyclesPerByte.value() * 1024 * 1024);
		}
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
	size_t iterations{};
	std::string name{};
	std::string test{};
	std::string url{};

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

	results_data(const std::string& nameNew, const std::string& testNew, const std::string& urlNew, size_t iterationsNew) {
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

	std::string jsonStats() const noexcept {
		std::string finalString{ "| [" + name + "](" + url + ") | " };
		if (readResult.jsonTime.has_value() && readResult.byteLength.has_value()) {
			std::ostringstream finalStream{};
			finalStream << std::setprecision(6) << readResult.jsonSpeed.value() << " | " << std::setprecision(6) << readResult.jsonSpeedVariation.value() << " | ";
			if (readResult.jsonCycles.has_value()) {
				finalStream << std::setprecision(6) << readResult.jsonCycles.value() << " | ";
			}
			finalStream << readResult.byteLength.value() << " | " << std::setprecision(6) << readResult.jsonTime.value() << " | ";
			finalString += finalStream.str();
		}
		if (writeResult.jsonTime.has_value() && writeResult.byteLength.has_value()) {
			std::ostringstream finalStream{};
			finalStream << std::setprecision(6) << writeResult.jsonSpeed.value() << " | " << std::setprecision(6) << writeResult.jsonSpeedVariation.value() << " | ";
			if (writeResult.jsonCycles.has_value()) {
				finalStream << std::setprecision(6) << writeResult.jsonCycles.value() << " | ";
			}
			finalStream << writeResult.byteLength.value() << " | " << std::setprecision(6) << writeResult.jsonTime.value() << " | ";
			finalString += finalStream.str();
		}
		return finalString;
	}
};

struct test_results {
	std::vector<results_data> results{};
	std::string markdownResults{};
	std::string testName{};
};

std::tm getTime() {
#if defined(JSONIFIER_WIN)
	std::time_t result = std::time(nullptr);
	std::tm resultTwo{};
	localtime_s(&resultTwo, &result);
	return resultTwo;
#else
	std::time_t result = std::time(nullptr);
	return *localtime(&result);
#endif
}
