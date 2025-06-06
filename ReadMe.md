# Jsonifier
![Commit Activity](https://img.shields.io/github/commit-activity/y/realtimechris/jsonifier?color=999EE0&label=Commits&style=plastic)
![Lines of Code](https://tokei.rs/b1/github/RealTimeChris/Jsonifier-Code-Only?color=light-blue&label=Lines%20Of%20Code%20&style=plastic)

#### Jsonifier is fully [RFC8259](https://datatracker.ietf.org/doc/html/rfc8259) compliant.

### A few classes for validating, serializing, parsing, prettifying, and minifying objects into/from JSON strings - very rapidly.
### [Benchmarks](https://github.com/RealTimeChris/Json-Performance)
### [More Benchmarks](https://github.com/Loki-Astari/JsonBenchmark)
### ***It achieves this through the usage of [simd-instructions](https://github.com/RealTimeChris/Jsonifier/tree/main/Include/jsonifier/Simd) as well as compile-time [hash-maps](https://github.com/RealTimeChris/Jsonifier/blob/main/Include/jsonifier/Utilities/HashMap.hpp) for the keys of the data being parsed.***
### ***Jsonifier doesn’t just parse JSON — it survives it. Every build is battle-hardened against malformed input by a custom fuzz-simulation test that systematically corrupts a 2000+ character JSON string one byte at a time, generating thousands of progressively invalid permutations. Each variant is force-fed through both parsing and schema validation. The result? Zero crashes. Zero memory violations. Zero undefined behavior. Jsonifier passes this gauntlet under UBSAN and ASAN with full memory and bounds safety — proving it’s not just fast, but ferociously robust in the face of chaos.***
----

### Compiler Support
----
![MSVC](https://img.shields.io/github/actions/workflow/status/RealTimeChris/Jsonifier/MSVC-Windows.yml?style=plastic&logo=microsoft&logoColor=green&label=MSVC&labelColor=pewter&color=blue&branch=dev)
![GCC](https://img.shields.io/github/actions/workflow/status/RealTimeChris/Jsonifier/GCC-Ubuntu.yml?style=plastic&logo=linux&logoColor=green&label=GCC&labelColor=pewter&color=blue&branch=dev)
![CLANG](https://img.shields.io/github/actions/workflow/status/RealTimeChris/Jsonifier/CLANG-MacOS.yml?style=plastic&logo=apple&logoColor=green&label=CLANG&labelColor=pewter&color=blue&branch=dev)

### Operating System Support
----
![Windows](https://img.shields.io/github/actions/workflow/status/RealTimeChris/Jsonifier/MSVC-Windows.yml?style=plastic&logo=microsoft&logoColor=green&label=Windows&labelColor=pewter&color=blue&branch=dev)
![Linux](https://img.shields.io/github/actions/workflow/status/RealTimeChris/Jsonifier/GCC-Ubuntu.yml?style=plastic&logo=linux&logoColor=green&label=Linux&labelColor=pewter&color=blue&branch=dev)
![Mac](https://img.shields.io/github/actions/workflow/status/RealTimeChris/Jsonifier/CLANG-MacOS.yml?style=plastic&logo=apple&logoColor=green&label=MacOS&labelColor=pewter&color=blue&branch=dev)

### Usage Guide:
----
- ## [Installation](https://github.com/RealTimeChris/Jsonifier/blob/main/Documentation/Installation.md)
- ## [Reflection](https://github.com/RealTimeChris/Jsonifier/blob/main/Documentation/Reflection.md)
- ## [Optimizing For Minified Json](https://github.com/RealTimeChris/Jsonifier/blob/main/Documentation/Optimizing_For_Minified_Json.md)
- ## [Partial Reading](https://github.com/RealTimeChris/Jsonifier/blob/main/Documentation/PartialReading.md)
- ## [Serializing-Parsing](https://github.com/RealTimeChris/Jsonifier/blob/main/Documentation/Usage_Serializing_Parsing.md)
- ## [Validating](https://github.com/RealTimeChris/Jsonifier/blob/main/Documentation/Validating.md)
- ## [Prettifying](https://github.com/RealTimeChris/Jsonifier/blob/main/Documentation/Prettifying.md)
- ## [Minifying](https://github.com/RealTimeChris/Jsonifier/blob/main/Documentation/Minifying.md)
- ## [Error Handling](https://github.com/RealTimeChris/Jsonifier/blob/main/Documentation/Errors.md)
- ## [CPU Architecture Selection](https://github.com/RealTimeChris/Jsonifier/blob/main/Documentation/CPU_Architecture_Selection.md)
- ## [Excluding Keys at Runtime](https://github.com/RealTimeChris/Jsonifier/blob/main/Documentation/Excluding_Keys.md)
- ## [Custom JSON Parsing/Serializing Behavior](https://github.com/RealTimeChris/Jsonifier/blob/main/Documentation/Custom_Parsing_And_Serializing.md)
- ## [Parsing Arbitrary/Unknown Data](https://github.com/RealTimeChris/Jsonifier/blob/main/Documentation/Parsing_Arbitrary_Data.md)
