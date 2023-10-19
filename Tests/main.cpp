
#include "glaze/core/macros.hpp"
#include "glaze/glaze.hpp"
#ifdef JSONIFIER_CPU_INSTRUCTIONS
//#undef JSONIFIER_CPU_INSTRUCTIONS
//#define JSONIFIER_CPU_INSTRUCTIONS (JSONIFIER_AVXF|JSONIFIER_BMI|JSONIFIER_POPCNT|JSONIFIER_BMI1|JSONIFIER_LZCNT)//
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
	jsonifier::vector<std::string> testString{};
	jsonifier::vector<uint64_t> testUint{};
	jsonifier::vector<double> testDouble{};
	jsonifier::vector<int64_t> testInt{};
	jsonifier::vector<bool> testBool{};
};

struct fixed_object_t {
	jsonifier::vector<uint64_t> int_array;
	jsonifier::vector<float> float_array;
	jsonifier::vector<double> double_array;
	std::unordered_map<std::string, jsonifier::vector<int32_t>> testMap{ { "TEST01", jsonifier::vector<int32_t>{ 232332, 2020 } },
		{ "TEST03434", jsonifier::vector<int32_t>{ 232333434, 232323 } } };
};

struct fixed_name_object_t {
	std::string name0{};
	std::string name1{};
	std::string name2{};
	std::string name3{};
	std::string name4{};
};

struct nested_object_t {
	jsonifier::vector<std::array<double, 3>> v3s{};
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
	jsonifier::vector<std::string> string_array{};
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
	jsonifier::vector<OTy> a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v, w, x, y, z;

	Test() {
		auto fill = [](auto& v) {
			v.resize(10);
			for (uint64_t x = 0; x < 10; ++x) {
				if constexpr (std::same_as<OTy, test_struct>) {
					for (uint64_t y = 0; y < 100; ++y) {
						v[x].testString.emplace_back(
							std::to_string(100000000000000) + std::to_string(100000000000000) + std::to_string(100000000000000) + std::to_string(100000000000000));
						v[x].testBool.emplace_back(false);
						v[x].testUint.emplace_back(1000000000000000);
						v[x].testInt.emplace_back(1000000000000000);
						v[x].testDouble.emplace_back(1000000000000000.0f);
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

GLZ_META(test_struct, testInt, testUint, testString, testDouble, testBool);
GLZ_META(Test<test_struct>, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v, w, x, y, z);

template<> struct jsonifier::core<Test<test_struct>> {
	using OTy						 = Test<test_struct>;
	constexpr static auto parseValue = createObject("a", &OTy::a, "b", &OTy::b, "c", &OTy::c, "d", &OTy::d, "e", &OTy::e, "f", &OTy::f, "g", &OTy::g, "h", &OTy::h, "i", &OTy::i,
		"j", &OTy::j, "k", &OTy::k, "l", &OTy::l, "m", &OTy::m, "n", &OTy::n, "o", &OTy::o, "p", &OTy::p, "q", &OTy::q, "r", &OTy::r, "s", &OTy::s, "t", &OTy::t, "u", &OTy::u, "v",
		&OTy::v, "w", &OTy::w, "x", &OTy::x, "y", &OTy::y, "z", &OTy::z);
};

template<typename OTy> struct AbcTest {
	jsonifier::vector<OTy> z, y, x, w, v, u, t, s, r, q, p, o, n, m, l, k, j, i, h, g, f, e, d, c, b, a;

	AbcTest() {
		auto fill = [](auto& v) {
			v.resize(10);
			for (uint64_t x = 0; x < 10; ++x) {
				if constexpr (std::same_as<OTy, test_struct>) {
					for (uint64_t y = 0; y < 100; ++y) {
						v[x].testString.emplace_back(
							std::to_string(100000000000000) + std::to_string(100000000000000) + std::to_string(100000000000000) + std::to_string(100000000000000));
						v[x].testBool.emplace_back(false);
						v[x].testUint.emplace_back(1000000000000000);
						v[x].testInt.emplace_back(1000000000000000);
						v[x].testDouble.emplace_back(1000000000000000.0f);
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

GLZ_META(AbcTest<test_struct>, z, y, x, w, v, u, t, s, r, q, p, o, n, m, l, k, j, i, h, g, f, e, d, c, b, a);

template<> struct jsonifier::core<test_struct> {
	using OTy = test_struct;
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
constexpr static uint64_t iterations	 = 100;
constexpr static uint64_t iterations_abc = 100;
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
			finalstring			  = fmt::format("| [**{}**]({}) | **{}** | **{}** | **{}** |", name, url, "Normal", write, read);
			wasThereOneBeforeThis = true;
		}
		if (json_read_mixed) {
			if (wasThereOneBeforeThis) {
				finalstring += +"\n";
			}
			write = json_write_mixed ? fmt::format("{}", static_cast<uint64_t>(*json_byte_length_mixed / (*json_write_mixed) * 1000.0f)) : "N/A";
			read  = fmt::format("{}", static_cast<uint64_t>(*json_byte_length_mixed / (*json_read_mixed) * 1000.0f));
			finalstring += fmt::format("| [**{}**]({}) | **{}** | **{}** | **{}** |", name, url, "Mixed", write, read);
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
		this->fileContents = inputStream.str();
		theStream.close();
	}

	void saveFile(std::string fileToSave) {
		auto theStream = std::ofstream{ filePath, std::ios::out | std::ios::in };
		theStream << "";
		theStream.write(fileToSave.data(), fileToSave.size());
		theStream.close();
	}

	operator std::string() {
		return this->fileContents;
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

auto glaze_test(bool doWePrint = true) {
	std::string buffer{};

	results r{ "glaze - Test:", "https://github.com/stephenberry/glaze", iterations_abc };
	Test<test_struct> uint64AbcTest{};
	glz::write_json(uint64AbcTest, buffer);

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
	r.json_write_mixed		 = result;

	if (auto error = glz::read_json(uint64AbcTest, buffer)) {
		std::cout << "glaze Error: " << error << std::endl;
	}

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
	buffer.clear();
	if (doWePrint) {
		r.print();
	}

	return r;
}

auto glaze_single_test(bool doWePrint = true) {
	std::string buffer{ json0 };

	obj_t obj{};

	results r{ "glaze - Single Test:", "https://github.com/stephenberry/glaze", 1 };

	if (auto error = glz::read_json(obj, buffer)) {
		std::cout << "glaze Error: " << error << std::endl;
	}
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

	glz::write_json(obj, buffer);
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
	if (doWePrint) {
		r.print();
	}

	return r;
}

auto glaze_abc_test(bool doWePrint = true) {
	std::string buffer{};

	results r{ "glaze - Abc Test:", "https://github.com/stephenberry/glaze", iterations_abc };
	AbcTest<test_struct> uint64AbcTest{};
	glz::write_json(uint64AbcTest, buffer);

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
	r.json_write_mixed		 = result;

	if (auto error = glz::read_json(uint64AbcTest, buffer)) {
		std::cout << "glaze Error: " << error << std::endl;
	}

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
	buffer.clear();
	if (doWePrint) {
		r.print();
	}

	return r;
}

auto jsonifier_test(bool doWePrint = true) {
	std::string buffer{};

	results r{ "jsonifier - Test:", "https://github.com/RealTimeChris/jsonifier", iterations_abc };
	Test<test_struct> uint64AbcTest{};
	jsonifier::jsonifier_core parser{};
	parser.serializeJson(uint64AbcTest, buffer);

	auto result = benchmark(
		[&]() {
			try {
				parser.serializeJson(uint64AbcTest, buffer);
			} catch (std::runtime_error& e) {
				std::cout << "jsonifier Error: " << e.what() << std::endl;
			}
		},
		iterations_abc);

	r.json_byte_length_mixed = buffer.size();
	r.json_write_mixed		 = result;

	parser.parseJson<false, true>(uint64AbcTest, buffer);

	result = benchmark(
		[&]() {
			parser.parseJson<false, true>(uint64AbcTest, buffer);
		},
		iterations_abc);

	r.json_read_mixed = result;
	buffer.clear();
	if (doWePrint) {
		r.print();
	}

	return r;
}

auto jsonifier_single_test(bool doWePrint = true) {
	std::string buffer{ json0 };

	obj_t obj{};

	results r{ "jsonifier - Single Test:", "https://github.com/RealTimeChris/jsonifier", 1 };

	jsonifier::jsonifier_core parser{};
	parser.parseJson<false, true>(obj, buffer);
	auto result = benchmark(
		[&]() {
			parser.parseJson<false, true>(obj, buffer);
		},
		1);

	r.json_byte_length = buffer.size();
	r.json_read		   = result;
	buffer.clear();

	parser.serializeJson(obj, buffer);
	result = benchmark(
		[&]() {
			try {
				parser.serializeJson(obj, buffer);
			} catch (std::runtime_error& e) {
				std::cout << "jsonifier Error: " << e.what() << std::endl;
			}
		},
		1);

	r.json_write = result;
	if (doWePrint) {
		r.print();
	}

	return r;
}

auto jsonifier_abc_test(bool doWePrint = true) {
	std::string buffer{};

	results r{ "jsonifier - Abc Test:", "https://github.com/RealTimeChris/jsonifier", iterations_abc };
	AbcTest<test_struct> uint64AbcTest{};
	jsonifier::jsonifier_core parser{};
	parser.serializeJson(uint64AbcTest, buffer);

	auto result = benchmark(
		[&]() {
			try {
				parser.serializeJson(uint64AbcTest, buffer);
			} catch (std::runtime_error& e) {
				std::cout << "jsonifier Error: " << e.what() << std::endl;
			}
		},
		iterations_abc);

	r.json_byte_length_mixed = buffer.size();
	r.json_write_mixed		 = result;

	parser.parseJson<false, true>(uint64AbcTest, buffer);

	result = benchmark(
		[&]() {
			parser.parseJson<false, true>(uint64AbcTest, buffer);
		},
		iterations_abc);

	r.json_read_mixed = result;
	buffer.clear();
	if (doWePrint) {
		r.print();
	}

	return r;
}

#include "simdjson.h"

using namespace simdjson;

struct on_demand {
	bool read_in_order(obj_t& obj, const padded_string& json);
	bool read_in_order(Test<test_struct>& obj, const padded_string& json);

  protected:
	ondemand::parser parser{};
};

template<typename OTy2> void simdPullArray(ondemand::array newX, jsonifier::vector<OTy2>& newVector);

template<> void simdPullArray<double>(ondemand::array newX, jsonifier::vector<double>& newVector) {
	if (newVector.size() < 1000) {
		newVector.resize(1000);
	}
	int32_t currentIndex{};
	for (ondemand::value value: newX) {
		newVector[currentIndex] = value.get_double();
	}
}

template<> void simdPullArray<int64_t>(ondemand::array newX, jsonifier::vector<int64_t>& newVector) {
	if (newVector.size() < 1000) {
		newVector.resize(1000);
	}
	int32_t currentIndex{};
	for (ondemand::value value: newX) {
		newVector[currentIndex] = value.get_int64();
	}
}

template<> void simdPullArray<uint64_t>(ondemand::array newX, jsonifier::vector<uint64_t>& newVector) {
	if (newVector.size() < 1000) {
		newVector.resize(1000);
	}
	int32_t currentIndex{};
	for (ondemand::value value: newX) {
		newVector[currentIndex] = value.get_uint64();
	}
}

template<> void simdPullArray<bool>(ondemand::array newX, jsonifier::vector<bool>& newVector) {
	if (newVector.size() < 1000) {
		newVector.resize(1000);
	}
	int32_t currentIndex{};
	for (ondemand::value value: newX) {
		newVector[currentIndex] = value.get_bool();
	}
}

template<> void simdPullArray<std::string>(ondemand::array newX, jsonifier::vector<std::string>& newVector) {
	if (newVector.size() < 1000) {
		newVector.resize(1000);
	}
	int32_t currentIndex{};
	for (ondemand::value value: newX) {
		newVector[currentIndex] = static_cast<std::string>(value.get_string().value());
	}
}



#define SIMD_PULL(x) \
	{ \
		ondemand::array newX = doc[#x].get_array().value(); \
		if (obj.x.size() < 1000) { \
			obj.x.resize(1000); \
		} \
		int32_t currentIndex{}; \
		test_struct newStruct{}; \
		for (ondemand::value value: newX) { \
			auto newObject = value.get_object(); \
			simdPullArray(newObject["testInt"].get_array().value(), newStruct.testInt); \
			simdPullArray(newObject["testDouble"].get_array().value(), newStruct.testDouble); \
			simdPullArray(newObject["testBool"].get_array().value(), newStruct.testBool); \
			simdPullArray(newObject["testUint"].get_array().value(), newStruct.testUint); \
			simdPullArray(newObject["testString"].get_array().value(), newStruct.testString); \
			obj.x[currentIndex] = newStruct; \
			++currentIndex; \
		} \
	}


bool on_demand::read_in_order(Test<test_struct>& obj, const padded_string& json) {
	ondemand::document doc = parser.iterate(json).value();
	SIMD_PULL(a);
	SIMD_PULL(b);
	SIMD_PULL(c);
	SIMD_PULL(d);
	SIMD_PULL(e);
	SIMD_PULL(f);
	SIMD_PULL(g);
	SIMD_PULL(h);
	SIMD_PULL(i);
	SIMD_PULL(j);
	SIMD_PULL(k);
	SIMD_PULL(l);
	SIMD_PULL(m);
	SIMD_PULL(n);
	SIMD_PULL(o);
	SIMD_PULL(p);
	SIMD_PULL(q);
	SIMD_PULL(r);
	SIMD_PULL(s);
	SIMD_PULL(t);
	SIMD_PULL(u);
	SIMD_PULL(v);
	SIMD_PULL(w);
	SIMD_PULL(x);
	SIMD_PULL(y);
	SIMD_PULL(z);
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
		size_t i   = 0;
		auto& back = obj.another_object.nested_object.v3s.emplace_back();
		for (double x: v3) {
			back[i++] = x;
		}
	}

	obj.another_object.nested_object.id = std::string_view(nested_object["id"]);

	ondemand::array string_array = doc["string_array"];
	obj.string_array.resize(string_array.count_elements());
	size_t index = 0;
	for (const std::string_view& x: string_array) {
		obj.string_array[index++] = x;
	}

	obj.string		 = std::string_view(doc["string"]);
	obj.number		 = double(doc["number"]);
	obj.boolean		 = bool(doc["boolean"]);
	obj.another_bool = bool(doc["another_bool"]);

	return false;
}

auto simdjson_test(bool doWePrint = true) {
	std::string buffer{};

	results r{ "simdjson - Test:", "https://github.com/simdjson/simdjson", iterations_abc };
	Test<test_struct> uint64AbcTest{};
	jsonifier::jsonifier_core parser{};
	parser.serializeJson(uint64AbcTest, buffer);

	auto result = benchmark(
		[&]() {
			try {
				parser.serializeJson(uint64AbcTest, buffer);
			} catch (std::runtime_error& e) {
				std::cout << "simdjson Error: " << e.what() << std::endl;
			}
		},
		iterations_abc);

	r.json_byte_length_mixed = buffer.size();
	on_demand parserTwo{};

	parserTwo.read_in_order(uint64AbcTest, buffer);

	result = benchmark(
		[&]() {
			parserTwo.read_in_order(uint64AbcTest, buffer);
		},
		iterations_abc);

	r.json_read_mixed = result;
	if (doWePrint) {
		r.print();
	}

	return r;
}

auto simdjson_single_test(bool doWePrint = true) {
	padded_string buffer{ json0 };

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

	results r{ "simdjson - Single Test:", "https://github.com/simdjson/simdjson", 1 };

	r.json_byte_length = buffer.size();
	r.json_read		   = result;
	if (doWePrint) {
		r.print();
	}

	return r;
}

struct on_demand_abc {
	bool read_out_of_order(AbcTest<test_struct>& obj, const padded_string& json);

  protected:
	ondemand::parser parser{};
};

bool on_demand_abc::read_out_of_order(AbcTest<test_struct>& obj, const padded_string& json) {
	ondemand::document doc = parser.iterate(json).value();
	SIMD_PULL(a);
	SIMD_PULL(b);
	SIMD_PULL(c);
	SIMD_PULL(d);
	SIMD_PULL(e);
	SIMD_PULL(f);
	SIMD_PULL(g);
	SIMD_PULL(h);
	SIMD_PULL(i);
	SIMD_PULL(j);
	SIMD_PULL(k);
	SIMD_PULL(l);
	SIMD_PULL(m);
	SIMD_PULL(n);
	SIMD_PULL(o);
	SIMD_PULL(p);
	SIMD_PULL(q);
	SIMD_PULL(r);
	SIMD_PULL(s);
	SIMD_PULL(t);
	SIMD_PULL(u);
	SIMD_PULL(v);
	SIMD_PULL(w);
	SIMD_PULL(x);
	SIMD_PULL(y);
	SIMD_PULL(z);
	return false;
}

auto simdjson_abc_test(bool doWePrint = true) {
	on_demand_abc parser{};

	AbcTest<test_struct> obj{};

	std::string buffer = glz::write_json(obj);

	auto error = parser.read_out_of_order(obj, buffer);

	auto result = benchmark(
		[&]() {
			error = parser.read_out_of_order(obj, buffer);
			if (error) {
				std::cerr << "simdjson error" << std::endl;
			}
		},
		iterations_abc);

	results r{ "simdjson - Abc Test:", "https://github.com/simdjson/simdjson", iterations_abc };

	r.json_byte_length_mixed = buffer.size();
	r.json_read_mixed		 = result;
	if (doWePrint) {
		r.print();
	}

	return r;
}

static std::string table_header = R"(
| Library                                           | Test       | Write (MB/s) | Read (MB/s) |
| ------------------------------------------------- | ---------- | ------------ | ----------- |)";

std::string regular_test() {
	std::vector<results> results{};
	glaze_test(false);
	results.emplace_back(glaze_test());
	jsonifier_test(false);
	results.emplace_back(jsonifier_test());
	simdjson_test(false);
	results.emplace_back(simdjson_test());

	std::string table{};
	const auto n = results.size();
	table += table_header + '\n';
	for (size_t i = 0; i < n; ++i) {
		table += results[i].json_stats();
		if (i != n - 1) {
			table += "\n";
		}
	}
	return table;
}

std::string abc_test() {
	std::vector<results> results{};
	glaze_abc_test(false);
	results.emplace_back(glaze_abc_test());
	jsonifier_abc_test(false);
	results.emplace_back(jsonifier_abc_test());
	simdjson_abc_test(false);
	results.emplace_back(simdjson_abc_test());

	std::string table{};
	const auto n = results.size();
	table += table_header + '\n';
	for (size_t i = 0; i < n; ++i) {
		table += results[i].json_stats();
		if (i != n - 1) {
			table += "\n";
		}
	}
	return table;
}

std::string single_test() {
	std::vector<results> results{};
	glaze_single_test(false);
	results.emplace_back(glaze_single_test());
	jsonifier_single_test(false);
	results.emplace_back(jsonifier_single_test());
	simdjson_single_test(false);
	results.emplace_back(simdjson_single_test());

	std::string table{};
	const auto n = results.size();
	table += table_header + '\n';
	for (size_t i = 0; i < n; ++i) {
		table += results[i].json_stats();
		if (i != n - 1) {
			table += "\n";
		}
	}
	return table;
}

int main() {
	try {
		auto singlTestResults = single_test();
		auto multiTestResults = regular_test();
		auto abcTestResults	  = abc_test();
#ifdef _WIN32
		FileLoader fileLoader{ "../../../ReadMe.md" };
#else
		FileLoader fileLoader{ "../ReadMe.md" };
#endif
		std::string newString = fileLoader;
		std::string section01 = newString.substr(0, newString.find("Single Iteration Test Results:") + std::string("Single Iteration Test Results:").size() + 1);
		auto section02		  = newString.substr(newString.find("> 100 iterations on a 6 core (Intel i7 8700k)"),
				   newString.find("performance regardless of the JSON document's scale.") + std::string{ "performance regardless of the JSON document's scale." }.size() -
					   newString.find("> 100 iterations on a 6 core (Intel i7 8700k)"));

		std::string newerString = section01 + singlTestResults + "\n\nMulti Iteration Test Results:\n" + multiTestResults + "\n" + section02;
		newerString += "\n" + abcTestResults + "\n> 101 iterations on a 6 core (Intel i7 8700k)";
		fileLoader.saveFile(newerString);
	} catch (std::exception& e) {
		std::cout << e.what() << std::endl;
	}
	return 0;
}
