# Json-Performance
Performance profiling of JSON libraries (Compiled and run on Ubuntu-22.04 using the Clang++18 compiler)

Latest Results: (Jul 31, 2024)
#### Using the following commits:
----
| Jsonifier: [](https://github.com/RealTimeChris/Jsonifier/commit/)  
| Glaze: [c0b18af](https://github.com/stephenberry/glaze/commit/c0b18af)  
| Simdjson: [ba1819f](https://github.com/simdjson/simdjson/commit/ba1819f)  

 > At least 40 iterations on a 6 core (Intel i7 8700k), until Median Absolute Percentage Error (MAPE) reduced below 5.0%.


### Json Test (Prettified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/JsonData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Json%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Json%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/s) | ReadLength (Bytes) | ReadTime (ns) | ReadMape (%) | Write (MB/s) | WriteLength (Bytes) | WriteTime (ns) | WriteMape (%) |
| ------------------------------------------------- | ---------- | ----------- | ---------- | ----------- | ---------- | ---------- | ---------- | ---------- |
| [glaze](https://github.com/stephenberry/glaze/commit/c0b18af) | 766.542 | 2688776 | 3.50767e+06 | 0.5566 | 1040.98 | 2688342 | 2.58251e+06 | 1.421 |
| [jsonifier](https://github.com/RealTimeChris/Jsonifier/commit/) | 604.335 | 2688776 | 4.44914e+06 | 0.5625 | 1144.49 | 2684963 | 2.346e+06 | 1.419 |
| [simdjson](https://github.com/simdjson/simdjson/commit/ba1819f) | 137.698 | 2688776 | 1.95266e+07 | 0.7953 | 

### Json Test (Minified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/JsonData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Json%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Json%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/s) | ReadLength (Bytes) | ReadTime (ns) | ReadMape (%) | Write (MB/s) | WriteLength (Bytes) | WriteTime (ns) | WriteMape (%) |
| ------------------------------------------------- | ---------- | ----------- | ---------- | ----------- | ---------- | ---------- | ---------- | ---------- |
| [glaze](https://github.com/stephenberry/glaze/commit/c0b18af) | 610.181 | 1820180 | 2.98302e+06 | 0.1628 | 805.08 | 1819746 | 2.26033e+06 | 0.9013 |
| [jsonifier](https://github.com/RealTimeChris/Jsonifier/commit/) | 578.048 | 1820180 | 3.14884e+06 | 0.5987 | 952.448 | 1816367 | 1.90705e+06 | 0.6153 |
| [simdjson](https://github.com/simdjson/simdjson/commit/ba1819f) | 96.0168 | 1820180 | 1.89569e+07 | 0.9551 | 

### ABC Test (Out of Sequence Performance - Prettified) [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/JsonData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Abc%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Abc%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>

The JSON documents in the previous tests featured keys ranging from "a" to "z," where each key corresponds to an array of values. Notably, the documents in this test arrange these keys in reverse order, deviating from the typical "a" to "z" arrangement.

This test effectively demonstrates the challenges encountered when utilizing simdjson and iterative parsers that lack the ability to efficiently allocate memory locations through hashing. In cases where the keys are not in the expected sequence, performance is significantly compromised, with the severity escalating as the document size increases.

In contrast, hash-based solutions offer a viable alternative by circumventing these issues and maintaining optimal performance regardless of the JSON document's scale, or ordering of the keys being parsed.

| Library | Read (MB/s) | ReadLength (Bytes) | ReadTime (ns) | ReadMape (%) | Write (MB/s) | WriteLength (Bytes) | WriteTime (ns) | WriteMape (%) |
| ------------------------------------------------- | ---------- | ----------- | ---------- | ----------- | ---------- | ---------- | ---------- | ---------- |
| [glaze](https://github.com/stephenberry/glaze/commit/c0b18af) | 725.855 | 2688776 | 3.70429e+06 | 0.74 | 1032.99 | 2688342 | 2.60249e+06 | 1.598 |
| [jsonifier](https://github.com/RealTimeChris/Jsonifier/commit/) | 598.335 | 2688776 | 4.49376e+06 | 0.7669 | 1138.14 | 2684963 | 2.35909e+06 | 1.417 |
| [simdjson](https://github.com/simdjson/simdjson/commit/ba1819f) | 235.956 | 2688776 | 1.13952e+07 | 1.03 | 

### ABC Test (Out of Sequence Performance - Minified) [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/JsonData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Abc%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Abc%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/s) | ReadLength (Bytes) | ReadTime (ns) | ReadMape (%) | Write (MB/s) | WriteLength (Bytes) | WriteTime (ns) | WriteMape (%) |
| ------------------------------------------------- | ---------- | ----------- | ---------- | ----------- | ---------- | ---------- | ---------- | ---------- |
| [jsonifier](https://github.com/RealTimeChris/Jsonifier/commit/) | 604.703 | 1820180 | 3.01004e+06 | 0.8663 | 874.831 | 1816367 | 2.07625e+06 | 1.323 |
| [glaze](https://github.com/stephenberry/glaze/commit/c0b18af) | 590.35 | 1820180 | 3.08322e+06 | 0.01918 | 761.875 | 1819746 | 2.38851e+06 | 1.29 |
| [simdjson](https://github.com/simdjson/simdjson/commit/ba1819f) | 167.257 | 1820180 | 1.08825e+07 | 1.291 | 

### Discord Test (Prettified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Discord%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Discord%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/s) | ReadLength (Bytes) | ReadTime (ns) | ReadMape (%) | Write (MB/s) | WriteLength (Bytes) | WriteTime (ns) | WriteMape (%) |
| ------------------------------------------------- | ---------- | ----------- | ---------- | ----------- | ---------- | ---------- | ---------- | ---------- |
| [glaze](https://github.com/stephenberry/glaze/commit/c0b18af) | 727.813 | 138774 | 190673 | 0.02246 | 2302.03 | 138774 | 60283.4 | 0.07691 |
| [jsonifier](https://github.com/RealTimeChris/Jsonifier/commit/) | 162.686 | 138774 | 853019 | 1.049 | 2160.19 | 138774 | 64241.7 | 0.1029 |
| [simdjson](https://github.com/simdjson/simdjson/commit/ba1819f) | 71.8868 | 138774 | 1.93045e+06 | 0.06053 | 

### Discord Test (Minified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Discord%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Discord%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/s) | ReadLength (Bytes) | ReadTime (ns) | ReadMape (%) | Write (MB/s) | WriteLength (Bytes) | WriteTime (ns) | WriteMape (%) |
| ------------------------------------------------- | ---------- | ----------- | ---------- | ----------- | ---------- | ---------- | ---------- | ---------- |
| [glaze](https://github.com/stephenberry/glaze/commit/c0b18af) | 522.588 | 69037 | 132106 | 0.01752 | 1494.05 | 69037 | 46208.1 | 0.02422 |
| [jsonifier](https://github.com/RealTimeChris/Jsonifier/commit/) | 88.2129 | 69037 | 782618 | 0.008035 | 1537.55 | 69037 | 44900.5 | 0.2753 |
| [simdjson](https://github.com/simdjson/simdjson/commit/ba1819f) | 38.2089 | 69037 | 1.80683e+06 | 1.144 | 

### Canada Test (Prettified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/CanadaData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Canada%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Canada%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/s) | ReadLength (Bytes) | ReadTime (ns) | ReadMape (%) | Write (MB/s) | WriteLength (Bytes) | WriteTime (ns) | WriteMape (%) |
| ------------------------------------------------- | ---------- | ----------- | ---------- | ----------- | ---------- | ---------- | ---------- | ---------- |
| [glaze](https://github.com/stephenberry/glaze/commit/c0b18af) | 843.352 | 6661897 | 7.89931e+06 | 0.01798 | 1082.76 | 6661897 | 6.1527e+06 | 0.7319 |
| [jsonifier](https://github.com/RealTimeChris/Jsonifier/commit/) | 741.497 | 6661897 | 8.98439e+06 | 1.115 | 1100.78 | 6661897 | 6.05197e+06 | 1.246 |
| [simdjson](https://github.com/simdjson/simdjson/commit/ba1819f) | 479.457 | 6661897 | 1.38947e+07 | 0.01345 | 

### Canada Test (Minified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/CanadaData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Canada%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Canada%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/s) | ReadLength (Bytes) | ReadTime (ns) | ReadMape (%) | Write (MB/s) | WriteLength (Bytes) | WriteTime (ns) | WriteMape (%) |
| ------------------------------------------------- | ---------- | ----------- | ---------- | ----------- | ---------- | ---------- | ---------- | ---------- |
| [jsonifier](https://github.com/RealTimeChris/Jsonifier/commit/) | 364.064 | 2090234 | 5.74139e+06 | 0.9336 | 430.826 | 2090234 | 4.85168e+06 | 1.158 |
| [glaze](https://github.com/stephenberry/glaze/commit/c0b18af) | 356.367 | 2090234 | 5.86541e+06 | 0.08075 | 392.193 | 2090234 | 5.3296e+06 | 1.053 |
| [simdjson](https://github.com/simdjson/simdjson/commit/ba1819f) | 228.96 | 2090234 | 9.12927e+06 | 1.045 | 

### Twitter Test (Prettified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/TwitterData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Twitter%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Twitter%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/s) | ReadLength (Bytes) | ReadTime (ns) | ReadMape (%) | Write (MB/s) | WriteLength (Bytes) | WriteTime (ns) | WriteMape (%) |
| ------------------------------------------------- | ---------- | ----------- | ---------- | ----------- | ---------- | ---------- | ---------- | ---------- |
| [glaze](https://github.com/stephenberry/glaze/commit/c0b18af) | 911.347 | 722038 | 792276 | 0.2021 | 2490.22 | 722415 | 290101 | 0.01675 |
| [jsonifier](https://github.com/RealTimeChris/Jsonifier/commit/) | 230.395 | 722038 | 3.13392e+06 | 1.345 | 2495.33 | 722038 | 289355 | 0.1616 |
| [simdjson](https://github.com/simdjson/simdjson/commit/ba1819f) | 116.934 | 722038 | 6.17474e+06 | 1.158 | 

### Twitter Test (Minified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/TwitterData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Twitter%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Twitter%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/s) | ReadLength (Bytes) | ReadTime (ns) | ReadMape (%) | Write (MB/s) | WriteLength (Bytes) | WriteTime (ns) | WriteMape (%) |
| ------------------------------------------------- | ---------- | ----------- | ---------- | ----------- | ---------- | ---------- | ---------- | ---------- |
| [glaze](https://github.com/stephenberry/glaze/commit/c0b18af) | 837.248 | 478986 | 572096 | 0.04336 | 2067.14 | 479363 | 231897 | 0.03711 |
| [jsonifier](https://github.com/RealTimeChris/Jsonifier/commit/) | 167.987 | 478986 | 2.85133e+06 | 1.672 | 2072.97 | 478986 | 231063 | 0.003006 |
| [simdjson](https://github.com/simdjson/simdjson/commit/ba1819f) | 78.4718 | 478986 | 6.10393e+06 | 0.7845 | 

### Minify Test Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Minify%20Test_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Minify%20Test_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Write (MB/s) | WriteLength (Bytes) | WriteTime (ns) | WriteMape (%) |
| ------------------------------------------------- | ---------- | ----------- | ---------- | ----------- |
| [jsonifier](https://github.com/RealTimeChris/Jsonifier/commit/) | 476.767 | 69037 | 144802 | 0.08284 |
| [glaze](https://github.com/stephenberry/glaze/commit/c0b18af) | 452.796 | 69037 | 152468 | 0.3889 |
| [simdjson](https://github.com/simdjson/simdjson/commit/ba1819f) | 148.275 | 69037 | 465601 | 0.07696 |

### Prettify Test Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Prettify%20Test_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Prettify%20Test_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Write (MB/s) | WriteLength (Bytes) | WriteTime (ns) | WriteMape (%) |
| ------------------------------------------------- | ---------- | ----------- | ---------- | ----------- |
| [jsonifier](https://github.com/RealTimeChris/Jsonifier/commit/) | 1235.04 | 2688776 | 2.17708e+06 | 1.636 |
| [glaze](https://github.com/stephenberry/glaze/commit/c0b18af) | 1021.68 | 2688776 | 2.63172e+06 | 1.074 |

### Validation Test Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Validate%20Test_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Validate%20Test_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/s) | ReadLength (Bytes) | ReadTime (ns) | ReadMape (%) |
| ------------------------------------------------- | ---------- | ----------- | ---------- | ----------- |
| [glaze](https://github.com/stephenberry/glaze/commit/c0b18af) | 889.107 | 138774 | 156082 | 0.03957 | 
| [jsonifier](https://github.com/RealTimeChris/Jsonifier/commit/) | 662.331 | 138774 | 209524 | 0.1001 | 