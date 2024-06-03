[![TESTING](https://tokei.rs/b1/github/XAMPPRocky/tokei_rs)](https://github.com/realtimechris/jsonifier)

# Json-Performance
Performance profiling of JSON libraries (Compiled and run on Ubuntu-22.04 using the Clang++18 compiler)

Latest Results: (Jun 05, 2024

### Json Test (Prettified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/JsonData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Json%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Json%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>

> 1 iterations on a 6 core (Intel i7 8700k)

| Library | Read (MB/s) | ReadLength (Bytes) | ReadTime (ns) | Write (MB/s) | WriteLength (Bytes) | WriteTime (ns)
| ------------------------------------------------- | ---------- | ----------- | ---------- | ----------- | ---------- | ---------- |
| [jsonifier](https://github.com/realtimechris/jsonifier) | 26.5891 | 3333447 | 1.25369e+08 | 108.7 | 3328687 | 3.06228e+07 |
| [glaze](https://github.com/stephenberry/glaze) | 23.5061 | 3333447 | 1.41812e+08 | 142.076 | 3332922 | 2.34588e+07 |
| [simdjson](https://github.com/simdjson/simdjson) | 11.7163 | 3333447 | 2.84513e+08 | N/A | N/A | N/A |

### Json Test (Minified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/JsonData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Json%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Json%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>

> 1 iterations on a 6 core (Intel i7 8700k)

| Library | Read (MB/s) | ReadLength (Bytes) | ReadTime (ns) | Write (MB/s) | WriteLength (Bytes) | WriteTime (ns)
| ------------------------------------------------- | ---------- | ----------- | ---------- | ----------- | ---------- | ---------- |
| [glaze](https://github.com/stephenberry/glaze) | 19.4794 | 2276561 | 1.1687e+08 | 97.1643 | 2276036 | 2.34246e+07 |
| [jsonifier](https://github.com/realtimechris/jsonifier) | 15.7782 | 2276561 | 1.44285e+08 | 103.338 | 2271801 | 2.19842e+07 |
| [simdjson](https://github.com/simdjson/simdjson) | 6.60691 | 2276561 | 3.44573e+08 | N/A | N/A | N/A |

### ABC Test (Out of Sequence Performance - Prettified) [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/JsonData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Abc%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Abc%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>

The JSON documents in the previous tests featured keys ranging from "a" to "z," where each key corresponds to an array of values. Notably, the documents in this test arrange these keys in reverse order, deviating from the typical "a" to "z" arrangement.

This test effectively demonstrates the challenges encountered when utilizing simdjson and iterative parsers that lack the ability to efficiently allocate memory locations through hashing. In cases where the keys are not in the expected sequence, performance is significantly compromised, with the severity escalating as the document size increases.

In contrast, hash-based solutions offer a viable alternative by circumventing these issues and maintaining optimal performance regardless of the JSON document's scale, or ordering of the keys being parsed.
> 1 iterations on a 6 core (Intel i7 8700k)

| Library | Read (MB/s) | ReadLength (Bytes) | ReadTime (ns) | Write (MB/s) | WriteLength (Bytes) | WriteTime (ns)
| ------------------------------------------------- | ---------- | ----------- | ---------- | ----------- | ---------- | ---------- |
| [jsonifier](https://github.com/realtimechris/jsonifier) | 16.793 | 3333447 | 1.98502e+08 | 108.81 | 3328687 | 3.05918e+07 |
| [glaze](https://github.com/stephenberry/glaze) | 16.2589 | 3333447 | 2.05023e+08 | 132.636 | 3332922 | 2.51283e+07 |
| [simdjson](https://github.com/simdjson/simdjson) | 3.84223 | 3333447 | 8.67581e+08 | N/A | N/A | N/A |

### ABC Test (Out of Sequence Performance - Minified) [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/JsonData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Abc%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Abc%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>

> 1 iterations on a 6 core (Intel i7 8700k)

| Library | Read (MB/s) | ReadLength (Bytes) | ReadTime (ns) | Write (MB/s) | WriteLength (Bytes) | WriteTime (ns)
| ------------------------------------------------- | ---------- | ----------- | ---------- | ----------- | ---------- | ---------- |
| [glaze](https://github.com/stephenberry/glaze) | 13.9609 | 2276561 | 1.63067e+08 | 89.8156 | 2276036 | 2.53412e+07 |
| [jsonifier](https://github.com/realtimechris/jsonifier) | 8.77005 | 2276561 | 2.59584e+08 | 61.4814 | 2271801 | 3.6951e+07 |
| [simdjson](https://github.com/simdjson/simdjson) | 2.35006 | 2276561 | 9.68725e+08 | N/A | N/A | N/A |

### Discord Test (Prettified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Discord%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Discord%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>

> 1 iterations on a 6 core (Intel i7 8700k)

| Library | Read (MB/s) | ReadLength (Bytes) | ReadTime (ns) | Write (MB/s) | WriteLength (Bytes) | WriteTime (ns)
| ------------------------------------------------- | ---------- | ----------- | ---------- | ----------- | ---------- | ---------- |
| [jsonifier](https://github.com/realtimechris/jsonifier) | 33.052 | 139410 | 4.2179e+06 | 190.555 | 139410 | 731600 |
| [glaze](https://github.com/stephenberry/glaze) | 32.9785 | 139410 | 4.2273e+06 | 110.162 | 139410 | 1.2655e+06 |
| [simdjson](https://github.com/simdjson/simdjson) | 6.84991 | 139410 | 2.03521e+07 | N/A | N/A | N/A |

### Discord Test (Minified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Discord%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Discord%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>

> 1 iterations on a 6 core (Intel i7 8700k)

| Library | Read (MB/s) | ReadLength (Bytes) | ReadTime (ns) | Write (MB/s) | WriteLength (Bytes) | WriteTime (ns)
| ------------------------------------------------- | ---------- | ----------- | ---------- | ----------- | ---------- | ---------- |
| [glaze](https://github.com/stephenberry/glaze) | 18.6585 | 69393 | 3.7191e+06 | 93.2702 | 69393 | 744000 |
| [jsonifier](https://github.com/realtimechris/jsonifier) | 9.31112 | 69393 | 7.4527e+06 | 114.472 | 69393 | 606200 |
| [simdjson](https://github.com/simdjson/simdjson) | 4.7322 | 69393 | 1.4664e+07 | N/A | N/A | N/A |

### Canada Test (Prettified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/CanadaData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Canada%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Canada%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>

> 1 iterations on a 6 core (Intel i7 8700k)

| Library | Read (MB/s) | ReadLength (Bytes) | ReadTime (ns) | Write (MB/s) | WriteLength (Bytes) | WriteTime (ns)
| ------------------------------------------------- | ---------- | ----------- | ---------- | ----------- | ---------- | ---------- |
| [glaze](https://github.com/stephenberry/glaze) | 38.5904 | 6661897 | 1.72631e+08 | 122.918 | 6661897 | 5.41979e+07 |
| [jsonifier](https://github.com/realtimechris/jsonifier) | 19.5834 | 6661897 | 3.40181e+08 | 55.8689 | 6661897 | 1.19242e+08 |
| [simdjson](https://github.com/simdjson/simdjson) | 12.9232 | 6661897 | 5.15499e+08 | N/A | N/A | N/A |

### Canada Test (Minified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/CanadaData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Canada%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Canada%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>

> 1 iterations on a 6 core (Intel i7 8700k)

| Library | Read (MB/s) | ReadLength (Bytes) | ReadTime (ns) | Write (MB/s) | WriteLength (Bytes) | WriteTime (ns)
| ------------------------------------------------- | ---------- | ----------- | ---------- | ----------- | ---------- | ---------- |
| [jsonifier](https://github.com/realtimechris/jsonifier) | 16.959 | 2090234 | 1.23252e+08 | 44.2117 | 2090234 | 4.72779e+07 |
| [glaze](https://github.com/stephenberry/glaze) | 16.0287 | 2090234 | 1.30406e+08 | 42.2578 | 2090234 | 4.94639e+07 |
| [simdjson](https://github.com/simdjson/simdjson) | 6.52064 | 2090234 | 3.20557e+08 | N/A | N/A | N/A |

### Twitter Test (Prettified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/TwitterData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Twitter%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Twitter%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>

> 1 iterations on a 6 core (Intel i7 8700k)

| Library | Read (MB/s) | ReadLength (Bytes) | ReadTime (ns) | Write (MB/s) | WriteLength (Bytes) | WriteTime (ns)
| ------------------------------------------------- | ---------- | ----------- | ---------- | ----------- | ---------- | ---------- |
| [glaze](https://github.com/stephenberry/glaze) | 43.6254 | 408617 | 9.3665e+06 | 183.223 | 411647 | 2.2467e+06 |
| [jsonifier](https://github.com/realtimechris/jsonifier) | 33.3584 | 408617 | 1.22493e+07 | 201.051 | 408617 | 2.0324e+06 |
| [simdjson](https://github.com/simdjson/simdjson) | 10.0293 | 408617 | 4.07424e+07 | N/A | N/A | N/A |

### Twitter Test (Minified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/TwitterData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Twitter%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Twitter%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>

> 1 iterations on a 6 core (Intel i7 8700k)

| Library | Read (MB/s) | ReadLength (Bytes) | ReadTime (ns) | Write (MB/s) | WriteLength (Bytes) | WriteTime (ns)
| ------------------------------------------------- | ---------- | ----------- | ---------- | ----------- | ---------- | ---------- |
| [jsonifier](https://github.com/realtimechris/jsonifier) | 33.9576 | 272710 | 8.0309e+06 | 188.57 | 272710 | 1.4462e+06 |
| [glaze](https://github.com/stephenberry/glaze) | 33.7187 | 272710 | 8.0878e+06 | 152.25 | 274340 | 1.8019e+06 |
| [simdjson](https://github.com/simdjson/simdjson) | 7.28244 | 272710 | 3.74476e+07 | N/A | N/A | N/A |

### Minify Test Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Minify%20Test_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Minify%20Test_Results.png?raw=true" 
alt="" width="400"/></p>

> 1 iterations on a 6 core (Intel i7 8700k)

| Library | Read (MB/s) | ReadLength (Bytes) | ReadTime (ns) | Write (MB/s) | WriteLength (Bytes) | WriteTime (ns)
| ------------------------------------------------- | ---------- | ----------- | ---------- | ----------- | ---------- | ---------- |
| [jsonifier](https://github.com/realtimechris/jsonifier) | N/A | N/A | N/A | 86.5465 | 69393 | 801800 |
| [glaze](https://github.com/stephenberry/glaze) | N/A | N/A | N/A | 62.7083 | 69393 | 1.1066e+06 |
| [simdjson](https://github.com/simdjson/simdjson) | N/A | N/A | N/A | 7.19419 | 69393 | 9.6457e+06 |

### Prettify Test Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Prettify%20Test_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Prettify%20Test_Results.png?raw=true" 
alt="" width="400"/></p>

> 1 iterations on a 6 core (Intel i7 8700k)

| Library | Read (MB/s) | ReadLength (Bytes) | ReadTime (ns) | Write (MB/s) | WriteLength (Bytes) | WriteTime (ns)
| ------------------------------------------------- | ---------- | ----------- | ---------- | ----------- | ---------- | ---------- |
| [glaze](https://github.com/stephenberry/glaze) | N/A | N/A | N/A | 267.328 | 3333447 | 1.24695e+07 |
| [jsonifier](https://github.com/realtimechris/jsonifier) | N/A | N/A | N/A | 161.253 | 3333447 | 2.06722e+07 |

### Validation Test Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Validate%20Test_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Validate%20Test_Results.png?raw=true" 
alt="" width="400"/></p>

> 1 iterations on a 6 core (Intel i7 8700k)

| Library | Read (MB/s) | ReadLength (Bytes) | ReadTime (ns) | Write (MB/s) | WriteLength (Bytes) | WriteTime (ns)
| ------------------------------------------------- | ---------- | ----------- | ---------- | ----------- | ---------- | ---------- |
| [jsonifier](https://github.com/realtimechris/jsonifier) | 154.659 | 139410 | 901400 | N/A | N/A | N/A |