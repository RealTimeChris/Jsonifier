
#include "glaze/core/macros.hpp"
	#include "glaze/glaze.hpp"
#ifdef JSONIFIER_CPU_INSTRUCTIONS
//#undef JSONIFIER_CPU_INSTRUCTIONS
	//#define JSONIFIER_CPU_INSTRUCTIONS JSONIFIER_AVX2
#endif
	#include <jsonifier/Index.hpp>
#include <unordered_set>

constexpr static std::string_view json0 = R"({"fixed_object": {
      "int_array": [0, 1, 2, 3, 4, 5, 6],
      "float_array": [0.1, 0.2, 0.3, 0.4, 0.5, 0.6],
      "double_array": [3288398.238, 233e22, 289e-1, 0.928759872, 0.22222848, 0.1, 0.2, 0.3, 0.4,
                        3288398.238, 233e22, 289e-1, 0.928759872, 0.22222848, 0.1, 0.2, 0.3, 0.4,
                        3288398.238, 233e22, 289e-1, 0.928759872, 0.22222848, 0.1, 0.2, 0.3, 0.4,
                        3288398.238, 233e22, 289e-1, 0.928759872, 0.22222848, 0.1, 0.2, 0.3, 0.4]
   },
   "fixed_name_object": {
      "name0": "James",
      "name1": "Abraham",
      "name2": "Susan",
      "name3": "Frank",
      "name4": "Alicia"
   },
   "another_object": {
      "string": "here is some text",
      "another_string": "Hello World",
      "boolean": false,
      "nested_object": {
         "v3s": [[0.12345, 0.23456, 0.001345],
                  [0.3894675, 97.39827, 297.92387],
                  [18.18, 87.289, 2988.298],
                  [0.3894675, 97.39827, 297.92387],
                  [18.18, 87.289, 2988.298],
                  [0.3894675, 97.39827, 297.92387],
                  [18.18, 87.289, 2988.298]],
         "id": "298728949872"
      }
   },
   "string_array": ["Cat", "Dog", "Elephant", "Tiger","SuperlongohmygoshIamsomuchlongerthanthepreviousstringsstring", 
	"Cat", "Dog", "Elephant", "Tiger","SuperlongohmygoshIamsomuchlongerthanthepreviousstringsstring", 
	"Cat", "Dog", "Elephant", "Tiger","SuperlongohmygoshIamsomuchlongerthanthepreviousstringsstring", 
	"Cat", "Dog", "Elephant", "Tiger","SuperlongohmygoshIamsomuchlongerthanthepreviousstringsstring", 
	"Cat", "Dog", "Elephant", "Tiger","SuperlongohmygoshIamsomuchlongerthanthepreviousstringsstring", 
	"Cat", "Dog", "Elephant", "Tiger","SuperlongohmygoshIamsomuchlongerthanthepreviousstringsstring", 
	"Cat", "Dog", "Elephant", "Tiger","SuperlongohmygoshIamsomuchlongerthanthepreviousstringsstring", 
	"Cat", "Dog", "Elephant", "Tiger","SuperlongohmygoshIamsomuchlongerthanthepreviousstringsstring", 
	"Cat", "Dog", "Elephant", "Tiger","SuperlongohmygoshIamsomuchlongerthanthepreviousstringsstring", 
	"Cat", "Dog", "Elephant", "Tiger","SuperlongohmygoshIamsomuchlongerthanthepreviousstringsstring", 
	"Cat", "Dog", "Elephant", "Tiger","SuperlongohmygoshIamsomuchlongerthanthepreviousstringsstring", 
	"Cat", "Dog", "Elephant", "Tiger","SuperlongohmygoshIamsomuchlongerthanthepreviousstringsstring", 
	"Cat", "Dog", "Elephant", "Tiger","SuperlongohmygoshIamsomuchlongerthanthepreviousstringsstring", 
	"Cat", "Dog", "Elephant", "Tiger","SuperlongohmygoshIamsomuchlongerthanthepreviousstringsstring", 
	"Cat", "Dog", "Elephant", "Tiger","SuperlongohmygoshIamsomuchlongerthanthepreviousstringsstring", 
	"Cat", "Dog", "Elephant", "Tiger","SuperlongohmygoshIamsomuchlongerthanthepreviousstringsstring", 
	"Cat", "Dog", "Elephant", "Tiger", "SuperlongohmygoshIamsomuchlongerthanthepreviousstringsstring", "SuperlongohmygoshIamsomuchlongerthanthepreviousstringsstring"],
   "string": "Hello world",
   "number": 3.14,
   "boolean": true,
   "another_bool": false
})";

#include <chrono>
#include <iostream>
#include <unordered_map>

#include "fmt/format.h"

struct test_struct {
	std::string testString{};
	uint64_t testUint{};
	double testDouble{};
	int64_t testInt{};
	bool testBool{};
};

struct fixed_object_t {
	std::unordered_set<std::string> excludedKeys{ "int_array" };
	std::vector<uint64_t> int_array;
	std::vector<float> float_array;
	std::vector<double> double_array;
	std::unordered_map<std::string, std::vector<int32_t>> testMap{ { "TEST01", std::vector<int32_t>{ 232332, 2020 } },
		{ "TEST03434", std::vector<int32_t>{ 232333434, 232323 } } };
};

struct fixed_name_object_t {
	std::string name0{};
	std::string name1{};
	std::string name2{};
	std::string name3{};
	std::string name4{};
};

struct nested_object_t {
	std::vector<std::array<double, 3>> v3s{};
	std::string id{};
};

struct another_object_t {
	std::string string{};
	std::string another_string{};
	bool boolean{};
	nested_object_t nested_object{};
};

struct obj_t {
	fixed_object_t fixed_object{};
	fixed_name_object_t fixed_name_object{};
	another_object_t another_object{};
	std::vector<std::string> string_array{};
	std::string string{};
	double number{};
	bool boolean{};
	bool another_bool{};
};

template<> struct glz::meta<fixed_object_t> {
	using OTy					= fixed_object_t;
	constexpr static auto value = object("int_array", &OTy::int_array, "float_array", &OTy::float_array, "double_array", &OTy::double_array);
};

template<> struct glz::meta<fixed_name_object_t> {
	using OTy					= fixed_name_object_t;
	constexpr static auto value = object("name0", &OTy::name0, "name1", &OTy::name1, "name2", &OTy::name2, "name3", &OTy::name3, "name4", &OTy::name4);
};

template<> struct glz::meta<nested_object_t> {
	using OTy					= nested_object_t;
	constexpr static auto value = object("v3s", &OTy::v3s, "id", &OTy::id);
};

template<> struct glz::meta<another_object_t> {
	using OTy					= another_object_t;
	constexpr static auto value = object("string", &OTy::string, "another_string", &OTy::another_string, "boolean", &OTy::boolean, "nested_object", &OTy::nested_object);
};

template<> struct glz::meta<obj_t> {
	using OTy					= obj_t;
	constexpr static auto value = glz::object("fixed_object", &OTy::fixed_object, "fixed_name_object", &OTy::fixed_name_object, "another_object", &OTy::another_object,
		"string_array", &OTy::string_array, "string", &OTy::string, "number", &OTy::number, "boolean", &OTy::boolean, "another_bool", &OTy::another_bool);
};

template<> struct jsonifier::core<fixed_object_t> {
	using OTy						 = fixed_object_t;
	constexpr static auto parseValue = createObject("int_array", &OTy::int_array, "float_array", &OTy::float_array, "double_array", &OTy::double_array, "testMap", &OTy::testMap);
};

template<> struct jsonifier::core<fixed_name_object_t> {
	using OTy						 = fixed_name_object_t;
	constexpr static auto parseValue = createObject("name0", &OTy::name0, "name1", &OTy::name1, "name2", &OTy::name2, "name3", &OTy::name3, "name4", &OTy::name4);
};

template<> struct jsonifier::core<nested_object_t> {
	using OTy						 = nested_object_t;
	constexpr static auto parseValue = createObject("v3s", &OTy::v3s, "id", &OTy::id);
};

template<> struct jsonifier::core<another_object_t> {
	using OTy						 = another_object_t;
	constexpr static auto parseValue = createObject("string", &OTy::string, "another_string", &OTy::another_string, "boolean", &OTy::boolean, "nested_object", &OTy::nested_object);
};

template<> struct jsonifier::core<obj_t> {
	using OTy						 = obj_t;
	constexpr static auto parseValue = createObject("fixed_object", &OTy::fixed_object, "fixed_name_object", &OTy::fixed_name_object, "another_object", &OTy::another_object,
		"string_array", &OTy::string_array, "string", &OTy::string, "number", &OTy::number, "boolean", &OTy::boolean, "another_bool", &OTy::another_bool);
};

template<typename OTy> struct Test {
	std::vector<OTy> a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v, w, x, y, z;

	Test() {
		auto fill = [](auto& v) {
			v.resize(1000);
			for (uint64_t x = 0; x < 1000; ++x) {
				if constexpr (std::same_as<OTy, test_struct>) {
					v[x].testString = std::to_string(1000000000000000) + std::to_string(1000000000000000) + std::to_string(1000000000000000) + std::to_string(1000000000000000) +
						std::to_string(1000000000000000) + std::to_string(1000000000000000) + std::to_string(1000000000000000) + std::to_string(1000000000000000) +
						std::to_string(1000000000000000) + std::to_string(1000000000000000) + std::to_string(1000000000000000) + std::to_string(1000000000000000) +
						std::to_string(1000000000000000) + std::to_string(1000000000000000) + std::to_string(1000000000000000) + std::to_string(1000000000000000);
					v[x].testBool = false;
					v[x].testUint = 1000000000000000;
					v[x].testInt  = 1000000000000000;
					v[x].testDouble = 1000000000000000.0f;
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

GLZ_META(Test<double>, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v, w, x, y, z);
GLZ_META(Test<int64_t>, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v, w, x, y, z);
GLZ_META(Test<uint64_t>, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v, w, x, y, z);
GLZ_META(Test<std::string>, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v, w, x, y, z);
GLZ_META(test_struct, testInt, testUint, testString, testDouble, testBool);
GLZ_META(Test<test_struct>, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v, w, x, y, z);

template<> struct jsonifier::core<Test<test_struct>> {
	using OTy						 = Test<test_struct>;
	constexpr static auto parseValue = createObject("a", &OTy::a, "b", &OTy::b, "c", &OTy::c, "d", &OTy::d, "e", &OTy::e, "f", &OTy::f, "g", &OTy::g, "h", &OTy::h, "i", &OTy::i,
		"j", &OTy::j, "k", &OTy::k, "l", &OTy::l, "m", &OTy::m, "n", &OTy::n, "o", &OTy::o, "p", &OTy::p, "q", &OTy::q, "r", &OTy::r, "s", &OTy::s, "t", &OTy::t, "u", &OTy::u, "v",
		&OTy::v, "w", &OTy::w, "x", &OTy::x, "y", &OTy::y, "z", &OTy::z);
};

template<typename OTy> struct AbcTest {
	std::vector<OTy> z, y, x, w, v, u, t, s, r, q, p, o, n, m, l, k, j, i, h, g, f, e, d, c, b, a;

	AbcTest() {
		auto fill = [](auto& v) {
			v.resize(1000);
			for (uint64_t x = 0; x < 1000; ++x) {
				if constexpr (std::same_as<OTy, test_struct>) {
					v[x].testString = std::to_string(1000000000000000) + std::to_string(1000000000000000) + std::to_string(1000000000000000) + std::to_string(1000000000000000) +
						std::to_string(1000000000000000) + std::to_string(1000000000000000) + std::to_string(1000000000000000) + std::to_string(1000000000000000);
					v[x].testBool	= false;
					v[x].testUint	= 1000000000000000;
					v[x].testInt	= 1000000000000000;
					v[x].testDouble = 1000000000000000.0f;
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

GLZ_META(AbcTest<double>, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v, w, x, y, z);
GLZ_META(AbcTest<int64_t>, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v, w, x, y, z);
GLZ_META(AbcTest<uint64_t>, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v, w, x, y, z);
GLZ_META(AbcTest<std::string>, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v, w, x, y, z);
GLZ_META(AbcTest<test_struct>, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v, w, x, y, z);

template<> struct jsonifier::core<test_struct> {
	using OTy						 = test_struct;
	constexpr static auto parseValue =
		createObject("testBool", &OTy::testBool, "testInt", &OTy::testInt, "testUint", &OTy::testUint, "testDouble", &OTy::testDouble, "testString", &OTy::testString);
};

template<> struct jsonifier::core<AbcTest<test_struct>> {
	using OTy						 = AbcTest<test_struct>;
	constexpr static auto parseValue = createObject("z", &OTy::z, "y", &OTy::y, "x", &OTy::x, "w", &OTy::w, "v", &OTy::v, "u", &OTy::u, "t", &OTy::t, "s", &OTy::s, "r", &OTy::r,
		"q", &OTy::q, "p", &OTy::p, "o", &OTy::o, "n", &OTy::n, "m", &OTy::m, "l", &OTy::l, "k", &OTy::k, "j", &OTy::j, "i", &OTy::i, "h", &OTy::h, "g", &OTy::g, "f", &OTy::f, "e",
		&OTy::e, "d", &OTy::d, "c", &OTy::c, "b", &OTy::b, "a", &OTy::a);
};

#if defined(NDEBUG)
constexpr static uint64_t iterations	 = 1000;
constexpr static uint64_t iterations_abc = 1000;
#else
constexpr static uint64_t iterations	 = 1;
constexpr static uint64_t iterations_abc = 1;
#endif

struct results {
	std::string name{};
	std::string url{};
	uint64_t iterations{};

	std::optional<uint64_t> json_byte_length_mixed{};
	std::optional<double> json_read_mixed{};
	std::optional<double> json_write_mixed{};
	std::optional<uint64_t> json_byte_length{};
	std::optional<double> json_read{};
	std::optional<double> json_write{};
	void print() {
		std::cout << std::string{ "| " } + name + ": " + url + "\n" +
				"| ------------------------------------------------------------ "
				"|\n";

		if (json_write) {
			const auto MBs = *json_byte_length / (*json_write) * 1000.0f;
			std::cout << name << " json length: " << *json_byte_length << std::endl;
			std::cout << name << " json write: " << *json_write << "ns, " << MBs << " MB/s\n";
		}

		if (json_read) {
			const auto MBs = *json_byte_length / (*json_read) * 1000.0f;
			std::cout << name << " json read: " << *json_read << "ns, " << MBs << " MB/s\n";
		}

		if (json_write_mixed) {
			const auto MBs = *json_byte_length_mixed / (*json_write_mixed) * 1000.0f;
			std::cout << name << " json mixed length: " << *json_byte_length_mixed << std::endl;
			std::cout << name << " json mixed write: " << *json_write_mixed << "ns, " << MBs << " MB/s\n";
		}

		if (json_read_mixed) {
			const auto MBs = *json_byte_length_mixed / (*json_read_mixed) * 1000.0f;
			std::cout << name << " json mixed read: " << *json_read_mixed << "ns, " << MBs << " MB/s\n";
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
			finalstring			  = fmt::format("| [**{}**]({}) | **{}** | **{}** | **{}** |", name, url, "Mixed", write, read);
			wasThereOneBeforeThis = true;
		}
		if (json_read_mixed) {
			if (wasThereOneBeforeThis) {
				finalstring += +"\n";
			}
			write = json_write_mixed ? fmt::format("{}", static_cast<uint64_t>(*json_byte_length_mixed / (*json_write_mixed) * 1000.0f)) : "N/A";
			read  = fmt::format("{}", static_cast<uint64_t>(*json_byte_length_mixed / (*json_read_mixed) * 1000.0f));
			finalstring += fmt::format("| [**{}**]({}) | **{}** | **{}** | **{}** |", name, url, "Double", write, read);
			wasThereOneBeforeThis = true;
		}
		return finalstring;
	}
};


class FileLoader {
  public:
	FileLoader(const char* filePathNew) {
		filePath	   = filePathNew;
		auto theStream = std::ofstream{ filePath, std::ios::out | std::ios::in };
		std::stringstream inputStream{};
		inputStream << theStream.rdbuf();
		fileContents = inputStream.str();
		theStream.close();
	}

	void saveFile(std::string fileToSave) {
		auto theStream = std::ofstream{ filePath, std::ios::out | std::ios::in };
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
	int64_t currentLowestTime{ std::numeric_limits<int64_t>::max() };
	for (int64_t x = 0; x < iterationCount; ++x) {
		auto startTime = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now().time_since_epoch());
		function();
		auto endTime = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now().time_since_epoch());
		auto newTime = endTime - startTime;
		if (static_cast<double>(newTime.count()) < currentLowestTime) {
			currentLowestTime = static_cast<double>(newTime.count());
		}
	}
	return currentLowestTime;
}

auto jsonifier_single_test() {
	std::string buffer{ json0 };

	obj_t obj{};

	results r{ "jsonifier", "https://github.com/RealTimeChris/jsonifier", 1 };
	jsonifier::jsonifier_core jsonifier{};

	auto result = benchmark(
		[&]() {
			try {
				jsonifier.parseJson(obj, buffer);
			} catch (std::runtime_error& e) {
				std::cout << "jsonifier Error: " << e.what() << std::endl;
			}
		},
		1);

	r.json_byte_length = buffer.size();
	r.json_read		   = result;
	buffer.clear();

	result = benchmark(
		[&]() {
			try {
				jsonifier.serializeJson(obj, buffer);
			} catch (std::runtime_error& e) {
				std::cout << "jsonifier Error: " << e.what() << std::endl;
			}
		},
		1);

	r.json_write = result;
	buffer.clear();
	Test<test_struct> uint64Test{};

	result = benchmark(
		[&]() {
			try {
				jsonifier.serializeJson(uint64Test, buffer);
			} catch (std::runtime_error& e) {
				std::cout << "jsonifier Error: " << e.what() << std::endl;
			}
		},
		1);

	r.json_byte_length_mixed = buffer.size();
	r.json_write_mixed		 = result;

	result = benchmark(
		[&]() {
			try {
				jsonifier.parseJson(uint64Test, buffer);
			} catch (std::runtime_error& e) {
				std::cout << "jsonifier Error: " << e.what() << std::endl;
			}
		},
		1);

	r.json_read_mixed = result;
	r.print();

	return r;
}

auto jsonifier_test() {
	std::string buffer{ json0 };

	obj_t obj{};

	results r{ "jsonifier", "https://github.com/RealTimeChris/jsonifier", iterations };
	jsonifier::jsonifier_core jsonifier{};

	auto result = benchmark(
		[&]() {
			try {
				jsonifier.parseJson(obj, buffer);
			} catch (std::runtime_error& e) {
				std::cout << "jsonifier Error: " << e.what() << std::endl;
			}
		},
		iterations);

	r.json_byte_length = buffer.size();
	r.json_read		   = result;
	buffer.clear();

	result = benchmark(
		[&]() {
			try {
				jsonifier.serializeJson(obj, buffer);
			} catch (std::runtime_error& e) {
				std::cout << "jsonifier Error: " << e.what() << std::endl;
			}
		},
		iterations);

	r.json_write = result;
	buffer.clear();
	Test<test_struct> uint64Test{};

	result = benchmark(
		[&]() {
			try {
				jsonifier.serializeJson(uint64Test, buffer);
			} catch (std::runtime_error& e) {
				std::cout << "jsonifier Error: " << e.what() << std::endl;
			}
		},
		iterations);

	r.json_byte_length_mixed = buffer.size();
	r.json_write_mixed		  = result;

	result = benchmark(
		[&]() {
			try {
				jsonifier.parseJson(uint64Test, buffer);
			} catch (std::runtime_error& e) {
				std::cout << "jsonifier Error: " << e.what() << std::endl;
			}
		},
		iterations);

	r.json_read_mixed = result;
	buffer.clear();
	r.print();

	return r;
}

auto jsonifier_abc_test() {
	std::string buffer{};

	results r{ "jsonifier", "https://github.com/RealTimeChris/jsonifier", iterations_abc };
	jsonifier::jsonifier_core jsonifier{};
	AbcTest<test_struct> uint64AbcTest{};

	auto result = benchmark(
		[&]() {
			try {
				jsonifier.serializeJson(uint64AbcTest, buffer);
			} catch (std::runtime_error& e) {
				std::cout << "jsonifier Error: " << e.what() << std::endl;
			}
		},
		iterations_abc);

	r.json_byte_length_mixed = buffer.size();
	r.json_write_mixed		  = result;

	result = benchmark(
		[&]() {
			try {
				jsonifier.parseJson(uint64AbcTest, buffer);
			} catch (std::runtime_error& e) {
				std::cout << "jsonifier Error: " << e.what() << std::endl;
			}
		},
		iterations_abc);

	r.json_read_mixed = result;
	r.print();

	return r;
}

auto glaze_single_test() {
	std::string buffer{ json0 };

	obj_t obj{};

	results r{ "glaze", "https://github.com/RealTimeChris/glaze", 1 };

	auto result = benchmark(
		[&]() {
			try {
				if (auto error = glz::read_json(obj, buffer)) {
					std::cout << "glaze Error: " << error << std::endl;
				}
			} catch (std::runtime_error& e) {
				std::cout << "glaze Error: " << e.what() << std::endl;
			}
		},
		1);

	r.json_byte_length = buffer.size();
	r.json_read		   = result;
	buffer.clear();

	result = benchmark(
		[&]() {
			try {
				glz::write_json(obj, buffer);
			} catch (std::runtime_error& e) {
				std::cout << "glaze Error: " << e.what() << std::endl;
			}
		},
		1);

	r.json_write = result;
	buffer.clear();
	Test<test_struct> uint64Test{};

	result = benchmark(
		[&]() {
			try {
				glz::write_json(uint64Test, buffer);
			} catch (std::runtime_error& e) {
				std::cout << "glaze Error: " << e.what() << std::endl;
			}
		},
		1);

	r.json_byte_length_mixed = buffer.size();
	r.json_write_mixed		 = result;

	result = benchmark(
		[&]() {
			try {
				if (auto error = glz::read_json(uint64Test, buffer)) {
					std::cout << "glaze Error: " << error << std::endl;
				}
			} catch (std::runtime_error& e) {
				std::cout << "glaze Error: " << e.what() << std::endl;
			}
		},
		1);

	r.json_read_mixed = result;
	r.print();

	return r;
}

auto glaze_test() {
	std::string buffer{ json0 };

	obj_t obj{};

	results r{ "glaze", "https://github.com/stephenberry/glaze", iterations };

	auto result = benchmark(
		[&]() {
			try {
				if (auto error = glz::read_json(obj, buffer)) {
					std::cout << "glaze Error: " << error << std::endl;
				}
			} catch (std::runtime_error& e) {
				std::cout << "glaze Error: " << e.what() << std::endl;
			}
		},
		iterations);

	r.json_byte_length = buffer.size();
	r.json_read		   = result;
	buffer.clear();

	result = benchmark(
		[&]() {
			try {
				glz::write_json(obj, buffer);
			} catch (std::runtime_error& e) {
				std::cout << "glaze Error: " << e.what() << std::endl;
			}
		},
		iterations);

	r.json_write = result;
	buffer.clear();
	Test<test_struct> uint64Test{};

	result = benchmark(
		[&]() {
			try {
				glz::write_json(uint64Test, buffer);
			} catch (std::runtime_error& e) {
				std::cout << "glaze Error: " << e.what() << std::endl;
			}
		},
		iterations);

	r.json_byte_length_mixed = buffer.size();
	r.json_write_mixed		  = result;

	result = benchmark(
		[&]() {
			try {
				if (auto error = glz::read_json(uint64Test, buffer)) {
					std::cout << "glaze Error: " << error << std::endl;
				}
			} catch (std::runtime_error& e) {
				std::cout << "glaze Error: " << e.what() << std::endl;
			}
		},
		iterations);

	r.json_read_mixed = result;
	buffer.clear();
	r.print();

	return r;
}

auto glaze_abc_test() {
	std::string buffer{};

	results r{ "glaze", "https://github.com/RealTimeChris/glaze", iterations_abc };
	AbcTest<test_struct> uint64AbcTest{};

	auto result = benchmark(
		[&]() {
			try {
				glz::write_json(uint64AbcTest, buffer);
			} catch (std::runtime_error& e) {
				std::cout << "glaze Error: " << e.what() << std::endl;
			}
		},
		iterations_abc);

	r.json_byte_length_mixed = buffer.size();
	r.json_write_mixed		  = result;

	result = benchmark(
		[&]() {
			try {
				if (auto error = glz::read_json(uint64AbcTest, buffer)) {
					std::cout << "glaze Error: " << error << std::endl;
				}
			} catch (std::runtime_error& e) {
				std::cout << "glaze Error: " << e.what() << std::endl;
			}
		},
		iterations_abc);

	r.json_read_mixed = result;
	r.print();

	return r;
}


#include "simdjson.h"

using namespace simdjson;

struct on_demand {
	bool read_in_order(obj_t& obj, const padded_string& json);
	bool readMixed(Test<test_struct>& obj, const padded_string& json); 

  protected:
	ondemand::parser parser{};
};

#define SIMD_DOUBLE_PULL(x) \
	{ \
		ondemand::array xNew = doc[#x].get_array().value(); \
		if (obj.x.size() < 1000) { \
			obj.x.resize(1000); \
		} \
	for (simdjson::ondemand::value value: xNew) {\
	test_struct newValue{};\
	auto newObject		= value.get_object();\
	newValue.testInt	= newObject["testInt"].get_int64().value();\
	newValue.testUint	= newObject["testUint"].get_uint64().value();\
	newValue.testBool	= newObject["testBool"].get_bool().value();\
	newValue.testString = newObject["testString"].get_string().value();\
	newValue.testDouble = newObject["testDouble"].get_double().value();\
	obj.x.emplace_back(newValue);\
}\
	}

bool on_demand::readMixed(Test<test_struct>& obj, const padded_string& json) {
	ondemand::document doc = parser.iterate(json).value();
	SIMD_DOUBLE_PULL(a);
	SIMD_DOUBLE_PULL(b);
	SIMD_DOUBLE_PULL(c);
	SIMD_DOUBLE_PULL(d);
	SIMD_DOUBLE_PULL(e);
	SIMD_DOUBLE_PULL(f);
	SIMD_DOUBLE_PULL(g);
	SIMD_DOUBLE_PULL(h);
	SIMD_DOUBLE_PULL(i);
	SIMD_DOUBLE_PULL(j);
	SIMD_DOUBLE_PULL(k);
	SIMD_DOUBLE_PULL(l);
	SIMD_DOUBLE_PULL(m);
	SIMD_DOUBLE_PULL(n);
	SIMD_DOUBLE_PULL(o);
	SIMD_DOUBLE_PULL(p);
	SIMD_DOUBLE_PULL(q);
	SIMD_DOUBLE_PULL(r);
	SIMD_DOUBLE_PULL(s);
	SIMD_DOUBLE_PULL(t);
	SIMD_DOUBLE_PULL(u);
	SIMD_DOUBLE_PULL(v);
	SIMD_DOUBLE_PULL(w);
	SIMD_DOUBLE_PULL(x);
	SIMD_DOUBLE_PULL(y);
	SIMD_DOUBLE_PULL(z);
	return false;
}

bool on_demand::read_in_order(obj_t& obj, const padded_string& json) {
	auto doc					  = parser.iterate(json);
	ondemand::object fixed_object = doc["fixed_object"];

	ondemand::array int_array = fixed_object["int_array"];
	obj.fixed_object.int_array.clear();
	for (const int64_t& x: int_array) {
		obj.fixed_object.int_array.emplace_back(x);
	}

	ondemand::array float_array = fixed_object["float_array"];
	obj.fixed_object.float_array.clear();
	for (const double& x: float_array) {
		obj.fixed_object.float_array.emplace_back(static_cast<float>(x));
	}

	ondemand::array double_array = fixed_object["double_array"];
	obj.fixed_object.double_array.clear();
	for (const double& x: double_array) {
		obj.fixed_object.double_array.emplace_back(x);
	}

	ondemand::object fixed_name_object = doc["fixed_name_object"];
	obj.fixed_name_object.name0		   = std::string_view(fixed_name_object["name0"]);
	obj.fixed_name_object.name1		   = std::string_view(fixed_name_object["name1"]);
	obj.fixed_name_object.name2		   = std::string_view(fixed_name_object["name2"]);
	obj.fixed_name_object.name3		   = std::string_view(fixed_name_object["name3"]);
	obj.fixed_name_object.name4		   = std::string_view(fixed_name_object["name4"]);

	ondemand::object another_object	  = doc["another_object"];
	obj.another_object.string		  = std::string_view(another_object["string"]);
	obj.another_object.another_string = std::string_view(another_object["another_string"]);
	obj.another_object.boolean		  = bool(another_object["boolean"]);

	ondemand::object nested_object = another_object["nested_object"];
	ondemand::array v3s			   = nested_object["v3s"];
	obj.another_object.nested_object.v3s.clear();
	for (ondemand::array v3: v3s) {
		uint64_t i = 0;
		auto& back = obj.another_object.nested_object.v3s.emplace_back();
		for (double x: v3) {
			back[i++] = x;
		}
	}

	obj.another_object.nested_object.id = std::string_view(nested_object["id"]);

	ondemand::array string_array = doc["string_array"];
	obj.string_array.resize(string_array.count_elements());
	uint64_t index = 0;
	for (const std::string_view& x: string_array) {
		obj.string_array[index++] = x;
	}

	obj.string		 = std::string_view(doc["string"]);
	obj.number		 = double(doc["number"]);
	obj.boolean		 = bool(doc["boolean"]);
	obj.another_bool = bool(doc["another_bool"]);

	return false;
}

auto simdjson_test() {
	std::string buffer{ json0 };

	on_demand parser{};

	obj_t obj{};

	auto result = benchmark(
		[&]() {
			auto error = parser.read_in_order(obj, buffer);
			if (error) {
				std::cerr << "simdjson error" << std::endl;
			}
		},
		iterations);

	results r{ "simdjson (on demand)", "https://github.com/simdjson/simdjson", iterations };

	r.json_byte_length = buffer.size();
	r.json_read		   = result;

	Test<test_struct> objDouble{};
	buffer.clear();
	buffer = glz::write_json(objDouble);

	result = benchmark(
		[&]() {
			auto error = parser.readMixed(objDouble, buffer);
			if (error) {
				std::cerr << "simdjson error" << std::endl;
			}
		},
		iterations);

	r.json_byte_length_mixed = buffer.size();
	r.json_read_mixed		  = result;
	r.print();

	return r;
}

auto simdjson_single_test() {
	std::string buffer{ json0 };

	on_demand parser{};

	obj_t obj{};

	auto error = parser.read_in_order(obj, buffer);

	auto result = benchmark(
		[&]() {
			error = parser.read_in_order(obj, buffer);
			if (error) {
				std::cerr << "simdjson error" << std::endl;
			}
		},
		1);

	results r{ "simdjson (on demand)", "https://github.com/simdjson/simdjson", 1 };

	r.json_byte_length = buffer.size();
	r.json_read		   = result;

	Test<test_struct> objDouble{};
	buffer.clear();
	buffer = glz::write_json(objDouble);

	error = parser.readMixed(objDouble, buffer);

	result = benchmark(
		[&]() {
			error = parser.readMixed(objDouble, buffer);
			if (error) {
				std::cerr << "simdjson error" << std::endl;
			}
		},
		1);

	r.json_byte_length_mixed = buffer.size();
	r.json_read_mixed		  = result;

	r.print();

	return r;
}

struct on_demand_abc {
	bool readMixed(AbcTest<test_struct>& obj, const padded_string& json);

  protected:
	ondemand::parser parser{};
};

bool on_demand_abc::readMixed(AbcTest<test_struct>& obj, const padded_string& json) {
	ondemand::document doc = parser.iterate(json).value();
	SIMD_DOUBLE_PULL(z);
	SIMD_DOUBLE_PULL(y);
	SIMD_DOUBLE_PULL(x);
	SIMD_DOUBLE_PULL(w);
	SIMD_DOUBLE_PULL(v);
	SIMD_DOUBLE_PULL(u);
	SIMD_DOUBLE_PULL(t);
	SIMD_DOUBLE_PULL(s);
	SIMD_DOUBLE_PULL(r);
	SIMD_DOUBLE_PULL(q);
	SIMD_DOUBLE_PULL(p);
	SIMD_DOUBLE_PULL(o);
	SIMD_DOUBLE_PULL(n);
	SIMD_DOUBLE_PULL(m);
	SIMD_DOUBLE_PULL(l);
	SIMD_DOUBLE_PULL(k);
	SIMD_DOUBLE_PULL(j);
	SIMD_DOUBLE_PULL(i);
	SIMD_DOUBLE_PULL(h);
	SIMD_DOUBLE_PULL(g);
	SIMD_DOUBLE_PULL(f);
	SIMD_DOUBLE_PULL(e);
	SIMD_DOUBLE_PULL(d);
	SIMD_DOUBLE_PULL(c);
	SIMD_DOUBLE_PULL(b);
	SIMD_DOUBLE_PULL(a);
	return false;
}

auto simdjson_abc_test() {
	on_demand_abc parser{};

	AbcTest<test_struct> obj{};

	std::string buffer = glz::write_json(obj);

	auto error = parser.readMixed(obj, buffer);

	auto result = benchmark(
		[&]() {
			error = parser.readMixed(obj, buffer);
			if (error) {
				std::cerr << "simdjson error" << std::endl;
			}
		},
		iterations_abc);

	results r{ "simdjson (on demand)", "https://github.com/simdjson/simdjson", iterations_abc };

	r.json_byte_length_mixed = buffer.size();
	r.json_read_mixed		  = result;

	AbcTest<std::string> objstring{};
	buffer.clear();

	r.print();

	return r;
}

static std::string table_header = R"(
| Library                                           | Test       | Write (MB/s) | Read (MB/s) |
| ------------------------------------------------- | ---------- | ------------ | ----------- |)";

std::string regular_test() {
	std::vector<results> results{};
	results.emplace_back(glaze_test());
	results.emplace_back(jsonifier_test());
	results.emplace_back(simdjson_test());

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

std::string abc_test() {
	std::vector<results> results{};
	results.emplace_back(glaze_abc_test());
	results.emplace_back(jsonifier_abc_test());
	results.emplace_back(simdjson_abc_test());

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

std::string single_test() {
	std::vector<results> results{};
	results.emplace_back(glaze_single_test());
	results.emplace_back(jsonifier_single_test());
	results.emplace_back(simdjson_single_test());

	std::string table{};
	const auto n = results.size();
	table += table_header + '\n';
	for (uint64_t i = 0; i < n; ++i) {
		//table += results[i].json_stats();
		if (i != n - 1) {
			table += "\n";
		}
	}
	return table;
}

struct User {
	bool verified;
	std::string username;
	bool mfa_enabled;
	std::string id;
	std::string global_name;
	int flags;
	std::string email;
	std::string discriminator;
	bool bot;
	std::string avatar;
};

struct Guild {
	bool unavailable;
	std::string id;
};

struct Application {
	std::string id;
	int flags;
};

struct Data {
	int v;
	std::unordered_set<std::string> excludedKeys{ "shard" };
	std::unordered_map<std::string, std::string> user_settings;
	User user;
	std::array<int32_t, 2> shard;
	std::string session_type;
	std::string session_id;
	std::string resume_gateway_url;
	std::vector<std::string> relationships;
	std::vector<std::string> private_channels;
	std::vector<std::string> presences;
	std::vector<Guild> guilds;
	std::vector<std::string> guild_join_requests;
	std::vector<std::string> geo_ordered_rtc_regions;
	std::unordered_map<std::string, std::string> auth;
	Application application;
	std::vector<std::string> _trace;
};

struct JSONData {
	std::string t;
	int s;
	int op;
	Data d;
};


template<> struct jsonifier::core<User> {
	using ValueType					 = User;
	static constexpr auto parseValue = createObject("verified", &ValueType::verified, "username", &ValueType::username, "mfa_enabled", &ValueType::mfa_enabled, "id",
		&ValueType::id, "global_name", &ValueType::global_name, "flags", &ValueType::flags, "email", &ValueType::email, "discriminator", &ValueType::discriminator, "bot",
		&ValueType::bot, "avatar", &ValueType::avatar);
};

template<> struct jsonifier::core<Guild> {
	using ValueType					 = Guild;
	static constexpr auto parseValue = createObject("unavailable", &ValueType::unavailable, "id", &ValueType::id);
};

template<> struct jsonifier::core<Application> {
	using ValueType					 = Application;
	static constexpr auto parseValue = createObject("id", &ValueType::id, "flags", &ValueType::flags);
};

template<> struct jsonifier::core<Data> {
	using ValueType = Data;
	static constexpr auto parseValue =
		createObject("v", &ValueType::v, "user_settings", &ValueType::user_settings, "user", &ValueType::user, "shard", &ValueType::shard, "session_type", &ValueType::session_type,
			"session_id", &ValueType::session_id, "resume_gateway_url", &ValueType::resume_gateway_url, "relationships", &ValueType::relationships, "private_channels",
			&ValueType::private_channels, "presences", &ValueType::presences, "guilds", &ValueType::guilds, "guild_join_requests", &ValueType::guild_join_requests,
			"geo_ordered_rtc_regions", &ValueType::geo_ordered_rtc_regions, "auth", &ValueType::auth, "application", &ValueType::application, "_trace", &ValueType::_trace);
};

template<> struct jsonifier::core<JSONData> {
	using ValueType					 = JSONData;
	static constexpr auto parseValue = createObject("t", &ValueType::t, "s", &ValueType::s, "op", &ValueType::op, "d", &ValueType::d);
};

int32_t main() {
	try {
		std::string jsonData{
			"{\"t\":\"READY\",\"s\":1,\"op\":0,\"d\":{\"v\":10,\"user_settings\":{},\"user\":{\"verified\":true,\"username\":\"MBot-MusicHouse-2THIS IS A TEST STRING CONTAINING "
			"ESCAPED CHARACTERS\",\"mfa_enabled\":false,\"id\":"
			"\"1142733646600614004\",\"global_name\":null,\"flags\":0,\"email\":null,\"discriminator\":\"3055\",\"bot\":true,\"avatar\":\"88bd9ce7bf889c0d36fb4afd3725900b\"},"
			"\"shard\":[0,1],\"session_type\":\"normal\",\"session_id\":\"d60a87c480db4e458474423ea1dddcc9\",\"resume_gateway_url\":\"wss://"
			"gateway-us-east1-c.discord.gg\",\"relationships\":[],\"private_channels\":[],\"presences\":[],\"guilds\":[{\"unavailable\":true,\"id\":\"318872312596267018\"},{"
			"\"unavailable\":true,\"id\":\"931640556814237706\"},{\"unavailable\":true,\"id\":\"991025447875784714\"},{\"unavailable\":true,\"id\":\"995048955215872071\"},{"
			"\"unavailable\":true,\"id\":\"1022405038922006538\"},{\"unavailable\":true,\"id\":\"1032783776184533022\"},{\"unavailable\":true,\"id\":\"1078501504119476282\"},{"
			"\"unavailable\":true,\"id\":\"1131853763506880522\"}],\"guild_join_requests\":[],\"geo_ordered_rtc_regions\":[\"newark\",\"us-east\",\"us-central\",\"atlanta\",\"us-"
			"south\"],\"auth\":{},\"application\":{\"id\":\"1142733646600614004\",\"flags\":27828224},\"_trace\":[\"[\\\"gateway-prd-us-east1-c-jqlr\\\",{\\\"micros\\\":111038,"
			"\\\"calls\\\":[\\\"id_created\\\",{\\\"micros\\\":1564,\\\"calls\\\":[]},\\\"session_lookup_time\\\",{\\\"micros\\\":266,\\\"calls\\\":[]},\\\"session_lookup_"
			"finished\\\",{\\\"micros\\\":16,\\\"calls\\\":[]},\\\"discord-sessions-prd-2-173\\\",{\\\"micros\\\":105860,\\\"calls\\\":[\\\"start_session\\\",{\\\"micros\\\":"
			"59291,\\\"calls\\\":[\\\"discord-api-7fb774cd5-lm2ws\\\",{\\\"micros\\\":55165,\\\"calls\\\":[\\\"get_user\\\",{\\\"micros\\\":26287},\\\"get_guilds\\\",{"
			"\\\"micros\\\":2891},\\\"send_scheduled_deletion_message\\\",{\\\"micros\\\":13},\\\"guild_join_requests\\\",{\\\"micros\\\":941},\\\"authorized_ip_coro\\\",{"
			"\\\"micros\\\":9}]}]},\\\"starting_guild_connect\\\",{\\\"micros\\\":211,\\\"calls\\\":[]},\\\"presence_started\\\",{\\\"micros\\\":329,\\\"calls\\\":[]},\\\"guilds_"
			"started\\\",{\\\"micros\\\":129,\\\"calls\\\":[]},\\\"guilds_connect\\\",{\\\"micros\\\":2,\\\"calls\\\":[]},\\\"presence_connect\\\",{\\\"micros\\\":45851,"
			"\\\"calls\\\":[]},\\\"connect_finished\\\",{\\\"micros\\\":45876,\\\"calls\\\":[]},\\\"build_ready\\\",{\\\"micros\\\":22,\\\"calls\\\":[]},\\\"clean_ready\\\",{"
			"\\\"micros\\\":0,\\\"calls\\\":[]},\\\"optimize_ready\\\",{\\\"micros\\\":1,\\\"calls\\\":[]},\\\"split_ready\\\",{\\\"micros\\\":0,\\\"calls\\\":[]}]}]}]\"]}}"
		};
		JSONData dataNew{};
		jsonifier::jsonifier_core parser{};
		int32_t valueNew{ '-' | '0' | '1' | '2' | '3' | '4' | '5' | '6' | '7' | '8' | '9' };
		int32_t valueTest{ '-' };
		
		std::cout << "DATA NEW: " << (valueNew && valueTest) << std::endl;
		parser.parseJson<true, true>(dataNew, jsonData);
		for (auto& value: parser.getErrors()) {
			std::cout << "DATA NEW: " << value.reportError() << std::endl;
		}
		//std::cout << "DATA NEW: " << dataNew.d.user.username << std::endl;
		//std::cout << "DATA NEW: " << dataNew.d.user.username << std::endl;
		//std::cout << "DATA NEW: " << dataNew.d.user.username << std::endl;
		//std::cout << "DATA NEW: " << dataNew.d._trace[0] << std::endl;
		//std::cout << "DATA NEW: " << dataNew.d.application.id << std::endl;
		auto singlTestResults = single_test();
		auto multiTestResults = regular_test();
		auto abcTestResults	  = abc_test();
#if defined(_WIN32)
		FileLoader fileLoader{ "../../../ReadMe.md" };
#else
		FileLoader fileLoader{ "../ReadMe.md" };
#endif
		std::string newstring = fileLoader;
		std::string section01 = newstring.substr(0, newstring.find("Single Iteration Test Results:") + std::string("Single Iteration Test Results:").size() + 1);
		auto section02		  = newstring.substr(newstring.find("> 1000 iterations on a 6 jsonifier::core (Intel i7 8700k)"),
				   newstring.find("performance regardless of the JSON document's scale.") + std::string{ "performance regardless of the JSON document's scale." }.size() -
					   newstring.find("> 1000 iterations on a 6 jsonifier::core (Intel i7 8700k)"));

		std::string newerstring = section01 + singlTestResults + "\n\nMulti Iteration Test Results:\n" + multiTestResults + "\n" + section02;
		newerstring += "\n" + abcTestResults + "\n> 1001 iterations on a 6 jsonifier::core (Intel i7 8700k)";
		fileLoader.saveFile(newerstring);
	} catch (std::exception& e) {
		std::cout << e.what() << std::endl;
	}
	return 0;
}