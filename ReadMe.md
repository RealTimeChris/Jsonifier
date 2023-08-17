# Jsonifier
![Commit Activity](https://img.shields.io/github/commit-activity/y/realtimechris/jsonifier?color=999EE0&label=Commits&style=flat)
[![Lines of Code](https://sloc.xyz/github/realtimechris/jsonifier)](https://github.com/RealTimeChris/Jsonifier/)

## A few classes for serializing and parsing objects into/from JSON strings - very rapidly.
### ***It achieves this through the usage of [simd-instructions](https://github.com/RealTimeChris/Jsonifier/blob/main/Include/jsonifier/Simd.hpp) as well as compile-time hash-maps for the keys of the data being parsed.***
## [Benchmarks](https://github.com/RealTimeChris/Json-Performance)
----

## Compiler Support
----
![MSVC_20922](https://img.shields.io/github/actions/workflow/status/RealTimeChris/Jsonifier/MSVC_2022-Windows.yml?style=plastic&logo=microsoft&logoColor=green&label=MSVC_2022&labelColor=pewter&color=blue)
![GCC_12](https://img.shields.io/github/actions/workflow/status/RealTimeChris/Jsonifier/GCC_12-Ubuntu.yml?style=plastic&logo=linux&logoColor=green&label=GNU_12&labelColor=pewter&color=blue)
![CLANG_16](https://img.shields.io/github/actions/workflow/status/RealTimeChris/Jsonifier/CLANG_16-MacOS.yml?style=plastic&logo=apple&logoColor=green&label=CLANG_16&labelColor=pewter&color=blue)

## Operating System Support
----
![Windows](https://img.shields.io/github/actions/workflow/status/RealTimeChris/Jsonifier/MSVC_2022-Windows.yml?style=plastic&logo=microsoft&logoColor=green&label=Windows&labelColor=pewter&color=blue)
![Linux](https://img.shields.io/github/actions/workflow/status/RealTimeChris/Jsonifier/GCC_12-Ubuntu.yml?style=plastic&logo=linux&logoColor=green&label=Linux&labelColor=pewter&color=blue)
![Mac](https://img.shields.io/github/actions/workflow/status/RealTimeChris/Jsonifier/CLANG_16-MacOS.yml?style=plastic&logo=apple&logoColor=green&label=Mac&labelColor=pewter&color=blue)

## Usage - Serialization/Parsing
----
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
		using ValueType = TestNS::fixed_object_t;
		static constexpr auto parseValue = object("int_array", &ValueType::int_array, "float_array", &ValueType::float_array, "double_array", &ValueType::double_array);
	};

	template<> struct Core<TestNS::fixed_name_object_t> {
		using ValueType = TestNS::fixed_name_object_t;
		static constexpr auto parseValue = object("name0", &ValueType::name0, "name1", &ValueType::name1, "name2", &ValueType::name2, "name3", &ValueType::name3, "name4", &ValueType::name4);
	};

	template<> struct Core<TestNS::nested_object_t> {
		using ValueType = TestNS::nested_object_t;
		static constexpr auto parseValue = object("v3s", &ValueType::v3s, "id", &ValueType::id);
	};

	template<> struct Core<TestNS::another_object_t> {
		using ValueType = TestNS::another_object_t;
		static constexpr auto parseValue =
			object("string", &ValueType::string, "another_string", &ValueType::another_string, "boolean", &ValueType::boolean, "nested_object", &ValueType::nested_object);
	};

	template<> struct Core<TestNS::obj_t> {
		using ValueType = TestNS::obj_t;
		static constexpr auto parseValue =
			object("fixed_object", &ValueType::fixed_object, "fixed_name_object", &ValueType::fixed_name_object, "another_object", &ValueType::another_object, "string_array",
				&ValueType::string_array, "string", &ValueType::string, "number", &ValueType::number, "boolean", &ValueType::boolean, "another_bool", &ValueType::another_bool);
	};
}

```
### Usage - Parsing
- Create an instance of the `Jsonifier::JsonifierCore` class, and pass to its function `parseJson()` a reference to the intended parsing target, along with a reference to a `std::string` or equivalent, to be parsed from, as follows...
- Note: You can save parsing time by reusing a previously-allocated object, that has been used for previous parses.
```cpp
std::string buffer{ json0 };

obj_t obj{};

Jsonifier::JsonifierCore parser{};
parser.parseJson(obj, buffer);
```
### Usage - Serialization
- Create an instance of the `Jsonifier::JsonifierCore` class, and pass to its function `serializeJson()` a reference to the intended serialization target, along with a reference to a `std::string` or equivalent, to be serialized into, as follows...
- Note: You can save serialization time by reusing a previously-allocated buffer, that has been used for previous serializations.
```cpp
std::string buffer{};

obj_t obj{};

Jsonifier::JsonifierCore serializer{};
serializer.serializeJson(obj, buffer);
```
## Excluding Keys from Serialization at Runtime
----
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

## Enabling Error Message Output in Jsonifier
----
Jsonifier is a powerful JSON parsing library that allows you to easily parse JSON data in your C++ applications. By default, Jsonifier does not output detailed error messages during the parsing process. However, you can enable error message output to aid in debugging and understanding parsing issues.

To enable error message output in Jsonifier, you need to set a specific template parameter when calling the parseJson function. By setting this parameter to true, Jsonifier will generate detailed error messages if any parsing errors occur. Follow the steps below to enable error message output:

Locate the code block where you invoke the parseJson function.

Look for the following line of code:
```cpp
parser.parseJson(/* ... */);
```
Modify the code to include the template parameter set to true, as shown below:

```cpp
parser.parseJson<true>(/* ... */);
```
Save the changes.

Rebuild and run your application.

### Interpreting Error Messages:
With error message output enabled, Jsonifier will provide detailed information about parsing errors encountered during the process. When a parsing error occurs, Jsonifier will output an error message similar to the following:

```ruby
Failed to collect a ',', at index: 486 instead found a 'i', in file: C:\Users\Chris\source\repos\Jsonifier\Build\Windows-Release-Dev\_deps\jsonifier-src\Include\jsonifier/Parse_Impl.hpp, at: 182:44, in function: void __cdecl JsonifierInternal::ParseNoKeys::op<true,struct DiscordCoreInternal::WebSocketMessage>(struct DiscordCoreInternal::WebSocketMessage &,class JsonifierInternal::StructuralIterator &)().
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

 #### Conclusion
 Enabling error message output in Jsonifier can greatly assist in debugging and resolving parsing issues in your C++ applications. By following the steps outlined above, you can easily set the template parameter to true and gain access to detailed error messages during the JSON parsing process.

If you have any further questions or require additional assistance, please refer to the Jsonifier documentation or reach out to our support team.

Happy parsing with Jsonifier!

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

#### AVX-512

AVX-512 is an extension of the AVX instruction set architecture, designed to provide even higher levels of vector parallelism. AVX-512 introduces 512-bit vector registers (ZMM registers) and a broad range of new instructions for both floating-point and integer operations. With AVX-512, CPUs can process larger amounts of data in parallel, offering significant performance improvements for applications that can effectively utilize these capabilities.

### Configuration Explanation

The configuration script in Jsonifier's CMakeLists.txt file detects the CPU architecture and sets the appropriate compiler flags based on the supported architectures. It ensures that the generated code takes full advantage of the available instruction sets and achieves the best possible performance on the target CPU.

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

target_link_libraries("${PROJECT_NAME}" PRIVATE Jsonifier::Jsonifier)
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
	6. Now within the CMakeLists.txt of the project you wish to use the library in, set Jsonifier_DIR to wherever you set the `CMAKE_INSTALL_PREFIX` to, and then use `find_package(Jsonifier CONFIG REQUIRED)` and then `target_link_libraries("${PROJECT_NAME}" PUBLIC/PRIVATE Jsonifier::Jsonifier)`.

