# Json-Performance
Performance profiling of JSON libraries (Compiled and run on Ubuntu-22.04 using the Clang++18 compiler)

Latest Results: (Jul 31, 2024)
#### Using the following commits:
----
| Jsonifier: [](https://github.com/RealTimeChris/Jsonifier/commit/)  
| Glaze: [2222977](https://github.com/stephenberry/glaze/commit/2222977)  
| Simdjson: [ba1819f](https://github.com/simdjson/simdjson/commit/ba1819f)  

 > At least 40 iterations on a 6 core (Intel i7 8700k), until Median Absolute Percentage Error (MAPE) reduced below 5.0%.


### Json Test (Prettified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/JsonData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Json%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Json%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/s) | ReadLength (Bytes) | ReadTime (ns) | ReadMape (%) | Write (MB/s) | WriteLength (Bytes) | WriteTime (ns) | WriteMape (%) |
| ------------------------------------------------- | ---------- | ----------- | ---------- | ----------- | ---------- | ---------- | ---------- | ---------- |
| [glaze](https://github.com/stephenberry/glaze/commit/2222977) | 831.176 | 3180391 | 3.82638e+06 | 0.002279 | 1241.25 | 3179915 | 2.56187e+06 | 0.7904 |
| [jsonifier](https://github.com/RealTimeChris/Jsonifier/commit/) | 720.91 | 3180391 | 4.41163e+06 | 0.5238 | 1269.61 | 3175785 | 2.50138e+06 | 0.5957 |
| [simdjson](https://github.com/simdjson/simdjson/commit/ba1819f) | 152.61 | 3180391 | 2.084e+07 | 0.575 | 

### Json Test (Minified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/JsonData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Json%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Json%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/s) | ReadLength (Bytes) | ReadTime (ns) | ReadMape (%) | Write (MB/s) | WriteLength (Bytes) | WriteTime (ns) | WriteMape (%) |
| ------------------------------------------------- | ---------- | ----------- | ---------- | ----------- | ---------- | ---------- | ---------- | ---------- |
| [jsonifier](https://github.com/RealTimeChris/Jsonifier/commit/) | 748.614 | 2168834 | 2.89713e+06 | 0.02533 | 1004.24 | 2164228 | 2.1551e+06 | 0.7349 |
| [glaze](https://github.com/stephenberry/glaze/commit/2222977) | 666.607 | 2168834 | 3.25354e+06 | 0.0595 | 906.161 | 2168358 | 2.39291e+06 | 0.03015 |
| [simdjson](https://github.com/simdjson/simdjson/commit/ba1819f) | 110.18 | 2168834 | 1.96845e+07 | 0.679 | 

### ABC Test (Out of Sequence Performance - Prettified) [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/JsonData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Abc%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Abc%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>

The JSON documents in the previous tests featured keys ranging from "a" to "z," where each key corresponds to an array of values. Notably, the documents in this test arrange these keys in reverse order, deviating from the typical "a" to "z" arrangement.

This test effectively demonstrates the challenges encountered when utilizing simdjson and iterative parsers that lack the ability to efficiently allocate memory locations through hashing. In cases where the keys are not in the expected sequence, performance is significantly compromised, with the severity escalating as the document size increases.

In contrast, hash-based solutions offer a viable alternative by circumventing these issues and maintaining optimal performance regardless of the JSON document's scale, or ordering of the keys being parsed.

| Library | Read (MB/s) | ReadLength (Bytes) | ReadTime (ns) | ReadMape (%) | Write (MB/s) | WriteLength (Bytes) | WriteTime (ns) | WriteMape (%) |
| ------------------------------------------------- | ---------- | ----------- | ---------- | ----------- | ---------- | ---------- | ---------- | ---------- |
| [glaze](https://github.com/stephenberry/glaze/commit/2222977) | 823.997 | 3180391 | 3.85971e+06 | 0.04546 | 1241.33 | 3179915 | 2.56169e+06 | 0.0377 |
| [jsonifier](https://github.com/RealTimeChris/Jsonifier/commit/) | 727.886 | 3180391 | 4.36935e+06 | 0.6232 | 1284.16 | 3175785 | 2.47305e+06 | 0.7388 |
| [simdjson](https://github.com/simdjson/simdjson/commit/ba1819f) | 266.815 | 3180391 | 1.19199e+07 | 0.6182 | 

### ABC Test (Out of Sequence Performance - Minified) [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/JsonData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Abc%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Abc%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/s) | ReadLength (Bytes) | ReadTime (ns) | ReadMape (%) | Write (MB/s) | WriteLength (Bytes) | WriteTime (ns) | WriteMape (%) |
| ------------------------------------------------- | ---------- | ----------- | ---------- | ----------- | ---------- | ---------- | ---------- | ---------- |
| [jsonifier](https://github.com/RealTimeChris/Jsonifier/commit/) | 726.213 | 2168834 | 2.9865e+06 | 0.02493 | 955.018 | 2164228 | 2.26617e+06 | 1.48 |
| [glaze](https://github.com/stephenberry/glaze/commit/2222977) | 686.09 | 2168834 | 3.16115e+06 | 1.154 | 848.382 | 2168358 | 2.55588e+06 | 1.96 |
| [simdjson](https://github.com/simdjson/simdjson/commit/ba1819f) | 189.595 | 2168834 | 1.14393e+07 | 1.677 | 

### Discord Test (Prettified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Discord%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Discord%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/s) | ReadLength (Bytes) | ReadTime (ns) | ReadMape (%) | Write (MB/s) | WriteLength (Bytes) | WriteTime (ns) | WriteMape (%) |
| ------------------------------------------------- | ---------- | ----------- | ---------- | ----------- | ---------- | ---------- | ---------- | ---------- |
| [glaze](https://github.com/stephenberry/glaze/commit/2222977) | 897.879 | 138774 | 154558 | 0.04271 | 2831.08 | 138774 | 49018 | 0.1001 |
| [jsonifier](https://github.com/RealTimeChris/Jsonifier/commit/) | 203.226 | 138774 | 682855 | 0.02148 | 2603.74 | 138774 | 53298 | 0.02969 |
| [simdjson](https://github.com/simdjson/simdjson/commit/ba1819f) | 84.3838 | 138774 | 1.64456e+06 | 0.06296 | 

### Discord Test (Minified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Discord%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Discord%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/s) | ReadLength (Bytes) | ReadTime (ns) | ReadMape (%) | Write (MB/s) | WriteLength (Bytes) | WriteTime (ns) | WriteMape (%) |
| ------------------------------------------------- | ---------- | ----------- | ---------- | ----------- | ---------- | ---------- | ---------- | ---------- |
| [glaze](https://github.com/stephenberry/glaze/commit/2222977) | 638.827 | 69037 | 108068 | 0.0611 | 1802.88 | 69037 | 38292.6 | 0.2005 |
| [jsonifier](https://github.com/RealTimeChris/Jsonifier/commit/) | 108.921 | 69037 | 633825 | 0.007731 | 1806.6 | 69037 | 38213.8 | 0.05051 |
| [simdjson](https://github.com/simdjson/simdjson/commit/ba1819f) | 44.0624 | 69037 | 1.5668e+06 | 0.02098 | 

### Canada Test (Prettified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/CanadaData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Canada%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Canada%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/s) | ReadLength (Bytes) | ReadTime (ns) | ReadMape (%) | Write (MB/s) | WriteLength (Bytes) | WriteTime (ns) | WriteMape (%) |
| ------------------------------------------------- | ---------- | ----------- | ---------- | ----------- | ---------- | ---------- | ---------- | ---------- |
| [jsonifier](https://github.com/RealTimeChris/Jsonifier/commit/) | 1156.11 | 6661897 | 5.76235e+06 | 0.9842 | 1223.05 | 6661897 | 5.44696e+06 | 1.058 |
| [glaze](https://github.com/stephenberry/glaze/commit/2222977) | 893.666 | 6661897 | 7.45457e+06 | 1.03 | 1083.98 | 6661897 | 6.14577e+06 | 1.089 |
| [simdjson](https://github.com/simdjson/simdjson/commit/ba1819f) | 521.86 | 6661897 | 1.27657e+07 | 0.1494 | 

### Canada Test (Minified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/CanadaData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Canada%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Canada%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/s) | ReadLength (Bytes) | ReadTime (ns) | ReadMape (%) | Write (MB/s) | WriteLength (Bytes) | WriteTime (ns) | WriteMape (%) |
| ------------------------------------------------- | ---------- | ----------- | ---------- | ----------- | ---------- | ---------- | ---------- | ---------- |
| [jsonifier](https://github.com/RealTimeChris/Jsonifier/commit/) | 777.976 | 2090234 | 2.68676e+06 | 0.01009 | 462.497 | 2090234 | 4.51945e+06 | 1.064 |
| [glaze](https://github.com/stephenberry/glaze/commit/2222977) | 376.264 | 2090234 | 5.55523e+06 | 0.02456 | 436.208 | 2090234 | 4.79183e+06 | 0.04216 |
| [simdjson](https://github.com/simdjson/simdjson/commit/ba1819f) | 227.258 | 2090234 | 9.19764e+06 | 0.055 | 

### Twitter Test (Prettified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/TwitterData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Twitter%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Twitter%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/s) | ReadLength (Bytes) | ReadTime (ns) | ReadMape (%) | Write (MB/s) | WriteLength (Bytes) | WriteTime (ns) | WriteMape (%) |
| ------------------------------------------------- | ---------- | ----------- | ---------- | ----------- | ---------- | ---------- | ---------- | ---------- |
| [glaze](https://github.com/stephenberry/glaze/commit/2222977) | 1035.87 | 722038 | 697036 | 0.102 | 2801.93 | 722415 | 257828 | 0.3127 |
| [jsonifier](https://github.com/RealTimeChris/Jsonifier/commit/) | 252.974 | 722038 | 2.8542e+06 | 0.007272 | 2736.15 | 722038 | 263888 | 0.1348 |
| [simdjson](https://github.com/simdjson/simdjson/commit/ba1819f) | 130.316 | 722038 | 5.54066e+06 | 0.03626 | 

### Twitter Test (Minified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/TwitterData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Twitter%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Twitter%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/s) | ReadLength (Bytes) | ReadTime (ns) | ReadMape (%) | Write (MB/s) | WriteLength (Bytes) | WriteTime (ns) | WriteMape (%) |
| ------------------------------------------------- | ---------- | ----------- | ---------- | ----------- | ---------- | ---------- | ---------- | ---------- |
| [glaze](https://github.com/stephenberry/glaze/commit/2222977) | 875.031 | 478986 | 547393 | 0.02642 | 2190.11 | 479363 | 218877 | 0.004759 |
| [jsonifier](https://github.com/RealTimeChris/Jsonifier/commit/) | 173.511 | 478986 | 2.76054e+06 | 0.974 | 2402.01 | 478986 | 199411 | 0.02245 |
| [simdjson](https://github.com/simdjson/simdjson/commit/ba1819f) | 90.7765 | 478986 | 5.27654e+06 | 0.6105 | 

### Minify Test Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Minify%20Test_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Minify%20Test_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Write (MB/s) | WriteLength (Bytes) | WriteTime (ns) | WriteMape (%) |
| ------------------------------------------------- | ---------- | ----------- | ---------- | ----------- |
| [jsonifier](https://github.com/RealTimeChris/Jsonifier/commit/) | 494.8 | 69037 | 139525 | 0.7651 |
| [glaze](https://github.com/stephenberry/glaze/commit/2222977) | 461.856 | 69037 | 149477 | 0.01626 |
| [simdjson](https://github.com/simdjson/simdjson/commit/ba1819f) | 159.201 | 69037 | 433646 | 0.09289 |

### Prettify Test Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Prettify%20Test_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Prettify%20Test_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Write (MB/s) | WriteLength (Bytes) | WriteTime (ns) | WriteMape (%) |
| ------------------------------------------------- | ---------- | ----------- | ---------- | ----------- |
| [jsonifier](https://github.com/RealTimeChris/Jsonifier/commit/) | 1144.73 | 3180391 | 2.77828e+06 | 2.213 |
| [glaze](https://github.com/stephenberry/glaze/commit/2222977) | 1102.26 | 3180391 | 2.88535e+06 | 0.7101 |

### Validation Test Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Validate%20Test_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Validate%20Test_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/s) | ReadLength (Bytes) | ReadTime (ns) | ReadMape (%) |
| ------------------------------------------------- | ---------- | ----------- | ---------- | ----------- |
| [glaze](https://github.com/stephenberry/glaze/commit/2222977) | 953.549 | 138774 | 145534 | 0.5759 | 
| [jsonifier](https://github.com/RealTimeChris/Jsonifier/commit/) | 783.763 | 138774 | 177061 | 0.6236 | 