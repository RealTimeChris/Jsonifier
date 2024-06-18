# Json-Performance
Performance profiling of JSON libraries (Compiled and run on Ubuntu-22.04 using the Clang++18 compiler)

Latest Results: (Jun 25, 2024)

 > At least 0 iterations on a 6 core (Intel i7 8700k), until Median Absolute Percentage Error (Mape) reduced below 5.0%.


### Json Test (Prettified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/JsonData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Json%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Json%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/s) | ReadLength (Bytes) | ReadTime (ns) | ReadMape (%) | Write (MB/s) | WriteLength (Bytes) | WriteTime (ns) | WriteMape (%) |
| ------------------------------------------------- | ---------- | ----------- | ---------- | ----------- | ---------- | ---------- | ---------- | ---------- |
| [glaze](https://github.com/stephenberry/glaze) | 94.9845 | 3076547 | 3.239e+07 | 0 | 124.827 | 3076067 | 2.46427e+07 | 0 |
| [jsonifier](https://github.com/realtimechris/jsonifier) | 93.8679 | 3076547 | 3.27753e+07 | 0 | 146.036 | 3072270 | 2.10377e+07 | 0 |
| [simdjson](https://github.com/simdjson/simdjson) | 6.9476 | 3076547 | 4.42822e+08 | 0 | 

### Json Test (Minified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/JsonData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Json%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Json%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/s) | ReadLength (Bytes) | ReadTime (ns) | ReadMape (%) | Write (MB/s) | WriteLength (Bytes) | WriteTime (ns) | WriteMape (%) |
| ------------------------------------------------- | ---------- | ----------- | ---------- | ----------- | ---------- | ---------- | ---------- | ---------- |
| [jsonifier](https://github.com/realtimechris/jsonifier) | 75.9035 | 2093708 | 2.75838e+07 | 0 | 103.729 | 2089431 | 2.01432e+07 | 0 |
| [glaze](https://github.com/stephenberry/glaze) | 65.1172 | 2093708 | 3.21529e+07 | 0 | 61.3715 | 2093228 | 3.41075e+07 | 0 |
| [simdjson](https://github.com/simdjson/simdjson) | 4.63466 | 2093708 | 4.5175e+08 | 0 | 

### ABC Test (Out of Sequence Performance - Prettified) [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/JsonData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Abc%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Abc%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>

The JSON documents in the previous tests featured keys ranging from "a" to "z," where each key corresponds to an array of values. Notably, the documents in this test arrange these keys in reverse order, deviating from the typical "a" to "z" arrangement.

This test effectively demonstrates the challenges encountered when utilizing simdjson and iterative parsers that lack the ability to efficiently allocate memory locations through hashing. In cases where the keys are not in the expected sequence, performance is significantly compromised, with the severity escalating as the document size increases.

In contrast, hash-based solutions offer a viable alternative by circumventing these issues and maintaining optimal performance regardless of the JSON document's scale, or ordering of the keys being parsed.

| Library | Read (MB/s) | ReadLength (Bytes) | ReadTime (ns) | ReadMape (%) | Write (MB/s) | WriteLength (Bytes) | WriteTime (ns) | WriteMape (%) |
| ------------------------------------------------- | ---------- | ----------- | ---------- | ----------- | ---------- | ---------- | ---------- | ---------- |
| [jsonifier](https://github.com/realtimechris/jsonifier) | 91.408 | 3076547 | 3.36573e+07 | 0 | 135.466 | 3072270 | 2.26792e+07 | 0 |
| [glaze](https://github.com/stephenberry/glaze) | 78.0746 | 3076547 | 3.94052e+07 | 0 | 96.3898 | 3076067 | 3.19128e+07 | 0 |
| [simdjson](https://github.com/simdjson/simdjson) | 2.85534 | 3076547 | 1.07747e+09 | 0 | 

### ABC Test (Out of Sequence Performance - Minified) [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/JsonData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Abc%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Abc%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/s) | ReadLength (Bytes) | ReadTime (ns) | ReadMape (%) | Write (MB/s) | WriteLength (Bytes) | WriteTime (ns) | WriteMape (%) |
| ------------------------------------------------- | ---------- | ----------- | ---------- | ----------- | ---------- | ---------- | ---------- | ---------- |
| [jsonifier](https://github.com/realtimechris/jsonifier) | 98.1009 | 2093708 | 2.13424e+07 | 0 | 106.484 | 2089431 | 1.9622e+07 | 0 |
| [glaze](https://github.com/stephenberry/glaze) | 63.1592 | 2093708 | 3.31497e+07 | 0 | 50.5398 | 2093228 | 4.14174e+07 | 0 |
| [simdjson](https://github.com/simdjson/simdjson) | 2.39687 | 2093708 | 8.73518e+08 | 0 | 

### Discord Test (Prettified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Discord%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Discord%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/s) | ReadLength (Bytes) | ReadTime (ns) | ReadMape (%) | Write (MB/s) | WriteLength (Bytes) | WriteTime (ns) | WriteMape (%) |
| ------------------------------------------------- | ---------- | ----------- | ---------- | ----------- | ---------- | ---------- | ---------- | ---------- |
| [glaze](https://github.com/stephenberry/glaze) | 87.5386 | 139414 | 1.5926e+06 | 0 | 156.346 | 139414 | 891700 | 0 |
| [jsonifier](https://github.com/realtimechris/jsonifier) | 73.8031 | 139414 | 1.889e+06 | 0 | 79.2125 | 139414 | 1.76e+06 | 0 |
| [simdjson](https://github.com/simdjson/simdjson) | 9.71824 | 139414 | 1.43456e+07 | 0 | 

### Discord Test (Minified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Discord%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Discord%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/s) | ReadLength (Bytes) | ReadTime (ns) | ReadMape (%) | Write (MB/s) | WriteLength (Bytes) | WriteTime (ns) | WriteMape (%) |
| ------------------------------------------------- | ---------- | ----------- | ---------- | ----------- | ---------- | ---------- | ---------- | ---------- |
| [glaze](https://github.com/stephenberry/glaze) | 54.8246 | 69397 | 1.2658e+06 | 0 | 100.183 | 69397 | 692700 | 0 |
| [jsonifier](https://github.com/realtimechris/jsonifier) | 20.1431 | 69397 | 3.4452e+06 | 0 | 129.536 | 15622 | 120600 | 0 |
| [simdjson](https://github.com/simdjson/simdjson) | 5.80558 | 69397 | 1.19535e+07 | 0 | 

### Canada Test (Prettified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/CanadaData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Canada%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Canada%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/s) | ReadLength (Bytes) | ReadTime (ns) | ReadMape (%) | Write (MB/s) | WriteLength (Bytes) | WriteTime (ns) | WriteMape (%) |
| ------------------------------------------------- | ---------- | ----------- | ---------- | ----------- | ---------- | ---------- | ---------- | ---------- |
| [jsonifier](https://github.com/realtimechris/jsonifier) | 91.9258 | 6661897 | 7.24704e+07 | 0 | 85.6989 | 6661897 | 7.77361e+07 | 0 |
| [glaze](https://github.com/stephenberry/glaze) | 44.8968 | 6661897 | 1.48382e+08 | 0 | 81.611 | 6661897 | 8.16299e+07 | 0 |
| [simdjson](https://github.com/simdjson/simdjson) | 12.1915 | 6661897 | 5.46436e+08 | 0 | 

### Canada Test (Minified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/CanadaData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Canada%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Canada%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/s) | ReadLength (Bytes) | ReadTime (ns) | ReadMape (%) | Write (MB/s) | WriteLength (Bytes) | WriteTime (ns) | WriteMape (%) |
| ------------------------------------------------- | ---------- | ----------- | ---------- | ----------- | ---------- | ---------- | ---------- | ---------- |
| [jsonifier](https://github.com/realtimechris/jsonifier) | 112.801 | 2090234 | 1.85303e+07 | 0 | 31.25 | 25 | 800 | 0 |
| [glaze](https://github.com/stephenberry/glaze) | 29.5632 | 2090234 | 7.07039e+07 | 0 | 30.8792 | 2090234 | 6.76906e+07 | 0 |
| [simdjson](https://github.com/simdjson/simdjson) | 5.26489 | 2090234 | 3.97014e+08 | 0 | 

### Twitter Test (Prettified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/TwitterData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Twitter%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Twitter%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/s) | ReadLength (Bytes) | ReadTime (ns) | ReadMape (%) | Write (MB/s) | WriteLength (Bytes) | WriteTime (ns) | WriteMape (%) |
| ------------------------------------------------- | ---------- | ----------- | ---------- | ----------- | ---------- | ---------- | ---------- | ---------- |
| [glaze](https://github.com/stephenberry/glaze) | 40.5719 | 722038 | 1.77965e+07 | 0 | 89.6002 | 721479 | 8.0522e+06 | 0 |
| [jsonifier](https://github.com/realtimechris/jsonifier) | 40.2732 | 722038 | 1.79285e+07 | 0 | 81.8081 | 722038 | 8.826e+06 | 0 |
| [simdjson](https://github.com/simdjson/simdjson) | 6.28907 | 722038 | 1.14808e+08 | 0 | 

### Twitter Test (Minified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/TwitterData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Twitter%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Twitter%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/s) | ReadLength (Bytes) | ReadTime (ns) | ReadMape (%) | Write (MB/s) | WriteLength (Bytes) | WriteTime (ns) | WriteMape (%) |
| ------------------------------------------------- | ---------- | ----------- | ---------- | ----------- | ---------- | ---------- | ---------- | ---------- |
| [jsonifier](https://github.com/realtimechris/jsonifier) | 330.312 | 478986 | 1.4501e+06 | 0 | 61.4815 | 166 | 2700 | 0 |
| [glaze](https://github.com/stephenberry/glaze) | 38.0096 | 478986 | 1.26017e+07 | 0 | 99.7637 | 478427 | 4.7956e+06 | 0 |
| [simdjson](https://github.com/simdjson/simdjson) | 4.04536 | 478986 | 1.18404e+08 | 0 | 

### Minify Test Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Minify%20Test_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Minify%20Test_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Write (MB/s) | WriteLength (Bytes) | WriteTime (ns) | WriteMape (%) |
| ------------------------------------------------- | ---------- | ----------- | ---------- | ----------- |
| [jsonifier](https://github.com/realtimechris/jsonifier) | 50.8962 | 69397 | 1.3635e+06 | 0 |
| [glaze](https://github.com/stephenberry/glaze) | 23.1965 | 69397 | 2.9917e+06 | 0 |
| [simdjson](https://github.com/simdjson/simdjson) | 3.09572 | 69397 | 2.24171e+07 | 0 |

### Prettify Test Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Prettify%20Test_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Prettify%20Test_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Write (MB/s) | WriteLength (Bytes) | WriteTime (ns) | WriteMape (%) |
| ------------------------------------------------- | ---------- | ----------- | ---------- | ----------- |
| [glaze](https://github.com/stephenberry/glaze) | 133.425 | 3076547 | 2.30582e+07 | 0 |
| [jsonifier](https://github.com/realtimechris/jsonifier) | 121.717 | 3076547 | 2.52763e+07 | 0 |

### Validation Test Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Validate%20Test_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Validate%20Test_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/s) | ReadLength (Bytes) | ReadTime (ns) | ReadMape (%) |
| ------------------------------------------------- | ---------- | ----------- | ---------- | ----------- |
| [jsonifier](https://github.com/realtimechris/jsonifier) | 93.9891 | 139414 | 1.4833e+06 | 0 | 