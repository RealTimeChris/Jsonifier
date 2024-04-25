# Jsonifier
![Commit Activity](https://img.shields.io/github/commit-activity/y/realtimechris/jsonifier?color=999EE0&label=Commits&style=plastic)
![Lines of Code](https://tokei.rs/b1/github/RealTimeChris/Jsonifier?color=light-blue&label=Lines%20Of%20Code%20&style=plastic)

## A few classes for validating, serializing, parsing, prettifying, and minifying objects into/from JSON strings - very rapidly.
### ***It achieves this through the usage of [simd-instructions](https://github.com/RealTimeChris/Jsonifier/blob/dev/Include/jsonifier/ISA/ISADetectionBase.hpp#L328-L454) as well as compile-time hash-maps for the keys of the data being parsed.***
## [Benchmarks](https://github.com/RealTimeChris/Json-Performance)
----

## Compiler Support
----
![MSVC_20922](https://img.shields.io/github/actions/workflow/status/RealTimeChris/Jsonifier/MSVC_2022-Windows.yml?style=plastic&logo=microsoft&logoColor=green&label=MSVC_2022&labelColor=pewter&color=blue&branch=main)
![GCC_13](https://img.shields.io/github/actions/workflow/status/RealTimeChris/Jsonifier/GCC_13-Ubuntu.yml?style=plastic&logo=linux&logoColor=green&label=GCC_13&labelColor=pewter&color=blue&branch=main)
![CLANG_18](https://img.shields.io/github/actions/workflow/status/RealTimeChris/Jsonifier/CLANG_18-MacOS.yml?style=plastic&logo=apple&logoColor=green&label=CLANG_18&labelColor=pewter&color=blue&branch=main)

## Operating System Support
----
![Windows](https://img.shields.io/github/actions/workflow/status/RealTimeChris/Jsonifier/MSVC_2022-Windows.yml?style=plastic&logo=microsoft&logoColor=green&label=Windows&labelColor=pewter&color=blue&branch=main)
![Linux](https://img.shields.io/github/actions/workflow/status/RealTimeChris/Jsonifier/GCC_13-Ubuntu.yml?style=plastic&logo=linux&logoColor=green&label=Linux&labelColor=pewter&color=blue&branch=main)
![Mac](https://img.shields.io/github/actions/workflow/status/RealTimeChris/Jsonifier/CLANG_18-MacOS.yml?style=plastic&logo=apple&logoColor=green&label=MacOS&labelColor=pewter&color=blue&branch=main)

# Usage Guide:
----
- ## [Installation](https://github.com/RealTimeChris/Jsonifier/blob/main/Documentation/Installation.md)
- ## [Reflection](https://github.com/RealTimeChris/Jsonifier/blob/main/Documentation/Reflection.md)
- ## [Optimizing For Minified Json](https://github.com/RealTimeChris/Jsonifier/blob/main/Documentation/Optimizing_For_Minified_Json.md)
- ## [Serializing-Parsing](https://github.com/RealTimeChris/Jsonifier/blob/main/Documentation/Usage_Serializing_Parsing.md)
- ## [Validating](https://github.com/RealTimeChris/Jsonifier/blob/main/Documentation/Validating.md)
- ## [Prettifying](https://github.com/RealTimeChris/Jsonifier/blob/main/Documentation/Prettifying.md)
- ## [Minifying](https://github.com/RealTimeChris/Jsonifier/blob/main/Documentation/Minifying.md)
- ## [Error Handling](https://github.com/RealTimeChris/Jsonifier/blob/main/Documentation/Errors.md)
- ## [CPU Architecture Selection](https://github.com/RealTimeChris/Jsonifier/blob/main/Documentation/CPU_Architecture_Selection.md)
- ## [Excluding Keys at Runtime](https://github.com/RealTimeChris/Jsonifier/blob/main/Documentation/Excluding_Keys.md)
- ## [Custom JSON Parsing/Serializing Behavior](https://github.com/RealTimeChris/Jsonifier/blob/main/Documentation/Custom_Parsing_And_Serializing.md)
