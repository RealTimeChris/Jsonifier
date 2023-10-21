
#include "glaze/core/macros.hpp"
#include "glaze/glaze.hpp"

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
						v[x].testString.emplace_back(std::to_string(100000000000000));
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
	jsonifier::vector<OTy> z, y, x, w, v, u, t, s, r, q, p, o, n, m, l, k, j, i, h, g, f, e, d, c, b, a;

	AbcTest() {
		auto fill = [](auto& v) {
			v.resize(10);
			for (uint64_t x = 0; x < 10; ++x) {
				if constexpr (std::same_as<OTy, test_struct>) {
					for (uint64_t y = 0; y < 100; ++y) {
						v[x].testString.emplace_back(std::to_string(100000000000000));
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

GLZ_META(AbcTest<double>, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v, w, x, y, z);
GLZ_META(AbcTest<int64_t>, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v, w, x, y, z);
GLZ_META(AbcTest<uint64_t>, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v, w, x, y, z);
GLZ_META(AbcTest<std::string>, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v, w, x, y, z);
GLZ_META(AbcTest<test_struct>, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v, w, x, y, z);

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
constexpr static uint64_t iterations = 100;
#else
constexpr static uint64_t iterations = 1;
#endif

struct results {
	std::string name{};
	std::string test{};
	std::string url{};
	uint64_t iterations{};

	std::optional<uint64_t> json_byte_length_mixed{};
	std::optional<double> json_read_mixed{};
	std::optional<double> json_write_mixed{};
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

		if (json_byte_length_mixed) {
			auto MBs = *json_byte_length_mixed / (*json_read_mixed) * 1000.0f;
			std::cout << "Mixed Length: " << *json_byte_length_mixed << std::endl;
			std::cout << "Mixed Read: " << *json_read_mixed << "ns, " << MBs << " MB/s\n";
			if (json_write_mixed) {
				MBs = (*json_byte_length_mixed) / (*json_write_mixed) * 1000.0f;
				std::cout << "Mixed Write: " << *json_write_mixed << "ns, " << MBs << " MB/s\n";
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

auto jsonifier_single_test(bool doWePrint = true) {
	jsonifier::string buffer{ json0 };

	obj_t obj{};

	results r{ "jsonifier", "Signle Test", "https://github.com/RealTimeChris/jsonifier", iterations };
	jsonifier::jsonifier_core jsonifier{};
	jsonifier.parseJson<false, true>(obj, buffer);

	auto result = benchmark(
		[&]() {
			jsonifier.parseJson<false, true>(obj, buffer);
		},
		iterations);
	for (auto& value: jsonifier.getErrors()) {
		std::cout << "jsonifier Error: " << value << std::endl;
	}

	r.json_byte_length = buffer.size();
	r.json_read		   = result;
	buffer.clear();
	jsonifier.serializeJson(obj, buffer);

	result = benchmark(
		[&]() {
			jsonifier.serializeJson(obj, buffer);
		},
		iterations);

	r.json_write = result;
	if (doWePrint) {
		r.print();
	}

	return r;
}

auto jsonifier_test(bool doWePrint = true) {
	jsonifier::string buffer{ json0 };

	results r{ "jsonifier", "Normal Test", "https://github.com/RealTimeChris/jsonifier", iterations };
	jsonifier::jsonifier_core jsonifier{};
	buffer.clear();
	Test<test_struct> uint64Test{};
	jsonifier.serializeJson(uint64Test, buffer);

	auto result = benchmark(
		[&]() {
			jsonifier.serializeJson(uint64Test, buffer);
		},
		iterations);

	r.json_byte_length_mixed = buffer.size();
	r.json_write_mixed		 = result;
	jsonifier.parseJson<false, true>(uint64Test, buffer);

	result = benchmark(
		[&]() {
			jsonifier.parseJson<false, true>(uint64Test, buffer);
		},
		iterations);
	for (auto& value: jsonifier.getErrors()) {
		std::cout << "jsonifier Error: " << value << std::endl;
	}

	r.json_read_mixed = result;
	if (doWePrint) {
		r.print();
	}

	return r;
}

auto jsonifier_abc_test(bool doWePrint = true) {
	jsonifier::string buffer{};

	results r{ "jsonifier", "Abc Test", "https://github.com/RealTimeChris/jsonifier", iterations };
	jsonifier::jsonifier_core jsonifier{};
	AbcTest<test_struct> uint64AbcTest{};
	jsonifier.serializeJson(uint64AbcTest, buffer);

	auto result = benchmark(
		[&]() {
			jsonifier.serializeJson(uint64AbcTest, buffer);
		},
		iterations);

	r.json_byte_length_mixed = buffer.size();
	r.json_write_mixed		 = result;
	jsonifier.parseJson<false, true>(uint64AbcTest, buffer);

	result = benchmark(
		[&]() {
			jsonifier.parseJson<false, true>(uint64AbcTest, buffer);
		},
		iterations);
	for (auto& value: jsonifier.getErrors()) {
		std::cout << "jsonifier Error: " << value << std::endl;
	}

	r.json_read_mixed = result;
	if (doWePrint) {
		r.print();
	}

	return r;
}

auto glaze_single_test(bool doWePrint = true) {
	std::string buffer{ json0 };

	obj_t obj{};

	results r{ "glaze", "Single Test", "https://github.com/RealTimeChris/glaze", 1 };
	if (auto error = glz::read_json(obj, buffer)) {
		std::cout << "glaze Error: " << error << std::endl;
	}

	auto result = benchmark(
		[&]() {
			if (auto error = glz::read_json(obj, buffer)) {
				std::cout << "glaze Error: " << error << std::endl;
			}
		},
		1);

	r.json_byte_length = buffer.size();
	r.json_read		   = result;
	buffer.clear();
	glz::write_json(obj, buffer);

	result = benchmark(
		[&]() {
			glz::write_json(obj, buffer);
		},
		1);

	r.json_write = result;
	if (doWePrint) {
		r.print();
	}

	return r;
}

auto glaze_test(bool doWePrint = true) {
	std::string buffer{};

	results r{ "glaze", "Normal Test", "https://github.com/stephenberry/glaze", iterations };
	Test<test_struct> uint64Test{};
	glz::write_json(uint64Test, buffer);

	auto result = benchmark(
		[&]() {
			glz::write_json(uint64Test, buffer);
		},
		iterations);

	r.json_byte_length_mixed = buffer.size();
	r.json_write_mixed		 = result;
	if (auto error = glz::read_json(uint64Test, buffer)) {
		std::cout << "glaze Error: " << error << std::endl;
	}

	result = benchmark(
		[&]() {
			if (auto error = glz::read_json(uint64Test, buffer)) {
				std::cout << "glaze Error: " << error << std::endl;
			}
		},
		iterations);

	r.json_read_mixed = result;
	buffer.clear();
	if (doWePrint) {
		r.print();
	}

	return r;
}

auto glaze_abc_test(bool doWePrint = true) {
	std::string buffer{};

	results r{ "glaze", "Abc Test", "https://github.com/RealTimeChris/glaze", iterations };
	AbcTest<test_struct> uint64AbcTest{};
	glz::write_json(uint64AbcTest, buffer);

	auto result = benchmark(
		[&]() {
			glz::write_json(uint64AbcTest, buffer);
		},
		iterations);

	r.json_byte_length_mixed = buffer.size();
	r.json_write_mixed		 = result;
	if (auto error = glz::read_json(uint64AbcTest, buffer)) {
		std::cout << "glaze Error: " << error << std::endl;
	}

	result = benchmark(
		[&]() {
			if (auto error = glz::read_json(uint64AbcTest, buffer)) {
				std::cout << "glaze Error: " << error << std::endl;
			}
		},
		iterations);

	r.json_read_mixed = result;
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



#define SIMD_Pull(x) \
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

auto simdjson_single_test(bool doWePrint = true) {
	std::string buffer{ json0 };

	obj_t obj{};
	on_demand parser{};

	results r{ "simdjson", "Single Test", "https://github.com/simdjson/simdjson", 1 };
	parser.read_in_order(obj, buffer);

	auto result = benchmark(
		[&]() {
			try {
				parser.read_in_order(obj, buffer);
			} catch (const std::runtime_error& error) {
				std::cerr << "simdjson Error: " << error.what() << std::endl;
			}
		},
		1);

	r.json_byte_length = buffer.size();
	r.json_read		   = result;
	buffer.clear();

	if (doWePrint) {
		r.print();
	}

	return r;
}

auto simdjson_test(bool doWePrint = true) {
	std::string buffer{ json0 };
	on_demand parser{};

	results r{ "simdjson", "Normal Test", "https://github.com/simdjson/simdjson", iterations };
	Test<test_struct> uint64Test{};
	jsonifier::jsonifier_core parserNew{};
	parserNew.serializeJson(uint64Test, buffer);

	r.json_byte_length_mixed = buffer.size();

	parser.read_in_order(uint64Test, buffer);

	auto result = benchmark(
		[&]() {
			try {
				parser.read_in_order(uint64Test, buffer);
			} catch (const std::runtime_error& error) {
				std::cerr << "simdjson Error: " << error.what() << std::endl;
			}
		},
		iterations);

	r.json_read_mixed = result;
	buffer.clear();
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

auto simdjson_abc_test(bool doWePrint = true) {
	on_demand_abc parser{};

	AbcTest<test_struct> obj{};

	std::string buffer = glz::write_json(obj);

	parser.read_out_of_order(obj, buffer);

	auto result = benchmark(
		[&]() {
			try {
				parser.read_out_of_order(obj, buffer);
			} catch (const std::runtime_error& error) {
				std::cerr << "simdjson Error: " << error.what() << std::endl;
			}
		},
		iterations);

	results r{ "simdjson", "Abc Test", "https://github.com/simdjson/simdjson", iterations };

	r.json_byte_length_mixed = buffer.size();
	r.json_read_mixed		 = result;
	if (doWePrint) {
		r.print();
	}

	return r;
}

static std::string table_header = R"(
| Library | Test | Write (MB/s) | Read (MB/s) |
| ------------------------------------------------- | ---------- | ------------ | ----------- |)";

std::string regular_test() {
	jsonifier::vector<results> results{};
	for (uint32_t x = 0; x < 2; ++x) {
		glaze_test(false);
	}
	results.emplace_back(glaze_test());
	for (uint32_t x = 0; x < 2; ++x) {
		jsonifier_test(false);
	}
	results.emplace_back(jsonifier_test());
	for (uint32_t x = 0; x < 2; ++x) {
		simdjson_test(false);
	}
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
	jsonifier::vector<results> results{};
	for (uint32_t x = 0; x < 2; ++x) {
		glaze_abc_test(false);
	}
	results.emplace_back(glaze_abc_test());
	for (uint32_t x = 0; x < 2; ++x) {
		jsonifier_abc_test(false);
	}
	results.emplace_back(jsonifier_abc_test());
	for (uint32_t x = 0; x < 2; ++x) {
		simdjson_abc_test(false);
	}
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
	jsonifier::vector<results> results{};
	for (uint32_t x = 0; x < 2; ++x) {
		glaze_single_test(false);
	}
	results.emplace_back(glaze_single_test());
	for (uint32_t x = 0; x < 2; ++x) {
		jsonifier_single_test(false);
	}
	results.emplace_back(jsonifier_single_test());
	for (uint32_t x = 0; x < 2; ++x) {
		simdjson_single_test(false);
	}
	results.emplace_back(simdjson_single_test());

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
		auto section02		  = newstring.substr(newstring.find("> 100 iterations on a 6 core (Intel i7 8700k)"),
				   newstring.find("performance regardless of the JSON document's scale.") + std::string{ "performance regardless of the JSON document's scale." }.size() -
					   newstring.find("> 100 iterations on a 6 core (Intel i7 8700k)"));

		std::string newerstring = section01 + singlTestResults + "\n\nMulti Iteration Test Results:\n" + multiTestResults + "\n" + section02;
		newerstring += "\n" + abcTestResults + "\n> 101 iterations on a 6 core (Intel i7 8700k)";
		fileLoader.saveFile(newerstring);
	} catch (std::exception& e) {
		std::cout << e.what() << std::endl;
	}
	return 0;
}