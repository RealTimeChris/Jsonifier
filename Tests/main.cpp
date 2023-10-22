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

	inline static json_data generateJsonData() {
		std::string buffer{};
		TestGenerator generator{};
		glz::write_json(generator, buffer);
		json_data returnData{};
		returnData.arraySizes = generator.arraySizes;
		returnData.theData	  = buffer;
		return returnData;
	}

	std::string generateString() {
		std::string returnString{};
		for (uint32_t x = 0; x < randomizeNumber(30.0f, 15.0f); x++) {
			auto theValue = static_cast<char>((static_cast<float>(this->randomEngine()) / static_cast<float>(this->randomEngine.max()) * 93.0f) + 35.0f);
			if (theValue == '\\') {
				continue;
			} else if (theValue == '"') {
				returnString.push_back('\"');
			} else {
				returnString.push_back(theValue);
			}
		}
		return returnString;
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
			v.resize(10);
			for (uint64_t x = 0; x < 10; ++x) {
				if constexpr (std::same_as<OTy, test_struct>) {
					auto arraySize01 = randomizeNumber(150, 50);
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

GLZ_META(test_struct, testInts, testUints, testStrings, testDoubles, testBools);
GLZ_META(Test<test_struct>, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v, w, x, y, z);
GLZ_META(TestGenerator<test_struct>, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v, w, x, y, z);

template<> struct jsonifier::core<test_struct> {
	using OTy = test_struct;
	constexpr static auto parseValue =
		createObject("testBools", &OTy::testBools, "testInts", &OTy::testInts, "testUints", &OTy::testUints, "testDoubles", &OTy::testDoubles, "testStrings", &OTy::testStrings);
};

template<> struct jsonifier::core<Test<test_struct>> {
	using OTy						 = Test<test_struct>;
	constexpr static auto parseValue = createObject("a", &OTy::a, "b", &OTy::b, "c", &OTy::c, "d", &OTy::d, "e", &OTy::e, "f", &OTy::f, "g", &OTy::g, "h", &OTy::h, "i", &OTy::i,
		"j", &OTy::j, "k", &OTy::k, "l", &OTy::l, "m", &OTy::m, "n", &OTy::n, "o", &OTy::o, "p", &OTy::p, "q", &OTy::q, "r", &OTy::r, "s", &OTy::s, "t", &OTy::t, "u", &OTy::u, "v",
		&OTy::v, "w", &OTy::w, "x", &OTy::x, "y", &OTy::y, "z", &OTy::z);
};

template<typename OTy> struct AbcTest {
	jsonifier::vector<OTy> z, y, x, w, v, u, t, s, r, q, p, o, n, m, l, k, j, i, h, g, f, e, d, c, b, a;
};

GLZ_META(AbcTest<test_struct>, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v, w, x, y, z);

template<> struct jsonifier::core<TestGenerator<test_struct>> {
	using OTy						 = TestGenerator<test_struct>;
	constexpr static auto parseValue = createObject("a", &OTy::a, "b", &OTy::b, "c", &OTy::c, "d", &OTy::d, "e", &OTy::e, "f", &OTy::f, "g", &OTy::g, "h", &OTy::h, "i", &OTy::i,
		"j", &OTy::j, "k", &OTy::k, "l", &OTy::l, "m", &OTy::m, "n", &OTy::n, "o", &OTy::o, "p", &OTy::p, "q", &OTy::q, "r", &OTy::r, "s", &OTy::s, "t", &OTy::t, "u", &OTy::u, "v",
		&OTy::v, "w", &OTy::w, "x", &OTy::x, "y", &OTy::y, "z", &OTy::z);
};

template<> struct jsonifier::core<AbcTest<test_struct>> {
	using OTy						 = AbcTest<test_struct>;
	constexpr static auto parseValue = createObject("z", &OTy::z, "y", &OTy::y, "x", &OTy::x, "w", &OTy::w, "v", &OTy::v, "u", &OTy::u, "t", &OTy::t, "s", &OTy::s, "r", &OTy::r,
		"q", &OTy::q, "p", &OTy::p, "o", &OTy::o, "n", &OTy::n, "m", &OTy::m, "l", &OTy::l, "k", &OTy::k, "j", &OTy::j, "i", &OTy::i, "h", &OTy::h, "g", &OTy::g, "f", &OTy::f, "e",
		&OTy::e, "d", &OTy::d, "c", &OTy::c, "b", &OTy::b, "a", &OTy::a);
};

#if defined(NDEBUG)
constexpr static uint64_t iterations = 1000;
#else
constexpr static uint64_t iterations = 1;
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
		auto theStream = std::ofstream{ filePath, std::ios::binary | std::ios::out | std::ios::in };
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
			parser.parseJson(uint64Test, buffer);
		},
		1);
	for (auto& value: parser.getErrors()) {
		std::cout << "Jsonifier Error: " << value << std::endl;
	}

	r.json_read = result;
	buffer.clear();

	result = benchmark(
		[&]() {
			parser.serializeJson(uint64Test, buffer);
		},
		1);

	r.json_byte_length = buffer.size();
	r.json_write	   = result;
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
			parser.parseJson(uint64Test, buffer);
		},
		iterations);
	for (auto& value: parser.getErrors()) {
		std::cout << "Jsonifier Error: " << value << std::endl;
	}

	r.json_read = result;
	buffer.clear();

	result = benchmark(
		[&]() {
			parser.serializeJson(uint64Test, buffer);
		},
		iterations);

	r.json_byte_length = buffer.size();
	r.json_write	   = result;
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
			parser.parseJson(uint64Test, buffer);
		},
		iterations);
	for (auto& value: parser.getErrors()) {
		std::cout << "Jsonifier Error: " << value << std::endl;
	}

	r.json_read = result;
	buffer.clear();

	result = benchmark(
		[&]() {
			parser.serializeJson(uint64Test, buffer);
		},
		iterations);

	r.json_byte_length = buffer.size();
	r.json_write	   = result;
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
			if (auto error = glz::read_json(uint64Test, buffer)) {
				std::cout << "glaze Error: " << error << std::endl;
			}
		},
		1);

	r.json_read = result;
	buffer.clear();

	result = benchmark(
		[&]() {
			glz::write_json(uint64Test, buffer);
		},
		1);

	r.json_byte_length = buffer.size();
	r.json_write	   = result;
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
			if (auto error = glz::read_json(uint64Test, buffer)) {
				std::cout << "glaze Error: " << error << std::endl;
			}
		},
		iterations);

	r.json_read = result;
	buffer.clear();

	result = benchmark(
		[&]() {
			glz::write_json(uint64Test, buffer);
		},
		iterations);

	r.json_byte_length = buffer.size();
	r.json_write	   = result;
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
			if (auto error = glz::read_json(uint64Test, buffer)) {
				std::cout << "glaze Error: " << error << std::endl;
			}
		},
		iterations);

	r.json_read = result;
	buffer.clear();

	result = benchmark(
		[&]() {
			glz::write_json(uint64Test, buffer);
		},
		iterations);

	r.json_byte_length = buffer.size();
	r.json_write	   = result;
	buffer.clear();
	if (doWePrint) {
		r.print();
	}

	return r;
}

#include "simdjson.h"

using namespace simdjson;

struct on_demand {
	bool read_in_order(Test<test_struct>& obj, const padded_string& json, jsonifier::vector<int32_t>& arraySizes);

  protected:
	ondemand::parser parser{};
};

template<typename OTy2> void simdPullArray(ondemand::array newX, jsonifier::vector<OTy2>& newVector);

template<> void simdPullArray<double>(ondemand::array newX, jsonifier::vector<double>& newVector) {
	if (newVector.size() < 1000) {
		newVector.resize(1000);
	}
	uint64_t currentIndex{};
	for (ondemand::value value: newX) {
		newVector[currentIndex++] = value.get_double();
	}
}

template<> void simdPullArray<int64_t>(ondemand::array newX, jsonifier::vector<int64_t>& newVector) {
	if (newVector.size() < 1000) {
		newVector.resize(1000);
	}
	uint64_t currentIndex{};
	for (ondemand::value value: newX) {
		newVector[currentIndex++] = value.get_int64();
	}
}

template<> void simdPullArray<uint64_t>(ondemand::array newX, jsonifier::vector<uint64_t>& newVector) {
	if (newVector.size() < 1000) {
		newVector.resize(1000);
	}
	uint64_t currentIndex{};
	for (ondemand::value value: newX) {
		newVector[currentIndex++] = value.get_uint64();
	}
}

template<> void simdPullArray<bool>(ondemand::array newX, jsonifier::vector<bool>& newVector) {
	if (newVector.size() < 1000) {
		newVector.resize(1000);
	}
	uint64_t currentIndex{};
	for (ondemand::value value: newX) {
		newVector[currentIndex++] = value.get_bool();
	}
}

template<> void simdPullArray<std::string>(ondemand::array newX, jsonifier::vector<std::string>& newVector) {
	if (newVector.size() < 1000) {
		newVector.resize(1000);
	}
	uint64_t currentIndex{};
	for (ondemand::value value: newX) {
		newVector[currentIndex++] = static_cast<std::string>(value.get_string().value());
	}
}

#define SIMD_Pull(x, y) \
	{ \
		if (obj.x.size() < y) { \
			obj.x.resize(y); \
		} \
		ondemand::array newX = doc[#x].get_array().value(); \
		test_struct newStruct{}; \
		int32_t currentIndex{}; \
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
			obj.x[currentIndex] = std::move(newStruct); \
		} \
	}

bool on_demand::read_in_order(Test<test_struct>& obj, const padded_string& json, jsonifier::vector<int32_t>& arraySizes) {
	ondemand::document doc = parser.iterate(json).value();
	SIMD_Pull(a, arraySizes[0]);
	SIMD_Pull(b, arraySizes[1]);
	SIMD_Pull(c, arraySizes[2]);
	SIMD_Pull(d, arraySizes[3]);
	SIMD_Pull(e, arraySizes[4]);
	SIMD_Pull(f, arraySizes[5]);
	SIMD_Pull(g, arraySizes[6]);
	SIMD_Pull(h, arraySizes[7]);
	SIMD_Pull(i, arraySizes[8]);
	SIMD_Pull(j, arraySizes[9]);
	SIMD_Pull(k, arraySizes[10]);
	SIMD_Pull(l, arraySizes[11]);
	SIMD_Pull(m, arraySizes[12]);
	SIMD_Pull(n, arraySizes[13]);
	SIMD_Pull(o, arraySizes[14]);
	SIMD_Pull(p, arraySizes[15]);
	SIMD_Pull(q, arraySizes[16]);
	SIMD_Pull(r, arraySizes[17]);
	SIMD_Pull(s, arraySizes[18]);
	SIMD_Pull(t, arraySizes[19]);
	SIMD_Pull(u, arraySizes[20]);
	SIMD_Pull(v, arraySizes[21]);
	SIMD_Pull(w, arraySizes[22]);
	SIMD_Pull(x, arraySizes[23]);
	SIMD_Pull(y, arraySizes[24]);
	SIMD_Pull(z, arraySizes[25]);
	return false;
}

auto simdjson_single_test(std::string bufferNew, jsonifier::vector<int32_t>& arraySizes, bool doWePrint = true) {
	std::string buffer{ bufferNew };
	on_demand parser{};

	results r{ "simdjson", "Single Test", "https://github.com/simdjson/simdjson", 1 };
	Test<test_struct> uint64Test{};

	r.json_byte_length = buffer.size();

	auto result = benchmark(
		[&]() {
			try {
				parser.read_in_order(uint64Test, buffer, arraySizes);
			} catch (const std::runtime_error& error) {
				std::cerr << "simdjson Error: " << error.what() << std::endl;
			}
		},
		1);

	r.json_read = result;
	buffer.clear();
	if (doWePrint) {
		r.print();
	}

	return r;
}

auto simdjson_test(std::string bufferNew, jsonifier::vector<int32_t>& arraySizes, bool doWePrint = true) {
	std::string buffer{ bufferNew };
	on_demand parser{};

	results r{ "simdjson", "Multi Test", "https://github.com/simdjson/simdjson", iterations };
	Test<test_struct> uint64Test{};

	r.json_byte_length = buffer.size();

	auto result = benchmark(
		[&]() {
			try {
				parser.read_in_order(uint64Test, buffer, arraySizes);
			} catch (const std::runtime_error& error) {
				std::cerr << "simdjson Error: " << error.what() << std::endl;
			}
		},
		iterations);

	r.json_read = result;
	buffer.clear();
	if (doWePrint) {
		r.print();
	}

	return r;
}

struct on_demand_abc {
	bool read_out_of_order(AbcTest<test_struct>& obj, const padded_string& json, jsonifier::vector<int32_t>& arraySizes);

  protected:
	ondemand::parser parser{};
};

bool on_demand_abc::read_out_of_order(AbcTest<test_struct>& obj, const padded_string& json, jsonifier::vector<int32_t>& arraySizes) {
	ondemand::document doc = parser.iterate(json).value();
	SIMD_Pull(z, arraySizes[25]);
	SIMD_Pull(y, arraySizes[24]);
	SIMD_Pull(x, arraySizes[23]);
	SIMD_Pull(w, arraySizes[22]);
	SIMD_Pull(v, arraySizes[21]);
	SIMD_Pull(u, arraySizes[20]);
	SIMD_Pull(t, arraySizes[19]);
	SIMD_Pull(s, arraySizes[18]);
	SIMD_Pull(r, arraySizes[17]);
	SIMD_Pull(q, arraySizes[16]);
	SIMD_Pull(p, arraySizes[15]);
	SIMD_Pull(o, arraySizes[14]);
	SIMD_Pull(n, arraySizes[13]);
	SIMD_Pull(m, arraySizes[12]);
	SIMD_Pull(l, arraySizes[11]);
	SIMD_Pull(k, arraySizes[10]);
	SIMD_Pull(j, arraySizes[9]);
	SIMD_Pull(i, arraySizes[8]);
	SIMD_Pull(h, arraySizes[7]);
	SIMD_Pull(g, arraySizes[6]);
	SIMD_Pull(f, arraySizes[5]);
	SIMD_Pull(e, arraySizes[4]);
	SIMD_Pull(d, arraySizes[3]);
	SIMD_Pull(c, arraySizes[2]);
	SIMD_Pull(b, arraySizes[1]);
	SIMD_Pull(a, arraySizes[0]);
	return false;
}

auto simdjson_abc_test(const std::string bufferNew, jsonifier::vector<int32_t>& arraySizes, bool doWePrint = true) {
	std::string buffer{ bufferNew };
	on_demand_abc parser{};

	AbcTest<test_struct> obj{};

	results r{ "simdjson", "Abc Test", "https://github.com/simdjson/simdjson", iterations };

	auto result = benchmark(
		[&]() {
			try {
				parser.read_out_of_order(obj, buffer, arraySizes);
			} catch (const std::runtime_error& error) {
				std::cerr << "simdjson Error: " << error.what() << std::endl;
			}
		},
		iterations);


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

std::string regular_test(json_data& jsonData) {
	jsonifier::vector<results> results{};
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

std::string abc_test(json_data& jsonData) {
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

std::string single_test(json_data& jsonData) {
	jsonifier::vector<results> results{};
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

int32_t main() {
	try {
		json_data jsonData{ TestGenerator<test_struct>::generateJsonData() };
		auto singlTestResults = single_test(jsonData);
		auto multiTestResults = regular_test(jsonData);
		auto abcTestResults	  = abc_test(jsonData);
#if defined(_WIN32)
		FileLoader fileLoader01{ "../../../ReadMe.md" };
		FileLoader fileLoader02{ "../../../JsonData.json" };
		fileLoader02.saveFile(glz::prettify(jsonData.theData));
#else
		FileLoader fileLoader01{ "../ReadMe.md" };
		FileLoader fileLoader02{ "../JsonData.json" };
		fileLoader02.saveFile(glz::prettify(jsonData.theData));
#endif
		std::string newstring = fileLoader01;
		std::string section01 = newstring.substr(0, newstring.find("Single Iteration Test Results:") + std::string("Single Iteration Test Results:").size() + 2);
		auto section02		  = newstring.substr(newstring.find("> 100 iterations on a 6 core (Intel i7 8700k)"),
				   newstring.find("performance regardless of the JSON document's scale.") + std::string{ "performance regardless of the JSON document's scale." }.size() -
					   newstring.find("> 100 iterations on a 6 core (Intel i7 8700k)"));

		std::string newerstring = section01 + singlTestResults + "\n\nMulti Iteration Test Results:\n" + multiTestResults + "\n" + section02;
		newerstring += "\n" + abcTestResults + "\n> 101 iterations on a 6 core (Intel i7 8700k)";

		fileLoader01.saveFile(newerstring);
	} catch (std::exception& e) {
		std::cout << e.what() << std::endl;
	}
	return 0;
}