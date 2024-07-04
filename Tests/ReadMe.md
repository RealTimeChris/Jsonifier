# Json-Performance
Performance profiling of JSON libraries (Compiled and run on Ubuntu-22.04 using the Clang++18 compiler)

Latest Results: (Jul 04, 2024)

 > At least 0 iterations on a 6 core (Intel i7 8700k), until Median Absolute Percentage Error (Mape) reduced below 5.0%.


### Json Test (Prettified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/JsonData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Json%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Json%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/s) | ReadLength (Bytes) | ReadTime (ns) | ReadMape (%) | Write (MB/s) | WriteLength (Bytes) | WriteTime (ns) | WriteMape (%) |
| ------------------------------------------------- | ---------- | ----------- | ---------- | ----------- | ---------- | ---------- | ---------- | ---------- |
| [glaze](https://github.com/stephenberry/glaze) | 104.827 | 3571883 | 3.40742e+07 | 0 | 126.466 | 3571338 | 2.82394e+07 | 0 |
| [jsonifier](https://github.com/realtimechris/jsonifier) | 58.3006 | 3571883 | 6.12667e+07 | 0 | 83.8734 | 3566782 | 4.25258e+07 | 0 |
| [simdjson](https://github.com/simdjson/simdjson) | 9.54435 | 3571883 | 3.7424e+08 | 0 | 

### Json Test (Minified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/JsonData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Json%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Json%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/s) | ReadLength (Bytes) | ReadTime (ns) | ReadMape (%) | Write (MB/s) | WriteLength (Bytes) | WriteTime (ns) | WriteMape (%) |
| ------------------------------------------------- | ---------- | ----------- | ---------- | ----------- | ---------- | ---------- | ---------- | ---------- |
| [glaze](https://github.com/stephenberry/glaze) | 64.6888 | 2448561 | 3.78514e+07 | 0 | 76.6425 | 2448016 | 3.19407e+07 | 0 |
| [jsonifier](https://github.com/realtimechris/jsonifier) | 58.6348 | 2448561 | 4.17595e+07 | 0 | 51.5353 | 2443460 | 4.74133e+07 | 0 |
| [simdjson](https://github.com/simdjson/simdjson) | 6.99592 | 2448561 | 3.49998e+08 | 0 | 

### ABC Test (Out of Sequence Performance - Prettified) [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/JsonData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Abc%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Abc%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>

The JSON documents in the previous tests featured keys ranging from "a" to "z," where each key corresponds to an array of values. Notably, the documents in this test arrange these keys in reverse order, deviating from the typical "a" to "z" arrangement.

This test effectively demonstrates the challenges encountered when utilizing simdjson and iterative parsers that lack the ability to efficiently allocate memory locations through hashing. In cases where the keys are not in the expected sequence, performance is significantly compromised, with the severity escalating as the document size increases.

In contrast, hash-based solutions offer a viable alternative by circumventing these issues and maintaining optimal performance regardless of the JSON document's scale, or ordering of the keys being parsed.

| Library | Read (MB/s) | ReadLength (Bytes) | ReadTime (ns) | ReadMape (%) | Write (MB/s) | WriteLength (Bytes) | WriteTime (ns) | WriteMape (%) |
| ------------------------------------------------- | ---------- | ----------- | ---------- | ----------- | ---------- | ---------- | ---------- | ---------- |
| [glaze](https://github.com/stephenberry/glaze) | 103.596 | 3571883 | 3.4479e+07 | 0 | 129.767 | 3571338 | 2.75211e+07 | 0 |
| [jsonifier](https://github.com/realtimechris/jsonifier) | 40.3647 | 3571883 | 8.84903e+07 | 0 | 77.0152 | 3566782 | 4.63127e+07 | 0 |
| [simdjson](https://github.com/simdjson/simdjson) | 3.4888 | 3571883 | 1.02381e+09 | 0 | 

### ABC Test (Out of Sequence Performance - Minified) [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/JsonData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Abc%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Abc%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/s) | ReadLength (Bytes) | ReadTime (ns) | ReadMape (%) | Write (MB/s) | WriteLength (Bytes) | WriteTime (ns) | WriteMape (%) |
| ------------------------------------------------- | ---------- | ----------- | ---------- | ----------- | ---------- | ---------- | ---------- | ---------- |
| [glaze](https://github.com/stephenberry/glaze) | 43.7525 | 2448561 | 5.59639e+07 | 0 | 52.3697 | 2448016 | 4.67449e+07 | 0 |
| [jsonifier](https://github.com/realtimechris/jsonifier) | 37.8449 | 2448561 | 6.46999e+07 | 0 | 39.1485 | 2443460 | 6.24151e+07 | 0 |
| [simdjson](https://github.com/simdjson/simdjson) | 2.48139 | 2448561 | 9.86772e+08 | 0 | 

### Discord Test (Prettified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Discord%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Discord%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/s) | ReadLength (Bytes) | ReadTime (ns) | ReadMape (%) | Write (MB/s) | WriteLength (Bytes) | WriteTime (ns) | WriteMape (%) |
| ------------------------------------------------- | ---------- | ----------- | ---------- | ----------- | ---------- | ---------- | ---------- | ---------- |
| [glaze](https://github.com/stephenberry/glaze) | 47.8987 | 139414 | 2.9106e+06 | 0 | 86.0633 | 139414 | 1.6199e+06 | 0 |
| [jsonifier](https://github.com/realtimechris/jsonifier) | 28.424 | 139414 | 4.9048e+06 | 0 | 108.866 | 139414 | 1.2806e+06 | 0 |
| [simdjson](https://github.com/simdjson/simdjson) | 3.24413 | 139414 | 4.29742e+07 | 0 | 

### Discord Test (Minified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Discord%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Discord%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/s) | ReadLength (Bytes) | ReadTime (ns) | ReadMape (%) | Write (MB/s) | WriteLength (Bytes) | WriteTime (ns) | WriteMape (%) |
| ------------------------------------------------- | ---------- | ----------- | ---------- | ----------- | ---------- | ---------- | ---------- | ---------- |
| [glaze](https://github.com/stephenberry/glaze) | 35.6284 | 69397 | 1.9478e+06 | 0 | 41.6224 | 69397 | 1.6673e+06 | 0 |
| [jsonifier](https://github.com/realtimechris/jsonifier) | 24.5471 | 69397 | 2.8271e+06 | 0 | 40.8338 | 69397 | 1.6995e+06 | 0 |
| [simdjson](https://github.com/simdjson/simdjson) | 3.64576 | 69397 | 1.9035e+07 | 0 | 

### Canada Test (Prettified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/CanadaData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Canada%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Canada%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/s) | ReadLength (Bytes) | ReadTime (ns) | ReadMape (%) | Write (MB/s) | WriteLength (Bytes) | WriteTime (ns) | WriteMape (%) |
| ------------------------------------------------- | ---------- | ----------- | ---------- | ----------- | ---------- | ---------- | ---------- | ---------- |
| [jsonifier](https://github.com/realtimechris/jsonifier) | 62.2158 | 6661897 | 1.07077e+08 | 0 | 73.3931 | 6661897 | 9.07701e+07 | 0 |
| [glaze](https://github.com/stephenberry/glaze) | 49.7123 | 6661897 | 1.34009e+08 | 0 | 64.0339 | 6661897 | 1.04037e+08 | 0 |
| [simdjson](https://github.com/simdjson/simdjson) | 10.3874 | 6661897 | 6.41342e+08 | 0 | 

### Canada Test (Minified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/CanadaData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Canada%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Canada%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/s) | ReadLength (Bytes) | ReadTime (ns) | ReadMape (%) | Write (MB/s) | WriteLength (Bytes) | WriteTime (ns) | WriteMape (%) |
| ------------------------------------------------- | ---------- | ----------- | ---------- | ----------- | ---------- | ---------- | ---------- | ---------- |
| [jsonifier](https://github.com/realtimechris/jsonifier) | 36.3237 | 2090234 | 5.75446e+07 | 0 | 19.3447 | 2090234 | 1.08052e+08 | 0 |
| [glaze](https://github.com/stephenberry/glaze) | 25.1484 | 2090234 | 8.31161e+07 | 0 | 24.2898 | 2090234 | 8.60539e+07 | 0 |
| [simdjson](https://github.com/simdjson/simdjson) | 4.49914 | 2090234 | 4.64586e+08 | 0 | 

### Twitter Test (Prettified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/TwitterData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Twitter%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Twitter%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/s) | ReadLength (Bytes) | ReadTime (ns) | ReadMape (%) | Write (MB/s) | WriteLength (Bytes) | WriteTime (ns) | WriteMape (%) |
| ------------------------------------------------- | ---------- | ----------- | ---------- | ----------- | ---------- | ---------- | ---------- | ---------- |
| [glaze](https://github.com/stephenberry/glaze) | 48.1937 | 722038 | 1.4982e+07 | 0 | 96.3732 | 721479 | 7.4863e+06 | 0 |
| [jsonifier](https://github.com/realtimechris/jsonifier) | 32.5835 | 722038 | 2.21596e+07 | 0 | 112.692 | 722038 | 6.4072e+06 | 0 |
| [simdjson](https://github.com/simdjson/simdjson) | 7.44249 | 722038 | 9.70156e+07 | 0 | 

### Twitter Test (Minified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/TwitterData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Twitter%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Twitter%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/s) | ReadLength (Bytes) | ReadTime (ns) | ReadMape (%) | Write (MB/s) | WriteLength (Bytes) | WriteTime (ns) | WriteMape (%) |
| ------------------------------------------------- | ---------- | ----------- | ---------- | ----------- | ---------- | ---------- | ---------- | ---------- |
| [glaze](https://github.com/stephenberry/glaze) | 40.9915 | 478986 | 1.1685e+07 | 0 | 79.0998 | 478427 | 6.0484e+06 | 0 |
| [jsonifier](https://github.com/realtimechris/jsonifier) | 31.5087 | 478986 | 1.52017e+07 | 0 | 84.9432 | 478986 | 5.6389e+06 | 0 |
| [simdjson](https://github.com/simdjson/simdjson) | 5.17897 | 478986 | 9.24868e+07 | 0 | 

### Minify Test Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Minify%20Test_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Minify%20Test_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Write (MB/s) | WriteLength (Bytes) | WriteTime (ns) | WriteMape (%) |
| ------------------------------------------------- | ---------- | ----------- | ---------- | ----------- |
| [jsonifier](https://github.com/realtimechris/jsonifier) | 35.2341 | 69397 | 1.9696e+06 | 0 |
| [glaze](https://github.com/stephenberry/glaze) | 31.8218 | 69397 | 2.1808e+06 | 0 |
| [simdjson](https://github.com/simdjson/simdjson) | 4.76258 | 69397 | 1.45713e+07 | 0 |

### Prettify Test Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Prettify%20Test_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Prettify%20Test_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Write (MB/s) | WriteLength (Bytes) | WriteTime (ns) | WriteMape (%) |
| ------------------------------------------------- | ---------- | ----------- | ---------- | ----------- |
| [glaze](https://github.com/stephenberry/glaze) | 212.683 | 3571883 | 1.67944e+07 | 0 |
| [jsonifier](https://github.com/realtimechris/jsonifier) | 106.137 | 3571883 | 3.36535e+07 | 0 |

### Validation Test Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Validate%20Test_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Validate%20Test_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/s) | ReadLength (Bytes) | ReadTime (ns) | ReadMape (%) |
| ------------------------------------------------- | ---------- | ----------- | ---------- | ----------- |
| [jsonifier](https://github.com/realtimechris/jsonifier) | 81.1679 | 139414 | 1.7176e+06 | 0 | 