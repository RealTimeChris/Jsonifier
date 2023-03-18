# Jsonifier
[![Build Jsonifier](https://img.shields.io/github/actions/workflow/status/RealTimeChris/Jsonifier/Release.yml?branch=main&style=plastic&color=purple)](https://github.com/RealTimeChris/Jsonifier/actions/workflows/Release.yml)
![Commit Activity](https://img.shields.io/github/commit-activity/m/realtimechris/Jsonifier?color=green&label=Commits&style=plastic)
![Lines of code](https://img.shields.io/tokei/lines/github/RealTimeChris/Jsonifier?&style=plastic&label=Lines%20of%20Code)


## A few classes for serializing and parsing objects into/from JSON strings - very rapidly (more rapidly than any other library).
---
## [Benchmarks](https://github.com/RealTimeChris/Json-Performance)
----
## Usage - Serialization/Parsing
- Create a specialization of the `Jsonifier::Core` class template as follows...
----
```cpp

struct fixed_object_t {
	std::vector<int> int_array;
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

template<> struct Jsonifier::Core<fixed_object_t> {
	using T = fixed_object_t;
	static constexpr auto value = object("int_array", &T::int_array, "float_array", &T::float_array, "double_array", &T::double_array);
};

template<> struct Jsonifier::Core<fixed_name_object_t> {
	using T = fixed_name_object_t;
	static constexpr auto value = object("name0", &T::name0, "name1", &T::name1, "name2", &T::name2, "name3", &T::name3, "name4", &T::name4);
};

template<> struct Jsonifier::Core<nested_object_t> {
	using T = nested_object_t;
	static constexpr auto value = object("v3s", &T::v3s, "id", &T::id);
};

template<> struct Jsonifier::Core<another_object_t> {
	using T = another_object_t;
	static constexpr auto value =
		object("string", &T::string, "another_string", &T::another_string, "boolean", &T::boolean, "nested_object", &T::nested_object);
};

template<> struct Jsonifier::Core<obj_t> {
	using T = obj_t;
	static constexpr auto value =
		object("fixed_object", &T::fixed_object, "fixed_name_object", &T::fixed_name_object, "another_object", &T::another_object, "string_array",
			&T::string_array, "string", &T::string, "number", &T::number, "boolean", &T::boolean, "another_bool", &T::another_bool);
};

```
## Usage - Serialization
- Create an instance of the `Jsonifier::Serializer` class, and pass to its function `serializeJson()` a reference to the intended serialization target, along with a reference to a `std::string` or equivalent, to be serialized into, as follows...
- Note: You can save serialization time by reusing a previously-allocated buffer, that has been used for previous serializations.
```cpp
std::string buffer{ };

obj_t obj{};

Jsonifier::Serializer serializer{};
serializer.serializeJson(obj, buffer);
```
## Usage - Parsing
- Create an instance of the `Jsonifier::Parser` class, and pass to its function `parseJson()` a reference to the intended parsing target, along with a reference to a `std::string` or equivalent, to be parsed from, as follows...
- Note: You can save parsing time by reusing a previously-allocated object, that has been used for previous parses.
```cpp
std::string buffer{ json0 };

obj_t obj{};

Jsonifier::Parser parser{ buffer };
parser.parseJson(obj, buffer);
```

## Installation (CMake-FetchContent)
- Requirements:
	- CMake 3.20 or later.
	- A C++20 or later compiler.	
```cpp
include(FetchContent)

FetchContent_Declare(
   Jsonifier
   GIT_REPOSITORY https://github.com/RealTimeChris/Jsonifier.git
   GIT_TAG Dev
)
FetchContent_MakeAvailable(Jsonifier)

target_link_libraries("${PROJECT_NAME}" PRIVATE Jsonifier::Jsonifier)
```
