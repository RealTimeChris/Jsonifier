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
#include <unordered_set>
#include <thread>
#include <random>

#if defined(NDEBUG)
static constexpr auto maxIterations{ 1400 };
static constexpr auto measuredIterations{ 20 };
#else
static constexpr auto maxIterations{ 200 };
static constexpr auto measuredIterations{ 25 };
#endif

constexpr auto getCurrentOperatingSystem() {
	constexpr bnch_swt::string_literal osName{ OPERATING_SYSTEM_NAME };
	constexpr auto osNameNew = bnch_swt::internal::toLower(osName);
	if constexpr (osNameNew.operator std::string_view().contains("linux")) {
		return bnch_swt::string_literal{ "Ubuntu" };
	} else if constexpr (osNameNew.operator std::string_view().contains("windows")) {
		return bnch_swt::string_literal{ "Windows" };
	} else if constexpr (osNameNew.operator std::string_view().contains("darwin")) {
		return bnch_swt::string_literal{ "MacOS" };
	} else {
		return bnch_swt::string_literal{ "" };
	}
}

constexpr auto getCurrentCompilerId() {
	constexpr bnch_swt::string_literal compilerId{ COMPILER_ID };
	constexpr auto osCompilerIdNew = bnch_swt::internal::toLower(compilerId);
	if constexpr (osCompilerIdNew.operator std::string_view().contains("gnu") || osCompilerIdNew.operator std::string_view().contains("gcc") ||
		osCompilerIdNew.operator std::string_view().contains("g++") || osCompilerIdNew.operator std::string_view().contains("apple")) {
		return bnch_swt::string_literal{ "GNUCXX" };
	} else if constexpr (osCompilerIdNew.operator std::string_view().contains("clang")) {
		return bnch_swt::string_literal{ "CLANG" };
	} else if constexpr (osCompilerIdNew.operator std::string_view().contains("msvc")) {
		return bnch_swt::string_literal{ "MSVC" };
	} else {
		return bnch_swt::string_literal{ "" };
	}
}

constexpr auto getCurrentPathImpl() {
	return getCurrentOperatingSystem() + "-" + getCurrentCompilerId();
}

constexpr bnch_swt::string_literal currentPath{ getCurrentPathImpl() };
constexpr bnch_swt::string_literal basePath{ BASE_PATH };
constexpr bnch_swt::string_literal testPath{ basePath + "/Source" };
constexpr bnch_swt::string_literal readMePath{ BASE_PATH };
constexpr bnch_swt::string_literal jsonPath{ basePath + "/Json" };
constexpr bnch_swt::string_literal jsonOutPath{ jsonPath + "/" + getCurrentPathImpl() };
constexpr bnch_swt::string_literal graphsPath{ basePath + "/Graphs/" + getCurrentPathImpl() + "/" };
constexpr bnch_swt::string_literal jsonifierLibraryName{ "jsonifier" };
constexpr bnch_swt::string_literal jsonifierCommitUrlBase{ "https://github.com/realtimechris/jsonifier/commit/" };
constexpr bnch_swt::string_literal simdjsonLibraryName{ "simdjson" };
constexpr bnch_swt::string_literal simdjsonCommitUrlBase{ "https://github.com/simdjson/simdjson/commit/" };
constexpr bnch_swt::string_literal glazeLibraryName{ "glaze" };
constexpr bnch_swt::string_literal glazeCommitUrlBase{ "https://github.com/stephenberry/glaze/commit/" };
constexpr bnch_swt::string_literal jsonifierCommitUrl{ jsonifierCommitUrlBase + JSONIFIER_COMMIT };
constexpr bnch_swt::string_literal simdjsonCommitUrl{ simdjsonCommitUrlBase + SIMDJSON_COMMIT };
constexpr bnch_swt::string_literal glazeCommitUrl{ glazeCommitUrlBase + GLAZE_COMMIT };

std::string getCPUInfo() {
	char brand[49]{};
	int32_t regs[12]{};
	uint64_t length{};
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

struct test_struct {
	std::string testString{};
	uint64_t testUint{};
	int64_t testInt{};
	double testDouble{};
	bool testBool{};
};

struct partial_test_struct {
	std::string testString{};
	bool testBool{};
};

struct abc_test_struct {
	bool testBool{};
	double testDouble{};
	int64_t testInt{};
	uint64_t testUint{};
	std::string testString{};
};

template<typename value_type> struct test {
	std::vector<value_type> a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v, w, x, y, z;
};

template<typename value_type> struct partial_test {
	std::vector<value_type> m, s;
};

struct test_generator {
	static constexpr std::string_view charSet{ "!#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[]^_`abcdefghijklmnopqrstuvwxyz{|}~!#$%&'()*+,-./"
											   "0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[]^_`abcdefghijklmnopqrstuvwxyz{|}~!#$%&'()*+,-./"
											   "0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[]^_`abcdefghijklmnopqrstuvwxyz{|}~!#$%&'()*+,-./"
											   "0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[]^_`abcdefghijklmnopqrstuvwxyz{|}~\"\\\b\f\n\r\t" };
	inline static std::uniform_real_distribution<double> disDouble{ log(std::numeric_limits<double>::min()), log(std::numeric_limits<double>::max()) };
	inline static std::uniform_int_distribution<int64_t> disInt{ std::numeric_limits<int64_t>::min(), std::numeric_limits<int64_t>::max() };
	inline static std::uniform_int_distribution<uint64_t> disUint{ std::numeric_limits<uint64_t>::min(), std::numeric_limits<uint64_t>::max() };
	inline static std::uniform_int_distribution<uint64_t> disCharSet{ 0ull, charSet.size() - 1 };
	inline static std::uniform_int_distribution<uint64_t> disString{ 16ull, 64ull };
	inline static std::uniform_int_distribution<uint64_t> disUnicodeEmoji{ 0ull, std::size(unicode_emoji::unicodeEmoji) - 1 };
	inline static std::uniform_int_distribution<uint64_t> disBool{ 0, 100 };
	inline static std::random_device randomEngine{};
	inline static std::mt19937_64 gen{ randomEngine() };

	template<jsonifier::concepts::integer_t value_type01, jsonifier::concepts::integer_t value_type02>
	static value_type01 randomizeNumberUniform(value_type01 start, value_type02 end) {
		std::uniform_int_distribution<value_type01> dis{ start, static_cast<value_type01>(end) };
		return dis(gen);
	}

	template<jsonifier::concepts::string_t value_type> static value_type generateValue() {
		auto length{ disString(gen) };

		std::string result{};
		for (uint64_t x = 0; x < length; ++x) {
			result += charSet[disCharSet(gen)];
		}

		return result;
	}

	template<jsonifier::concepts::float_t value_type> static value_type generateValue() {
		double logValue = disDouble(gen);
		bool negative{ generateValue<bool>() };
		return negative ? -std::exp(logValue) : std::exp(logValue);
	}

	template<jsonifier::concepts::bool_t value_type> static value_type generateValue() {
		return static_cast<bool>(disBool(gen) >= 50);
	}

	template<jsonifier::concepts::uns64_t value_type> static value_type generateValue() {
		size_t length{ randomizeNumberUniform(1ull, 20ull) };

		uint64_t min_val = (length == 1) ? 0 : static_cast<uint64_t>(std::pow(10, length - 1));
		uint64_t max_val = static_cast<uint64_t>(std::pow(10, length)) - 1;
		if (min_val > max_val) {
			std::swap(min_val, max_val);
		}
		std::uniform_int_distribution<uint64_t> dis(min_val, max_val);
		return dis(gen);
	}

	template<jsonifier::concepts::sig64_t value_type> static value_type generateValue() {
		size_t length{ randomizeNumberUniform(1ull, 19ull) };

		int64_t min_val = (length == 1) ? 0 : static_cast<int64_t>(std::pow(10, length - 1));
		int64_t max_val = static_cast<int64_t>(std::pow(10, length)) - 1;
		if (min_val > max_val) {
			std::swap(min_val, max_val);
		}
		std::uniform_int_distribution<int64_t> dis(min_val, max_val);
		auto returnValue{ dis(gen) };
		return generateValue<bool>() ? returnValue : -returnValue;
	}

	static test_struct generateTestStruct() {
		test_struct returnValues{};
		returnValues.testBool = generateValue<bool>();
		returnValues.testDouble = generateValue<double>();
		returnValues.testInt	= generateValue<int64_t>();
		returnValues.testUint	= generateValue<uint64_t>();
		returnValues.testString = generateValue<std::string>();
		return returnValues;
	}

	template<typename value_type> static std::vector<std::vector<value_type>> generateValues(size_t vecCount, size_t valueCount) {
		std::vector<std::vector<value_type>> returnValues{};
		returnValues.resize(vecCount);
		for (size_t x = 0; x < vecCount; ++x) {
			for (size_t y = 0; y < valueCount; ++y) {
				returnValues[x].emplace_back(generateValue<value_type>());
			}
		}
		return returnValues;
	}

	static test<test_struct> generateTest() {
		test<test_struct> returnValues{};
		auto fill = [&](auto& v) {
			const auto arraySize01 = randomizeNumberUniform(1ull, 15ull);
			v.resize(arraySize01);
			for (uint64_t x = 0; x < arraySize01; ++x) {
				v[x] = generateTestStruct();
			}
		};

		fill(returnValues.a);
		fill(returnValues.b);
		fill(returnValues.c);
		fill(returnValues.d);
		fill(returnValues.e);
		fill(returnValues.f);
		fill(returnValues.g);
		fill(returnValues.h);
		fill(returnValues.i);
		fill(returnValues.j);
		fill(returnValues.k);
		fill(returnValues.l);
		fill(returnValues.m);
		fill(returnValues.n);
		fill(returnValues.o);
		fill(returnValues.p);
		fill(returnValues.q);
		fill(returnValues.r);
		fill(returnValues.s);
		fill(returnValues.t);
		fill(returnValues.u);
		fill(returnValues.v);
		fill(returnValues.w);
		fill(returnValues.x);
		fill(returnValues.y);
		fill(returnValues.z);
		return returnValues;
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
		return bnch_swt::string_literal{ "Read" };
	} else {
		return bnch_swt::string_literal{ "Write" };
	}
}

template<result_type type> struct result {
	std::optional<double> jsonSpeedPercentageDeviation{};
	std::optional<uint64_t> byteLength{};
	std::optional<double> jsonCycles{};
	std::optional<double> jsonSpeed{};
	std::optional<double> jsonTime{};
	std::string color{};

	result() noexcept = default;

	result& operator=(result&&) noexcept	  = default;
	result(result&&) noexcept				  = default;
	result& operator=(const result&) noexcept = default;
	result(const result&) noexcept			  = default;

	result(const std::string& colorNew, const bnch_swt::performance_metrics& results) {
		byteLength.emplace(results.bytesProcessed);
		jsonTime.emplace(results.timeInNs);
		jsonSpeedPercentageDeviation.emplace(results.throughputPercentageDeviation);
		jsonSpeed.emplace(results.throughputMbPerSec);
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

	results_data() noexcept = default;

	results_data(const std::string& nameNew, const std::string& testNew, const std::string& urlNew) {
		name = nameNew;
		test = testNew;
		url	 = urlNew;
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
			finalStream << std::setprecision(6) << readResult.jsonSpeed.value() << " | " << std::setprecision(6) << readResult.jsonSpeedPercentageDeviation.value() << " | ";
			if (readResult.jsonCycles.has_value()) {
				finalStream << std::setprecision(6) << readResult.jsonCycles.value() << " | ";
			}
			finalStream << static_cast<uint64_t>(readResult.byteLength.value()) << " | " << std::setprecision(6) << readResult.jsonTime.value() << " | ";
			finalString += finalStream.str();
		}
		if (writeResult.jsonTime.has_value() && writeResult.byteLength.has_value()) {
			std::ostringstream finalStream{};
			finalStream << std::setprecision(6) << writeResult.jsonSpeed.value() << " | " << std::setprecision(6) << writeResult.jsonSpeedPercentageDeviation.value() << " | ";
			if (writeResult.jsonCycles.has_value()) {
				finalStream << std::setprecision(6) << writeResult.jsonCycles.value() << " | ";
			}
			finalStream << static_cast<uint64_t>(writeResult.byteLength.value()) << " | " << std::setprecision(6) << writeResult.jsonTime.value() << " | ";
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

static std::string urlEncode(std::string value) {
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