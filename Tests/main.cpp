#if defined(JSONIFIER_CPU_INSTRUCTIONS)
#undef JSONIFIER_CPU_INSTRUCTIONS
	#define JSONIFIER_CPU_INSTRUCTIONS (JSONIFIER_AVX2|JSONIFIER_BMI|JSONIFIIER_BMI2|JSONIFIER_POPCNT|JSONIFIER_LZCNT)
#endif
	#include "glaze/core/macros.hpp"
#include <jsonifier/Index.hpp>
#include "glaze/glaze.hpp"
#include <unordered_set>
#include <unordered_map>
#include "fmt/format.h"
#include <iostream>
#include <chrono>

struct test_struct {
	jsonifier::vector<std::string> testStrings{};
	jsonifier::vector<uint64_t> testUints{};
	jsonifier::vector<double> testDoubles{};
	jsonifier::vector<int64_t> testInts{};
	jsonifier::vector<bool> testBools{};
};

struct json_data {
	std::string theData{};
	jsonifier::vector<int32_t> arraySizes{};
};

template<typename OTy> struct TestGenerator {
	jsonifier::vector<OTy> a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v, w, x, y, z;

	std::random_device randomEngine{};
	std::mt19937 gen{ randomEngine() };
	jsonifier::vector<int32_t> arraySizes{};

	uint64_t randomizeNumber(double mean, double stdDeviation) {
		std::normal_distribution<> normalDistributionTwo{ mean, stdDeviation };
		auto theResult = normalDistributionTwo(this->randomEngine);
		if (theResult < 0) {
			theResult = -theResult;
		}
		return static_cast<uint64_t>(theResult);
	}

	jsonifier_inline static json_data generateJsonData() {
		std::string buffer{};
		TestGenerator generator{};
		jsonifier::jsonifier_core parser{};
		parser.serializeJson(generator, buffer);
		std::cout << "CURRENT BUFFER (JSONIFIER): " << buffer << std::endl;
		buffer = std::string{};
		glz::write_json(generator, buffer);
		std::cout << "CURRENT BUFFER (GLAZE): " << buffer << std::endl;
		json_data returnData{};
		returnData.arraySizes = generator.arraySizes;
		returnData.theData	  = buffer;
		return returnData;
	}

	std::string generateString() {
		auto length{ randomizeNumber(22.0f, 2.0f) * 2 };
		static constexpr char charset[]	 = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789\\\"\r\b\t\n\f";
		static constexpr int charsetSize = sizeof(charset) - 1;
		std::mt19937 generator(std::random_device{}());
		std::uniform_int_distribution<int> distribution(0, charsetSize - 1);
		std::string result;

		for (int i = 0; i < length; ++i) {
			char randomChar = charset[distribution(generator)];
			result += randomChar;
		}
		return result;
	}

	double generateDouble() {
		return static_cast<double>(randomizeNumber(1000000.0f, 10000.0f));
	};

	bool generateBool() {
		return static_cast<bool>(randomizeNumber(50.0f, 25.0f) >= 50.0f);
	};

	uint64_t generateUint() {
		return static_cast<uint64_t>(randomizeNumber(1000000.0f, 10000.0f));
	};

	int64_t generateInt() {
		return static_cast<int64_t>(randomizeNumber(1000000.0f, 10000.0f));
	};

	TestGenerator() {
		auto fill = [&](auto& v) {
			v.resize(5);
			for (uint64_t x = 0; x < 5; ++x) {
				if jsonifier_constexpr (std::same_as<OTy, test_struct>) {
					auto arraySize01 = randomizeNumber(15, 3);
					arraySizes.emplace_back(arraySize01);
					for (uint64_t y = 0; y < arraySize01; ++y) {
						v[x].testStrings.emplace_back(generateString());
						v[x].testBools.emplace_back(generateBool());
						v[x].testUints.emplace_back(generateUint());
						v[x].testInts.emplace_back(generateInt());
						v[x].testDoubles.emplace_back(generateDouble());
					}
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

template<typename OTy> struct Test {
	jsonifier::vector<OTy> a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v, w, x, y, z;
};

GLZ_META(test_struct, testBools, testInts, testUints, testDoubles, testStrings);
GLZ_META(Test<test_struct>, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v, w, x, y, z);
GLZ_META(TestGenerator<test_struct>, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v, w, x, y, z);

template<> struct jsonifier::core<test_struct> {
	using OTy = test_struct;
	jsonifier_constexpr static auto parseValue =
		createObject("testBools", &OTy::testBools, "testInts", &OTy::testInts, "testUints", &OTy::testUints, "testDoubles", &OTy::testDoubles, "testStrings", &OTy::testStrings);
};

template<> struct jsonifier::core<Test<test_struct>> {
	using OTy								   = Test<test_struct>;
	jsonifier_constexpr static auto parseValue = createObject("a", &OTy::a, "b", &OTy::b, "c", &OTy::c, "d", &OTy::d, "e", &OTy::e, "f", &OTy::f, "g", &OTy::g, "h", &OTy::h, "i",
		&OTy::i, "j", &OTy::j, "k", &OTy::k, "l", &OTy::l, "m", &OTy::m, "n", &OTy::n, "o", &OTy::o, "p", &OTy::p, "q", &OTy::q, "r", &OTy::r, "s", &OTy::s, "t", &OTy::t, "u",
		&OTy::u, "v", &OTy::v, "w", &OTy::w, "x", &OTy::x, "y", &OTy::y, "z", &OTy::z);
};

template<typename OTy> struct AbcTest {
	jsonifier::vector<OTy> z, y, x, w, v, u, t, s, r, q, p, o, n, m, l, k, j, i, h, g, f, e, d, c, b, a;
};

GLZ_META(AbcTest<test_struct>, z, y, x, w, v, u, t, s, r, q, p, o, n, m, l, k, j, i, h, g, f, e, d, c, b, a);

template<> struct jsonifier::core<TestGenerator<test_struct>> {
	using OTy								   = TestGenerator<test_struct>;
	jsonifier_constexpr static auto parseValue = createObject("a", &OTy::a, "b", &OTy::b, "c", &OTy::c, "d", &OTy::d, "e", &OTy::e, "f", &OTy::f, "g", &OTy::g, "h", &OTy::h, "i",
		&OTy::i, "j", &OTy::j, "k", &OTy::k, "l", &OTy::l, "m", &OTy::m, "n", &OTy::n, "o", &OTy::o, "p", &OTy::p, "q", &OTy::q, "r", &OTy::r, "s", &OTy::s, "t", &OTy::t, "u",
		&OTy::u, "v", &OTy::v, "w", &OTy::w, "x", &OTy::x, "y", &OTy::y, "z", &OTy::z);
};

template<> struct jsonifier::core<AbcTest<test_struct>> {
	using OTy								   = AbcTest<test_struct>;
	jsonifier_constexpr static auto parseValue = createObject("z", &OTy::z, "y", &OTy::y, "x", &OTy::x, "w", &OTy::w, "v", &OTy::v, "u", &OTy::u, "t", &OTy::t, "s", &OTy::s, "r",
		&OTy::r, "q", &OTy::q, "p", &OTy::p, "o", &OTy::o, "n", &OTy::n, "m", &OTy::m, "l", &OTy::l, "k", &OTy::k, "j", &OTy::j, "i", &OTy::i, "h", &OTy::h, "g", &OTy::g, "f",
		&OTy::f, "e", &OTy::e, "d", &OTy::d, "c", &OTy::c, "b", &OTy::b, "a", &OTy::a);
};

#if defined(NDEBUG)
jsonifier_constexpr static uint64_t iterations = 100;
#else
jsonifier_constexpr static uint64_t iterations = 1;
#endif

struct results {
	std::string name{};
	std::string test{};
	std::string url{};
	uint64_t iterations{};

	std::optional<uint64_t> json_byte_length{};
	std::optional<double> json_read{};
	std::optional<double> json_write{};

	void print() {
		std::cout << std::string{ "| " } + name + " " + test + ": " + url + "\n" +
				"| ------------------------------------------------------------ "
				"|\n";

		if (json_byte_length) {
			auto MBs = *json_byte_length / (*json_read) * 1000.0f;
			std::cout << "Length: " << *json_byte_length << std::endl;
			std::cout << "Read: " << *json_read << "ns, " << MBs << " MB/s\n";
			if (json_write) {
				MBs = (*json_byte_length) / (*json_write) * 1000.0f;
				std::cout << "Write: " << *json_write << "ns, " << MBs << " MB/s\n";
			}
		}

		std::cout << "\n---" << std::endl;
	}

	std::string json_stats() {
		std::string write{};
		std::string read{};
		std::string finalstring{};
		bool wasThereOneBeforeThis{ false };
		if (json_read) {
			write				  = json_write ? fmt::format("{}", static_cast<uint64_t>(*json_byte_length / (*json_write) * 1000.0f)) : "N/A";
			read				  = json_read ? fmt::format("{}", static_cast<uint64_t>(*json_byte_length / (*json_read) * 1000.0f)) : "N/A";
			finalstring			  = fmt::format("| [**{}**]({}) | **{}** | **{}** | **{}** |", name, url, "Normal", write, read);
			wasThereOneBeforeThis = true;
		}
		return finalstring;
	}
};

class FileLoader {
  public:
	FileLoader(const char* filePathNew) {
		filePath	   = filePathNew;
		auto theStream = std::ofstream{ filePath, std::ios::binary | std::ios::out | std::ios::in };
		std::stringstream inputStream{};
		inputStream << theStream.rdbuf();
		fileContents = inputStream.str();
		theStream.close();
	}

	void saveFile(std::string fileToSave) {
		auto theStream = std::ofstream{ filePath, std::ios::binary | std::ios::out | std::ios::in | std::ios::trunc };
		theStream << "";
		theStream.write(fileToSave.data(), fileToSave.size());
		theStream.close();
	}

	operator std::string() {
		return fileContents;
	}

	~FileLoader() {
	}

  protected:
	std::string fileContents{};
	std::string filePath{};
};

template<typename Function> double benchmark(Function function, int64_t iterationCount) {
	double currentLowestTime{ static_cast<double>(std::numeric_limits<int64_t>::max()) };
	for (int64_t x = 0; x < iterationCount; ++x) {
		auto startTime = std::chrono::duration_cast<std::chrono::duration<double, std::nano>>(std::chrono::high_resolution_clock::now().time_since_epoch());
		function();
		auto endTime = std::chrono::duration_cast<std::chrono::duration<double, std::nano>>(std::chrono::high_resolution_clock::now().time_since_epoch());
		auto newTime = endTime - startTime;
		if (static_cast<double>(newTime.count()) < currentLowestTime) {
			currentLowestTime = static_cast<double>(newTime.count());
		}
	}
	return currentLowestTime;
}

auto jsonifier_single_test(const std::string bufferNew, bool doWePrint = true) {
	std::string buffer{ bufferNew };

	results r{ "jsonifier", "Single Test", "https://github.com/realtimechris/jsonifier", 1 };
	Test<test_struct> uint64Test{};
	jsonifier::jsonifier_core parser{};
	auto result = benchmark(
		[&]() {
			parser.parseJson<false, true>(uint64Test, buffer);
		},
		1);
	for (auto& value: parser.getErrors()) {
		std::cout << "Jsonifier Error: " << value << std::endl;
	}
	r.json_read = result;
	r.json_byte_length = buffer.size();
	buffer.clear();

	result = benchmark(
		[&]() {
			parser.serializeJson(uint64Test, buffer);
		},
		1);
	r.json_write = result;
	buffer.clear();
	if (doWePrint) {
		r.print();
	}

	return r;
}

auto jsonifier_test(const std::string bufferNew, bool doWePrint = true) {
	std::string buffer{ bufferNew };

	results r{ "jsonifier", "Multi Test", "https://github.com/realtimechris/jsonifier", iterations };
	Test<test_struct> uint64Test{};
	jsonifier::jsonifier_core parser{};

	auto result = benchmark(
		[&]() {
			parser.parseJson<false, true>(uint64Test, buffer);
		},
		iterations);
	for (auto& value: parser.getErrors()) {
		std::cout << "Jsonifier Error: " << value << std::endl;
	}

	r.json_read		   = result;
	r.json_byte_length = buffer.size();
	buffer.clear();

	result = benchmark(
		[&]() {
			parser.serializeJson(uint64Test, buffer);
		},
		iterations);

	r.json_write = result;
	buffer.clear();
	if (doWePrint) {
		r.print();
	}

	return r;
}

auto jsonifier_abc_test(const std::string bufferNew, bool doWePrint = true) {
	std::string buffer{ bufferNew };

	results r{ "jsonifier", "Abc Test", "https://github.com/realtimechris/jsonifier", iterations };
	AbcTest<test_struct> uint64Test{};
	jsonifier::jsonifier_core parser{};

	auto result = benchmark(
		[&]() {
			parser.parseJson<false, true>(uint64Test, buffer);
		},
		iterations);
	for (auto& value: parser.getErrors()) {
		std::cout << "Jsonifier Error: " << value << std::endl;
	}

	r.json_read = result;

	r.json_byte_length = buffer.size();
	buffer.clear();

	result = benchmark(
		[&]() {
			parser.serializeJson(uint64Test, buffer);
		},
		iterations);
	r.json_write = result;
	buffer.clear();
	if (doWePrint) {
		r.print();
	}

	return r;
}

auto glaze_single_test(const std::string bufferNew, bool doWePrint = true) {
	std::string buffer{ bufferNew };

	results r{ "glaze", "Single Test", "https://github.com/stephenberry/glaze", 1 };
	Test<test_struct> uint64Test{};

	auto result = benchmark(
		[&]() {
			if (auto error = glz::read_json(uint64Test, buffer); error) {
				std::cout << "glaze Error: " << glz::format_error(error, buffer) << std::endl;
			}
		},
		1);

	r.json_read = result;
	r.json_byte_length = buffer.size();
	buffer.clear();

	result = benchmark(
		[&]() {
			glz::write_json(uint64Test, buffer);
		},
		1);
	r.json_write = result;
	buffer.clear();
	if (doWePrint) {
		r.print();
	}

	return r;
}

auto glaze_test(const std::string bufferNew, bool doWePrint = true) {
	std::string buffer{ bufferNew };

	results r{ "glaze", "Multi Test", "https://github.com/stephenberry/glaze", iterations };
	Test<test_struct> uint64Test{};

	auto result = benchmark(
		[&]() {
			if (auto error = glz::read_json(uint64Test, buffer); error) {
				std::cout << "glaze Error: " << glz::format_error(error, buffer) << std::endl;
			}
		},
		iterations);

	r.json_read = result;

	r.json_byte_length = buffer.size();
	buffer.clear();

	result = benchmark(
		[&]() {
			glz::write_json(uint64Test, buffer);
		},
		iterations);
	r.json_write = result;
	buffer.clear();
	if (doWePrint) {
		r.print();
	}

	return r;
}

auto glaze_abc_test(const std::string bufferNew, bool doWePrint = true) {
	std::string buffer{ bufferNew };

	results r{ "glaze", "Abc Test", "https://github.com/stephenberry/glaze", iterations };
	AbcTest<test_struct> uint64Test{};

	auto result = benchmark(
		[&]() {
			if (auto error = glz::read_json(uint64Test, buffer); error) {
				std::cout << "glaze Error: " << glz::format_error(error, buffer) << std::endl;
			}
		},
		iterations);

	r.json_read = result;

	r.json_byte_length = buffer.size();
	buffer.clear();

	result = benchmark(
		[&]() {
			glz::write_json(uint64Test, buffer);
		},
		iterations);
	r.json_write = result;
	buffer.clear();
	if (doWePrint) {
		r.print();
	}

	return r;
}

#include "simdjson.h"

using namespace simdjson;

struct on_demand {
	bool read_in_order(Test<test_struct>& obj, const padded_string& json, const jsonifier::vector<int32_t>& arraySizes);

  protected:
	ondemand::parser parser{};
};

template<typename OTy2> void simdPullArray(ondemand::array newX, jsonifier::vector<OTy2>& newVector);

template<> void simdPullArray<double>(ondemand::array newX, jsonifier::vector<double>& newVector) {
	for (ondemand::value value: newX) {
		newVector.emplace_back(value.get_double());
	}
}

template<> void simdPullArray<int64_t>(ondemand::array newX, jsonifier::vector<int64_t>& newVector) {
	for (ondemand::value value: newX) {
		newVector.emplace_back(value.get_int64());
	}
}

template<> void simdPullArray<uint64_t>(ondemand::array newX, jsonifier::vector<uint64_t>& newVector) {
	for (ondemand::value value: newX) {
		newVector.emplace_back(value.get_uint64());
	}
}

template<> void simdPullArray<bool>(ondemand::array newX, jsonifier::vector<bool>& newVector) {
	for (ondemand::value value: newX) {
		newVector.emplace_back(value.get_bool());
	}
}

template<> void simdPullArray<std::string>(ondemand::array newX, jsonifier::vector<std::string>& newVector) {
	for (ondemand::value value: newX) {
		newVector.emplace_back(static_cast<std::string>(value.get_string().value()));
	}
}

#define SIMD_Pull(x) \
	{ \
		ondemand::array newX = doc[#x].get_array().value(); \
		test_struct newStruct{}; \
		ondemand::array newArray{}; \
		ondemand::object newObject{}; \
		for (ondemand::value value: newX) { \
			newObject = value.get_object(); \
			newArray  = newObject["testInts"].get_array().value(); \
			simdPullArray(newArray, newStruct.testInts); \
			newArray = newObject["testDoubles"].get_array().value(); \
			simdPullArray(newArray, newStruct.testDoubles); \
			newArray = newObject["testStrings"].get_array().value(); \
			simdPullArray(newArray, newStruct.testStrings); \
			newArray = newObject["testUints"].get_array().value(); \
			simdPullArray(newArray, newStruct.testUints); \
			newArray = newObject["testBools"].get_array().value(); \
			simdPullArray(newArray, newStruct.testBools); \
			obj.x.emplace_back(std::move(newStruct)); \
		} \
	}

bool on_demand::read_in_order(Test<test_struct>& obj, const padded_string& json, const jsonifier::vector<int32_t>& arraySizes) {
	ondemand::document doc = parser.iterate(json).value();
	SIMD_Pull(a);
	SIMD_Pull(b);
	SIMD_Pull(c);
	SIMD_Pull(d);
	SIMD_Pull(e);
	SIMD_Pull(f);
	SIMD_Pull(g);
	SIMD_Pull(h);
	SIMD_Pull(i);
	SIMD_Pull(j);
	SIMD_Pull(k);
	SIMD_Pull(l);
	SIMD_Pull(m);
	SIMD_Pull(n);
	SIMD_Pull(o);
	SIMD_Pull(p);
	SIMD_Pull(q);
	SIMD_Pull(r);
	SIMD_Pull(s);
	SIMD_Pull(t);
	SIMD_Pull(u);
	SIMD_Pull(v);
	SIMD_Pull(w);
	SIMD_Pull(x);
	SIMD_Pull(y);
	SIMD_Pull(z);
	return false;
}

auto simdjson_single_test(const std::string& bufferNew, const jsonifier::vector<int32_t>& arraySizes, bool doWePrint = true) {
	std::string buffer{ bufferNew };
	on_demand parser{};

	results r{ "simdjson", "Single Test", "https://github.com/simdjson/simdjson", 1 };
	Test<test_struct> uint64Test{};

	r.json_byte_length = buffer.size();
	double result{};
	try {
		result = benchmark(
			[&]() {
				parser.read_in_order(uint64Test, buffer, arraySizes);
			},
			1);
	} catch (const std::exception& error) {
		std::cerr << "simdjson Error: " << error.what() << std::endl;
	}

	r.json_read = result;
	buffer.clear();
	if (doWePrint) {
		r.print();
	}

	return r;
}

auto simdjson_test(const std::string& bufferNew, const jsonifier::vector<int32_t>& arraySizes, bool doWePrint = true) {
	std::string buffer{ bufferNew };
	on_demand parser{};

	results r{ "simdjson", "Multi Test", "https://github.com/simdjson/simdjson", iterations };
	Test<test_struct> uint64Test{};

	r.json_byte_length = buffer.size();
	double result{};
	try {
		result = benchmark(
			[&]() {
				parser.read_in_order(uint64Test, buffer, arraySizes);
			},
			iterations);
	} catch (const std::exception& error) {
		std::cerr << "simdjson Error: " << error.what() << std::endl;
	}

	r.json_read = result;
	buffer.clear();
	if (doWePrint) {
		r.print();
	}

	return r;
}

struct on_demand_abc {
	bool read_out_of_order(AbcTest<test_struct>& obj, const padded_string& json, const jsonifier::vector<int32_t>& arraySizes);

  protected:
	ondemand::parser parser{};
};

bool on_demand_abc::read_out_of_order(AbcTest<test_struct>& obj, const padded_string& json, const jsonifier::vector<int32_t>& arraySizes) {
	ondemand::document doc = parser.iterate(json).value();
	SIMD_Pull(z);
	SIMD_Pull(y);
	SIMD_Pull(x);
	SIMD_Pull(w);
	SIMD_Pull(v);
	SIMD_Pull(u);
	SIMD_Pull(t);
	SIMD_Pull(s);
	SIMD_Pull(r);
	SIMD_Pull(q);
	SIMD_Pull(p);
	SIMD_Pull(o);
	SIMD_Pull(n);
	SIMD_Pull(m);
	SIMD_Pull(l);
	SIMD_Pull(k);
	SIMD_Pull(j);
	SIMD_Pull(i);
	SIMD_Pull(h);
	SIMD_Pull(g);
	SIMD_Pull(f);
	SIMD_Pull(e);
	SIMD_Pull(d);
	SIMD_Pull(c);
	SIMD_Pull(b);
	SIMD_Pull(a);
	return false;
}

auto simdjson_abc_test(const std::string& bufferNew, const jsonifier::vector<int32_t>& arraySizes, bool doWePrint = true) {
	std::string buffer{ bufferNew };
	on_demand_abc parser{};

	AbcTest<test_struct> obj{};

	results r{ "simdjson", "Abc Test", "https://github.com/simdjson/simdjson", iterations };
	double result{};

	try {
	result = benchmark(
		[&]() {
			parser.read_out_of_order(obj, buffer, arraySizes);
		},
			iterations);
	} catch (const std::exception& error) {
		std::cerr << "simdjson Error: " << error.what() << std::endl;
	}

	r.json_byte_length = buffer.size();
	r.json_read		   = result;
	if (doWePrint) {
		r.print();
	}

	return r;
}

static std::string table_header = R"(
| Library | Test | Write (MB/s) | Read (MB/s) |
| ------------------------------------------------- | ---------- | ------------ | ----------- |)";

std::string regular_test(const json_data& jsonData) {
	jsonifier::vector<results> results{};
	for (uint32_t x = 0; x < 2; ++x) {
		simdjson_test(jsonData.theData, jsonData.arraySizes, false);
	}
	for (uint32_t x = 0; x < 2; ++x) {
		glaze_test(jsonData.theData, false);
	}
	results.emplace_back(glaze_test(jsonData.theData));
	for (uint32_t x = 0; x < 2; ++x) {
		jsonifier_test(jsonData.theData, false);
	}
	results.emplace_back(jsonifier_test(jsonData.theData));
	for (uint32_t x = 0; x < 2; ++x) {
		simdjson_test(jsonData.theData, jsonData.arraySizes, false);
	}
	results.emplace_back(simdjson_test(jsonData.theData, jsonData.arraySizes));

	std::string table{};
	const auto n = results.size();
	table += table_header + '\n';
	for (uint64_t i = 0; i < n; ++i) {
		table += results[i].json_stats();
		if (i != n - 1) {
			table += "\n";
		}
	}
	return table;
}

std::string abc_test(const json_data& jsonData) {
	jsonifier::vector<results> results{};
	for (uint32_t x = 0; x < 2; ++x) {
		glaze_abc_test(jsonData.theData, false);
	}
	results.emplace_back(glaze_abc_test(jsonData.theData));
	for (uint32_t x = 0; x < 2; ++x) {
		jsonifier_abc_test(jsonData.theData, false);
	}
	results.emplace_back(jsonifier_abc_test(jsonData.theData));
	for (uint32_t x = 0; x < 2; ++x) {
		simdjson_abc_test(jsonData.theData, jsonData.arraySizes, false);
	}
	results.emplace_back(simdjson_abc_test(jsonData.theData, jsonData.arraySizes));

	std::string table{};
	const auto n = results.size();
	table += table_header + '\n';
	for (uint64_t i = 0; i < n; ++i) {
		table += results[i].json_stats();
		if (i != n - 1) {
			table += "\n";
		}
	}
	return table;
}

std::string single_test(const json_data& jsonData) {
	jsonifier::vector<results> results{};
	for (uint32_t x = 0; x < 2; ++x) {
		simdjson_single_test(jsonData.theData, jsonData.arraySizes, false);
	}
	for (uint32_t x = 0; x < 2; ++x) {
		glaze_single_test(jsonData.theData, false);
	}
	results.emplace_back(glaze_single_test(jsonData.theData));
	for (uint32_t x = 0; x < 2; ++x) {
		jsonifier_single_test(jsonData.theData, false);
	}
	results.emplace_back(jsonifier_single_test(jsonData.theData));
	for (uint32_t x = 0; x < 2; ++x) {
		simdjson_single_test(jsonData.theData, jsonData.arraySizes, false);
	}
	results.emplace_back(simdjson_single_test(jsonData.theData, jsonData.arraySizes));

	std::string table{};
	const auto n = results.size();
	table += table_header + '\n';
	for (uint64_t i = 0; i < n; ++i) {
		table += results[i].json_stats();
		if (i != n - 1) {
			table += "\n";
		}
	}
	return table;
};

struct Calls {
	std::string name;
	uint64_t micros;
	std::vector<Calls> calls;
};

struct GatewayTrace {
	std::string traceId;
	uint64_t micros;
	std::vector<Calls> calls;
};

struct Guild {
	uint64_t id;
	bool unavailable;
};

struct User {
	std::string avatar;
	bool bot;
	std::string discriminator;
};

struct Application {
	uint64_t flags;
	uint64_t id;
};
struct Auth {
	int32_t value{};
};

struct ReadyData {
	jsonifier::vector<std::string> trace;
	Application application{};
	Auth auth;
	std::vector<std::string> geoOrderedRtcRegions;
	std::vector<std::string> guildJoinRequests;
	std::vector<Guild> guilds;
	std::vector<std::string> presences;
	std::vector<std::string> privateChannels;
	std::vector<std::string> relationships;
	std::string resumeGatewayUrl;
	std::string sessionId;
	std::string sessionType;
	std::string shard;
	User user;
	int32_t v;
};

struct ReadyMessage {
	int32_t op;
	int32_t s;
	std::string t;
	ReadyData d;
};

template<> struct jsonifier::core<Auth> {
	using OTy								   = Auth;
	jsonifier_constexpr static auto parseValue = createObject("value", &OTy::value);
};

template<> struct jsonifier::core<Application> {
	using OTy								   = Application;
	jsonifier_constexpr static auto parseValue = createObject("flags", &OTy::flags, "id", &OTy::id);
};

template<> struct jsonifier::core<Calls> {
	using OTy								   = Calls;
	jsonifier_constexpr static auto parseValue = createObject("name", &OTy::name, "micros", &OTy::micros, "calls", &OTy::calls);
};

template<> struct jsonifier::core<GatewayTrace> {
	using OTy								   = GatewayTrace;
	jsonifier_constexpr static auto parseValue = createObject("trace_id", &OTy::traceId, "micros", &OTy::micros, "calls", &OTy::calls);
};

template<> struct jsonifier::core<Guild> {
	using OTy								   = Guild;
	jsonifier_constexpr static auto parseValue = createObject("id", &OTy::id, "unavailable", &OTy::unavailable);
};

template<> struct jsonifier::core<User> {
	using OTy								   = User;
	jsonifier_constexpr static auto parseValue = createObject("avatar", &OTy::avatar, "bot", &OTy::bot, "discriminator", &OTy::discriminator);
	// Add other attributes as needed.
};

template<> struct jsonifier::core<ReadyData> {
	using OTy = ReadyData;
	jsonifier_constexpr static auto parseValue =
		createObject("_trace", &OTy::trace, "application", &OTy::application, "auth", &OTy::auth, "geo_ordered_rtc_regions", &OTy::geoOrderedRtcRegions, "guild_join_requests",
			&OTy::guildJoinRequests, "guilds", &OTy::guilds, "presences", &OTy::presences, "private_channels", &OTy::privateChannels, "relationships", &OTy::relationships,
			"resume_gateway_url", &OTy::resumeGatewayUrl, "session_id", &OTy::sessionId, "session_type", &OTy::sessionType, "shard", &OTy::shard, "user", &OTy::user, "v", &OTy::v);
};

template<> struct jsonifier::core<ReadyMessage> {
	using OTy								   = ReadyMessage;
	jsonifier_constexpr static auto parseValue = createObject("op", &OTy::op, "s", &OTy::s, "t", &OTy::t, "d", &OTy::d);
};


void printBits(__m256i avxVector) {
	// Use a pointer to access the data in the AVX vector
	const int* data = reinterpret_cast<const int*>(&avxVector);

	for (int i = 7; i >= 0; i--) {
		for (int j = 31; j >= 0; j--) {
			int bit = (data[i] >> j) & 1;
			std::cout << bit;
		}
	}
	std::cout << std::endl;
}

int32_t main() {
	try {
		__m256i lowerVal{ _mm256_insert_epi64(__m256i{}, 0x01, 0x00) };
		//lowerVal.m256i_u64[3] = 0x01ull;
		//jsonifier_internal::printBits(lowerVal, "FINAL BITS: ");
		//lowerVal = jsonifier_internal::simd_base::shl<1>(lowerVal);
		printBits(lowerVal);
		auto theValue{ static_cast<int64_t>(std::numeric_limits<uint64_t>::max()) };
		alignas(BytesPerStep) uint8_t values[sizeof(uint64_t)]{};
		std::memcpy(values, &theValue, sizeof(uint64_t));
		for (string_parsing_type x = 0; x < sizeof(uint64_t); ++x) {
			for (string_parsing_type y = 0; y < 8; ++y) {
				std::cout << std::bitset<1>{ static_cast<uint64_t>(*(values + x)) >> y };
			}
		}
		for (uint64_t x = 0; x < 64; ++x) {
			std::cout << (theValue & (1ull << x)) << std::endl;
		}
		jsonifier::string newString01{ "{\"d\":{\"_trace\":[\"[\\\"gateway-prd-us-east1-d-26rq\\\",{\\\"micros\\\":122986,\\\"calls\\\":[\\\"id_created\\\",{\\\"micros\\\":861,"
									   "\\\"calls\\\":[]},\\\"session_"
									   "lookup_time\\\",{\\\"micros\\\":4526,\\\"calls\\\":[]},\\\"session_lookup_finished\\\",{\\\"micros\\\":17,\\\"calls\\\":[]},\\\"discord-"
									   "sessions-prd-2-51\\\",{"
									   "\\\"micros\\\":117233,\\\"calls\\\":[\\\"start_session\\\",{\\\"micros\\\":66751,\\\"calls\\\":[\\\"discord-api-79bdc49487-hv95g\\\",{"
									   "\\\"micros\\\":59968,"
									   "\\\"calls\\\":[\\\"get_user\\\",{\\\"micros\\\":9341},\\\"get_guilds\\\",{\\\"micros\\\":7529},\\\"send_scheduled_deletion_message\\\",{"
									   "\\\"micros\\\":11},\\\"guild_"
									   "join_requests\\\",{\\\"micros\\\":1},\\\"authorized_ip_coro\\\",{\\\"micros\\\":12}]}]},\\\"starting_guild_connect\\\",{\\\"micros\\\":"
									   "419,\\\"calls\\\":[]},"
									   "\\\"presence_started\\\",{\\\"micros\\\":272,\\\"calls\\\":[]},\\\"guilds_started\\\",{\\\"micros\\\":157,\\\"calls\\\":[]},\\\"guilds_"
									   "connect\\\",{\\\"micros\\\":29,"
									   "\\\"calls\\\":[]},\\\"presence_connect\\\",{\\\"micros\\\":49563,\\\"calls\\\":[]},\\\"connect_finished\\\",{\\\"micros\\\":49598,"
									   "\\\"calls\\\":[]},\\\"build_"
									   "ready\\\",{\\\"micros\\\":33,\\\"calls\\\":[]},\\\"clean_ready\\\",{\\\"micros\\\":1,\\\"calls\\\":[]},\\\"optimize_ready\\\",{"
									   "\\\"micros\\\":0,\\\"calls\\\":[]},"
									   "\\\"split_ready\\\",{\\\"micros\\\":0,\\\"calls\\\":[]}]}]}]\"],\"application\":{\"flags\":27828224,\"id\":1142733646600614004},"
									   "\"auth\":{},\"geo_ordered_rtc_"
									   "regions\":[\"newark\",\"us-east\",\"us-central\",\"atlanta\",\"us-south\"],\"guild_join_requests\":[],\"guilds\":[{\"id\":"
									   "318872312596267018,\"unavailable\":true},{"
									   "\"id\":931640556814237706,\"unavailable\":true},{\"id\":991025447875784714,\"unavailable\":true},{\"id\":995048955215872071,"
									   "\"unavailable\":true},{\"id\":"
									   "1022405038922006538,\"unavailable\":true},{\"id\":1032783776184533022,\"unavailable\":true},{\"id\":1078501504119476282,\"unavailable\":"
									   "true},{\"id\":"
									   "1131853763506880522,\"unavailable\":true}],\"presences\":[],\"private_channels\":[],\"relationships\":[],\"resume_gateway_url\":\"wss://"
									   "gateway-us-east1-d.discord.gg\",\"session_id\":\"5b405a8282550f72114b460169cd08f6\",\"session_type\":\"normal\",\"shard\":\"01\","
									   "\"user\":{\"avatar\":"
									   "\"88bd9ce7bf889c0d36fb4afd3725900b\",\"bot\":true,\"discriminator\":\"3055\",\"email\":null,\"flags\":0,\"global_name\":null,\"id\":"
									   "1142733646600614004,\"mfa_"
									   "enabled\":false,\"username\":\"MBot-MusicHouse-2\",\"verified\":true},\"user_settings\":{},\"v\":10},\"op\":0,\"s\":1,\"t\":\"READY\"}" };
		ReadyMessage dataNew{};
		jsonifier::jsonifier_core parser{};
		parser.parseJson<false, true>(dataNew, newString01);
		for (auto& value: parser.getErrors()) {
			std::cout << "Jsonifier Error: " << value << std::endl;
		}
		parser.serializeJson(dataNew, newString01);
		json_data jsonData{ TestGenerator<test_struct>::generateJsonData() };
		FileLoader fileLoader02{ "C:/users/chris/source/repos/jsonifier/JsonData.json" };
		//jsonData.theData = fileLoader02;
		fileLoader02.saveFile(jsonData.theData);
		auto singlTestResults = single_test(jsonData);
		auto multiTestResults = regular_test(jsonData);
		auto abcTestResults	  = abc_test(jsonData);
	} catch (std::exception& e) {
		std::cout << e.what() << std::endl;
	}
	return 0;
}