# Json-Performance
Performance profiling of JSON libraries (Compiled and run on Ubuntu-22.04 using the Clang++18 compiler)

Latest Results: (Aug 02, 2024)
#### Using the following commits:
----
| Jsonifier: [](https://github.com/RealTimeChris/Jsonifier/commit/)  
| Glaze: [411f3d9](https://github.com/stephenberry/glaze/commit/411f3d9)  
| Simdjson: [ba1819f](https://github.com/simdjson/simdjson/commit/ba1819f)  

 > At least 40 iterations on a 6 core (Intel i7 8700k), until Median Absolute Percentage Error (MAPE) reduced below 5.0%.


### Json Test (Prettified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/JsonData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Json%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Json%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/s) | ReadLength (Bytes) | ReadTime (ns) | ReadMape (%) | Write (MB/s) | WriteLength (Bytes) | WriteTime (ns) | WriteMape (%) |
| ------------------------------------------------- | ---------- | ----------- | ---------- | ----------- | ---------- | ---------- | ---------- | ---------- |
| [glaze](https://github.com/stephenberry/glaze/commit/411f3d9) | 795.164 | 4083469 | 5.13538e+06 | 0.5527 | 1231.14 | 4082809 | 3.31629e+06 | 1.385 |
| [jsonifier](https://github.com/RealTimeChris/Jsonifier/commit/) | 680.144 | 4083469 | 6.00383e+06 | 0.639 | 1073.89 | 4077679 | 3.79711e+06 | 2.723 |
| [simdjson](https://github.com/simdjson/simdjson/commit/ba1819f) | 145.801 | 4083469 | 2.80071e+07 | 2.193 | 

### Json Test (Minified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/JsonData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Json%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Json%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/s) | ReadLength (Bytes) | ReadTime (ns) | ReadMape (%) | Write (MB/s) | WriteLength (Bytes) | WriteTime (ns) | WriteMape (%) |
| ------------------------------------------------- | ---------- | ----------- | ---------- | ----------- | ---------- | ---------- | ---------- | ---------- |
| [jsonifier](https://github.com/RealTimeChris/Jsonifier/commit/) | 695.09 | 2803115 | 4.03274e+06 | 1.63 | 858.365 | 2797325 | 3.2589e+06 | 1.144 |
| [glaze](https://github.com/stephenberry/glaze/commit/411f3d9) | 661.362 | 2803115 | 4.23839e+06 | 1.015 | 855.745 | 2802455 | 3.27487e+06 | 1.111 |
| [simdjson](https://github.com/simdjson/simdjson/commit/ba1819f) | 96.5491 | 2803115 | 2.9033e+07 | 1.753 | 

### ABC Test (Out of Sequence Performance - Prettified) [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/JsonData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Abc%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Abc%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>

The JSON documents in the previous tests featured keys ranging from "a" to "z," where each key corresponds to an array of values. Notably, the documents in this test arrange these keys in reverse order, deviating from the typical "a" to "z" arrangement.

This test effectively demonstrates the challenges encountered when utilizing simdjson and iterative parsers that lack the ability to efficiently allocate memory locations through hashing. In cases where the keys are not in the expected sequence, performance is significantly compromised, with the severity escalating as the document size increases.

In contrast, hash-based solutions offer a viable alternative by circumventing these issues and maintaining optimal performance regardless of the JSON document's scale, or ordering of the keys being parsed.

| Library | Read (MB/s) | ReadLength (Bytes) | ReadTime (ns) | ReadMape (%) | Write (MB/s) | WriteLength (Bytes) | WriteTime (ns) | WriteMape (%) |
| ------------------------------------------------- | ---------- | ----------- | ---------- | ----------- | ---------- | ---------- | ---------- | ---------- |
| [glaze](https://github.com/stephenberry/glaze/commit/411f3d9) | 717.222 | 4083469 | 5.69345e+06 | 1.153 | 1047.85 | 4082809 | 3.89637e+06 | 1.32 |
| [jsonifier](https://github.com/RealTimeChris/Jsonifier/commit/) | 618.328 | 4083469 | 6.60405e+06 | 1.776 | 1134.31 | 4077679 | 3.59484e+06 | 1.151 |
| [simdjson](https://github.com/simdjson/simdjson/commit/ba1819f) | 252.161 | 4083469 | 1.61939e+07 | 3.511 | 

### ABC Test (Out of Sequence Performance - Minified) [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/JsonData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Abc%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Abc%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/s) | ReadLength (Bytes) | ReadTime (ns) | ReadMape (%) | Write (MB/s) | WriteLength (Bytes) | WriteTime (ns) | WriteMape (%) |
| ------------------------------------------------- | ---------- | ----------- | ---------- | ----------- | ---------- | ---------- | ---------- | ---------- |
| [glaze](https://github.com/stephenberry/glaze/commit/411f3d9) | 667.073 | 2803115 | 4.20211e+06 | 1.599 | 852.95 | 2802455 | 3.2856e+06 | 1.656 |
| [jsonifier](https://github.com/RealTimeChris/Jsonifier/commit/) | 653.513 | 2803115 | 4.2893e+06 | 1.84 | 868.856 | 2797325 | 3.21955e+06 | 3.54 |
| [simdjson](https://github.com/simdjson/simdjson/commit/ba1819f) | 186.991 | 2803115 | 1.49906e+07 | 1.27 | 

### Discord Test (Prettified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Discord%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Discord%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/s) | ReadLength (Bytes) | ReadTime (ns) | ReadMape (%) | Write (MB/s) | WriteLength (Bytes) | WriteTime (ns) | WriteMape (%) |
| ------------------------------------------------- | ---------- | ----------- | ---------- | ----------- | ---------- | ---------- | ---------- | ---------- |
| [glaze](https://github.com/stephenberry/glaze/commit/411f3d9) | 793.008 | 138774 | 174997 | 0.1898 | 2351.53 | 138774 | 59014.5 | 0.08899 |
| [jsonifier](https://github.com/RealTimeChris/Jsonifier/commit/) | 168.512 | 138774 | 823527 | 0.136 | 1853.33 | 138774 | 74878.1 | 0.2104 |
| [simdjson](https://github.com/simdjson/simdjson/commit/ba1819f) | 76.0207 | 138774 | 1.82548e+06 | 0.1007 | 

### Discord Test (Minified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Discord%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Discord%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/s) | ReadLength (Bytes) | ReadTime (ns) | ReadMape (%) | Write (MB/s) | WriteLength (Bytes) | WriteTime (ns) | WriteMape (%) |
| ------------------------------------------------- | ---------- | ----------- | ---------- | ----------- | ---------- | ---------- | ---------- | ---------- |
| [glaze](https://github.com/stephenberry/glaze/commit/411f3d9) | 554.973 | 69037 | 124397 | 0.09251 | 1580.87 | 69037 | 43670.2 | 0.6582 |
| [jsonifier](https://github.com/RealTimeChris/Jsonifier/commit/) | 97.0112 | 69037 | 711640 | 0.01356 | 1399.83 | 69037 | 49318.3 | 0.05245 |
| [simdjson](https://github.com/simdjson/simdjson/commit/ba1819f) | 36.4289 | 69037 | 1.89511e+06 | 0.1883 | 

### Canada Test (Prettified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/CanadaData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Canada%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Canada%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/s) | ReadLength (Bytes) | ReadTime (ns) | ReadMape (%) | Write (MB/s) | WriteLength (Bytes) | WriteTime (ns) | WriteMape (%) |
| ------------------------------------------------- | ---------- | ----------- | ---------- | ----------- | ---------- | ---------- | ---------- | ---------- |
| [glaze](https://github.com/stephenberry/glaze/commit/411f3d9) | 871.412 | 6661897 | 7.64495e+06 | 1.805 | 1146.38 | 6661897 | 5.81125e+06 | 2.112 |
| [jsonifier](https://github.com/RealTimeChris/Jsonifier/commit/) | 706.517 | 6661897 | 9.42921e+06 | 1.736 | 1095.8 | 6661897 | 6.07949e+06 | 1.424 |
| [simdjson](https://github.com/simdjson/simdjson/commit/ba1819f) | 516.696 | 6661897 | 1.28933e+07 | 1.67 | 

### Canada Test (Minified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/CanadaData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Canada%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Canada%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/s) | ReadLength (Bytes) | ReadTime (ns) | ReadMape (%) | Write (MB/s) | WriteLength (Bytes) | WriteTime (ns) | WriteMape (%) |
| ------------------------------------------------- | ---------- | ----------- | ---------- | ----------- | ---------- | ---------- | ---------- | ---------- |
| [jsonifier](https://github.com/RealTimeChris/Jsonifier/commit/) | 359.836 | 2090234 | 5.80885e+06 | 1.358 | 447.91 | 2090234 | 4.66664e+06 | 1.526 |
| [glaze](https://github.com/stephenberry/glaze/commit/411f3d9) | 348.895 | 2090234 | 5.99102e+06 | 1.429 | 387.335 | 2090234 | 5.39645e+06 | 1.598 |
| [simdjson](https://github.com/simdjson/simdjson/commit/ba1819f) | 215.568 | 2090234 | 9.69641e+06 | 1.943 | 

### Twitter Test (Prettified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/TwitterData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Twitter%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Twitter%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/s) | ReadLength (Bytes) | ReadTime (ns) | ReadMape (%) | Write (MB/s) | WriteLength (Bytes) | WriteTime (ns) | WriteMape (%) |
| ------------------------------------------------- | ---------- | ----------- | ---------- | ----------- | ---------- | ---------- | ---------- | ---------- |
| [glaze](https://github.com/stephenberry/glaze/commit/411f3d9) | 952.673 | 722038 | 757907 | 0.02963 | 2497.65 | 722415 | 289238 | 0.9852 |
| [jsonifier](https://github.com/RealTimeChris/Jsonifier/commit/) | 229.4 | 722038 | 3.14751e+06 | 1.121 | 2249.48 | 722038 | 320980 | 0.5715 |
| [simdjson](https://github.com/simdjson/simdjson/commit/ba1819f) | 115.534 | 722038 | 6.24956e+06 | 2.687 | 

### Twitter Test (Minified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/TwitterData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Twitter%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Twitter%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/s) | ReadLength (Bytes) | ReadTime (ns) | ReadMape (%) | Write (MB/s) | WriteLength (Bytes) | WriteTime (ns) | WriteMape (%) |
| ------------------------------------------------- | ---------- | ----------- | ---------- | ----------- | ---------- | ---------- | ---------- | ---------- |
| [glaze](https://github.com/stephenberry/glaze/commit/411f3d9) | 864.154 | 478986 | 554283 | 0.7224 | 1902.34 | 479363 | 251987 | 0.8811 |
| [jsonifier](https://github.com/RealTimeChris/Jsonifier/commit/) | 155.505 | 478986 | 3.0802e+06 | 2.007 | 2026.21 | 478986 | 236396 | 1.102 |
| [simdjson](https://github.com/simdjson/simdjson/commit/ba1819f) | 79.3125 | 478986 | 6.03922e+06 | 2.304 | 

### Minify Test Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Minify%20Test_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Minify%20Test_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Write (MB/s) | WriteLength (Bytes) | WriteTime (ns) | WriteMape (%) |
| ------------------------------------------------- | ---------- | ----------- | ---------- | ----------- |
| [jsonifier](https://github.com/RealTimeChris/Jsonifier/commit/) | 482.705 | 69037 | 143021 | 0.01187 |
| [glaze](https://github.com/stephenberry/glaze/commit/411f3d9) | 453.663 | 69037 | 152177 | 0.1447 |
| [simdjson](https://github.com/simdjson/simdjson/commit/ba1819f) | 153.578 | 69037 | 449523 | 0.1101 |

### Prettify Test Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Prettify%20Test_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Prettify%20Test_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Write (MB/s) | WriteLength (Bytes) | WriteTime (ns) | WriteMape (%) |
| ------------------------------------------------- | ---------- | ----------- | ---------- | ----------- |
| [jsonifier](https://github.com/RealTimeChris/Jsonifier/commit/) | 1301.99 | 4083469 | 3.13634e+06 | 3.176 |
| [glaze](https://github.com/stephenberry/glaze/commit/411f3d9) | 1177.82 | 4083469 | 3.46697e+06 | 0.7409 |

### Validation Test Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Validate%20Test_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Validate%20Test_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/s) | ReadLength (Bytes) | ReadTime (ns) | ReadMape (%) |
| ------------------------------------------------- | ---------- | ----------- | ---------- | ----------- |
| [glaze](https://github.com/stephenberry/glaze/commit/411f3d9) | 937.077 | 138774 | 148092 | 0.0112 | 
| [jsonifier](https://github.com/RealTimeChris/Jsonifier/commit/) | 785.017 | 138774 | 176778 | 0.1684 | 