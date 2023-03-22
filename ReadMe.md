# Jsonifier
![Commit Activity](https://img.shields.io/github/commit-activity/m/RealTimeChris/Jsonifier?color=999EE0&label=Commits)
[![Lines of Code](https://sloc.xyz/github/boyter/scc/)](https://github.com/RealTimeChris/Jsonifier/)

## A few classes for serializing and parsing objects into/from JSON strings - very rapidly (more rapidly than any other library).
### ***It achieves this through the usage of simd-instructions as well as compile-time hash-maps for the keys of the data being parsed.***
## [Benchmarks](https://github.com/RealTimeChris/Json-Performance)
----

## Compiler Support
![MSVC_20922](https://img.shields.io/github/actions/workflow/status/RealTimeChris/Jsonifier/MSVC_2022.yml?color=00ff90&label=MSVC_2022)
![GCC_12](https://img.shields.io/github/actions/workflow/status/RealTimeChris/Jsonifier/GCC_12.yml?color=00ff90&label=GCC_12)

## Usage - Serialization/Parsing
- Create a specialization of the `Jsonifier::Core` class template, within the Jsonifier namespace as follows...
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
namespace Jsonifier {
	template<> struct Core<fixed_object_t> {
		using T = fixed_object_t;
		static constexpr auto value = object("int_array", &T::int_array, "float_array", &T::float_array, "double_array", &T::double_array);
	};

	template<> struct Core<fixed_name_object_t> {
		using T = fixed_name_object_t;
		static constexpr auto value = object("name0", &T::name0, "name1", &T::name1, "name2", &T::name2, "name3", &T::name3, "name4", &T::name4);
	};

	template<> struct Core<nested_object_t> {
		using T = nested_object_t;
		static constexpr auto value = object("v3s", &T::v3s, "id", &T::id);
	};

	template<> struct Core<another_object_t> {
		using T = another_object_t;
		static constexpr auto value =
			object("string", &T::string, "another_string", &T::another_string, "boolean", &T::boolean, "nested_object", &T::nested_object);
	};

	template<> struct Core<obj_t> {
		using T = obj_t;
		static constexpr auto value =
			object("fixed_object", &T::fixed_object, "fixed_name_object", &T::fixed_name_object, "another_object", &T::another_object, "string_array",
				&T::string_array, "string", &T::string, "number", &T::number, "boolean", &T::boolean, "another_bool", &T::another_bool);
	};
}

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


## Installation (Vcpkg)
- Requirements:
	- CMake 3.18 or later.
	- A C++23 or later compiler.
- Steps:   
	1. Install vcpkg, if need be.
	2. Make sure to run vcpkg integrate install.
	3. Enter within a terminal vcpkg install jsonifier:x64-windows_OR_linux.
	4. Set up a console project in your IDE and make sure to set the C++ standard to C++23 or later - and include jsonifier/jsonifier.hpp.
	5. Build and run!
	
## Installation (CMake-FetchContent)
- Requirements:
	- CMake 3.18 or later.
	- A C++23 or later compiler.
- Steps:   Add the following to your CMakeLists.txt build script.
```cpp
include(FetchContent)

FetchContent_Declare(
   Jsonifier
   GIT_REPOSITORY https://github.com/RealTimeChris/Jsonifier.git
   GIT_TAG main
)
FetchContent_MakeAvailable(Jsonifier)

target_link_libraries("${PROJECT_NAME}" PRIVATE Jsonifier::Jsonifier)
```

## Installation (CMake)
- Requirements:
	- CMake 3.18 or later.
	- A C++23 or later compiler.
- Steps:   
	1. Clone this repo into a folder.
	2. Set the installation directory if you wish, using the `CMAKE_INSTALL_PREFIX` variable in CMakeLists.txt.
	3. Enter the directory in a terminal, and enter `cmake -S . --preset=Windows_OR_Linux-Release_OR_Debug`.
	4. Enter within the same terminal, `cmake --build --preset=Windows_OR_Linux-Release_OR_Debug`.
	5. Enter within the same terminal, `cmake --install ./Build/Release_OR_Debug`.
	6. Now within the CMakeLists.txt of the project you wish to use the library in, set Jsonifier_DIR to wherever you set the `CMAKE_INSTALL_PREFIX` to, and then use `find_package(Jsonifier CONFIG REQUIRED)` and then `target_link_libraries("${PROJECT_NAME}" PUBLIC/PRIVATE Jsonifier::Jsonifier)`.

