# Json-Performance
Performance profiling of JSON libraries (Compiled and run on Ubuntu-22.04 using the Clang++18 compiler)

Latest Results: (May 20, 2024

### Json Test (Prettified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/JsonData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Json%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Json%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>

> 100 iterations on a 6 core (Intel i7 8700k)

| Library | Read (MB/s) | ReadLength (Bytes) | ReadTime (ns) | Write (MB/s) | WriteLength (Bytes) | WriteTime (ns)
| ------------------------------------------------- | ---------- | ----------- | ---------- | ----------- | ---------- | ---------- |
| [glaze](https://github.com/stephenberry/glaze) | 637.675 | 2716494 | 4.26e+06 | 811.148 | 2716494 | 3.34895e+06 |
| [jsonifier](https://github.com/realtimechris/jsonifier) | 620.757 | 2716494 | 4.3761e+06 | 752.731 | 2716494 | 3.60885e+06 |
| [simdjson](https://github.com/simdjson/simdjson) | 197.889 | 2716494 | 1.37274e+07 | N/A | N/A | N/A |

### Json Test (Minified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/JsonData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Json%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Json%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>

> 100 iterations on a 6 core (Intel i7 8700k)

| Library | Read (MB/s) | ReadLength (Bytes) | ReadTime (ns) | Write (MB/s) | WriteLength (Bytes) | WriteTime (ns)
| ------------------------------------------------- | ---------- | ----------- | ---------- | ----------- | ---------- | ---------- |
| [glaze](https://github.com/stephenberry/glaze) | 491.332 | 1843651 | 3.75235e+06 | 558.208 | 1843651 | 3.3028e+06 |
| [jsonifier](https://github.com/realtimechris/jsonifier) | 485.907 | 1843651 | 3.79425e+06 | 596.448 | 1843651 | 3.09105e+06 |
| [simdjson](https://github.com/simdjson/simdjson) | 143.654 | 1843651 | 1.2834e+07 | N/A | N/A | N/A |

### ABC Test (Out of Sequence Performance - Prettified) [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/JsonData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Abc%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Abc%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>

The JSON documents in the previous tests featured keys ranging from "a" to "z," where each key corresponds to an array of values. Notably, the documents in this test arrange these keys in reverse order, deviating from the typical "a" to "z" arrangement.

This test effectively demonstrates the challenges encountered when utilizing simdjson and iterative parsers that lack the ability to efficiently allocate memory locations through hashing. In cases where the keys are not in the expected sequence, performance is significantly compromised, with the severity escalating as the document size increases.

In contrast, hash-based solutions offer a viable alternative by circumventing these issues and maintaining optimal performance regardless of the JSON document's scale, or ordering of the keys being parsed.
> 100 iterations on a 6 core (Intel i7 8700k)

| Library | Read (MB/s) | ReadLength (Bytes) | ReadTime (ns) | Write (MB/s) | WriteLength (Bytes) | WriteTime (ns)
| ------------------------------------------------- | ---------- | ----------- | ---------- | ----------- | ---------- | ---------- |
| [jsonifier](https://github.com/realtimechris/jsonifier) | 664.058 | 2716494 | 4.09075e+06 | 842.311 | 2716494 | 3.22505e+06 |
| [glaze](https://github.com/stephenberry/glaze) | 611.355 | 2716494 | 4.4434e+06 | 760.656 | 2716494 | 3.57125e+06 |
| [simdjson](https://github.com/simdjson/simdjson) | 102.848 | 2716494 | 2.64126e+07 | N/A | N/A | N/A |

### ABC Test (Out of Sequence Performance - Minified) [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/JsonData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Abc%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Abc%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>

> 100 iterations on a 6 core (Intel i7 8700k)

| Library | Read (MB/s) | ReadLength (Bytes) | ReadTime (ns) | Write (MB/s) | WriteLength (Bytes) | WriteTime (ns)
| ------------------------------------------------- | ---------- | ----------- | ---------- | ----------- | ---------- | ---------- |
| [glaze](https://github.com/stephenberry/glaze) | 540.082 | 1843651 | 3.41365e+06 | 584.905 | 1843651 | 3.15205e+06 |
| [jsonifier](https://github.com/realtimechris/jsonifier) | 345.635 | 1843651 | 5.3341e+06 | 451.56 | 1843651 | 4.08285e+06 |
| [simdjson](https://github.com/simdjson/simdjson) | 77.8436 | 1843651 | 2.3684e+07 | N/A | N/A | N/A |

### Discord Test (Prettified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Discord%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Discord%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>

> 100 iterations on a 6 core (Intel i7 8700k)

| Library | Read (MB/s) | ReadLength (Bytes) | ReadTime (ns) | Write (MB/s) | WriteLength (Bytes) | WriteTime (ns)
| ------------------------------------------------- | ---------- | ----------- | ---------- | ----------- | ---------- | ---------- |
| [jsonifier](https://github.com/realtimechris/jsonifier) | 655.14 | 136695 | 208650 | 417.644 | 136695 | 327300 |
| [glaze](https://github.com/stephenberry/glaze) | 437.844 | 136695 | 312200 | 949.719 | 136712 | 143950 |
| [simdjson](https://github.com/simdjson/simdjson) | 222.088 | 136695 | 615500 | N/A | N/A | N/A |

### Discord Test (Minified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Discord%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Discord%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>

> 100 iterations on a 6 core (Intel i7 8700k)

| Library | Read (MB/s) | ReadLength (Bytes) | ReadTime (ns) | Write (MB/s) | WriteLength (Bytes) | WriteTime (ns)
| ------------------------------------------------- | ---------- | ----------- | ---------- | ----------- | ---------- | ---------- |
| [glaze](https://github.com/stephenberry/glaze) | 449.523 | 73025 | 162450 | 625.482 | 73025 | 116750 |
| [jsonifier](https://github.com/realtimechris/jsonifier) | 404.907 | 73025 | 180350 | 795.479 | 73025 | 91800 |
| [simdjson](https://github.com/simdjson/simdjson) | 111.048 | 73025 | 657600 | N/A | N/A | N/A |

### Canada Test (Prettified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/CanadaData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Canada%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Canada%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>

> 100 iterations on a 6 core (Intel i7 8700k)

| Library | Read (MB/s) | ReadLength (Bytes) | ReadTime (ns) | Write (MB/s) | WriteLength (Bytes) | WriteTime (ns)
| ------------------------------------------------- | ---------- | ----------- | ---------- | ----------- | ---------- | ---------- |
| [glaze](https://github.com/stephenberry/glaze) | 1155.95 | 6661897 | 5.76315e+06 | 870.029 | 6661897 | 7.6571e+06 |
| [jsonifier](https://github.com/realtimechris/jsonifier) | 996.73 | 6661897 | 6.68375e+06 | 901.127 | 6661897 | 7.39285e+06 |
| [simdjson](https://github.com/simdjson/simdjson) | 415.011 | 6661897 | 1.60524e+07 | N/A | N/A | N/A |

### Canada Test (Minified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/CanadaData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Canada%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Canada%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>

> 100 iterations on a 6 core (Intel i7 8700k)

| Library | Read (MB/s) | ReadLength (Bytes) | ReadTime (ns) | Write (MB/s) | WriteLength (Bytes) | WriteTime (ns)
| ------------------------------------------------- | ---------- | ----------- | ---------- | ----------- | ---------- | ---------- |
| [glaze](https://github.com/stephenberry/glaze) | 694.741 | 2090234 | 3.00865e+06 | 360.199 | 2090234 | 5.803e+06 |
| [jsonifier](https://github.com/realtimechris/jsonifier) | 413.748 | 2090234 | 5.05195e+06 | 346.849 | 2090234 | 6.02635e+06 |
| [simdjson](https://github.com/simdjson/simdjson) | 196.117 | 2090234 | 1.06581e+07 | N/A | N/A | N/A |

### Twitter Test (Prettified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/TwitterData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Twitter%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Twitter%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>

> 100 iterations on a 6 core (Intel i7 8700k)

| Library | Read (MB/s) | ReadLength (Bytes) | ReadTime (ns) | Write (MB/s) | WriteLength (Bytes) | WriteTime (ns)
| ------------------------------------------------- | ---------- | ----------- | ---------- | ----------- | ---------- | ---------- |
| [glaze](https://github.com/stephenberry/glaze) | 723.85 | 399674 | 552150 | 2058.58 | 399674 | 194150 |
| [jsonifier](https://github.com/realtimechris/jsonifier) | 683.203 | 399674 | 585000 | 1159.32 | 399674 | 344750 |
| [simdjson](https://github.com/simdjson/simdjson) | 272.434 | 399674 | 1.46705e+06 | N/A | N/A | N/A |

### Twitter Test (Minified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/TwitterData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Twitter%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Twitter%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>

> 100 iterations on a 6 core (Intel i7 8700k)

| Library | Read (MB/s) | ReadLength (Bytes) | ReadTime (ns) | Write (MB/s) | WriteLength (Bytes) | WriteTime (ns)
| ------------------------------------------------- | ---------- | ----------- | ---------- | ----------- | ---------- | ---------- |
| [jsonifier](https://github.com/realtimechris/jsonifier) | 592.794 | 268921 | 453650 | 1329.32 | 268921 | 202300 |
| [glaze](https://github.com/stephenberry/glaze) | 578.947 | 268921 | 464500 | 1769.22 | 268921 | 152000 |
| [simdjson](https://github.com/simdjson/simdjson) | 205.991 | 268921 | 1.3055e+06 | N/A | N/A | N/A |

### Minify Test Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Minify%20Test_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Minify%20Test_Results.png?raw=true" 
alt="" width="400"/></p>

> 100 iterations on a 6 core (Intel i7 8700k)

| Library | Read (MB/s) | ReadLength (Bytes) | ReadTime (ns) | Write (MB/s) | WriteLength (Bytes) | WriteTime (ns)
| ------------------------------------------------- | ---------- | ----------- | ---------- | ----------- | ---------- | ---------- |
| [jsonifier](https://github.com/realtimechris/jsonifier) | N/A | N/A | N/A | 438.589 | 73025 | 166500 |
| [glaze](https://github.com/stephenberry/glaze) | N/A | N/A | N/A | 415.151 | 73025 | 175900 |
| [simdjson](https://github.com/simdjson/simdjson) | N/A | N/A | N/A | 134.907 | 73025 | 541300 |

### Prettify Test Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Prettify%20Test_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Prettify%20Test_Results.png?raw=true" 
alt="" width="400"/></p>

> 100 iterations on a 6 core (Intel i7 8700k)

| Library | Read (MB/s) | ReadLength (Bytes) | ReadTime (ns) | Write (MB/s) | WriteLength (Bytes) | WriteTime (ns)
| ------------------------------------------------- | ---------- | ----------- | ---------- | ----------- | ---------- | ---------- |
| [glaze](https://github.com/stephenberry/glaze) | N/A | N/A | N/A | 615.748 | 2716494 | 4.4117e+06 |
| [jsonifier](https://github.com/realtimechris/jsonifier) | N/A | N/A | N/A | 583.797 | 2716494 | 4.65315e+06 |

### Validation Test Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Validate%20Test_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Validate%20Test_Results.png?raw=true" 
alt="" width="400"/></p>

> 100 iterations on a 6 core (Intel i7 8700k)

| Library | Read (MB/s) | ReadLength (Bytes) | ReadTime (ns) | Write (MB/s) | WriteLength (Bytes) | WriteTime (ns)
| ------------------------------------------------- | ---------- | ----------- | ---------- | ----------- | ---------- | ---------- |
| [jsonifier](https://github.com/realtimechris/jsonifier) | 599.408 | 136695 | 228050 | N/A | N/A | N/A |