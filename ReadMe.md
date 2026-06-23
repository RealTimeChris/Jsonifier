# Jsonifier

![Commit Activity](https://img.shields.io/github/commit-activity/y/realtimechris/jsonifier?color=999EE0&label=Commits&style=plastic)
![License](https://img.shields.io/badge/License-MIT-blue.svg?style=plastic)
![C++23](https://img.shields.io/badge/C%2B%2B-23-blue.svg?style=plastic)

**Jsonifier is fully [RFC8259](https://datatracker.ietf.org/doc/html/rfc8259) compliant.**

A high-performance C++ library for validating, serializing, parsing, prettifying, and minifying JSON data - **very rapidly**.

It achieves this through the usage of [SIMD instructions](./include/jsonifier-incl/simd) as well as compile-time [hash maps](/include/jsonifier-incl/utilities/HashMap.hpp) for efficient key lookups during parsing.

- [Benchmarks](https://github.com/RealTimeChris/Json-Performance)
- [More Benchmarks](https://github.com/Loki-Astari/JsonBenchmark)

---

## Compiler Support

| Compiler | Status |
|----------|--------|
| ![MSVC](https://img.shields.io/github/actions/workflow/status/realtimechris/jsonifier/unit-tests.yml?style=plastic&logo=microsoft&logoColor=green&label=MSVC&labelColor=pewter&color=blue&branch=main) | Visual Studio (Latest) |
| ![GCC](https://img.shields.io/github/actions/workflow/status/realtimechris/jsonifier/unit-tests.yml?style=plastic&logo=linux&logoColor=green&label=GCC&labelColor=pewter&color=blue&branch=main) | GCC (Latest) |
| ![CLANG](https://img.shields.io/github/actions/workflow/status/realtimechris/jsonifier/unit-tests.yml?style=plastic&logo=apple&logoColor=green&label=CLANG&labelColor=pewter&color=blue&branch=main) | Clang (Latest) |

## Operating System Support

| OS | Status |
|----|--------|
| ![Windows](https://img.shields.io/github/actions/workflow/status/realtimechris/jsonifier/unit-tests.yml?style=plastic&logo=microsoft&logoColor=green&label=Windows&labelColor=pewter&color=blue&branch=main) | Windows (Latest) |
| ![Linux](https://img.shields.io/github/actions/workflow/status/realtimechris/jsonifier/unit-tests.yml?style=plastic&logo=linux&logoColor=green&label=Linux&labelColor=pewter&color=blue&branch=main) | Ubuntu (Latest) |
| ![Mac](https://img.shields.io/github/actions/workflow/status/realtimechris/jsonifier/unit-tests.yml?style=plastic&logo=apple&logoColor=green&label=MacOS&labelColor=pewter&color=blue&branch=main) | macOS (Latest) |

## CPU Architecture Support

Jsonifier automatically detects and optimizes for your CPU architecture:

- **x64 / AMD64** - 64-bit extension of x86 with enhanced memory addressing
- **AVX** - 128-bit vector registers for SIMD operations
- **AVX2** - 256-bit vector registers with additional integer operations
- **AVX-512** - 512-bit vector registers for maximum parallelism
- **ARM-NEON** - SIMD instructions for ARM processors

Manual configuration is also available via `JSONIFIER_CPU_FLAGS` in CMake.

---

## Features

### 🚀 High Performance
- SIMD-accelerated parsing and validation
- Compile-time reflection eliminates runtime overhead
- Specialized hash maps for different object sizes (1, 2, 3+ fields)
- Zero-copy parsing where possible

### 🛠️ Flexible API
- Parse into existing objects or create new ones
- Serialize to existing buffers or return strings
- Partial reading for unordered or unknown JSON structures
- Support for minified JSON optimization

### 📋 Complete JSON Support
- Full RFC8259 compliance
- Unicode and escape sequence handling
- All JSON data types (objects, arrays, strings, numbers, booleans, null)
- Raw JSON data preservation

### 🔧 Advanced Features
- Runtime key exclusion during serialization
- Custom parsing/serialization for specialized types
- JSON validation with detailed error reporting
- Pretty-printing with customizable indentation
- Minification for compact output

### 🛡️ Safety & Reliability
- Comprehensive error handling with source location tracking
- AddressSanitizer (ASAN) and UndefinedBehaviorSanitizer (UBSAN) support
- Continuous integration with sanitizers enabled
- Extensive test suite including conformance tests

---

## CI/CD with unit-tests

Jsonifier uses GitHub Actions to continuously test across multiple platforms and compilers with sanitizers enabled:

```yaml
name: unit-tests
on:
  push:
    branches: [ "**" ]
  pull_request:
    branches: [ "**" ]
  workflow_dispatch:

jobs:
  build:
    strategy:
      fail-fast: false
      matrix:
        include:
          - os: ubuntu-latest
            compiler: clang
            name: "Ubuntu Clang"
          - os: ubuntu-latest
            compiler: gcc
            name: "Ubuntu GCC"
          - os: macos-latest
            compiler: clang
            name: "macOS Clang"
          - os: macos-latest
            compiler: gcc
            name: "macOS GCC"
          - os: windows-latest
            compiler: msvc
            name: "Windows MSVC"
    runs-on: ${{ matrix.os }}
    # ... full build and test with ASAN + UBSAN
```

This ensures memory safety and undefined behavior detection across all supported platforms.

---

## Comprehensive Test Suite

Jsonifier includes an extensive test suite that runs on **every push** across **all supported platforms and compilers** with **ASAN (AddressSanitizer) and UBSAN (UndefinedBehaviorSanitizer)** enabled:

### Test Categories

| Test Category | Description |
|---------------|-------------|
| **Conformance Tests** | Full RFC8259 compliance testing with 30+ JSON test files (pass/fail cases) |
| **Round-Trip Tests** | Serialize → Parse → Compare to ensure data integrity across all types |
| **Float Validation** | 64+ edge cases including denormals, infinities, and extreme values |
| **Integer Validation** | Bounds testing for signed/unsigned integers (8-bit to 64-bit) |
| **String Validation** | Unicode, escape sequences, control characters, and emoji support |
| **Bounds/Truncation** | Validates behavior with truncated and malformed JSON input |
| **Type Coverage** | Primitives, containers (vector, array, map, unordered_map), tuples, optional, shared_ptr, enums |

### What Gets Tested

- **100+ individual test cases** covering all JSON data types
- **30+ conformance tests** from the official JSON test suite
- **27 round-trip tests** including edge cases (null, empty, large numbers, special floats)
- **Memory safety** - No leaks, double-frees, or use-after-free
- **Undefined behavior** - No signed overflow, null pointer dereference, or invalid casts
- **Bounds checking** - Proper handling of truncated input
- **Unicode and emoji** - Full UTF-8 support with emoji validation
- **Edge cases** - Infinity, NaN, denormal numbers, integer overflow boundaries

### Running Tests Locally

```bash
# Clone the repository
git clone https://github.com/RealTimeChris/Jsonifier.git
cd Jsonifier

# Configure with tests enabled
cmake -B build -DJSONIFIER_UNIT_TESTS=ON

# Build and run tests
cmake --build build --target jsonifier_unit_tests
./build/Tests/jsonifier_unit_tests
```

---

## Quick Example

```cpp
#include <jsonifier>

// Define your structure
struct Person {
    std::string name;
    int32_t age;
    double height;
    bool active;
};

// Register it with Jsonifier
template<> struct jsonifier::core<Person> {
    using value_type = Person;
    static constexpr auto parseValue = createValue<
        &value_type::name,
        &value_type::age,
        &value_type::height,
        &value_type::active
    >();
};

int main() {
    jsonifier::jsonifier_core<> parser;
    
    // Parse JSON
    std::string json = R"({"name":"John","age":30,"height":1.85,"active":true})";
    Person person;
    parser.parseJson(person, json);
    
    // Serialize to JSON
    std::string output;
    parser.serializeJson(person, output);
    // output: {"name":"John","age":30,"height":1.85,"active":true}
    
    return 0;
}
```

---

## Documentation

### Getting Started
- [Installation](Documentation/Installation.md) - Install via vcpkg, CMake FetchContent, or source

### core Usage
- [Reflection](Documentation/Reflection.md) - Registering structures with compile-time reflection
- [Serializing & Parsing](Documentation/Usage_Serializing_Parsing.md) - Complete serialization/parsing guide
- [Validating](Documentation/Validating.md) - JSON validation against RFC standards

### Optimization
- [Optimizing For Minified JSON](Documentation/Optimizing_For_Minified_Json.md) - Boost performance for minified input
- [Partial Reading](Documentation/PartialReading.md) - Parse unordered or partial JSON structures
- [CPU Architecture Selection](Documentation/CPU_Architecture_Selection.md) - Manual CPU instruction set configuration

### Output Formatting
- [Prettifying](Documentation/Prettifying.md) - Pretty-print JSON with customizable indentation
- [Minifying](Documentation/Minifying.md) - Minify JSON for compact output

### Advanced Topics
- [Error Handling](Documentation/Errors.md) - Detailed error reporting and handling
- [Excluding Keys at Runtime](Documentation/Excluding_Keys.md) - Dynamic key exclusion during serialization
- [Custom Parsing & Serializing](Documentation/Custom_Parsing_And_Serializing.md) - Specialized type handling
- [Parsing Arbitrary Data](Documentation/Parsing_Arbitrary_Data.md) - Working with unknown JSON structures

---

## Requirements

- CMake 3.18 or later
- C++23 compliant compiler (MSVC 2022+, GCC 11+, Clang 14+)
- Supported CPU (x64, ARM64 with NEON)

---

## License

This library is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.

---

## Acknowledgments

- SIMD parsing techniques inspired by [simdjson](https://github.com/simdjson/simdjson)
- Reflection interface inspired by [Glaze](https://github.com/stephenberry/glaze)
- Dragonbox algorithm for float conversion
- FastFloat for number parsing

---

## Contributing

Contributions are welcome! Please feel free to submit pull requests or open issues on GitHub.

---

**Star this repository if you find it useful!** ⭐
