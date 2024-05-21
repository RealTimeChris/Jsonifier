# Json-Performance
Performance profiling of JSON libraries (Compiled and run on Ubuntu-22.04 using the Clang++18 compiler)

Latest Results: (May 21, 2024

### Json Test (Prettified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/JsonData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Json%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Json%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>

> 100 iterations on a 6 core (Intel i7 8700k)

| Library | Read (MB/s) | ReadLength (Bytes) | ReadTime (ns) | Write (MB/s) | WriteLength (Bytes) | WriteTime (ns)
| ------------------------------------------------- | ---------- | ----------- | ---------- | ----------- | ---------- | ---------- |
| [jsonifier](https://github.com/realtimechris/jsonifier) | 824.164 | 2338771 | 2.83775e+06 | 699.833 | 2338771 | 3.3419e+06 |
| [glaze](https://github.com/stephenberry/glaze) | 613.738 | 2338771 | 3.8107e+06 | 752.621 | 2338771 | 3.1075e+06 |
| [simdjson](https://github.com/simdjson/simdjson) | 174.276 | 2338771 | 1.34199e+07 | N/A | N/A | N/A |

### Json Test (Minified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/JsonData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Json%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Json%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>

> 100 iterations on a 6 core (Intel i7 8700k)

| Library | Read (MB/s) | ReadLength (Bytes) | ReadTime (ns) | Write (MB/s) | WriteLength (Bytes) | WriteTime (ns)
| ------------------------------------------------- | ---------- | ----------- | ---------- | ----------- | ---------- | ---------- |
| [jsonifier](https://github.com/realtimechris/jsonifier) | 679.939 | 1588202 | 2.3358e+06 | 712.582 | 1588202 | 2.2288e+06 |
| [glaze](https://github.com/stephenberry/glaze) | 459.27 | 1588202 | 3.4581e+06 | 568.169 | 1588202 | 2.7953e+06 |
| [simdjson](https://github.com/simdjson/simdjson) | 153.931 | 1588202 | 1.03176e+07 | N/A | N/A | N/A |

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
| [jsonifier](https://github.com/realtimechris/jsonifier) | 859.258 | 2338771 | 2.72185e+06 | 847.719 | 2338771 | 2.7589e+06 |
| [glaze](https://github.com/stephenberry/glaze) | 678.692 | 2338771 | 3.446e+06 | 911.802 | 2338771 | 2.565e+06 |
| [simdjson](https://github.com/simdjson/simdjson) | 78.2711 | 2338771 | 2.98804e+07 | N/A | N/A | N/A |

### ABC Test (Out of Sequence Performance - Minified) [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/JsonData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Abc%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Abc%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>

> 100 iterations on a 6 core (Intel i7 8700k)

| Library | Read (MB/s) | ReadLength (Bytes) | ReadTime (ns) | Write (MB/s) | WriteLength (Bytes) | WriteTime (ns)
| ------------------------------------------------- | ---------- | ----------- | ---------- | ----------- | ---------- | ---------- |
| [jsonifier](https://github.com/realtimechris/jsonifier) | 621.192 | 1588202 | 2.5567e+06 | 717.767 | 1588202 | 2.2127e+06 |
| [glaze](https://github.com/stephenberry/glaze) | 445.299 | 1588202 | 3.5666e+06 | 619.194 | 1588202 | 2.56495e+06 |
| [simdjson](https://github.com/simdjson/simdjson) | 71.478 | 1588202 | 2.22194e+07 | N/A | N/A | N/A |

### Discord Test (Prettified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Discord%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Discord%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>

> 100 iterations on a 6 core (Intel i7 8700k)

| Library | Read (MB/s) | ReadLength (Bytes) | ReadTime (ns) | Write (MB/s) | WriteLength (Bytes) | WriteTime (ns)
| ------------------------------------------------- | ---------- | ----------- | ---------- | ----------- | ---------- | ---------- |
| [glaze](https://github.com/stephenberry/glaze) | 638.245 | 136712 | 214200 | 1237.77 | 136712 | 110450 |
| [jsonifier](https://github.com/realtimechris/jsonifier) | 494.616 | 136712 | 276400 | 506.341 | 136712 | 270000 |
| [simdjson](https://github.com/simdjson/simdjson) | 325.66 | 136712 | 419800 | N/A | N/A | N/A |

### Discord Test (Minified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Discord%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Discord%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>

> 100 iterations on a 6 core (Intel i7 8700k)

| Library | Read (MB/s) | ReadLength (Bytes) | ReadTime (ns) | Write (MB/s) | WriteLength (Bytes) | WriteTime (ns)
| ------------------------------------------------- | ---------- | ----------- | ---------- | ----------- | ---------- | ---------- |
| [jsonifier](https://github.com/realtimechris/jsonifier) | 545.573 | 73025 | 133850 | 716.282 | 73025 | 101950 |
| [glaze](https://github.com/stephenberry/glaze) | 430.065 | 73025 | 169800 | 635.553 | 73025 | 114900 |
| [simdjson](https://github.com/simdjson/simdjson) | 139.707 | 73025 | 522700 | N/A | N/A | N/A |

### Canada Test (Prettified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/CanadaData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Canada%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Canada%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>

> 100 iterations on a 6 core (Intel i7 8700k)

| Library | Read (MB/s) | ReadLength (Bytes) | ReadTime (ns) | Write (MB/s) | WriteLength (Bytes) | WriteTime (ns)
| ------------------------------------------------- | ---------- | ----------- | ---------- | ----------- | ---------- | ---------- |
| [glaze](https://github.com/stephenberry/glaze) | 1185.05 | 6661897 | 5.6216e+06 | 774.752 | 6661897 | 8.59875e+06 |
| [jsonifier](https://github.com/realtimechris/jsonifier) | 1094.53 | 6661897 | 6.08655e+06 | 589.475 | 6661897 | 1.13014e+07 |
| [simdjson](https://github.com/simdjson/simdjson) | 422.496 | 6661897 | 1.5768e+07 | N/A | N/A | N/A |

### Canada Test (Minified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/CanadaData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Canada%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Canada%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>

> 100 iterations on a 6 core (Intel i7 8700k)

| Library | Read (MB/s) | ReadLength (Bytes) | ReadTime (ns) | Write (MB/s) | WriteLength (Bytes) | WriteTime (ns)
| ------------------------------------------------- | ---------- | ----------- | ---------- | ----------- | ---------- | ---------- |
| [jsonifier](https://github.com/realtimechris/jsonifier) | 555.19 | 2090234 | 3.7649e+06 | 348.605 | 2090234 | 5.996e+06 |
| [glaze](https://github.com/stephenberry/glaze) | 511.566 | 2090234 | 4.08595e+06 | 302.38 | 2090234 | 6.9126e+06 |
| [simdjson](https://github.com/simdjson/simdjson) | 195 | 2090234 | 1.07192e+07 | N/A | N/A | N/A |

### Twitter Test (Prettified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/TwitterData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Twitter%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Twitter%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>

> 100 iterations on a 6 core (Intel i7 8700k)

| Library | Read (MB/s) | ReadLength (Bytes) | ReadTime (ns) | Write (MB/s) | WriteLength (Bytes) | WriteTime (ns)
| ------------------------------------------------- | ---------- | ----------- | ---------- | ----------- | ---------- | ---------- |
| [jsonifier](https://github.com/realtimechris/jsonifier) | 1106.06 | 399674 | 361350 | 1122.84 | 399674 | 355950 |
| [glaze](https://github.com/stephenberry/glaze) | 723.13 | 399674 | 552700 | 2206.32 | 399674 | 181150 |
| [simdjson](https://github.com/simdjson/simdjson) | 276.448 | 399674 | 1.44575e+06 | N/A | N/A | N/A |

### Twitter Test (Minified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/TwitterData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Twitter%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Twitter%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>

> 100 iterations on a 6 core (Intel i7 8700k)

| Library | Read (MB/s) | ReadLength (Bytes) | ReadTime (ns) | Write (MB/s) | WriteLength (Bytes) | WriteTime (ns)
| ------------------------------------------------- | ---------- | ----------- | ---------- | ----------- | ---------- | ---------- |
| [jsonifier](https://github.com/realtimechris/jsonifier) | 791.177 | 268921 | 339900 | 1133.97 | 268921 | 237150 |
| [glaze](https://github.com/stephenberry/glaze) | 561.891 | 268921 | 478600 | 1757.65 | 268921 | 153000 |
| [simdjson](https://github.com/simdjson/simdjson) | 185.809 | 268921 | 1.4473e+06 | N/A | N/A | N/A |

### Minify Test Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Minify%20Test_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Minify%20Test_Results.png?raw=true" 
alt="" width="400"/></p>

> 100 iterations on a 6 core (Intel i7 8700k)

| Library | Read (MB/s) | ReadLength (Bytes) | ReadTime (ns) | Write (MB/s) | WriteLength (Bytes) | WriteTime (ns)
| ------------------------------------------------- | ---------- | ----------- | ---------- | ----------- | ---------- | ---------- |
| [jsonifier](https://github.com/realtimechris/jsonifier) | N/A | N/A | N/A | 395.478 | 73025 | 184650 |
| [glaze](https://github.com/stephenberry/glaze) | N/A | N/A | N/A | 267.491 | 73025 | 273000 |
| [simdjson](https://github.com/simdjson/simdjson) | N/A | N/A | N/A | 149.412 | 73025 | 488750 |

### Prettify Test Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Prettify%20Test_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Prettify%20Test_Results.png?raw=true" 
alt="" width="400"/></p>

> 100 iterations on a 6 core (Intel i7 8700k)

| Library | Read (MB/s) | ReadLength (Bytes) | ReadTime (ns) | Write (MB/s) | WriteLength (Bytes) | WriteTime (ns)
| ------------------------------------------------- | ---------- | ----------- | ---------- | ----------- | ---------- | ---------- |
| [jsonifier](https://github.com/realtimechris/jsonifier) | N/A | N/A | N/A | 1023.46 | 2338771 | 2.28515e+06 |
| [glaze](https://github.com/stephenberry/glaze) | N/A | N/A | N/A | 675.838 | 2338771 | 3.46055e+06 |

### Validation Test Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Validate%20Test_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Validate%20Test_Results.png?raw=true" 
alt="" width="400"/></p>

> 100 iterations on a 6 core (Intel i7 8700k)

| Library | Read (MB/s) | ReadLength (Bytes) | ReadTime (ns) | Write (MB/s) | WriteLength (Bytes) | WriteTime (ns)
| ------------------------------------------------- | ---------- | ----------- | ---------- | ----------- | ---------- | ---------- |
| [jsonifier](https://github.com/realtimechris/jsonifier) | 659.171 | 136712 | 207400 | N/A | N/A | N/A |