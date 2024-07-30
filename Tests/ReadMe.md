# Json-Performance
Performance profiling of JSON libraries (Compiled and run on Ubuntu-22.04 using the Clang++18 compiler)

Latest Results: (Jul 30, 2024)
#### Using the following commits:
----
| Jsonifier: [](https://github.com/RealTimeChris/Jsonifier/commit/)  
| Glaze: [158ac74](https://github.com/stephenberry/glaze/commit/158ac74)  
| Simdjson: [ba1819f](https://github.com/simdjson/simdjson/commit/ba1819f)  

 > At least 40 iterations on a 6 core (Intel i7 8700k), until Median Absolute Percentage Error (MAPE) reduced below 5.0%.


### Json Test (Prettified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/JsonData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Json%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Json%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/s) | ReadLength (Bytes) | ReadTime (ns) | ReadMape (%) | Write (MB/s) | WriteLength (Bytes) | WriteTime (ns) | WriteMape (%) |
| ------------------------------------------------- | ---------- | ----------- | ---------- | ----------- | ---------- | ---------- | ---------- | ---------- |
| [glaze](https://github.com/stephenberry/glaze/commit/158ac74) | 653.955 | 3583236 | 5.47934e+06 | 0.907 | 853.912 | 3582693 | 4.19562e+06 | 2.572 |
| [jsonifier](https://github.com/RealTimeChris/Jsonifier/commit/) | 581.202 | 3583236 | 6.16522e+06 | 3.744 | 1075.61 | 3578180 | 3.32665e+06 | 1.623 |
| [simdjson](https://github.com/simdjson/simdjson/commit/ba1819f) | 204.729 | 3583236 | 1.75024e+07 | 4.375 | 

### Json Test (Minified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/JsonData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Json%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Json%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/s) | ReadLength (Bytes) | ReadTime (ns) | ReadMape (%) | Write (MB/s) | WriteLength (Bytes) | WriteTime (ns) | WriteMape (%) |
| ------------------------------------------------- | ---------- | ----------- | ---------- | ----------- | ---------- | ---------- | ---------- | ---------- |
| [jsonifier](https://github.com/RealTimeChris/Jsonifier/commit/) | 623.981 | 2446781 | 3.92124e+06 | 1.179 | 827.199 | 2441725 | 2.9518e+06 | 0.9041 |
| [glaze](https://github.com/stephenberry/glaze/commit/158ac74) | 552.352 | 2446781 | 4.42975e+06 | 1.173 | 694.856 | 2446238 | 3.5205e+06 | 0.1198 |
| [simdjson](https://github.com/simdjson/simdjson/commit/ba1819f) | 160.697 | 2446781 | 1.5226e+07 | 0.9937 | 

### ABC Test (Out of Sequence Performance - Prettified) [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/JsonData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Abc%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Abc%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>

The JSON documents in the previous tests featured keys ranging from "a" to "z," where each key corresponds to an array of values. Notably, the documents in this test arrange these keys in reverse order, deviating from the typical "a" to "z" arrangement.

This test effectively demonstrates the challenges encountered when utilizing simdjson and iterative parsers that lack the ability to efficiently allocate memory locations through hashing. In cases where the keys are not in the expected sequence, performance is significantly compromised, with the severity escalating as the document size increases.

In contrast, hash-based solutions offer a viable alternative by circumventing these issues and maintaining optimal performance regardless of the JSON document's scale, or ordering of the keys being parsed.

| Library | Read (MB/s) | ReadLength (Bytes) | ReadTime (ns) | ReadMape (%) | Write (MB/s) | WriteLength (Bytes) | WriteTime (ns) | WriteMape (%) |
| ------------------------------------------------- | ---------- | ----------- | ---------- | ----------- | ---------- | ---------- | ---------- | ---------- |
| [glaze](https://github.com/stephenberry/glaze/commit/158ac74) | 698.352 | 3583236 | 5.13099e+06 | 1.105 | 1000.94 | 3582693 | 3.57934e+06 | 0.8798 |
| [jsonifier](https://github.com/RealTimeChris/Jsonifier/commit/) | 608.059 | 3583236 | 5.89291e+06 | 0.6514 | 1070.29 | 3578180 | 3.34318e+06 | 0.6291 |
| [simdjson](https://github.com/simdjson/simdjson/commit/ba1819f) | 125.581 | 3583236 | 2.85333e+07 | 0.7179 | 

### ABC Test (Out of Sequence Performance - Minified) [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/JsonData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Abc%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Abc%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/s) | ReadLength (Bytes) | ReadTime (ns) | ReadMape (%) | Write (MB/s) | WriteLength (Bytes) | WriteTime (ns) | WriteMape (%) |
| ------------------------------------------------- | ---------- | ----------- | ---------- | ----------- | ---------- | ---------- | ---------- | ---------- |
| [jsonifier](https://github.com/RealTimeChris/Jsonifier/commit/) | 653.254 | 2446781 | 3.74553e+06 | 1.502 | 847.042 | 2441725 | 2.88265e+06 | 0.6658 |
| [glaze](https://github.com/stephenberry/glaze/commit/158ac74) | 578.509 | 2446781 | 4.22946e+06 | 1.055 | 735.56 | 2446238 | 3.32568e+06 | 1.178 |
| [simdjson](https://github.com/simdjson/simdjson/commit/ba1819f) | 91.9279 | 2446781 | 2.66163e+07 | 0.9313 | 

### Discord Test (Prettified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Discord%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Discord%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/s) | ReadLength (Bytes) | ReadTime (ns) | ReadMape (%) | Write (MB/s) | WriteLength (Bytes) | WriteTime (ns) | WriteMape (%) |
| ------------------------------------------------- | ---------- | ----------- | ---------- | ----------- | ---------- | ---------- | ---------- | ---------- |
| [glaze](https://github.com/stephenberry/glaze/commit/158ac74) | 384.129 | 1629 | 4240.76 | 0 | 1370 | 1629 | 1189.05 | 2.715 |
| [jsonifier](https://github.com/RealTimeChris/Jsonifier/commit/) | 147.712 | 1629 | 11028.2 | 0.006996 | 2152.06 | 1629 | 756.948 | 0.6079 |
| [simdjson](https://github.com/simdjson/simdjson/commit/ba1819f) | 83.3251 | 1629 | 19549.9 | 0.08879 | 

### Discord Test (Minified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Discord%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Discord%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/s) | ReadLength (Bytes) | ReadTime (ns) | ReadMape (%) | Write (MB/s) | WriteLength (Bytes) | WriteTime (ns) | WriteMape (%) |
| ------------------------------------------------- | ---------- | ----------- | ---------- | ----------- | ---------- | ---------- | ---------- | ---------- |
| [glaze](https://github.com/stephenberry/glaze/commit/158ac74) | 465.56 | 1156 | 2483.03 | 0.1397 | 2092.41 | 1156 | 552.473 | 0.5618 |
| [jsonifier](https://github.com/RealTimeChris/Jsonifier/commit/) | 107.894 | 1156 | 10714.2 | 0.01801 | 2070.71 | 1156 | 558.262 | 0.9066 |
| [simdjson](https://github.com/simdjson/simdjson/commit/ba1819f) | 61.9181 | 1156 | 18669.8 | 0.1677 | 

### Canada Test (Prettified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/CanadaData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Canada%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Canada%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/s) | ReadLength (Bytes) | ReadTime (ns) | ReadMape (%) | Write (MB/s) | WriteLength (Bytes) | WriteTime (ns) | WriteMape (%) |
| ------------------------------------------------- | ---------- | ----------- | ---------- | ----------- | ---------- | ---------- | ---------- | ---------- |
| [jsonifier](https://github.com/RealTimeChris/Jsonifier/commit/) | 1029.52 | 6661897 | 6.4709e+06 | 1.276 | 1013.14 | 6661897 | 6.57547e+06 | 3.745 |
| [glaze](https://github.com/stephenberry/glaze/commit/158ac74) | 873.411 | 6661897 | 7.62745e+06 | 0.7644 | 1085.53 | 6661897 | 6.13702e+06 | 0.896 |
| [simdjson](https://github.com/simdjson/simdjson/commit/ba1819f) | 249.329 | 6661897 | 2.67193e+07 | 0.7333 | 

### Canada Test (Minified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/CanadaData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Canada%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Canada%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/s) | ReadLength (Bytes) | ReadTime (ns) | ReadMape (%) | Write (MB/s) | WriteLength (Bytes) | WriteTime (ns) | WriteMape (%) |
| ------------------------------------------------- | ---------- | ----------- | ---------- | ----------- | ---------- | ---------- | ---------- | ---------- |
| [jsonifier](https://github.com/RealTimeChris/Jsonifier/commit/) | 797.917 | 2090234 | 2.61961e+06 | 2.207 | 420.906 | 2090234 | 4.96604e+06 | 1.868 |
| [glaze](https://github.com/stephenberry/glaze/commit/158ac74) | 390.179 | 2090234 | 5.35712e+06 | 1.698 | 405.908 | 2090234 | 5.14953e+06 | 3.102 |
| [simdjson](https://github.com/simdjson/simdjson/commit/ba1819f) | 91.7736 | 2090234 | 2.2776e+07 | 1.412 | 

### Twitter Test (Prettified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/TwitterData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Twitter%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Twitter%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/s) | ReadLength (Bytes) | ReadTime (ns) | ReadMape (%) | Write (MB/s) | WriteLength (Bytes) | WriteTime (ns) | WriteMape (%) |
| ------------------------------------------------- | ---------- | ----------- | ---------- | ----------- | ---------- | ---------- | ---------- | ---------- |
| [glaze](https://github.com/stephenberry/glaze/commit/158ac74) | 842.447 | 722038 | 857073 | 1.169 | 2350.32 | 721479 | 306971 | 0.01483 |
| [jsonifier](https://github.com/RealTimeChris/Jsonifier/commit/) | 224.405 | 722038 | 3.21757e+06 | 1.108 | 2384.82 | 722038 | 302764 | 0.02001 |
| [simdjson](https://github.com/simdjson/simdjson/commit/ba1819f) | 95.3363 | 722038 | 7.57359e+06 | 2.276 | 

### Twitter Test (Minified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/TwitterData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Twitter%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Twitter%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/s) | ReadLength (Bytes) | ReadTime (ns) | ReadMape (%) | Write (MB/s) | WriteLength (Bytes) | WriteTime (ns) | WriteMape (%) |
| ------------------------------------------------- | ---------- | ----------- | ---------- | ----------- | ---------- | ---------- | ---------- | ---------- |
| [glaze](https://github.com/stephenberry/glaze/commit/158ac74) | 708.462 | 478986 | 676092 | 0.1607 | 2045.37 | 478427 | 233908 | 0.7837 |
| [jsonifier](https://github.com/RealTimeChris/Jsonifier/commit/) | 135.966 | 478986 | 3.52285e+06 | 4.441 | 2141.38 | 478986 | 223681 | 0.01811 |
| [simdjson](https://github.com/simdjson/simdjson/commit/ba1819f) | 64.4546 | 478986 | 7.43137e+06 | 0.899 | 

### Minify Test Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Minify%20Test_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Minify%20Test_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Write (MB/s) | WriteLength (Bytes) | WriteTime (ns) | WriteMape (%) |
| ------------------------------------------------- | ---------- | ----------- | ---------- | ----------- |
| [glaze](https://github.com/stephenberry/glaze/commit/158ac74) | 616.532 | 1156 | 1875 | 0.5379 |
| [jsonifier](https://github.com/RealTimeChris/Jsonifier/commit/) | 424.282 | 1156 | 2724.61 | 3.918 |
| [simdjson](https://github.com/simdjson/simdjson/commit/ba1819f) | 155.042 | 1156 | 7456.04 | 0.651 |

### Prettify Test Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Prettify%20Test_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Prettify%20Test_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Write (MB/s) | WriteLength (Bytes) | WriteTime (ns) | WriteMape (%) |
| ------------------------------------------------- | ---------- | ----------- | ---------- | ----------- |
| [jsonifier](https://github.com/RealTimeChris/Jsonifier/commit/) | 1155.08 | 3583236 | 3.10214e+06 | 3.237 |
| [glaze](https://github.com/stephenberry/glaze/commit/158ac74) | 1087.92 | 3583236 | 3.29366e+06 | 2.329 |

### Validation Test Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Validate%20Test_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Validate%20Test_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/s) | ReadLength (Bytes) | ReadTime (ns) | ReadMape (%) |
| ------------------------------------------------- | ---------- | ----------- | ---------- | ----------- |
| [glaze](https://github.com/stephenberry/glaze/commit/158ac74) | 836.774 | 1629 | 1946.76 | 1.01 | 
| [jsonifier](https://github.com/RealTimeChris/Jsonifier/commit/) | 374.381 | 1629 | 4351.18 | 0.8266 | 