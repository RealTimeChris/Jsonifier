# Json-Performance
Performance profiling of JSON libraries (Compiled and run on Ubuntu-22.04 using the Clang++18 compiler)

Latest Results: (Aug 04, 2024)
#### Using the following commits:
----
| Jsonifier: [](https://github.com/RealTimeChris/Jsonifier/commit/)  
| Glaze: [f4dab2d](https://github.com/stephenberry/glaze/commit/f4dab2d)  
| Simdjson: [ba1819f](https://github.com/simdjson/simdjson/commit/ba1819f)  

 > At least 5 iterations on a 6 core (Intel i7 8700k), until Empirical Confidence is at or above 95.0% and mdape (Median Absolute Percentage Error) is at or below 5%.


### Json Test (Prettified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/JsonData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Json%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Json%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/s) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/s) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------------------------------------------------- | ---------- | ----------- | ---------- | ----------- | ---------- | ---------- | ---------- | ---------- |
| [simdjson](https://github.com/simdjson/simdjson/commit/ba1819f) | 0 | 3147975 | inf | 180 | 
| [glaze](https://github.com/stephenberry/glaze/commit/f4dab2d) | 0 | 3147975 | inf | 180 | 0 | 3147451 | inf | 180 |
| [jsonifier](https://github.com/RealTimeChris/Jsonifier/commit/) | 0 | 3147975 | inf | 180 | 0 | 3143480 | inf | 180 |

### Json Test (Minified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/JsonData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Json%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Json%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/s) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/s) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------------------------------------------------- | ---------- | ----------- | ---------- | ----------- | ---------- | ---------- | ---------- | ---------- |
| [simdjson](https://github.com/simdjson/simdjson/commit/ba1819f) | 0 | 2145204 | inf | 180 | 
| [glaze](https://github.com/stephenberry/glaze/commit/f4dab2d) | 0 | 2145204 | inf | 180 | 0 | 2144680 | inf | 180 |
| [jsonifier](https://github.com/RealTimeChris/Jsonifier/commit/) | 0 | 2145204 | inf | 180 | 0 | 2140709 | inf | 180 |

### ABC Test (Out of Sequence Performance - Prettified) [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/JsonData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Abc%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Abc%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>

The JSON documents in the previous tests featured keys ranging from "a" to "z," where each key corresponds to an array of values. Notably, the documents in this test arrange these keys in reverse order, deviating from the typical "a" to "z" arrangement.

This test effectively demonstrates the challenges encountered when utilizing simdjson and iterative parsers that lack the ability to efficiently allocate memory locations through hashing. In cases where the keys are not in the expected sequence, performance is significantly compromised, with the severity escalating as the document size increases.

In contrast, hash-based solutions offer a viable alternative by circumventing these issues and maintaining optimal performance regardless of the JSON document's scale, or ordering of the keys being parsed.

| Library | Read (MB/s) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/s) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------------------------------------------------- | ---------- | ----------- | ---------- | ----------- | ---------- | ---------- | ---------- | ---------- |
| [jsonifier](https://github.com/RealTimeChris/Jsonifier/commit/) | 58.5676 | 3147975 | 5.37494e+07 | 89 | 0 | 3143480 | inf | 180 |
| [simdjson](https://github.com/simdjson/simdjson/commit/ba1819f) | 0 | 3147975 | inf | 180 | 
| [glaze](https://github.com/stephenberry/glaze/commit/f4dab2d) | 0 | 3147975 | inf | 180 | 0 | 3147451 | inf | 180 |

### ABC Test (Out of Sequence Performance - Minified) [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/JsonData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Abc%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Abc%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/s) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/s) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------------------------------------------------- | ---------- | ----------- | ---------- | ----------- | ---------- | ---------- | ---------- | ---------- |
| [simdjson](https://github.com/simdjson/simdjson/commit/ba1819f) | 0 | 2145204 | inf | 180 | 
| [glaze](https://github.com/stephenberry/glaze/commit/f4dab2d) | 0 | 2145204 | inf | 180 | 0 | 2144680 | inf | 180 |
| [jsonifier](https://github.com/RealTimeChris/Jsonifier/commit/) | 0 | 2145204 | inf | 180 | 0 | 2140709 | inf | 180 |

### Discord Test (Prettified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Discord%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Discord%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/s) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/s) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------------------------------------------------- | ---------- | ----------- | ---------- | ----------- | ---------- | ---------- | ---------- | ---------- |
| [simdjson](https://github.com/simdjson/simdjson/commit/ba1819f) | 0 | 138774 | inf | 180 | 
| [glaze](https://github.com/stephenberry/glaze/commit/f4dab2d) | 0 | 138774 | inf | 180 | 0 | 138774 | inf | 180 |
| [jsonifier](https://github.com/RealTimeChris/Jsonifier/commit/) | 0 | 138774 | inf | 180 | 0 | 138774 | inf | 180 |

### Discord Test (Minified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Discord%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Discord%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/s) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/s) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------------------------------------------------- | ---------- | ----------- | ---------- | ----------- | ---------- | ---------- | ---------- | ---------- |
| [simdjson](https://github.com/simdjson/simdjson/commit/ba1819f) | 0 | 69037 | inf | 180 | 
| [glaze](https://github.com/stephenberry/glaze/commit/f4dab2d) | 0 | 69037 | inf | 180 | 0 | 69037 | inf | 180 |
| [jsonifier](https://github.com/RealTimeChris/Jsonifier/commit/) | 0 | 69037 | inf | 180 | 0 | 69037 | inf | 180 |

### Canada Test (Prettified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/CanadaData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Canada%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Canada%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/s) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/s) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------------------------------------------------- | ---------- | ----------- | ---------- | ----------- | ---------- | ---------- | ---------- | ---------- |
| [simdjson](https://github.com/simdjson/simdjson/commit/ba1819f) | 0 | 6661897 | inf | 180 | 
| [glaze](https://github.com/stephenberry/glaze/commit/f4dab2d) | 0 | 6661897 | inf | 180 | 0 | 6661897 | inf | 180 |
| [jsonifier](https://github.com/RealTimeChris/Jsonifier/commit/) | 0 | 6661897 | inf | 180 | 58.9217 | 6661897 | 1.13064e+08 | 179 |

### Canada Test (Minified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/CanadaData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Canada%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Canada%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/s) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/s) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------------------------------------------------- | ---------- | ----------- | ---------- | ----------- | ---------- | ---------- | ---------- | ---------- |
| [simdjson](https://github.com/simdjson/simdjson/commit/ba1819f) | 0 | 2090234 | inf | 180 | 
| [glaze](https://github.com/stephenberry/glaze/commit/f4dab2d) | 0 | 2090234 | inf | 180 | 0 | 2090234 | inf | 180 |
| [jsonifier](https://github.com/RealTimeChris/Jsonifier/commit/) | 0 | 2090234 | inf | 180 | 0 | 2090234 | inf | 180 |

### Twitter Test (Prettified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/TwitterData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Twitter%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Twitter%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/s) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/s) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------------------------------------------------- | ---------- | ----------- | ---------- | ----------- | ---------- | ---------- | ---------- | ---------- |
| [simdjson](https://github.com/simdjson/simdjson/commit/ba1819f) | 0 | 722038 | inf | 180 | 
| [glaze](https://github.com/stephenberry/glaze/commit/f4dab2d) | 0 | 722038 | inf | 180 | 0 | 722415 | inf | 180 |
| [jsonifier](https://github.com/RealTimeChris/Jsonifier/commit/) | 0 | 722038 | inf | 180 | 0 | 722038 | inf | 180 |

### Twitter Test (Minified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/TwitterData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Twitter%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Twitter%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/s) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/s) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------------------------------------------------- | ---------- | ----------- | ---------- | ----------- | ---------- | ---------- | ---------- | ---------- |
| [jsonifier](https://github.com/RealTimeChris/Jsonifier/commit/) | 20.6763 | 478986 | 2.3166e+07 | 179 | 0 | 478986 | inf | 180 |
| [simdjson](https://github.com/simdjson/simdjson/commit/ba1819f) | 1.7339 | 478986 | 2.76248e+08 | 89 | 
| [glaze](https://github.com/stephenberry/glaze/commit/f4dab2d) | 0 | 478986 | inf | 180 | 0 | 479363 | inf | 180 |

### Minify Test Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Minify%20Test_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Minify%20Test_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Write (MB/s) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------------------------------------------------- | ---------- | ----------- | ---------- | ----------- |
| [simdjson](https://github.com/simdjson/simdjson/commit/ba1819f) | 0 | 69037 | inf | 180 |
| [glaze](https://github.com/stephenberry/glaze/commit/f4dab2d) | 0 | 69037 | inf | 180 |
| [jsonifier](https://github.com/RealTimeChris/Jsonifier/commit/) | 0 | 69037 | inf | 180 |

### Prettify Test Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Prettify%20Test_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Prettify%20Test_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Write (MB/s) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------------------------------------------------- | ---------- | ----------- | ---------- | ----------- |
| [glaze](https://github.com/stephenberry/glaze/commit/f4dab2d) | 0 | 3147975 | inf | 180 |
| [jsonifier](https://github.com/RealTimeChris/Jsonifier/commit/) | 0 | 3147975 | inf | 180 |

### Validation Test Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Validate%20Test_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Validate%20Test_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/s) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count |
| ------------------------------------------------- | ---------- | ----------- | ---------- | ----------- |
| [glaze](https://github.com/stephenberry/glaze/commit/f4dab2d) | 0 | 138774 | inf | 180 | 
| [jsonifier](https://github.com/RealTimeChris/Jsonifier/commit/) | 0 | 138774 | inf | 180 | 