# Jsonifier
![Commit Activity](https://img.shields.io/github/commit-activity/y/realtimechris/jsonifier?color=999EE0&label=Commits&style=plastic)
![Lines of Code](https://tokei.rs/b1/github/RealTimeChris/Jsonifier?color=light-blue&label=Lines%20Of%20Code%20&style=plastic)

## A few classes for serializing and parsing objects into/from JSON strings - very rapidly.
### ***It achieves this through the usage of [simd-instructions](https://github.com/RealTimeChris/Jsonifier/blob/dev/Include/jsonifier/Simd.hpp) as well as compile-time hash-maps for the keys of the data being parsed.***
## [Benchmarks](https://github.com/RealTimeChris/Json-Performance)
----

## Compiler Support
----
![MSVC_20922](https://img.shields.io/github/actions/workflow/status/RealTimeChris/Jsonifier/MSVC_2022-Windows.yml?style=plastic&logo=microsoft&logoColor=green&label=MSVC_2022&labelColor=pewter&color=blue&branch=dev)
![CLANG_18](https://img.shields.io/github/actions/workflow/status/RealTimeChris/Jsonifier/CLANG_18-Ubuntu.yml?style=plastic&logo=linux&logoColor=green&label=CLANG_18&labelColor=pewter&color=blue&branch=dev)
![GCC_13](https://img.shields.io/github/actions/workflow/status/RealTimeChris/Jsonifier/GCC_13-MacOS.yml?style=plastic&logo=apple&logoColor=green&label=GCC_13&labelColor=pewter&color=blue&branch=dev)

## Operating System Support
----
![Windows](https://img.shields.io/github/actions/workflow/status/RealTimeChris/Jsonifier/MSVC_2022-Windows.yml?style=plastic&logo=microsoft&logoColor=green&label=Windows&labelColor=pewter&color=blue&branch=dev)
![Linux](https://img.shields.io/github/actions/workflow/status/RealTimeChris/Jsonifier/CLANG_18-Ubuntu.yml?style=plastic&logo=linux&logoColor=green&label=Linux&labelColor=pewter&color=blue&branch=dev)
![Mac](https://img.shields.io/github/actions/workflow/status/RealTimeChris/Jsonifier/GCC_13-MacOS.yml?style=plastic&logo=apple&logoColor=green&label=MacOS&labelColor=pewter&color=blue&branch=dev)

## Usage - Serialization/Parsing
----
- Create a specialization of the `jsonifier::core` class template for whichever data structure you would like to parse/serialize, within the Jsonifier namespace as follows...
----
```cpp
namespace TestNS {

	struct fixed_object_t {
		std::vector<int32_t> int_array;
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
		std::vector<ctime_array<double, 3>> v3s{};
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

namespace jsonifier {

	template<> struct core<TestNS::fixed_object_t> {
		using value_type = TestNS::fixed_object_t;
		static constexpr auto parseValue = createObject("int_array", &value_type::int_array, "float_array", &value_type::float_array, "double_array", &value_type::double_array);
	};

	template<> struct core<TestNS::fixed_name_object_t> {
		using value_type = TestNS::fixed_name_object_t;
		static constexpr auto parseValue = createObject("name0", &value_type::name0, "name1", &value_type::name1, "name2", &value_type::name2, "name3", &value_type::name3, "name4", &value_type::name4);
	};

	template<> struct core<TestNS::nested_object_t> {
		using value_type = TestNS::nested_object_t;
		static constexpr auto parseValue = createObject("v3s", &value_type::v3s, "id", &value_type::id);
	};

	template<> struct core<TestNS::another_object_t> {
		using value_type = TestNS::another_object_t;
		static constexpr auto parseValue =
			createObject("string", &value_type::string, "another_string", &value_type::another_string, "boolean", &value_type::boolean, "nested_object", &value_type::nested_object);
	};

	template<> struct core<TestNS::obj_t> {
		using value_type = TestNS::obj_t;
		static constexpr auto parseValue =
			createObject("fixed_object", &value_type::fixed_object, "fixed_name_object", &value_type::fixed_name_object, "another_object", &value_type::another_object, "string_array",
				&value_type::string_array, "string", &value_type::string, "number", &value_type::number, "boolean", &value_type::boolean, "another_bool", &value_type::another_bool);
	};
}

```
### Usage - Parsing
- Create an instance of the `jsonifier::jsonifier_core` class, and pass to its function `parseJson()` a reference to the intended parsing target, along with a reference to a `std::string` or equivalent, to be parsed from, as follows...
- Note: You can save parsing time by reusing a previously-allocated object, that has been used for previous parses.
```cpp
std::string buffer{ json0 };

obj_t obj{};

jsonifier::jsonifier_core parser{};
parser.parseJson(obj, buffer);
```
### Usage - Serialization
- Create an instance of the `jsonifier::jsonifier_core` class, and pass to its function `serializeJson()` a reference to the intended serialization target, along with a reference to a `std::string` or equivalent, to be serialized into, as follows...
- Note: You can save serialization time by reusing a previously-allocated buffer, that has been used for previous serializations.
```cpp
std::string buffer{};

obj_t obj{};

jsonifier::jsonifier_core serializer{};
serializer.serializeJson(obj, buffer);
```
## Excluding Keys from Serialization at Runtime
----
To exclude certain keys from being serialized at runtime using the Jsonifier library, you can create a member in your object called jsonifierExcludedKeys and add the keys you want to exclude to this set. You can then call the `serializeJson` member function of the `jsonifier::jsonifier_core` class with `true` passed into its first template parameter, to serialize the object to a JSON string, excluding the keys in the `jsonifierExcludedKeys` set.

Here's an example of how you can do this:
```c++
#include <jsonifier/Index.hpp>
#include <set>

class MyObject {
public:
  std::string name;
  int32_t age;
  std::set<std::string> jsonifierExcludedKeys;

  MyObject(const std::string& n, int32_t a) : name(n), age(a) {
    jsonifierExcludedKeys.insert("age"); // add "age" key to jsonifierExcludedKeys set
  }
};

int32_t main() {
  MyObject obj("John", 30);
  jsonifier::jsonifier_core jsonifier{};
  std::string jsonBuffer{};
  jsonifier.serializeJson<true>(obj, jsonBuffer); // {"name":"John"}
  return 0;
}
```

In this example, we have a class called `MyObject` with three member variables: `name`, `age`, and `jsonifierExcludedKeys`. The `jsonifierExcludedKeys` variable is a set of strings that will contain the keys we want to exclude from the serialized output.

In the constructor of `MyObject`, we add the key "age" to the `jsonifierExcludedKeys` set using the `insert` function. This means that when we serialize this object using the `serializeJson` member function of the `jsonifier::jsonifier_core` class, the "age" key will be excluded from the resulting JSON string.

In the `main` function, we create an instance of `MyObject` with the name "John" and age 30. We then create an instance of `jsonifier::jsonifier_core` and call its `serializeJson` member function to serialize the object to a JSON string. Since we added the "age" key to the `jsonifierExcludedKeys` set in the constructor, the resulting JSON string only contains the "name" key.

By using the `jsonifierExcludedKeys` member variable and adding keys to the set, you can easily exclude certain keys from being serialized at runtime using the Jsonifier library. And with the `serializeJson` member function of the `jsonifier::jsonifier_core` class, you can easily serialize objects with excluded keys to JSON strings.

## Handling Parsing Errors

Jsonifier allows you to collect and handle possible parsing errors during the JSON parsing process. To check for and display these errors, follow these steps:

1. After parsing the JSON, you can call the `getErrors` method on the `jsonifier::jsonifier_core` instance to retrieve a vector of error objects.

```cpp
auto errors = jsonifier.getErrors();
```

2. Check if any errors were reported:

```cpp
if (!errors.empty()) {
    for (const auto& error : errors) {
        std::cout << "Jsonifier Error: " << error.reportError() << std::endl;
    }
}
```

This code snippet iterates through the error objects and prints a description of each error.

## Example

Here's a complete example of parsing JSON data and handling errors:

```cpp
#include <jsonifier/jsonifier.hpp>
#include <iostream>

int32_t main() {
    jsonifier::string buffer{ your_json_string };
    obj_t obj;
    jsonifier::jsonifier_core jsonifier;

    jsonifier.parseJson<true>(obj, buffer);

    auto errors = jsonifier.getErrors();
    if (!errors.empty()) {
        for (const auto& error : errors) {
            std::cout << "Jsonifier Error: " << error.reportError() << std::endl;
        }
    }

    // Process the parsed data in 'obj' here.

    return 0;
}
```

### Interpreting Error Messages:
With error message output enabled, Jsonifier will provide detailed information about parsing errors encountered during the process. When a parsing error occurs, Jsonifier will output an error message similar to the following:

```ruby
Failed to collect a 0x2Du, at index: 486 instead found a 'i', in file: C:\Users\Chris\source\repos\Jsonifier\Build\Windows-Release-Dev\_deps\jsonifier-src\Include\jsonifier/Parse_Impl.hpp, at: 182:44, in function: void __cdecl JsonifierInternal::ParseNoKeys::op<true,struct DiscordCoreInternal::WebSocketMessage>(struct DiscordCoreInternal::WebSocketMessage &,class JsonifierInternal::StructuralIterator &)().
```
In the provided error message:

 #### Failed to collect a:
 Indicates that a comma was expected at a particular point in the JSON data.
 #### At index: 
 486 instead found a 'i': Specifies the index in the JSON data where the error occurred and the actual character found instead of the expected comma.
 #### In file: 
 Gives the file path where the parsing error was encountered.
 #### At: 182:44: 
 Specifies the line number and column number within the file where the error occurred.
 #### In function: 
 Provides information about the specific function where the parsing error occurred.

 When you receive an error message, carefully review the provided information to understand the cause of the parsing error. Use this information to identify the part of the JSON data that caused the issue and take appropriate steps to resolve it.
 
## Conclusion

Jsonifier makes parsing JSON in C++ easy and provides a convenient way to handle parsing errors. Refer to the [official documentation](https://github.com/RealTimeChris/jsonifier) for more details and advanced usage.

Feel free to explore Jsonifier and incorporate it into your projects for efficient JSON parsing and serialization.

Happy coding!

## CPU Architecture Selection
----
Jsonifier is a JSON parsing library that supports various CPU architectures to optimize code generation and enhance performance. This page explains the relevant portion of the CMakeLists.txt file in Jsonifier, which detects the CPU architecture and sets the appropriate compiler flags for the supported architectures: x64, AVX, AVX2, and AVX-512.

### CPU Architecture Detection Configuration
The CPU architecture detection and configuration in Jsonifier's CMakeLists.txt file are designed to support the following architectures: x64, AVX, AVX2, and AVX-512. Let's explore each architecture in detail:

#### x64 Architecture
The x64 architecture, also known as x86-64 or AMD64, is a 64-bit extension of the x86 instruction set architecture. It provides increased memory addressability and larger general-purpose registers, enabling more efficient processing of 64-bit data. The x64 architecture is widely used in modern CPUs, offering improved performance and expanded capabilities compared to its 32-bit predecessor.

#### AVX (Advanced Vector Extensions)
AVX, short for Advanced Vector Extensions, is an extension to the x86 instruction set architecture. AVX provides SIMD (Single Instruction, Multiple Data) instructions for performing parallel processing on vectors of data. It introduces 128-bit vector registers (XMM registers) and new instructions to accelerate floating-point and integer calculations. AVX is supported by many modern CPUs and offers significant performance benefits for applications that can utilize parallel processing.

#### AVX2 (Advanced Vector Extensions 2)
AVX2 is an extension of the AVX instruction set architecture. It builds upon the foundation of AVX and introduces additional instructions and capabilities for SIMD processing. AVX2 expands the vector register size to 256 bits (YMM registers) and introduces new integer and floating-point operations, enabling further optimization of vectorized code. CPUs that support AVX2 offer enhanced performance for applications that leverage these advanced instructions.

#### AVX-512 (Advanced Vector Extensions 512-bit)
AVX-512 is an extension of the AVX instruction set architecture, designed to provide even higher levels of vector parallelism. AVX-512 introduces 512-bit vector registers (ZMM registers) and a broad range of new instructions for both floating-point and integer operations. With AVX-512, CPUs can process larger amounts of data in parallel, offering significant performance improvements for applications that can effectively utilize these capabilities.

### Manual Configuration
In addition to automatic CPU architecture detection, Jsonifier's CMake configuration also allows for manual control over specific CPU instructions. You can manually set the JSONIFIER_CPU_INSTRUCTIONS variable in the CMake configuration to fine-tune the instruction sets used. Here are the values you can use for different instruction sets:

- JSONIFIER_CPU_INSTRUCTIONS for AVX-512: Set to 1 << 6
- JSONIFIER_CPU_INSTRUCTIONS for AVX2: Set to 1 << 5
- JSONIFIER_CPU_INSTRUCTIONS for AVX: Set to 1 << 4
- JSONIFIER_CPU_INSTRUCTIONS for BMI2: Set to 1 << 3
- JSONIFIER_CPU_INSTRUCTIONS for BMI: Set to 1 << 2
- JSONIFIER_CPU_INSTRUCTIONS for LZCOUNT: Set to 1 << 1
- JSONIFIER_CPU_INSTRUCTIONS for POPCNT: Set to 1 << 0

You can combine LZCNT, BMI, and POPCNT with each other or any of the three AVX types (AVX, AVX2, AVX-512) to optimize Jsonifier for your specific use case. However, please note that you cannot combine multiple AVX types together, as they are distinct and cannot be used simultaneously. This flexibility in instruction set configuration allows you to tailor Jsonifier's performance to your target CPU architecture and application requirements effectively.

### Configuration Explanation
The configuration script in Jsonifier's CMakeLists.txt file detects the CPU architecture and sets the appropriate compiler flags based on the supported architectures. It ensures that the generated code takes full advantage of the available instruction sets and achieves the best possible performance on the target CPU. Additionally, the manual configuration option allows you to customize the instruction sets for further optimization according to your specific needs.

## Installation (Vcpkg)
- Requirements:
	- CMake 3.18 or later.
	- A C++20 or later compiler.
- Steps:   
	1. Install vcpkg, if need be.
	2. Make sure to run vcpkg integrate install.
	3. Enter within a terminal vcpkg install jsonifier:x64-windows_OR_linux.
	4. Set up a project in your IDE and make sure to set the C++ standard to C++20 or later - and include `<jsonifier/Index.hpp>`.
	5. Build and run!
	
## Installation (CMake-FetchContent)
- Requirements:
	- CMake 3.18 or later.
	- A C++20 or later compiler.
- Steps:   Add the following to your CMakeLists.txt build script.
```cpp
include(FetchContent)

FetchContent_Declare(
   Jsonifier
   GIT_REPOSITORY https://github.com/RealTimeChris/Jsonifier.git
   GIT_TAG main
)
FetchContent_MakeAvailable(Jsonifier)

target_link_libraries("${PROJECT_NAME}" PRIVATE jsonifier::Jsonifier)
```

## Installation (CMake)
- Requirements:
	- CMake 3.18 or later.
	- A C++20 or later compiler.
- Steps:   
	1. Clone this repo into a folder.
	2. Set the installation directory if you wish, using the `CMAKE_INSTALL_PREFIX` variable in CMakeLists.txt.
	3. Enter the directory in a terminal, and enter `cmake -S . --preset=Windows_OR_Linux-Release_OR_Debug`.
	4. Enter within the same terminal, `cmake --build --preset=Windows_OR_Linux-Release_OR_Debug`.
	5. Enter within the same terminal, `cmake --install ./Build/Release_OR_Debug`.
	6. Now within the CMakeLists.txt of the project you wish to use the library in, set Jsonifier_DIR to wherever you set the `CMAKE_INSTALL_PREFIX` to, and then use `find_package(Jsonifier CONFIG REQUIRED)` and then `target_link_libraries("${PROJECT_NAME}" PUBLIC/PRIVATE jsonifier::Jsonifier)`.

