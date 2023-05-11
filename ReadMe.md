# Jsonifier
![Commit Activity](https://img.shields.io/github/commit-activity/m/RealTimeChris/Jsonifier?color=999EE0&label=Commits)
[![Lines of Code](https://sloc.xyz/github/boyter/scc/)](https://github.com/RealTimeChris/Jsonifier/)

## A few classes for serializing and parsing objects into/from JSON strings - very rapidly.
### ***It achieves this through the usage of [simd-instructions](https://github.com/RealTimeChris/Jsonifier/blob/main/Include/jsonifier/Simd.hpp) as well as compile-time hash-maps for the keys of the data being parsed.***
## [Benchmarks](https://github.com/RealTimeChris/Json-Performance)
----

## Compiler Support
![MSVC_20922](https://img.shields.io/github/actions/workflow/status/RealTimeChris/Jsonifier/MSVC_2022.yml?color=00ff90&label=MSVC_2022)
![GCC_12](https://img.shields.io/github/actions/workflow/status/RealTimeChris/Jsonifier/GCC_12.yml?color=00ff90&label=GCC_12)
![CLANG_17](https://img.shields.io/github/actions/workflow/status/RealTimeChris/Jsonifier/CLANG_17.yml?color=00ff90&label=CLANG_17)

## Usage - Serialization/Parsing
- Create a specialization of the `Jsonifier::Core` class template for whichever data structure you would like to parse/serialize, within the Jsonifier namespace as follows...
----
```cpp
namespace TestNS {

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
}

namespace Jsonifier {

	template<> struct Core<TestNS::fixed_object_t> {
		using OTy = TestNS::fixed_object_t;
		static constexpr auto parseValue = object("int_array", &OTy::int_array, "float_array", &OTy::float_array, "double_array", &OTy::double_array);
	};

	template<> struct Core<TestNS::fixed_name_object_t> {
		using OTy = TestNS::fixed_name_object_t;
		static constexpr auto parseValue = object("name0", &OTy::name0, "name1", &OTy::name1, "name2", &OTy::name2, "name3", &OTy::name3, "name4", &OTy::name4);
	};

	template<> struct Core<TestNS::nested_object_t> {
		using OTy = TestNS::nested_object_t;
		static constexpr auto parseValue = object("v3s", &OTy::v3s, "id", &OTy::id);
	};

	template<> struct Core<TestNS::another_object_t> {
		using OTy = TestNS::another_object_t;
		static constexpr auto parseValue =
			object("string", &OTy::string, "another_string", &OTy::another_string, "boolean", &OTy::boolean, "nested_object", &OTy::nested_object);
	};

	template<> struct Core<TestNS::obj_t> {
		using OTy = TestNS::obj_t;
		static constexpr auto parseValue =
			object("fixed_object", &OTy::fixed_object, "fixed_name_object", &OTy::fixed_name_object, "another_object", &OTy::another_object, "string_array",
				&OTy::string_array, "string", &OTy::string, "number", &OTy::number, "boolean", &OTy::boolean, "another_bool", &OTy::another_bool);
	};
}

```
## Usage - Parsing
- Create an instance of the `Jsonifier::JsonifierCore` class, and pass to its function `parseJson()` a reference to the intended parsing target, along with a reference to a `std::string` or equivalent, to be parsed from, as follows...
- Note: You can save parsing time by reusing a previously-allocated object, that has been used for previous parses.
```cpp
std::string buffer{ json0 };

obj_t obj{};

Jsonifier::JsonifierCore parser{};
parser.parseJson(obj, buffer);
```
## Usage - Serialization
- Create an instance of the `Jsonifier::JsonifierCore` class, and pass to its function `serializeJson()` a reference to the intended serialization target, along with a reference to a `std::string` or equivalent, to be serialized into, as follows...
- Note: You can save serialization time by reusing a previously-allocated buffer, that has been used for previous serializations.
```cpp
std::string buffer{};

obj_t obj{};

Jsonifier::JsonifierCore serializer{};
serializer.serializeJson(obj, buffer);
```
## Excluding Keys from Serialization at Runtime
To exclude certain keys from being serialized at runtime using the Jsonifier library, you can create a member in your object called excludedKeys and add the keys you want to exclude to this set. You can then call the `serializeJson` member function of the `Jsonifier::JsonifierCore` class with `true` passed into its first template parameter, to serialize the object to a JSON string, excluding the keys in the `excludedKeys` set.

Here's an example of how you can do this:
```c++
#include <jsonifier/Index.hpp>
#include <set>

class MyObject {
public:
  std::string name;
  int age;
  std::set<std::string> excludedKeys;

  MyObject(const std::string& n, int a) : name(n), age(a) {
    excludedKeys.insert("age"); // add "age" key to excludedKeys set
  }
};

int main() {
  MyObject obj("John", 30);
  Jsonifier::JsonifierCore jsonifier{};
  std::string jsonBuffer{};
  jsonifier.serializeJson<true>(obj, jsonBuffer); // {"name":"John"}
  return 0;
}
```

In this example, we have a class called `MyObject` with three member variables: `name`, `age`, and `excludedKeys`. The `excludedKeys` variable is a set of strings that will contain the keys we want to exclude from the serialized output.

In the constructor of `MyObject`, we add the key "age" to the `excludedKeys` set using the `insert` function. This means that when we serialize this object using the `serializeJson` member function of the `Jsonifier::JsonifierCore` class, the "age" key will be excluded from the resulting JSON string.

In the `main` function, we create an instance of `MyObject` with the name "John" and age 30. We then create an instance of `Jsonifier::JsonifierCore` and call its `serializeJson` member function to serialize the object to a JSON string. Since we added the "age" key to the `excludedKeys` set in the constructor, the resulting JSON string only contains the "name" key.

By using the `excludedKeys` member variable and adding keys to the set, you can easily exclude certain keys from being serialized at runtime using the Jsonifier library. And with the `serializeJson` member function of the `Jsonifier::JsonifierCore` class, you can easily serialize objects with excluded keys to JSON strings.

## Installation (Vcpkg)
- Requirements:
	- CMake 3.18 or later.
	- A C++23 or later compiler.
- Steps:   
	1. Install vcpkg, if need be.
	2. Make sure to run vcpkg integrate install.
	3. Enter within a terminal vcpkg install jsonifier:x64-windows_OR_linux.
	4. Set up a project in your IDE and make sure to set the C++ standard to C++23 or later - and include jsonifier/Index.hpp.
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

