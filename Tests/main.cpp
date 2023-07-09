
#include "glaze/core/macros.hpp"
#include "glaze/glaze.hpp"
#include <jsonifier/Index.hpp>

inline constexpr static std::string_view json0 = R"({"fixed_object": {
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

struct fixed_object_t {
	std::vector<uint64_t> int_array;
	std::vector<float> float_array;
	std::vector<double> double_array;
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
	using OTy = fixed_object_t;
	inline constexpr static auto value = object("int_array", &OTy::int_array, "float_array", &OTy::float_array, "double_array", &OTy::double_array);
};

template<> struct glz::meta<fixed_name_object_t> {
	using OTy = fixed_name_object_t;
	inline constexpr static auto value =
		object("name0", &OTy::name0, "name1", &OTy::name1, "name2", &OTy::name2, "name3", &OTy::name3, "name4", &OTy::name4);
};

template<> struct glz::meta<nested_object_t> {
	using OTy = nested_object_t;
	inline constexpr static auto value = object("v3s", &OTy::v3s, "id", &OTy::id);
};

template<> struct glz::meta<another_object_t> {
	using OTy = another_object_t;
	inline constexpr static auto value =
		object("string", &OTy::string, "another_string", &OTy::another_string, "boolean", &OTy::boolean, "nested_object", &OTy::nested_object);
};

template<> struct glz::meta<obj_t> {
	using OTy = obj_t;
	inline constexpr static auto value = glz::object("fixed_object", &OTy::fixed_object, "fixed_name_object", &OTy::fixed_name_object,
		"another_object", &OTy::another_object, "string_array", &OTy::string_array, "string", &OTy::string, "number", &OTy::number, "boolean",
		&OTy::boolean, "another_bool", &OTy::another_bool);
};

template<> struct Jsonifier::Core<fixed_object_t> {
	using OTy = fixed_object_t;
	inline constexpr static auto parseValue =
		object("int_array", &OTy::int_array, "float_array", &OTy::float_array, "double_array", &OTy::double_array);
};

template<> struct Jsonifier::Core<fixed_name_object_t> {
	using OTy = fixed_name_object_t;
	inline constexpr static auto parseValue =
		object("name0", &OTy::name0, "name1", &OTy::name1, "name2", &OTy::name2, "name3", &OTy::name3, "name4", &OTy::name4);
};

template<> struct Jsonifier::Core<nested_object_t> {
	using OTy = nested_object_t;
	inline constexpr static auto parseValue = object("v3s", &OTy::v3s, "id", &OTy::id);
};

template<> struct Jsonifier::Core<another_object_t> {
	using OTy = another_object_t;
	inline constexpr static auto parseValue =
		object("string", &OTy::string, "another_string", &OTy::another_string, "boolean", &OTy::boolean, "nested_object", &OTy::nested_object);
};

template<> struct Jsonifier::Core<obj_t> {
	using OTy = obj_t;
	inline constexpr static auto parseValue = object("fixed_object", &OTy::fixed_object, "fixed_name_object", &OTy::fixed_name_object,
		"another_object", &OTy::another_object, "string_array", &OTy::string_array, "string", &OTy::string, "number", &OTy::number, "boolean",
		&OTy::boolean, "another_bool", &OTy::another_bool);
};

template<typename OTy> struct Test {
	std::vector<OTy> a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v, w, x, y, z;

	Test() {
		auto fill = [](auto& v) {
			v.resize(1000);
			for (size_t x = 0; x < 1000; ++x) {
				if constexpr (std::same_as<OTy, std::string> || std::same_as<OTy, std::string>) {
					v[x] = std::to_string(1000000000000000) + std::to_string(1000000000000000) + std::to_string(1000000000000000) +
						std::to_string(1000000000000000) + std::to_string(1000000000000000) + std::to_string(1000000000000000) +
						std::to_string(1000000000000000) + std::to_string(1000000000000000);
				} else {
					v[x] = static_cast<OTy>(100000000000000000);
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

GLZ_META(Test<std::string>, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v, w, x, y, z);
GLZ_META(Test<uint64_t>, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v, w, x, y, z);
GLZ_META(Test<int64_t>, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v, w, x, y, z);
GLZ_META(Test<double>, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v, w, x, y, z);

template<> struct Jsonifier::Core<Test<std::string>> {
	using OTy = Test<std::string>;
	inline constexpr static auto parseValue = object("a", &OTy::a, "b", &OTy::b, "c", &OTy::c, "d", &OTy::d, "e", &OTy::e, "f", &OTy::f, "g", &OTy::g,
		"h", &OTy::h, "i", &OTy::i, "j", &OTy::j, "k", &OTy::k, "l", &OTy::l, "m", &OTy::m, "n", &OTy::n, "o", &OTy::o, "p", &OTy::p, "q", &OTy::q,
		"r", &OTy::r, "s", &OTy::s, "t", &OTy::t, "u", &OTy::u, "v", &OTy::v, "w", &OTy::w, "x", &OTy::x, "y", &OTy::y, "z", &OTy::z);
};

template<> struct Jsonifier::Core<Test<uint64_t>> {
	using OTy = Test<uint64_t>;
	inline constexpr static auto parseValue = object("a", &OTy::a, "b", &OTy::b, "c", &OTy::c, "d", &OTy::d, "e", &OTy::e, "f", &OTy::f, "g", &OTy::g,
		"h", &OTy::h, "i", &OTy::i, "j", &OTy::j, "k", &OTy::k, "l", &OTy::l, "m", &OTy::m, "n", &OTy::n, "o", &OTy::o, "p", &OTy::p, "q", &OTy::q,
		"r", &OTy::r, "s", &OTy::s, "t", &OTy::t, "u", &OTy::u, "v", &OTy::v, "w", &OTy::w, "x", &OTy::x, "y", &OTy::y, "z", &OTy::z);
};

template<> struct Jsonifier::Core<Test<int64_t>> {
	using OTy = Test<int64_t>;
	inline constexpr static auto parseValue = object("a", &OTy::a, "b", &OTy::b, "c", &OTy::c, "d", &OTy::d, "e", &OTy::e, "f", &OTy::f, "g", &OTy::g,
		"h", &OTy::h, "i", &OTy::i, "j", &OTy::j, "k", &OTy::k, "l", &OTy::l, "m", &OTy::m, "n", &OTy::n, "o", &OTy::o, "p", &OTy::p, "q", &OTy::q,
		"r", &OTy::r, "s", &OTy::s, "t", &OTy::t, "u", &OTy::u, "v", &OTy::v, "w", &OTy::w, "x", &OTy::x, "y", &OTy::y, "z", &OTy::z);
};

template<> struct Jsonifier::Core<Test<double>> {
	using OTy = Test<double>;
	inline constexpr static auto parseValue = object("a", &OTy::a, "b", &OTy::b, "c", &OTy::c, "d", &OTy::d, "e", &OTy::e, "f", &OTy::f, "g", &OTy::g,
		"h", &OTy::h, "i", &OTy::i, "j", &OTy::j, "k", &OTy::k, "l", &OTy::l, "m", &OTy::m, "n", &OTy::n, "o", &OTy::o, "p", &OTy::p, "q", &OTy::q,
		"r", &OTy::r, "s", &OTy::s, "t", &OTy::t, "u", &OTy::u, "v", &OTy::v, "w", &OTy::w, "x", &OTy::x, "y", &OTy::y, "z", &OTy::z);
};

template<typename OTy> struct AbcTest {
	std::vector<OTy> z, y, x, w, v, u, t, s, r, q, p, o, n, m, l, k, j, i, h, g, f, e, d, c, b, a;

	AbcTest() {
		auto fill = [](auto& v) {
			v.resize(1000);
			for (size_t x = 0; x < 1000; ++x) {
				if constexpr (std::same_as<OTy, std::string> || std::same_as<OTy, std::string>) {
					v[x] = std::to_string(1000000000000000) + std::to_string(1000000000000000) + std::to_string(1000000000000000) +
						std::to_string(1000000000000000) + std::to_string(1000000000000000) + std::to_string(1000000000000000) +
						std::to_string(1000000000000000) + std::to_string(1000000000000000);
				} else {
					v[x] = static_cast<OTy>(100000000000000000);
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

GLZ_META(AbcTest<std::string>, z, y, x, w, v, u, t, s, r, q, p, o, n, m, l, k, j, i, h, g, f, e, d, c, b, a);
GLZ_META(AbcTest<uint64_t>, z, y, x, w, v, u, t, s, r, q, p, o, n, m, l, k, j, i, h, g, f, e, d, c, b, a);
GLZ_META(AbcTest<int64_t>, z, y, x, w, v, u, t, s, r, q, p, o, n, m, l, k, j, i, h, g, f, e, d, c, b, a);
GLZ_META(AbcTest<double>, z, y, x, w, v, u, t, s, r, q, p, o, n, m, l, k, j, i, h, g, f, e, d, c, b, a);

template<> struct Jsonifier::Core<AbcTest<std::string>> {
	using OTy = AbcTest<std::string>;
	inline constexpr static auto parseValue = object("z", &OTy::z, "y", &OTy::y, "x", &OTy::x, "w", &OTy::w, "v", &OTy::v, "u", &OTy::u, "t", &OTy::t,
		"s", &OTy::s, "r", &OTy::r, "q", &OTy::q, "p", &OTy::p, "o", &OTy::o, "n", &OTy::n, "m", &OTy::m, "l", &OTy::l, "k", &OTy::k, "j", &OTy::j,
		"i", &OTy::i, "h", &OTy::h, "g", &OTy::g, "f", &OTy::f, "e", &OTy::e, "d", &OTy::d, "c", &OTy::c, "b", &OTy::b, "a", &OTy::a);
};

template<> struct Jsonifier::Core<AbcTest<uint64_t>> {
	using OTy = AbcTest<uint64_t>;
	inline constexpr static auto parseValue = object("z", &OTy::z, "y", &OTy::y, "x", &OTy::x, "w", &OTy::w, "v", &OTy::v, "u", &OTy::u, "t", &OTy::t,
		"s", &OTy::s, "r", &OTy::r, "q", &OTy::q, "p", &OTy::p, "o", &OTy::o, "n", &OTy::n, "m", &OTy::m, "l", &OTy::l, "k", &OTy::k, "j", &OTy::j,
		"i", &OTy::i, "h", &OTy::h, "g", &OTy::g, "f", &OTy::f, "e", &OTy::e, "d", &OTy::d, "c", &OTy::c, "b", &OTy::b, "a", &OTy::a);
};

template<> struct Jsonifier::Core<AbcTest<int64_t>> {
	using OTy = AbcTest<int64_t>;
	inline constexpr static auto parseValue = object("z", &OTy::z, "y", &OTy::y, "x", &OTy::x, "w", &OTy::w, "v", &OTy::v, "u", &OTy::u, "t", &OTy::t,
		"s", &OTy::s, "r", &OTy::r, "q", &OTy::q, "p", &OTy::p, "o", &OTy::o, "n", &OTy::n, "m", &OTy::m, "l", &OTy::l, "k", &OTy::k, "j", &OTy::j,
		"i", &OTy::i, "h", &OTy::h, "g", &OTy::g, "f", &OTy::f, "e", &OTy::e, "d", &OTy::d, "c", &OTy::c, "b", &OTy::b, "a", &OTy::a);
};

template<> struct Jsonifier::Core<AbcTest<double>> {
	using OTy = AbcTest<double>;
	inline constexpr static auto parseValue = object("z", &OTy::z, "y", &OTy::y, "x", &OTy::x, "w", &OTy::w, "v", &OTy::v, "u", &OTy::u, "t", &OTy::t,
		"s", &OTy::s, "r", &OTy::r, "q", &OTy::q, "p", &OTy::p, "o", &OTy::o, "n", &OTy::n, "m", &OTy::m, "l", &OTy::l, "k", &OTy::k, "j", &OTy::j,
		"i", &OTy::i, "h", &OTy::h, "g", &OTy::g, "f", &OTy::f, "e", &OTy::e, "d", &OTy::d, "c", &OTy::c, "b", &OTy::b, "a", &OTy::a);
};

#ifdef NDEBUG
constexpr static size_t iterations = 1000;
constexpr static size_t iterations_abc = 1000;
#else
constexpr static size_t iterations = 1;
constexpr static size_t iterations_abc = 1;
#endif

struct results {
	std::string name{};
	std::string url{};
	size_t iterations{};

	std::optional<size_t> json_byte_length_uint64{};
	std::optional<double> json_read_uint64{};
	std::optional<double> json_write_uint64{};
	std::optional<size_t> json_byte_length_double{};
	std::optional<double> json_read_double{};
	std::optional<double> json_write_double{};
	std::optional<size_t> json_byte_length_int64{};
	std::optional<double> json_read_int64{};
	std::optional<double> json_write_int64{};
	std::optional<size_t> json_byte_length_string{};
	std::optional<double> json_read_string{};
	std::optional<double> json_write_string{};
	std::optional<size_t> json_byte_length{};
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

		if (json_write_double) {
			const auto MBs = *json_byte_length_double / (*json_write_double) * 1000.0f;
			std::cout << name << " json double length: " << *json_byte_length_double << std::endl;
			std::cout << name << " json double write: " << *json_write_double << "ns, " << MBs << " MB/s\n";
		}

		if (json_read_double) {
			const auto MBs = *json_byte_length_double / (*json_read_double) * 1000.0f;
			std::cout << name << " json double read: " << *json_read_double << "ns, " << MBs << " MB/s\n";
		}

		if (json_write_string) {
			const auto MBs = *json_byte_length_string / (*json_write_string) * 1000.0f;
			std::cout << name << " json std::string length: " << *json_byte_length_string << std::endl;
			std::cout << name << " json std::string write: " << *json_write_string << "ns, " << MBs << " MB/s\n";
		}

		if (json_read_string) {
			const auto MBs = *json_byte_length_string / (*json_read_string) * 1000.0f;
			std::cout << name << " json std::string read: " << *json_read_string << "ns, " << MBs << " MB/s\n";
		}

		if (json_write_uint64) {
			const auto MBs = *json_byte_length_uint64 / (*json_write_uint64) * 1000.0f;
			std::cout << name << " json uint64_t length: " << *json_byte_length_uint64 << std::endl;
			std::cout << name << " json uint64 write: " << *json_write_uint64 << "ns, " << MBs << " MB/s\n";
		}

		if (json_read_uint64) {
			const auto MBs = *json_byte_length_uint64 / (*json_read_uint64) * 1000.0f;
			std::cout << name << " json uint64 read: " << *json_read_uint64 << "ns, " << MBs << " MB/s\n";
		}

		if (json_write_int64) {
			const auto MBs = *json_byte_length_int64 / (*json_write_int64) * 1000.0f;
			std::cout << name << " json int64_t length: " << *json_byte_length_int64 << std::endl;
			std::cout << name << " json int64 write: " << *json_write_int64 << "ns, " << MBs << " MB/s\n";
		}

		if (json_read_int64) {
			const auto MBs = *json_byte_length_int64 / (*json_read_int64) * 1000.0f;
			std::cout << name << " json int64 read: " << *json_read_int64 << "ns, " << MBs << " MB/s\n";
		}

		std::cout << "\n---" << std::endl;
	}

	std::string json_stats() {
		std::string write{};
		std::string read{};
		std::string finalString{};
		bool wasThereOneBeforeThis{ false };
		if (json_read) {
			write = json_write ? fmt::format("{}", static_cast<size_t>(*json_byte_length / (*json_write) * 1000.0f)) : "N/A";
			read = json_read ? fmt::format("{}", static_cast<size_t>(*json_byte_length / (*json_read) * 1000.0f)) : "N/A";
			finalString = fmt::format("| [**{}**]({}) | **{}** | **{}** | **{}** |", name, url, "Mixed", write, read);
			wasThereOneBeforeThis = true;
		}
		if (json_read_double) {
			if (wasThereOneBeforeThis) {
				finalString += +"\n";
			}
			write = json_write_double ? fmt::format("{}", static_cast<size_t>(*json_byte_length_double / (*json_write_double) * 1000.0f)) : "N/A";
			read = fmt::format("{}", static_cast<size_t>(*json_byte_length_double / (*json_read_double) * 1000.0f));
			finalString += fmt::format("| [**{}**]({}) | **{}** | **{}** | **{}** |", name, url, "Double", write, read);
			wasThereOneBeforeThis = true;
		}
		if (json_read_string) {
			if (wasThereOneBeforeThis) {
				finalString += +"\n";
			}
			write = json_write_string ? fmt::format("{}", static_cast<size_t>(*json_byte_length_string / (*json_write_string) * 1000.0f)) : "N/A";
			read = fmt::format("{}", static_cast<size_t>(*json_byte_length_string / (*json_read_string) * 1000.0f));
			finalString += fmt::format("| [**{}**]({}) | **{}** | **{}** | **{}** |", name, url, "String", write, read);
			wasThereOneBeforeThis = true;
		}
		if (json_read_uint64) {
			if (wasThereOneBeforeThis) {
				finalString += +"\n";
			}
			write = json_write_uint64 ? fmt::format("{}", static_cast<size_t>(*json_byte_length_uint64 / (*json_write_uint64) * 1000.0f)) : "N/A";
			read = fmt::format("{}", static_cast<size_t>(*json_byte_length_uint64 / (*json_read_uint64) * 1000.0f));
			finalString += fmt::format("| [**{}**]({}) | **{}** | **{}** | **{}** |", name, url, "Uint64", write, read);
			wasThereOneBeforeThis = true;
		}
		if (json_read_int64) {
			if (wasThereOneBeforeThis) {
				finalString += +"\n";
			}
			write = json_write_int64 ? fmt::format("{}", static_cast<size_t>(*json_byte_length_int64 / (*json_write_int64) * 1000.0f)) : "N/A";
			read = fmt::format("{}", static_cast<size_t>(*json_byte_length_int64 / (*json_read_int64) * 1000.0f));
			finalString += fmt::format("| [**{}**]({}) | **{}** | **{}** | **{}** |", name, url, "Int64", write, read);
			wasThereOneBeforeThis = true;
		}
		return finalString;
	}
};


class FileLoader {
  public:
	FileLoader(const char* filePathNew) {
		filePath = filePathNew;
		auto theStream = std::ofstream{ filePath, std::ios::out | std::ios::in };
		std::stringstream inputStream{};
		inputStream << theStream.rdbuf();
		this->fileContents = inputStream.str();
	}

	void saveFile(std::string fileToSave) {
		auto theStream = std::ofstream{ filePath, std::ios::out | std::ios::in };
		theStream << "";
		theStream.write(fileToSave.data(), fileToSave.size());
	}

	operator std::string() {
		return this->fileContents;
	}

	~FileLoader() {
		theStream.close();
	}

  protected:
	std::string filePath{};
	std::string fileContents{};
	std::ofstream theStream{};
};

template<typename Function> size_t benchmark(Function function, size_t iterationCount) {
	size_t currentLowestTime{ std::numeric_limits<uint64_t>::max() };
	for (size_t x = 0; x < iterationCount; ++x) {
		auto startTime = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now().time_since_epoch());
		function();
		auto endTime = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now().time_since_epoch());
		auto newTime = endTime - startTime;
		if (newTime.count() < currentLowestTime) {
			currentLowestTime = newTime.count();
		}
	}
	return currentLowestTime;
}

auto glaze_test() {
	std::string buffer{ json0 };

	obj_t obj{};

	results r{ "glaze", "https://github.com/stephenberry/glaze", iterations };

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
		iterations);

	r.json_byte_length = buffer.size();
	r.json_read = result;
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
		iterations);

	r.json_write = result;
	buffer.clear();
	Test<uint64_t> uint64Test{};
	glz::write_json(uint64Test, buffer);

	result = benchmark(
		[&]() {
			try {
				glz::write_json(uint64Test, buffer);
			} catch (std::runtime_error& e) {
				std::cout << "glaze Error: " << e.what() << std::endl;
			}
		},
		iterations);

	r.json_byte_length_uint64 = buffer.size();
	r.json_write_uint64 = result;

	if (auto error = glz::read_json(uint64Test, buffer)) {
		std::cout << "glaze Error: " << error << std::endl;
	}

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

	r.json_read_uint64 = result;
	buffer.clear();

	Test<int64_t> int64Test{};

	glz::write_json(int64Test, buffer);

	result = benchmark(
		[&]() {
			try {
				glz::write_json(int64Test, buffer);

			} catch (std::runtime_error& e) {
				std::cout << "glaze Error: " << e.what() << std::endl;
			}
		},
		iterations);

	r.json_byte_length_int64 = buffer.size();
	r.json_write_int64 = result;

	if (auto error = glz::read_json(int64Test, buffer)) {
		std::cout << "glaze Error: " << error << std::endl;
	}

	result = benchmark(
		[&]() {
			try {
				if (auto error = glz::read_json(int64Test, buffer)) {
					std::cout << "glaze Error: " << error << std::endl;
				}

			} catch (std::runtime_error& e) {
				std::cout << "glaze Error: " << e.what() << std::endl;
			}
		},
		iterations);

	r.json_read_int64 = result;
	buffer.clear();

	Test<std::string> stringTest{};

	glz::write_json(stringTest, buffer);

	result = benchmark(
		[&]() {
			try {
				glz::write_json(stringTest, buffer);

			} catch (std::runtime_error& e) {
				std::cout << "glaze Error: " << e.what() << std::endl;
			}
		},
		iterations);

	r.json_byte_length_string = buffer.size();
	r.json_write_string = result;

	if (auto error = glz::read_json(stringTest, buffer)) {
		std::cout << "glaze Error: " << error << std::endl;
	}

	result = benchmark(
		[&]() {
			try {
				if (auto error = glz::read_json(stringTest, buffer)) {
					std::cout << "glaze Error: " << error << std::endl;
				}

			} catch (std::runtime_error& e) {
				std::cout << "glaze Error: " << e.what() << std::endl;
			}
		},
		iterations);

	r.json_read_string = result;
	buffer.clear();

	Test<double> doubleTest{};

	glz::write_json(doubleTest, buffer);

	result = benchmark(
		[&]() {
			try {
				glz::write_json(doubleTest, buffer);

			} catch (std::runtime_error& e) {
				std::cout << "glaze Error: " << e.what() << std::endl;
			}
		},
		iterations);

	r.json_byte_length_double = buffer.size();
	r.json_write_double = result;

	if (auto error = glz::read_json(doubleTest, buffer)) {
		std::cout << "glaze Error: " << error << std::endl;
	}

	result = benchmark(
		[&]() {
			try {
				if (auto error = glz::read_json(doubleTest, buffer)) {
					std::cout << "glaze Error: " << error << std::endl;
				}

			} catch (std::runtime_error& e) {
				std::cout << "glaze Error: " << e.what() << std::endl;
			}
		},
		iterations);
	r.json_read_double = result;
	buffer.clear();
	r.print();

	return r;
}

auto glaze_single_test() {
	std::string buffer{ json0 };

	obj_t obj{};

	results r{ "glaze", "https://github.com/RealTimeChris/glaze", 1 };

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
	r.json_read = result;
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
	buffer.clear();
	Test<uint64_t> uint64Test{};
	glz::write_json(uint64Test, buffer);

	result = benchmark(
		[&]() {
			try {
				glz::write_json(uint64Test, buffer);
			} catch (std::runtime_error& e) {
				std::cout << "glaze Error: " << e.what() << std::endl;
			}
		},
		1);

	r.json_byte_length_uint64 = buffer.size();
	r.json_write_uint64 = result;

	if (auto error = glz::read_json(uint64Test, buffer)) {
		std::cout << "glaze Error: " << error << std::endl;
	}

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

	r.json_read_uint64 = result;
	buffer.clear();

	Test<int64_t> int64Test{};

	glz::write_json(int64Test, buffer);

	result = benchmark(
		[&]() {
			try {
				glz::write_json(int64Test, buffer);

			} catch (std::runtime_error& e) {
				std::cout << "glaze Error: " << e.what() << std::endl;
			}
		},
		1);

	r.json_byte_length_int64 = buffer.size();
	r.json_write_int64 = result;

	if (auto error = glz::read_json(int64Test, buffer)) {
		std::cout << "glaze Error: " << error << std::endl;
	}

	result = benchmark(
		[&]() {
			try {
				if (auto error = glz::read_json(int64Test, buffer)) {
					std::cout << "glaze Error: " << error << std::endl;
				}

			} catch (std::runtime_error& e) {
				std::cout << "glaze Error: " << e.what() << std::endl;
			}
		},
		1);

	r.json_read_int64 = result;
	buffer.clear();

	Test<std::string> stringTest{};

	glz::write_json(stringTest, buffer);

	result = benchmark(
		[&]() {
			try {
				glz::write_json(stringTest, buffer);

			} catch (std::runtime_error& e) {
				std::cout << "glaze Error: " << e.what() << std::endl;
			}
		},
		1);

	r.json_byte_length_string = buffer.size();
	r.json_write_string = result;

	if (auto error = glz::read_json(stringTest, buffer)) {
		std::cout << "glaze Error: " << error << std::endl;
	}

	result = benchmark(
		[&]() {
			try {
				if (auto error = glz::read_json(stringTest, buffer)) {
					std::cout << "glaze Error: " << error << std::endl;
				}

			} catch (std::runtime_error& e) {
				std::cout << "glaze Error: " << e.what() << std::endl;
			}
		},
		1);

	r.json_read_string = result;
	buffer.clear();

	Test<double> doubleTest{};

	glz::write_json(doubleTest, buffer);

	result = benchmark(
		[&]() {
			try {
				glz::write_json(doubleTest, buffer);

			} catch (std::runtime_error& e) {
				std::cout << "glaze Error: " << e.what() << std::endl;
			}
		},
		1);

	r.json_byte_length_double = buffer.size();
	r.json_write_double = result;

	if (auto error = glz::read_json(doubleTest, buffer)) {
		std::cout << "glaze Error: " << error << std::endl;
	}

	result = benchmark(
		[&]() {
			try {
				if (auto error = glz::read_json(doubleTest, buffer)) {
					std::cout << "glaze Error: " << error << std::endl;
				}
			} catch (std::runtime_error& e) {
				std::cout << "glaze Error: " << e.what() << std::endl;
			}
		},
		1);
	r.json_read_double = result;
	buffer.clear();
	r.print();

	return r;
}

auto glaze_abc_test() {
	std::string buffer{};

	results r{ "glaze", "https://github.com/RealTimeChris/glaze", iterations_abc };
	AbcTest<uint64_t> uint64AbcTest{};
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

	r.json_byte_length_uint64 = buffer.size();
	r.json_write_uint64 = result;

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

	r.json_read_uint64 = result;
	buffer.clear();

	AbcTest<int64_t> int64AbcTest{};

	glz::write_json(int64AbcTest, buffer);

	result = benchmark(
		[&]() {
			try {
				glz::write_json(int64AbcTest, buffer);

			} catch (std::runtime_error& e) {
				std::cout << "glaze Error: " << e.what() << std::endl;
			}
		},
		iterations_abc);

	r.json_byte_length_int64 = buffer.size();
	r.json_write_int64 = result;

	if (auto error = glz::read_json(int64AbcTest, buffer)) {
		std::cout << "glaze Error: " << error << std::endl;
	}

	result = benchmark(
		[&]() {
			try {
				if (auto error = glz::read_json(int64AbcTest, buffer)) {
					std::cout << "glaze Error: " << error << std::endl;
				}
			} catch (std::runtime_error& e) {
				std::cout << "glaze Error: " << e.what() << std::endl;
			}
		},
		iterations_abc);

	r.json_read_int64 = result;
	buffer.clear();

	AbcTest<std::string> stringAbcTest{};

	glz::write_json(stringAbcTest, buffer);

	result = benchmark(
		[&]() {
			try {
				glz::write_json(stringAbcTest, buffer);

			} catch (std::runtime_error& e) {
				std::cout << "glaze Error: " << e.what() << std::endl;
			}
		},
		iterations_abc);

	r.json_byte_length_string = buffer.size();
	r.json_write_string = result;

	if (auto error = glz::read_json(stringAbcTest, buffer)) {
		std::cout << "glaze Error: " << error << std::endl;
	}

	result = benchmark(
		[&]() {
			try {
				if (auto error = glz::read_json(stringAbcTest, buffer)) {
					std::cout << "glaze Error: " << error << std::endl;
				}

			} catch (std::runtime_error& e) {
				std::cout << "glaze Error: " << e.what() << std::endl;
			}
		},
		iterations_abc);

	r.json_read_string = result;
	buffer.clear();

	AbcTest<double> doubleAbcTest{};

	glz::write_json(doubleAbcTest, buffer);

	result = benchmark(
		[&]() {
			try {
				glz::write_json(doubleAbcTest, buffer);

			} catch (std::runtime_error& e) {
				std::cout << "glaze Error: " << e.what() << std::endl;
			}
		},
		iterations_abc);

	r.json_byte_length_double = buffer.size();
	r.json_write_double = result;

	if (auto error = glz::read_json(doubleAbcTest, buffer)) {
		std::cout << "glaze Error: " << error << std::endl;
	}

	result = benchmark(
		[&]() {
			try {
				if (auto error = glz::read_json(doubleAbcTest, buffer)) {
					std::cout << "glaze Error: " << error << std::endl;
				}

			} catch (std::runtime_error& e) {
				std::cout << "glaze Error: " << e.what() << std::endl;
			}
		},
		iterations_abc);
	r.json_read_double = result;
	buffer.clear();
	r.print();

	return r;
}

auto Jsonifier_test() {
	std::string buffer{ json0 };

	obj_t obj{};

	results r{ "Jsonifier", "https://github.com/RealTimeChris/Jsonifier", iterations };
	Jsonifier::JsonifierCore jsonifier{};

	jsonifier.parseJson<true>(obj, buffer);
	auto result = benchmark(
		[&]() {
			try {
				jsonifier.parseJson<true>(obj, buffer);
			} catch (std::runtime_error& e) {
				std::cout << "Jsonifier Error: " << e.what() << std::endl;
			}
		},
		iterations);

	r.json_byte_length = buffer.size();
	r.json_read = result;
	buffer.clear();

	jsonifier.serializeJson(obj, buffer);
	result = benchmark(
		[&]() {
			try {
				jsonifier.serializeJson(obj, buffer);
			} catch (std::runtime_error& e) {
				std::cout << "Jsonifier Error: " << e.what() << std::endl;
			}
		},
		iterations);

	r.json_write = result;
	buffer.clear();
	Test<uint64_t> uint64Test{};
	jsonifier.serializeJson(uint64Test, buffer);

	result = benchmark(
		[&]() {
			try {
				jsonifier.serializeJson(uint64Test, buffer);
			} catch (std::runtime_error& e) {
				std::cout << "Jsonifier Error: " << e.what() << std::endl;
			}
		},
		iterations);

	r.json_byte_length_uint64 = buffer.size();
	r.json_write_uint64 = result;

	jsonifier.parseJson<true>(uint64Test, buffer);

	result = benchmark(
		[&]() {
			try {
				jsonifier.parseJson<true>(uint64Test, buffer);
			} catch (std::runtime_error& e) {
				std::cout << "Jsonifier Error: " << e.what() << std::endl;
			}
		},
		iterations);

	r.json_read_uint64 = result;
	buffer.clear();

	Test<int64_t> int64Test{};

	jsonifier.serializeJson(int64Test, buffer);

	result = benchmark(
		[&]() {
			try {
				jsonifier.serializeJson(int64Test, buffer);

			} catch (std::runtime_error& e) {
				std::cout << "Jsonifier Error: " << e.what() << std::endl;
			}
		},
		iterations);

	r.json_byte_length_int64 = buffer.size();
	r.json_write_int64 = result;

	jsonifier.parseJson<true>(int64Test, buffer);

	result = benchmark(
		[&]() {
			try {
				jsonifier.parseJson<true>(int64Test, buffer);

			} catch (std::runtime_error& e) {
				std::cout << "Jsonifier Error: " << e.what() << std::endl;
			}
		},
		iterations);

	r.json_read_int64 = result;
	buffer.clear();

	Test<std::string> stringTest{};

	jsonifier.serializeJson(stringTest, buffer);

	result = benchmark(
		[&]() {
			try {
				jsonifier.serializeJson(stringTest, buffer);

			} catch (std::runtime_error& e) {
				std::cout << "Jsonifier Error: " << e.what() << std::endl;
			}
		},
		iterations);

	r.json_byte_length_string = buffer.size();
	r.json_write_string = result;

	jsonifier.parseJson<true>(stringTest, buffer);

	result = benchmark(
		[&]() {
			try {
				jsonifier.parseJson<true>(stringTest, buffer);

			} catch (std::runtime_error& e) {
				std::cout << "Jsonifier Error: " << e.what() << std::endl;
			}
		},
		iterations);

	r.json_read_string = result;
	buffer.clear();

	Test<double> doubleTest{};

	jsonifier.serializeJson(doubleTest, buffer);

	result = benchmark(
		[&]() {
			try {
				jsonifier.serializeJson(doubleTest, buffer);

			} catch (std::runtime_error& e) {
				std::cout << "Jsonifier Error: " << e.what() << std::endl;
			}
		},
		iterations);

	r.json_byte_length_double = buffer.size();
	r.json_write_double = result;

	jsonifier.parseJson<true>(doubleTest, buffer);

	result = benchmark(
		[&]() {
			try {
				jsonifier.parseJson<true>(doubleTest, buffer);

			} catch (std::runtime_error& e) {
				std::cout << "Jsonifier Error: " << e.what() << std::endl;
			}
		},
		iterations);
	r.json_read_double = result;
	buffer.clear();
	r.print();

	return r;
}

auto Jsonifier_single_test() {
	std::string buffer{ json0 };

	obj_t obj{};

	results r{ "Jsonifier", "https://github.com/RealTimeChris/Jsonifier", 1 };
	Jsonifier::JsonifierCore jsonifier{};

	jsonifier.parseJson<true>(obj, buffer);
	auto result = benchmark(
		[&]() {
			try {
				jsonifier.parseJson<true>(obj, buffer);
			} catch (std::runtime_error& e) {
				std::cout << "Jsonifier Error: " << e.what() << std::endl;
			}
		},
		1);

	r.json_byte_length = buffer.size();
	r.json_read = result;
	buffer.clear();

	jsonifier.serializeJson(obj, buffer);
	result = benchmark(
		[&]() {
			try {
				jsonifier.serializeJson(obj, buffer);
			} catch (std::runtime_error& e) {
				std::cout << "Jsonifier Error: " << e.what() << std::endl;
			}
		},
		1);

	r.json_write = result;
	buffer.clear();
	Test<uint64_t> uint64Test{};
	jsonifier.serializeJson(uint64Test, buffer);

	result = benchmark(
		[&]() {
			try {
				jsonifier.serializeJson(uint64Test, buffer);
			} catch (std::runtime_error& e) {
				std::cout << "Jsonifier Error: " << e.what() << std::endl;
			}
		},
		1);

	r.json_byte_length_uint64 = buffer.size();
	r.json_write_uint64 = result;

	jsonifier.parseJson<true>(uint64Test, buffer);

	result = benchmark(
		[&]() {
			try {
				jsonifier.parseJson<true>(uint64Test, buffer);
			} catch (std::runtime_error& e) {
				std::cout << "Jsonifier Error: " << e.what() << std::endl;
			}
		},
		1);

	r.json_read_uint64 = result;
	buffer.clear();

	Test<int64_t> int64Test{};

	jsonifier.serializeJson(int64Test, buffer);

	result = benchmark(
		[&]() {
			try {
				jsonifier.serializeJson(int64Test, buffer);

			} catch (std::runtime_error& e) {
				std::cout << "Jsonifier Error: " << e.what() << std::endl;
			}
		},
		1);

	r.json_byte_length_int64 = buffer.size();
	r.json_write_int64 = result;

	jsonifier.parseJson<true>(int64Test, buffer);

	result = benchmark(
		[&]() {
			try {
				jsonifier.parseJson<true>(int64Test, buffer);

			} catch (std::runtime_error& e) {
				std::cout << "Jsonifier Error: " << e.what() << std::endl;
			}
		},
		1);

	r.json_read_int64 = result;
	buffer.clear();

	Test<std::string> stringTest{};

	jsonifier.serializeJson(stringTest, buffer);

	result = benchmark(
		[&]() {
			try {
				jsonifier.serializeJson(stringTest, buffer);

			} catch (std::runtime_error& e) {
				std::cout << "Jsonifier Error: " << e.what() << std::endl;
			}
		},
		1);

	r.json_byte_length_string = buffer.size();
	r.json_write_string = result;

	jsonifier.parseJson<true>(stringTest, buffer);

	result = benchmark(
		[&]() {
			try {
				jsonifier.parseJson<true>(stringTest, buffer);

			} catch (std::runtime_error& e) {
				std::cout << "Jsonifier Error: " << e.what() << std::endl;
			}
		},
		1);

	r.json_read_string = result;
	buffer.clear();

	Test<double> doubleTest{};

	jsonifier.serializeJson(doubleTest, buffer);

	result = benchmark(
		[&]() {
			try {
				jsonifier.serializeJson(doubleTest, buffer);

			} catch (std::runtime_error& e) {
				std::cout << "Jsonifier Error: " << e.what() << std::endl;
			}
		},
		1);

	r.json_byte_length_double = buffer.size();
	r.json_write_double = result;

	jsonifier.parseJson<true>(doubleTest, buffer);

	result = benchmark(
		[&]() {
			try {
				jsonifier.parseJson<true>(doubleTest, buffer);

			} catch (std::runtime_error& e) {
				std::cout << "Jsonifier Error: " << e.what() << std::endl;
			}
		},
		1);
	r.json_read_double = result;
	buffer.clear();
	r.print();

	return r;
}

auto Jsonifier_abc_test() {
	std::string buffer{};

	results r{ "Jsonifier", "https://github.com/RealTimeChris/Jsonifier", iterations_abc };
	Jsonifier::JsonifierCore jsonifier{};
	AbcTest<uint64_t> uint64AbcTest{};
	jsonifier.serializeJson(uint64AbcTest, buffer);

	auto result = benchmark(
		[&]() {
			try {
				jsonifier.serializeJson(uint64AbcTest, buffer);
			} catch (std::runtime_error& e) {
				std::cout << "Jsonifier Error: " << e.what() << std::endl;
			}
		},
		iterations_abc);

	r.json_byte_length_uint64 = buffer.size();
	r.json_write_uint64 = result;

	jsonifier.parseJson<true>(uint64AbcTest, buffer);

	result = benchmark(
		[&]() {
			try {
				jsonifier.parseJson<true>(uint64AbcTest, buffer);
			} catch (std::runtime_error& e) {
				std::cout << "Jsonifier Error: " << e.what() << std::endl;
			}
		},
		iterations_abc);

	r.json_read_uint64 = result;
	buffer.clear();

	AbcTest<int64_t> int64AbcTest{};

	jsonifier.serializeJson(int64AbcTest, buffer);

	result = benchmark(
		[&]() {
			try {
				jsonifier.serializeJson(int64AbcTest, buffer);

			} catch (std::runtime_error& e) {
				std::cout << "Jsonifier Error: " << e.what() << std::endl;
			}
		},
		iterations_abc);

	r.json_byte_length_int64 = buffer.size();
	r.json_write_int64 = result;

	jsonifier.parseJson<true>(int64AbcTest, buffer);

	result = benchmark(
		[&]() {
			try {
				jsonifier.parseJson<true>(int64AbcTest, buffer);

			} catch (std::runtime_error& e) {
				std::cout << "Jsonifier Error: " << e.what() << std::endl;
			}
		},
		iterations_abc);

	r.json_read_int64 = result;
	buffer.clear();

	AbcTest<std::string> stringAbcTest{};

	jsonifier.serializeJson(stringAbcTest, buffer);

	result = benchmark(
		[&]() {
			try {
				jsonifier.serializeJson(stringAbcTest, buffer);

			} catch (std::runtime_error& e) {
				std::cout << "Jsonifier Error: " << e.what() << std::endl;
			}
		},
		iterations_abc);

	r.json_byte_length_string = buffer.size();
	r.json_write_string = result;

	jsonifier.parseJson<true>(stringAbcTest, buffer);

	result = benchmark(
		[&]() {
			try {
				jsonifier.parseJson<true>(stringAbcTest, buffer);

			} catch (std::runtime_error& e) {
				std::cout << "Jsonifier Error: " << e.what() << std::endl;
			}
		},
		iterations_abc);

	r.json_read_string = result;
	buffer.clear();

	AbcTest<double> doubleAbcTest{};

	jsonifier.serializeJson(doubleAbcTest, buffer);

	result = benchmark(
		[&]() {
			try {
				jsonifier.serializeJson(doubleAbcTest, buffer);

			} catch (std::runtime_error& e) {
				std::cout << "Jsonifier Error: " << e.what() << std::endl;
			}
		},
		iterations_abc);

	r.json_byte_length_double = buffer.size();
	r.json_write_double = result;

	jsonifier.parseJson<true>(doubleAbcTest, buffer);

	result = benchmark(
		[&]() {
			try {
				jsonifier.parseJson<true>(doubleAbcTest, buffer);

			} catch (std::runtime_error& e) {
				std::cout << "Jsonifier Error: " << e.what() << std::endl;
			}
		},
		iterations_abc);
	r.json_read_double = result;
	buffer.clear();
	r.print();

	return r;
}

#include "simdjson.h"

using namespace simdjson;

struct on_demand {
	bool read_in_order(obj_t& obj, const padded_string& json);
	bool readDouble(Test<double>& obj, const padded_string& json);
	bool readUint64(Test<uint64_t>& obj, const padded_string& json);
	bool readInt64(Test<int64_t>& obj, const padded_string& json);
	bool readString(Test<std::string>& obj, const padded_string& json);

  protected:
	ondemand::parser parser{};
};

template<typename OTy, typename OTy2> void simdPull(const char* x, OTy2& obj, simdjson::ondemand::document& doc) {
	ondemand::array xNew = doc[x];
	for (const OTy& value: xNew) {
		obj.x.emplace_back(value);
	}
}

#define SIMD_DOUBLE_PULL(x) \
	{ \
		ondemand::array newX = doc[#x].get_array().value(); \
		if (obj.x.size() < 1000) { \
			obj.x.resize(1000); \
		} \
		int32_t currentIndex{}; \
		for (double value: newX) { \
			obj.x[currentIndex] = value; \
			++currentIndex; \
		} \
	}

#define SIMD_UINT64_PULL(x) \
	{ \
		ondemand::array newX = doc[#x].get_array().value(); \
		if (obj.x.size() < 1000) { \
			obj.x.resize(1000); \
		} \
		int32_t currentIndex{}; \
		for (uint64_t value: newX) { \
			obj.x[currentIndex] = value; \
			++currentIndex; \
		} \
	}

#define SIMD_INT64_PULL(x) \
	{ \
		ondemand::array newX = doc[#x].get_array().value(); \
		if (obj.x.size() < 1000) { \
			obj.x.resize(1000); \
		} \
		int32_t currentIndex{}; \
		for (int64_t value: newX) { \
			obj.x[currentIndex] = value; \
			++currentIndex; \
		} \
	}

#define SIMD_STRING_PULL(x) \
	{ \
		ondemand::array newX = doc[#x].get_array().value(); \
		if (obj.x.size() < 1000) { \
			obj.x.resize(1000); \
		} \
		int32_t currentIndex{}; \
		for (std::string_view value: newX) { \
			obj.x[currentIndex] = static_cast<std::string>(value); \
			++currentIndex; \
		} \
	}

bool on_demand::readDouble(Test<double>& obj, const padded_string& json) {
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

bool on_demand::readUint64(Test<uint64_t>& obj, const padded_string& json) {
	ondemand::document doc = parser.iterate(json).value();

	SIMD_UINT64_PULL(a);
	SIMD_UINT64_PULL(b);
	SIMD_UINT64_PULL(c);
	SIMD_UINT64_PULL(d);
	SIMD_UINT64_PULL(e);
	SIMD_UINT64_PULL(f);
	SIMD_UINT64_PULL(g);
	SIMD_UINT64_PULL(h);
	SIMD_UINT64_PULL(i);
	SIMD_UINT64_PULL(j);
	SIMD_UINT64_PULL(k);
	SIMD_UINT64_PULL(l);
	SIMD_UINT64_PULL(m);
	SIMD_UINT64_PULL(n);
	SIMD_UINT64_PULL(o);
	SIMD_UINT64_PULL(p);
	SIMD_UINT64_PULL(q);
	SIMD_UINT64_PULL(r);
	SIMD_UINT64_PULL(s);
	SIMD_UINT64_PULL(t);
	SIMD_UINT64_PULL(u);
	SIMD_UINT64_PULL(v);
	SIMD_UINT64_PULL(w);
	SIMD_UINT64_PULL(x);
	SIMD_UINT64_PULL(y);
	SIMD_UINT64_PULL(z);

	return false;
}

bool on_demand::readInt64(Test<int64_t>& obj, const padded_string& json) {
	ondemand::document doc = parser.iterate(json).value();

	SIMD_INT64_PULL(a);
	SIMD_INT64_PULL(b);
	SIMD_INT64_PULL(c);
	SIMD_INT64_PULL(d);
	SIMD_INT64_PULL(e);
	SIMD_INT64_PULL(f);
	SIMD_INT64_PULL(g);
	SIMD_INT64_PULL(h);
	SIMD_INT64_PULL(i);
	SIMD_INT64_PULL(j);
	SIMD_INT64_PULL(k);
	SIMD_INT64_PULL(l);
	SIMD_INT64_PULL(m);
	SIMD_INT64_PULL(n);
	SIMD_INT64_PULL(o);
	SIMD_INT64_PULL(p);
	SIMD_INT64_PULL(q);
	SIMD_INT64_PULL(r);
	SIMD_INT64_PULL(s);
	SIMD_INT64_PULL(t);
	SIMD_INT64_PULL(u);
	SIMD_INT64_PULL(v);
	SIMD_INT64_PULL(w);
	SIMD_INT64_PULL(x);
	SIMD_INT64_PULL(y);
	SIMD_INT64_PULL(z);

	return false;
}

bool on_demand::readString(Test<std::string>& obj, const padded_string& json) {
	ondemand::document doc = parser.iterate(json).value();

	SIMD_STRING_PULL(a);
	SIMD_STRING_PULL(b);
	SIMD_STRING_PULL(c);
	SIMD_STRING_PULL(d);
	SIMD_STRING_PULL(e);
	SIMD_STRING_PULL(f);
	SIMD_STRING_PULL(g);
	SIMD_STRING_PULL(h);
	SIMD_STRING_PULL(i);
	SIMD_STRING_PULL(j);
	SIMD_STRING_PULL(k);
	SIMD_STRING_PULL(l);
	SIMD_STRING_PULL(m);
	SIMD_STRING_PULL(n);
	SIMD_STRING_PULL(o);
	SIMD_STRING_PULL(p);
	SIMD_STRING_PULL(q);
	SIMD_STRING_PULL(r);
	SIMD_STRING_PULL(s);
	SIMD_STRING_PULL(t);
	SIMD_STRING_PULL(u);
	SIMD_STRING_PULL(v);
	SIMD_STRING_PULL(w);
	SIMD_STRING_PULL(x);
	SIMD_STRING_PULL(y);
	SIMD_STRING_PULL(z);

	return false;
}

bool on_demand::read_in_order(obj_t& obj, const padded_string& json) {
	auto doc = parser.iterate(json);
	ondemand::object fixed_object = doc["fixed_object"];

	ondemand::array int_array = fixed_object["int_array"];
	obj.fixed_object.int_array.clear();
	for (int64_t x: int_array) {
		obj.fixed_object.int_array.emplace_back(x);
	}

	ondemand::array float_array = fixed_object["float_array"];
	obj.fixed_object.float_array.clear();
	for (double x: float_array) {
		obj.fixed_object.float_array.emplace_back(static_cast<float>(x));
	}

	ondemand::array double_array = fixed_object["double_array"];
	obj.fixed_object.double_array.clear();
	for (double x: double_array) {
		obj.fixed_object.double_array.emplace_back(x);
	}

	ondemand::object fixed_name_object = doc["fixed_name_object"];
	obj.fixed_name_object.name0 = std::string_view(fixed_name_object["name0"]);
	obj.fixed_name_object.name1 = std::string_view(fixed_name_object["name1"]);
	obj.fixed_name_object.name2 = std::string_view(fixed_name_object["name2"]);
	obj.fixed_name_object.name3 = std::string_view(fixed_name_object["name3"]);
	obj.fixed_name_object.name4 = std::string_view(fixed_name_object["name4"]);

	ondemand::object another_object = doc["another_object"];
	obj.another_object.string = std::string_view(another_object["string"]);
	obj.another_object.another_string = std::string_view(another_object["another_string"]);
	obj.another_object.boolean = bool(another_object["boolean"]);

	ondemand::object nested_object = another_object["nested_object"];
	ondemand::array v3s = nested_object["v3s"];
	obj.another_object.nested_object.v3s.clear();
	for (ondemand::array v3: v3s) {
		size_t i = 0;
		auto& back = obj.another_object.nested_object.v3s.emplace_back();
		for (double x: v3) {
			back[i++] = x;
		}
	}

	obj.another_object.nested_object.id = std::string_view(nested_object["id"]);

	ondemand::array string_array = doc["string_array"];
	obj.string_array.resize(string_array.count_elements());
	size_t index = 0;
	for (std::string_view x: string_array) {
		obj.string_array[index++] = x;
	}

	obj.string = std::string_view(doc["string"]);
	obj.number = double(doc["number"]);
	obj.boolean = bool(doc["boolean"]);
	obj.another_bool = bool(doc["another_bool"]);

	return false;
}

auto simdjson_test() {
	std::string buffer{ json0 };

	on_demand parser{};

	obj_t obj{};

	auto error = parser.read_in_order(obj, buffer);

	auto result = benchmark(
		[&]() {
			const auto error = parser.read_in_order(obj, buffer);
			if (error) {
				std::cerr << "simdjson error" << std::endl;
			}
		},
		iterations);

	results r{ "simdjson (on demand)", "https://github.com/simdjson/simdjson", iterations };

	r.json_byte_length = buffer.size();
	r.json_read = result;

	Test<double> objDouble{};
	buffer.clear();
	buffer = glz::write_json(objDouble);

	error = parser.readDouble(objDouble, buffer);

	result = benchmark(
		[&]() {
			const auto error = parser.readDouble(objDouble, buffer);
			if (error) {
				std::cerr << "simdjson error" << std::endl;
			}
		},
		iterations);

	r.json_byte_length_double = buffer.size();
	r.json_read_double = result;

	Test<std::string> objString{};
	buffer.clear();
	buffer = glz::write_json(objString);

	error = parser.readString(objString, buffer);

	result = benchmark(
		[&]() {
			const auto error = parser.readString(objString, buffer);
			if (error) {
				std::cerr << "simdjson error" << std::endl;
			}
		},
		iterations);

	r.json_byte_length_string = buffer.size();
	r.json_read_string = result;

	Test<int64_t> objInt64{};
	buffer.clear();
	buffer = glz::write_json(objInt64);

	error = parser.readInt64(objInt64, buffer);

	result = benchmark(
		[&]() {
			const auto error = parser.readInt64(objInt64, buffer);
			if (error) {
				std::cerr << "simdjson error" << std::endl;
			}
		},
		iterations);

	r.json_byte_length_int64 = buffer.size();
	r.json_read_int64 = result;

	Test<uint64_t> objUint64{};
	buffer.clear();
	buffer = glz::write_json(objUint64);

	error = parser.readUint64(objUint64, buffer);

	result = benchmark(
		[&]() {
			const auto error = parser.readUint64(objUint64, buffer);
			if (error) {
				std::cerr << "simdjson error" << std::endl;
			}
		},
		iterations);

	r.json_byte_length_uint64 = buffer.size();
	r.json_read_uint64 = result;

	r.print();

	return r;
}

auto simdjson_single_test() {
	std::string buffer{ json0 };

	on_demand parser{};

	obj_t obj{};

	const auto error = parser.read_in_order(obj, buffer);

	auto result = benchmark(
		[&]() {
			const auto error = parser.read_in_order(obj, buffer);
			if (error) {
				std::cerr << "simdjson error" << std::endl;
			}
		},
		1);

	results r{ "simdjson (on demand)", "https://github.com/simdjson/simdjson", 1 };

	r.json_byte_length = buffer.size();
	r.json_read = result;

	Test<double> objDouble{};
	buffer.clear();
	buffer = glz::write_json(objDouble);

	auto errorNew = parser.readDouble(objDouble, buffer);

	result = benchmark(
		[&]() {
			const auto error = parser.readDouble(objDouble, buffer);
			if (error) {
				std::cerr << "simdjson error" << std::endl;
			}
		},
		1);

	r.json_byte_length_double = buffer.size();
	r.json_read_double = result;

	Test<std::string> objString{};
	buffer.clear();
	buffer = glz::write_json(objString);

	errorNew = parser.readString(objString, buffer);

	result = benchmark(
		[&]() {
			const auto error = parser.readString(objString, buffer);
			if (error) {
				std::cerr << "simdjson error" << std::endl;
			}
		},
		1);

	r.json_byte_length_string = buffer.size();
	r.json_read_string = result;

	Test<int64_t> objInt64{};
	buffer.clear();
	buffer = glz::write_json(objInt64);

	errorNew = parser.readInt64(objInt64, buffer);

	result = benchmark(
		[&]() {
			const auto error = parser.readInt64(objInt64, buffer);
			if (error) {
				std::cerr << "simdjson error" << std::endl;
			}
		},
		1);

	r.json_byte_length_int64 = buffer.size();
	r.json_read_int64 = result;

	Test<uint64_t> objUint64{};
	buffer.clear();
	buffer = glz::write_json(objUint64);

	errorNew = parser.readUint64(objUint64, buffer);

	result = benchmark(
		[&]() {
			const auto error = parser.readUint64(objUint64, buffer);
			if (error) {
				std::cerr << "simdjson error" << std::endl;
			}
		},
		1);

	r.json_byte_length_uint64 = buffer.size();
	r.json_read_uint64 = result;


	r.print();

	return r;
}

struct on_demand_abc {
	bool readDouble(AbcTest<double>& obj, const padded_string& json);
	bool readUint64(AbcTest<uint64_t>& obj, const padded_string& json);
	bool readInt64(AbcTest<int64_t>& obj, const padded_string& json);
	bool readString(AbcTest<std::string>& obj, const padded_string& json);

  protected:
	ondemand::parser parser{};
};

bool on_demand_abc::readDouble(AbcTest<double>& obj, const padded_string& json) {
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

bool on_demand_abc ::readUint64(AbcTest<uint64_t>& obj, const padded_string& json) {
	ondemand::document doc = parser.iterate(json).value();

	SIMD_UINT64_PULL(a);
	SIMD_UINT64_PULL(b);
	SIMD_UINT64_PULL(c);
	SIMD_UINT64_PULL(d);
	SIMD_UINT64_PULL(e);
	SIMD_UINT64_PULL(f);
	SIMD_UINT64_PULL(g);
	SIMD_UINT64_PULL(h);
	SIMD_UINT64_PULL(i);
	SIMD_UINT64_PULL(j);
	SIMD_UINT64_PULL(k);
	SIMD_UINT64_PULL(l);
	SIMD_UINT64_PULL(m);
	SIMD_UINT64_PULL(n);
	SIMD_UINT64_PULL(o);
	SIMD_UINT64_PULL(p);
	SIMD_UINT64_PULL(q);
	SIMD_UINT64_PULL(r);
	SIMD_UINT64_PULL(s);
	SIMD_UINT64_PULL(t);
	SIMD_UINT64_PULL(u);
	SIMD_UINT64_PULL(v);
	SIMD_UINT64_PULL(w);
	SIMD_UINT64_PULL(x);
	SIMD_UINT64_PULL(y);
	SIMD_UINT64_PULL(z);

	return false;
}

bool on_demand_abc ::readInt64(AbcTest<int64_t>& obj, const padded_string& json) {
	ondemand ::document doc = parser.iterate(json).value();

	SIMD_INT64_PULL(a);
	SIMD_INT64_PULL(b);
	SIMD_INT64_PULL(c);
	SIMD_INT64_PULL(d);
	SIMD_INT64_PULL(e);
	SIMD_INT64_PULL(f);
	SIMD_INT64_PULL(g);
	SIMD_INT64_PULL(h);
	SIMD_INT64_PULL(i);
	SIMD_INT64_PULL(j);
	SIMD_INT64_PULL(k);
	SIMD_INT64_PULL(l);
	SIMD_INT64_PULL(m);
	SIMD_INT64_PULL(n);
	SIMD_INT64_PULL(o);
	SIMD_INT64_PULL(p);
	SIMD_INT64_PULL(q);
	SIMD_INT64_PULL(r);
	SIMD_INT64_PULL(s);
	SIMD_INT64_PULL(t);
	SIMD_INT64_PULL(u);
	SIMD_INT64_PULL(v);
	SIMD_INT64_PULL(w);
	SIMD_INT64_PULL(x);
	SIMD_INT64_PULL(y);
	SIMD_INT64_PULL(z);

	return false;
}

bool on_demand_abc ::readString(AbcTest<std::string>& obj, const padded_string& json) {
	ondemand ::document doc = parser.iterate(json).value();

	SIMD_STRING_PULL(a);
	SIMD_STRING_PULL(b);
	SIMD_STRING_PULL(c);
	SIMD_STRING_PULL(d);
	SIMD_STRING_PULL(e);
	SIMD_STRING_PULL(f);
	SIMD_STRING_PULL(g);
	SIMD_STRING_PULL(h);
	SIMD_STRING_PULL(i);
	SIMD_STRING_PULL(j);
	SIMD_STRING_PULL(k);
	SIMD_STRING_PULL(l);
	SIMD_STRING_PULL(m);
	SIMD_STRING_PULL(n);
	SIMD_STRING_PULL(o);
	SIMD_STRING_PULL(p);
	SIMD_STRING_PULL(q);
	SIMD_STRING_PULL(r);
	SIMD_STRING_PULL(s);
	SIMD_STRING_PULL(t);
	SIMD_STRING_PULL(u);
	SIMD_STRING_PULL(v);
	SIMD_STRING_PULL(w);
	SIMD_STRING_PULL(x);
	SIMD_STRING_PULL(y);
	SIMD_STRING_PULL(z);

	return false;
}

auto simdjson_abc_test() {
	on_demand_abc parser{};

	AbcTest<double> obj{};

	std::string buffer = glz::write_json(obj);

	auto result = benchmark(
		[&]() {
			const auto error = parser.readDouble(obj, buffer);
			if (error) {
				std::cerr << "simdjson error" << std::endl;
			}
		},
		iterations_abc);

	results r{ "simdjson (on demand)", "https://github.com/simdjson/simdjson", iterations_abc };

	r.json_byte_length_double = buffer.size();
	r.json_read_double = result;

	AbcTest<std::string> objString{};
	buffer.clear();
	buffer = glz::write_json(objString);

	result = benchmark(
		[&]() {
			const auto error = parser.readString(objString, buffer);
			if (error) {
				std::cerr << "simdjson error" << std::endl;
			}
		},
		iterations_abc);

	r.json_byte_length_string = buffer.size();
	r.json_read_string = result;

	AbcTest<int64_t> objInt64{};
	buffer.clear();
	buffer = glz::write_json(objInt64);

	result = benchmark(
		[&]() {
			const auto error = parser.readInt64(objInt64, buffer);
			if (error) {
				std::cerr << "simdjson error" << std::endl;
			}
		},
		iterations_abc);

	r.json_byte_length_int64 = buffer.size();
	r.json_read_int64 = result;

	AbcTest<uint64_t> objUint64{};
	buffer.clear();
	buffer = glz::write_json(objUint64);

	result = benchmark(
		[&]() {
			const auto error = parser.readUint64(objUint64, buffer);
			if (error) {
				std::cerr << "simdjson error" << std::endl;
			}
		},
		iterations_abc);

	r.json_byte_length_uint64 = buffer.size();
	r.json_read_uint64 = result;

	r.print();

	return r;
}

static std::string table_header = R"(
| Library                                           | Test       | Write (MB/s) | Read (MB/s) |
| ------------------------------------------------- | ---------- | ------------ | ----------- |)";

std::string regular_test() {
	std::vector<results> results{};
	results.emplace_back(glaze_test());
	results.emplace_back(Jsonifier_test());
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
	results.emplace_back(glaze_abc_test());
	results.emplace_back(Jsonifier_abc_test());
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
	results.emplace_back(glaze_single_test());
	results.emplace_back(Jsonifier_single_test());
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
		bool result{};
		std::vector<std::string> vector01{};
		vector01.resize(1024 * 1024);
		vector01[1024 * 1024 - 2] = "TESTING";
		std::vector<std::string> vector02{};
		vector02.resize(1024 * 1024);
		std::cout << "BENCHMARK 01: std::vector<std::string> of size 1024*1024: "
				  << benchmark(
						 [&] {
							 result = vector01 == vector02;
						 },
						 1024)
				  << std::endl;
		std::cout << "EQUALS: " << result << std::endl;

		Jsonifier::Vector<std::string> vector03{};
		vector03.resize(1024 * 1024);
		vector03[1024 * 1024 - 2] = "TESTING";
		Jsonifier::Vector<std::string> vector04{};
		vector04.resize(1024 * 1024);
		std::cout << "BENCHMARK 02: Jsonifier::Vector<std::string> of size 1024*1024: "
				  << benchmark(
						 [&] {
							 result = vector03 == vector04;
						 },
						 1024)
				  << std::endl;
		std::cout << "EQUALS: " << result << std::endl;
		auto singlTestResults = single_test();
		auto multiTestResults = regular_test();
		auto abcTestResults = abc_test();
#ifdef _WIN32
		FileLoader fileLoader{ "../../../ReadMe.md" };
#else
		FileLoader fileLoader{ "../ReadMe.md" };
#endif
		std::string newString = fileLoader;
		std::string section01 =
			newString.substr(0, newString.find("Single Iteration Test Results:") + std::string("Single Iteration Test Results:").size() + 1);
		auto section02 = newString.substr(newString.find("> 1000 iterations on a 6 core (Intel i7 8700k)"),
			newString.find("performance regardless of the JSON document's scale.") +
				std::string{ "performance regardless of the JSON document's scale." }.size() -
				newString.find("> 1000 iterations on a 6 core (Intel i7 8700k)"));

		std::string newerString = section01 + singlTestResults + "\n\nMulti Iteration Test Results:\n" + multiTestResults + "\n" + section02;
		newerString += "\n" + abcTestResults + "\n> 1001 iterations on a 6 core (Intel i7 8700k)";
		fileLoader.saveFile(newerString);
	} catch (std::exception& e) {
		std::cout << e.what() << std::endl;
	}
	return 0;
}
