# Json-Performance
Performance profiling of JSON libraries (Compiled and run on Ubuntu-22.04 using the Clang++18 compiler)

Latest Results: (Jul 27, 2024)
#### Using the following commits:
----
| Jsonifier: [](https://github.com/RealTimeChris/Jsonifier/commit/)  \n| Glaze: [158ac74](https://github.com/stephenberry/glaze/commit/158ac74)  \n| Simdjson: [417e760](https://github.com/simdjson/simdjson/commit/417e760)  \n
 > At least 0 iterations on a 6 core (Intel i7 8700k), until Median Absolute Percentage Error (MAPE) reduced below 5.0%.


### Json Test (Prettified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/JsonData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Json%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Json%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/s) | ReadLength (Bytes) | ReadTime (ns) | ReadMape (%) | Write (MB/s) | WriteLength (Bytes) | WriteTime (ns) | WriteMape (%) |
| ------------------------------------------------- | ---------- | ----------- | ---------- | ----------- | ---------- | ---------- | ---------- | ---------- |
| [simdjson](https://github.com/simdjson/simdjson/commit/417e760) | inf | 2245007 | 0 | 0 | 
| [glaze](https://github.com/stephenberry/glaze/commit/158ac74) | inf | 2245007 | 0 | 0 | inf | 2244652 | 0 | 0 |
| [jsonifier](https://github.com/RealTimeChris/Jsonifier/commit/) | inf | 2245007 | 0 | 0 | inf | 2241922 | 0 | 0 |

### Json Test (Minified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/JsonData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Json%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Json%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/s) | ReadLength (Bytes) | ReadTime (ns) | ReadMape (%) | Write (MB/s) | WriteLength (Bytes) | WriteTime (ns) | WriteMape (%) |
| ------------------------------------------------- | ---------- | ----------- | ---------- | ----------- | ---------- | ---------- | ---------- | ---------- |
| [simdjson](https://github.com/simdjson/simdjson/commit/417e760) | inf | 1521724 | 0 | 0 | 
| [glaze](https://github.com/stephenberry/glaze/commit/158ac74) | inf | 1521724 | 0 | 0 | inf | 1521369 | 0 | 0 |
| [jsonifier](https://github.com/RealTimeChris/Jsonifier/commit/) | inf | 1521724 | 0 | 0 | inf | 1518639 | 0 | 0 |

### ABC Test (Out of Sequence Performance - Prettified) [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/JsonData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Abc%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Abc%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>

The JSON documents in the previous tests featured keys ranging from "a" to "z," where each key corresponds to an array of values. Notably, the documents in this test arrange these keys in reverse order, deviating from the typical "a" to "z" arrangement.

This test effectively demonstrates the challenges encountered when utilizing simdjson and iterative parsers that lack the ability to efficiently allocate memory locations through hashing. In cases where the keys are not in the expected sequence, performance is significantly compromised, with the severity escalating as the document size increases.

In contrast, hash-based solutions offer a viable alternative by circumventing these issues and maintaining optimal performance regardless of the JSON document's scale, or ordering of the keys being parsed.

| Library | Read (MB/s) | ReadLength (Bytes) | ReadTime (ns) | ReadMape (%) | Write (MB/s) | WriteLength (Bytes) | WriteTime (ns) | WriteMape (%) |
| ------------------------------------------------- | ---------- | ----------- | ---------- | ----------- | ---------- | ---------- | ---------- | ---------- |
| [simdjson](https://github.com/simdjson/simdjson/commit/417e760) | inf | 2245007 | 0 | 0 | 
| [glaze](https://github.com/stephenberry/glaze/commit/158ac74) | inf | 2245007 | 0 | 0 | inf | 2244652 | 0 | 0 |
| [jsonifier](https://github.com/RealTimeChris/Jsonifier/commit/) | inf | 2245007 | 0 | 0 | inf | 2241922 | 0 | 0 |

### ABC Test (Out of Sequence Performance - Minified) [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/JsonData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Abc%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Abc%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/s) | ReadLength (Bytes) | ReadTime (ns) | ReadMape (%) | Write (MB/s) | WriteLength (Bytes) | WriteTime (ns) | WriteMape (%) |
| ------------------------------------------------- | ---------- | ----------- | ---------- | ----------- | ---------- | ---------- | ---------- | ---------- |
| [simdjson](https://github.com/simdjson/simdjson/commit/417e760) | inf | 1521724 | 0 | 0 | 
| [glaze](https://github.com/stephenberry/glaze/commit/158ac74) | inf | 1521724 | 0 | 0 | inf | 1521369 | 0 | 0 |
| [jsonifier](https://github.com/RealTimeChris/Jsonifier/commit/) | inf | 1521724 | 0 | 0 | inf | 1518639 | 0 | 0 |

### Discord Test (Prettified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Discord%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Discord%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/s) | ReadLength (Bytes) | ReadTime (ns) | ReadMape (%) | Write (MB/s) | WriteLength (Bytes) | WriteTime (ns) | WriteMape (%) |
| ------------------------------------------------- | ---------- | ----------- | ---------- | ----------- | ---------- | ---------- | ---------- | ---------- |
| [simdjson](https://github.com/simdjson/simdjson/commit/417e760) | inf | 138774 | 0 | 0 | 
| [glaze](https://github.com/stephenberry/glaze/commit/158ac74) | inf | 138774 | 0 | 0 | inf | 138774 | 0 | 0 |
| [jsonifier](https://github.com/RealTimeChris/Jsonifier/commit/) | inf | 138774 | 0 | 0 | inf | 138774 | 0 | 0 |

### Discord Test (Minified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Discord%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Discord%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/s) | ReadLength (Bytes) | ReadTime (ns) | ReadMape (%) | Write (MB/s) | WriteLength (Bytes) | WriteTime (ns) | WriteMape (%) |
| ------------------------------------------------- | ---------- | ----------- | ---------- | ----------- | ---------- | ---------- | ---------- | ---------- |
| [simdjson](https://github.com/simdjson/simdjson/commit/417e760) | inf | 69037 | 0 | 0 | 
| [glaze](https://github.com/stephenberry/glaze/commit/158ac74) | inf | 69037 | 0 | 0 | inf | 69037 | 0 | 0 |
| [jsonifier](https://github.com/RealTimeChris/Jsonifier/commit/) | inf | 69037 | 0 | 0 | inf | 69037 | 0 | 0 |

### Canada Test (Prettified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/CanadaData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Canada%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Canada%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/s) | ReadLength (Bytes) | ReadTime (ns) | ReadMape (%) | Write (MB/s) | WriteLength (Bytes) | WriteTime (ns) | WriteMape (%) |
| ------------------------------------------------- | ---------- | ----------- | ---------- | ----------- | ---------- | ---------- | ---------- | ---------- |
| [simdjson](https://github.com/simdjson/simdjson/commit/417e760) | inf | 6661897 | 0 | 0 | 
| [glaze](https://github.com/stephenberry/glaze/commit/158ac74) | inf | 6661897 | 0 | 0 | inf | 6661897 | 0 | 0 |
| [jsonifier](https://github.com/RealTimeChris/Jsonifier/commit/) | inf | 6661897 | 0 | 0 | inf | 6661897 | 0 | 0 |

### Canada Test (Minified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/CanadaData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Canada%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Canada%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/s) | ReadLength (Bytes) | ReadTime (ns) | ReadMape (%) | Write (MB/s) | WriteLength (Bytes) | WriteTime (ns) | WriteMape (%) |
| ------------------------------------------------- | ---------- | ----------- | ---------- | ----------- | ---------- | ---------- | ---------- | ---------- |
| [simdjson](https://github.com/simdjson/simdjson/commit/417e760) | inf | 2090234 | 0 | 0 | 
| [glaze](https://github.com/stephenberry/glaze/commit/158ac74) | inf | 2090234 | 0 | 0 | inf | 2090234 | 0 | 0 |
| [jsonifier](https://github.com/RealTimeChris/Jsonifier/commit/) | inf | 2090234 | 0 | 0 | inf | 2090234 | 0 | 0 |

### Twitter Test (Prettified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/TwitterData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Twitter%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Twitter%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/s) | ReadLength (Bytes) | ReadTime (ns) | ReadMape (%) | Write (MB/s) | WriteLength (Bytes) | WriteTime (ns) | WriteMape (%) |
| ------------------------------------------------- | ---------- | ----------- | ---------- | ----------- | ---------- | ---------- | ---------- | ---------- |
| [simdjson](https://github.com/simdjson/simdjson/commit/417e760) | inf | 722038 | 0 | 0 | 
| [glaze](https://github.com/stephenberry/glaze/commit/158ac74) | inf | 722038 | 0 | 0 | inf | 721479 | 0 | 0 |
| [jsonifier](https://github.com/RealTimeChris/Jsonifier/commit/) | inf | 722038 | 0 | 0 | inf | 722038 | 0 | 0 |

### Twitter Test (Minified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/TwitterData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Twitter%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Twitter%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/s) | ReadLength (Bytes) | ReadTime (ns) | ReadMape (%) | Write (MB/s) | WriteLength (Bytes) | WriteTime (ns) | WriteMape (%) |
| ------------------------------------------------- | ---------- | ----------- | ---------- | ----------- | ---------- | ---------- | ---------- | ---------- |
| [simdjson](https://github.com/simdjson/simdjson/commit/417e760) | inf | 478986 | 0 | 0 | 
| [glaze](https://github.com/stephenberry/glaze/commit/158ac74) | inf | 478986 | 0 | 0 | inf | 478427 | 0 | 0 |
| [jsonifier](https://github.com/RealTimeChris/Jsonifier/commit/) | inf | 478986 | 0 | 0 | inf | 478986 | 0 | 0 |

### Minify Test Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Minify%20Test_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Minify%20Test_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Write (MB/s) | WriteLength (Bytes) | WriteTime (ns) | WriteMape (%) |
| ------------------------------------------------- | ---------- | ----------- | ---------- | ----------- |
| [simdjson](https://github.com/simdjson/simdjson/commit/417e760) | inf | 69037 | 0 | 0 |
| [glaze](https://github.com/stephenberry/glaze/commit/158ac74) | inf | 69037 | 0 | 0 |
| [jsonifier](https://github.com/RealTimeChris/Jsonifier/commit/) | inf | 69037 | 0 | 0 |

### Prettify Test Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Prettify%20Test_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Prettify%20Test_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Write (MB/s) | WriteLength (Bytes) | WriteTime (ns) | WriteMape (%) |
| ------------------------------------------------- | ---------- | ----------- | ---------- | ----------- |
| [glaze](https://github.com/stephenberry/glaze/commit/158ac74) | inf | 2245007 | 0 | 0 |
| [jsonifier](https://github.com/RealTimeChris/Jsonifier/commit/) | inf | 2245007 | 0 | 0 |

### Validation Test Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Validate%20Test_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Validate%20Test_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/s) | ReadLength (Bytes) | ReadTime (ns) | ReadMape (%) |
| ------------------------------------------------- | ---------- | ----------- | ---------- | ----------- |
| [glaze](https://github.com/stephenberry/glaze/commit/158ac74) | inf | 138774 | 0 | 0 | 
| [jsonifier](https://github.com/RealTimeChris/Jsonifier/commit/) | inf | 138774 | 0 | 0 | 