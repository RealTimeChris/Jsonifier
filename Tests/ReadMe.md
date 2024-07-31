# Json-Performance
Performance profiling of JSON libraries (Compiled and run on Ubuntu-22.04 using the Clang++18 compiler)

Latest Results: (Jul 30, 2024)
#### Using the following commits:
----
| Jsonifier: [](https://github.com/RealTimeChris/Jsonifier/commit/)  
| Glaze: [4887061](https://github.com/stephenberry/glaze/commit/4887061)  
| Simdjson: [ba1819f](https://github.com/simdjson/simdjson/commit/ba1819f)  

 > At least 40 iterations on a 6 core (Intel i7 8700k), until Median Absolute Percentage Error (MAPE) reduced below 5.0%.


### Json Test (Prettified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/JsonData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Json%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Json%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/s) | ReadLength (Bytes) | ReadTime (ns) | ReadMape (%) | Write (MB/s) | WriteLength (Bytes) | WriteTime (ns) | WriteMape (%) |
| ------------------------------------------------- | ---------- | ----------- | ---------- | ----------- | ---------- | ---------- | ---------- | ---------- |
| [glaze](https://github.com/stephenberry/glaze/commit/4887061) | 808.777 | 3063044 | 3.78726e+06 | 0.04034 | 1259.64 | 3062565 | 2.43131e+06 | 0.8428 |
| [jsonifier](https://github.com/RealTimeChris/Jsonifier/commit/) | 696.611 | 3063044 | 4.39707e+06 | 0.02366 | 1325.07 | 3058653 | 2.3083e+06 | 0.8155 |
| [simdjson](https://github.com/simdjson/simdjson/commit/ba1819f) | 148.784 | 3063044 | 2.05872e+07 | 0.7264 | 

### Json Test (Minified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/JsonData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Json%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Json%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/s) | ReadLength (Bytes) | ReadTime (ns) | ReadMape (%) | Write (MB/s) | WriteLength (Bytes) | WriteTime (ns) | WriteMape (%) |
| ------------------------------------------------- | ---------- | ----------- | ---------- | ----------- | ---------- | ---------- | ---------- | ---------- |
| [jsonifier](https://github.com/RealTimeChris/Jsonifier/commit/) | 755.103 | 2091044 | 2.76922e+06 | 0.03075 | 1060.28 | 2086653 | 1.96802e+06 | 0.02775 |
| [glaze](https://github.com/stephenberry/glaze/commit/4887061) | 692.076 | 2091044 | 3.02141e+06 | 0.6543 | 931.803 | 2090565 | 2.24357e+06 | 0.9171 |
| [simdjson](https://github.com/simdjson/simdjson/commit/ba1819f) | 106.9 | 2091044 | 1.95608e+07 | 0.5544 | 

### ABC Test (Out of Sequence Performance - Prettified) [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/JsonData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Abc%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Abc%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>

The JSON documents in the previous tests featured keys ranging from "a" to "z," where each key corresponds to an array of values. Notably, the documents in this test arrange these keys in reverse order, deviating from the typical "a" to "z" arrangement.

This test effectively demonstrates the challenges encountered when utilizing simdjson and iterative parsers that lack the ability to efficiently allocate memory locations through hashing. In cases where the keys are not in the expected sequence, performance is significantly compromised, with the severity escalating as the document size increases.

In contrast, hash-based solutions offer a viable alternative by circumventing these issues and maintaining optimal performance regardless of the JSON document's scale, or ordering of the keys being parsed.

| Library | Read (MB/s) | ReadLength (Bytes) | ReadTime (ns) | ReadMape (%) | Write (MB/s) | WriteLength (Bytes) | WriteTime (ns) | WriteMape (%) |
| ------------------------------------------------- | ---------- | ----------- | ---------- | ----------- | ---------- | ---------- | ---------- | ---------- |
| [glaze](https://github.com/stephenberry/glaze/commit/4887061) | 819.499 | 3063044 | 3.7377e+06 | 0.02271 | 1293.28 | 3062565 | 2.36806e+06 | 0.892 |
| [jsonifier](https://github.com/RealTimeChris/Jsonifier/commit/) | 716.285 | 3063044 | 4.27629e+06 | 0.02733 | 1364.05 | 3058653 | 2.24233e+06 | 0.7913 |
| [simdjson](https://github.com/simdjson/simdjson/commit/ba1819f) | 258.345 | 3063044 | 1.18564e+07 | 0.5859 | 

### ABC Test (Out of Sequence Performance - Minified) [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/JsonData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Abc%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Abc%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/s) | ReadLength (Bytes) | ReadTime (ns) | ReadMape (%) | Write (MB/s) | WriteLength (Bytes) | WriteTime (ns) | WriteMape (%) |
| ------------------------------------------------- | ---------- | ----------- | ---------- | ----------- | ---------- | ---------- | ---------- | ---------- |
| [jsonifier](https://github.com/RealTimeChris/Jsonifier/commit/) | 729.933 | 2091044 | 2.86471e+06 | 1.441 | 1024.18 | 2086653 | 2.03738e+06 | 0.8936 |
| [glaze](https://github.com/stephenberry/glaze/commit/4887061) | 699.71 | 2091044 | 2.98844e+06 | 0.557 | 869.706 | 2090565 | 2.40376e+06 | 1.386 |
| [simdjson](https://github.com/simdjson/simdjson/commit/ba1819f) | 189.903 | 2091044 | 1.10111e+07 | 0.6624 | 

### Discord Test (Prettified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Discord%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Discord%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/s) | ReadLength (Bytes) | ReadTime (ns) | ReadMape (%) | Write (MB/s) | WriteLength (Bytes) | WriteTime (ns) | WriteMape (%) |
| ------------------------------------------------- | ---------- | ----------- | ---------- | ----------- | ---------- | ---------- | ---------- | ---------- |
| [glaze](https://github.com/stephenberry/glaze/commit/4887061) | 867.865 | 138774 | 159903 | 0.01665 | 2778.98 | 138774 | 49937.1 | 0.09744 |
| [jsonifier](https://github.com/RealTimeChris/Jsonifier/commit/) | 199.819 | 138774 | 694497 | 0.1045 | 2572.61 | 138774 | 53942.8 | 0.01788 |
| [simdjson](https://github.com/simdjson/simdjson/commit/ba1819f) | 79.6578 | 138774 | 1.74213e+06 | 0.05306 | 

### Discord Test (Minified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Discord%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Discord%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/s) | ReadLength (Bytes) | ReadTime (ns) | ReadMape (%) | Write (MB/s) | WriteLength (Bytes) | WriteTime (ns) | WriteMape (%) |
| ------------------------------------------------- | ---------- | ----------- | ---------- | ----------- | ---------- | ---------- | ---------- | ---------- |
| [glaze](https://github.com/stephenberry/glaze/commit/4887061) | 626.118 | 69037 | 110262 | 0.04548 | 1821.79 | 69037 | 37895.2 | 0.05501 |
| [jsonifier](https://github.com/RealTimeChris/Jsonifier/commit/) | 107.08 | 69037 | 644725 | 0.0448 | 1799.67 | 69037 | 38360.9 | 0.05233 |
| [simdjson](https://github.com/simdjson/simdjson/commit/ba1819f) | 41.0527 | 69037 | 1.68167e+06 | 0.02206 | 

### Canada Test (Prettified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/CanadaData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Canada%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Canada%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/s) | ReadLength (Bytes) | ReadTime (ns) | ReadMape (%) | Write (MB/s) | WriteLength (Bytes) | WriteTime (ns) | WriteMape (%) |
| ------------------------------------------------- | ---------- | ----------- | ---------- | ----------- | ---------- | ---------- | ---------- | ---------- |
| [jsonifier](https://github.com/RealTimeChris/Jsonifier/commit/) | 1179.1 | 6661897 | 5.65e+06 | 1.651 | 1190.79 | 6661897 | 5.59451e+06 | 2.427 |
| [glaze](https://github.com/stephenberry/glaze/commit/4887061) | 957.731 | 6661897 | 6.95592e+06 | 1.333 | 1155.17 | 6661897 | 5.76705e+06 | 1.903 |
| [simdjson](https://github.com/simdjson/simdjson/commit/ba1819f) | 274.917 | 6661897 | 2.42324e+07 | 2.046 | 

### Canada Test (Minified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/CanadaData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Canada%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Canada%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/s) | ReadLength (Bytes) | ReadTime (ns) | ReadMape (%) | Write (MB/s) | WriteLength (Bytes) | WriteTime (ns) | WriteMape (%) |
| ------------------------------------------------- | ---------- | ----------- | ---------- | ----------- | ---------- | ---------- | ---------- | ---------- |
| [jsonifier](https://github.com/RealTimeChris/Jsonifier/commit/) | 702.068 | 2090234 | 2.97725e+06 | 2.07 | 427.757 | 2090234 | 4.8865e+06 | 1.398 |
| [glaze](https://github.com/stephenberry/glaze/commit/4887061) | 428.908 | 2090234 | 4.87339e+06 | 1.674 | 377.937 | 2090234 | 5.53065e+06 | 1.094 |
| [simdjson](https://github.com/simdjson/simdjson/commit/ba1819f) | 101.174 | 2090234 | 2.06598e+07 | 2.193 | 

### Twitter Test (Prettified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/TwitterData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Twitter%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Twitter%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/s) | ReadLength (Bytes) | ReadTime (ns) | ReadMape (%) | Write (MB/s) | WriteLength (Bytes) | WriteTime (ns) | WriteMape (%) |
| ------------------------------------------------- | ---------- | ----------- | ---------- | ----------- | ---------- | ---------- | ---------- | ---------- |
| [glaze](https://github.com/stephenberry/glaze/commit/4887061) | 875.785 | 722038 | 824446 | 0.235 | 2670.79 | 722415 | 270487 | 0.01983 |
| [jsonifier](https://github.com/RealTimeChris/Jsonifier/commit/) | 220.64 | 722038 | 3.27247e+06 | 1.923 | 2563.89 | 722038 | 281618 | 0.01302 |
| [simdjson](https://github.com/simdjson/simdjson/commit/ba1819f) | 115.81 | 722038 | 6.23467e+06 | 1.569 | 

### Twitter Test (Minified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/TwitterData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Twitter%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Twitter%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/s) | ReadLength (Bytes) | ReadTime (ns) | ReadMape (%) | Write (MB/s) | WriteLength (Bytes) | WriteTime (ns) | WriteMape (%) |
| ------------------------------------------------- | ---------- | ----------- | ---------- | ----------- | ---------- | ---------- | ---------- | ---------- |
| [glaze](https://github.com/stephenberry/glaze/commit/4887061) | 739.384 | 478986 | 647818 | 0.09085 | 2163.08 | 479363 | 221611 | 0.02924 |
| [jsonifier](https://github.com/RealTimeChris/Jsonifier/commit/) | 168.559 | 478986 | 2.84165e+06 | 2.72 | 2323.01 | 478986 | 206192 | 0.01496 |
| [simdjson](https://github.com/simdjson/simdjson/commit/ba1819f) | 78.4668 | 478986 | 6.10431e+06 | 1.867 | 

### Minify Test Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Minify%20Test_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Minify%20Test_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Write (MB/s) | WriteLength (Bytes) | WriteTime (ns) | WriteMape (%) |
| ------------------------------------------------- | ---------- | ----------- | ---------- | ----------- |
| [glaze](https://github.com/stephenberry/glaze/commit/4887061) | 511.502 | 69037 | 134969 | 0.634 |
| [jsonifier](https://github.com/RealTimeChris/Jsonifier/commit/) | 446.533 | 69037 | 154607 | 0.01797 |
| [simdjson](https://github.com/simdjson/simdjson/commit/ba1819f) | 162.803 | 69037 | 424053 | 0.05116 |

### Prettify Test Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Prettify%20Test_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Prettify%20Test_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Write (MB/s) | WriteLength (Bytes) | WriteTime (ns) | WriteMape (%) |
| ------------------------------------------------- | ---------- | ----------- | ---------- | ----------- |
| [jsonifier](https://github.com/RealTimeChris/Jsonifier/commit/) | 1274.84 | 3063044 | 2.40269e+06 | 2.625 |
| [glaze](https://github.com/stephenberry/glaze/commit/4887061) | 1204.12 | 3063044 | 2.54381e+06 | 0.747 |

### Validation Test Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Validate%20Test_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Validate%20Test_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/s) | ReadLength (Bytes) | ReadTime (ns) | ReadMape (%) |
| ------------------------------------------------- | ---------- | ----------- | ---------- | ----------- |
| [glaze](https://github.com/stephenberry/glaze/commit/4887061) | 866.707 | 138774 | 160116 | 0.05232 | 
| [jsonifier](https://github.com/RealTimeChris/Jsonifier/commit/) | 588.826 | 138774 | 235679 | 7.704 | 