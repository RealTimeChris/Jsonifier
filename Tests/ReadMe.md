# Json-Performance
Performance profiling of JSON libraries (Compiled and run on Ubuntu-22.04 using the Clang++18 compiler)

Latest Results: (Jul 29, 2024)
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
| [glaze](https://github.com/stephenberry/glaze/commit/158ac74) | 822.629 | 2811064 | 3.41717e+06 | 1.707 | 1178.45 | 2810635 | 2.38502e+06 | 0.9538 |
| [jsonifier](https://github.com/RealTimeChris/Jsonifier/commit/) | 609.333 | 2811064 | 4.61334e+06 | 1.2 | 1309.42 | 2807170 | 2.14382e+06 | 1.54 |
| [simdjson](https://github.com/simdjson/simdjson/commit/ba1819f) | 248.83 | 2811064 | 1.12971e+07 | 0.7983 | 

### Json Test (Minified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/JsonData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Json%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Json%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/s) | ReadLength (Bytes) | ReadTime (ns) | ReadMape (%) | Write (MB/s) | WriteLength (Bytes) | WriteTime (ns) | WriteMape (%) |
| ------------------------------------------------- | ---------- | ----------- | ---------- | ----------- | ---------- | ---------- | ---------- | ---------- |
| [jsonifier](https://github.com/RealTimeChris/Jsonifier/commit/) | 717.912 | 1909501 | 2.6598e+06 | 0.831 | 1051.62 | 1905607 | 1.81207e+06 | 0.021 |
| [glaze](https://github.com/stephenberry/glaze/commit/158ac74) | 627.034 | 1909501 | 3.04529e+06 | 0.9747 | 904.022 | 1909072 | 2.11175e+06 | 1.595 |
| [simdjson](https://github.com/simdjson/simdjson/commit/ba1819f) | 178.296 | 1909501 | 1.07097e+07 | 2.158 | 

### ABC Test (Out of Sequence Performance - Prettified) [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/JsonData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Abc%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Abc%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>

The JSON documents in the previous tests featured keys ranging from "a" to "z," where each key corresponds to an array of values. Notably, the documents in this test arrange these keys in reverse order, deviating from the typical "a" to "z" arrangement.

This test effectively demonstrates the challenges encountered when utilizing simdjson and iterative parsers that lack the ability to efficiently allocate memory locations through hashing. In cases where the keys are not in the expected sequence, performance is significantly compromised, with the severity escalating as the document size increases.

In contrast, hash-based solutions offer a viable alternative by circumventing these issues and maintaining optimal performance regardless of the JSON document's scale, or ordering of the keys being parsed.

| Library | Read (MB/s) | ReadLength (Bytes) | ReadTime (ns) | ReadMape (%) | Write (MB/s) | WriteLength (Bytes) | WriteTime (ns) | WriteMape (%) |
| ------------------------------------------------- | ---------- | ----------- | ---------- | ----------- | ---------- | ---------- | ---------- | ---------- |
| [glaze](https://github.com/stephenberry/glaze/commit/158ac74) | 815.735 | 2811064 | 3.44605e+06 | 0.1247 | 1205.22 | 2810635 | 2.33206e+06 | 1.938 |
| [jsonifier](https://github.com/RealTimeChris/Jsonifier/commit/) | 614.387 | 2811064 | 4.5754e+06 | 1.095 | 1192.16 | 2807170 | 2.35469e+06 | 1.089 |
| [simdjson](https://github.com/simdjson/simdjson/commit/ba1819f) | 141.414 | 2811064 | 1.98783e+07 | 0.7857 | 

### ABC Test (Out of Sequence Performance - Minified) [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/JsonData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Abc%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Abc%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/s) | ReadLength (Bytes) | ReadTime (ns) | ReadMape (%) | Write (MB/s) | WriteLength (Bytes) | WriteTime (ns) | WriteMape (%) |
| ------------------------------------------------- | ---------- | ----------- | ---------- | ----------- | ---------- | ---------- | ---------- | ---------- |
| [jsonifier](https://github.com/RealTimeChris/Jsonifier/commit/) | 676.885 | 1909501 | 2.82101e+06 | 1.892 | 1060.58 | 1905607 | 1.79675e+06 | 1.257 |
| [glaze](https://github.com/stephenberry/glaze/commit/158ac74) | 638.396 | 1909501 | 2.99109e+06 | 0.9611 | 862.412 | 1909072 | 2.21364e+06 | 1.216 |
| [simdjson](https://github.com/simdjson/simdjson/commit/ba1819f) | 95.5618 | 1909501 | 1.99818e+07 | 1.392 | 

### Discord Test (Prettified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Discord%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Discord%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/s) | ReadLength (Bytes) | ReadTime (ns) | ReadMape (%) | Write (MB/s) | WriteLength (Bytes) | WriteTime (ns) | WriteMape (%) |
| ------------------------------------------------- | ---------- | ----------- | ---------- | ----------- | ---------- | ---------- | ---------- | ---------- |
| [glaze](https://github.com/stephenberry/glaze/commit/158ac74) | 884.72 | 138774 | 156856 | 0.03667 | 2782.79 | 138774 | 49868.6 | 0.01703 |
| [jsonifier](https://github.com/RealTimeChris/Jsonifier/commit/) | 203.714 | 138774 | 681221 | 1.059 | 2021.31 | 138774 | 68655.5 | 0.01911 |
| [simdjson](https://github.com/simdjson/simdjson/commit/ba1819f) | 94.8354 | 138774 | 1.46331e+06 | 0.03796 | 

### Discord Test (Minified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Discord%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Discord%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/s) | ReadLength (Bytes) | ReadTime (ns) | ReadMape (%) | Write (MB/s) | WriteLength (Bytes) | WriteTime (ns) | WriteMape (%) |
| ------------------------------------------------- | ---------- | ----------- | ---------- | ----------- | ---------- | ---------- | ---------- | ---------- |
| [glaze](https://github.com/stephenberry/glaze/commit/158ac74) | 531.455 | 69037 | 129902 | 0.01634 | 1594.03 | 69037 | 43309.7 | 0.02584 |
| [jsonifier](https://github.com/RealTimeChris/Jsonifier/commit/) | 92.8557 | 69037 | 743487 | 0.2229 | 1601.87 | 69037 | 43097.8 | 0.02507 |
| [simdjson](https://github.com/simdjson/simdjson/commit/ba1819f) | 41.8147 | 69037 | 1.65102e+06 | 3.019 | 

### Canada Test (Prettified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/CanadaData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Canada%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Canada%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/s) | ReadLength (Bytes) | ReadTime (ns) | ReadMape (%) | Write (MB/s) | WriteLength (Bytes) | WriteTime (ns) | WriteMape (%) |
| ------------------------------------------------- | ---------- | ----------- | ---------- | ----------- | ---------- | ---------- | ---------- | ---------- |
| [jsonifier](https://github.com/RealTimeChris/Jsonifier/commit/) | 1219.37 | 6661897 | 5.46341e+06 | 0.7374 | 1186.71 | 6661897 | 5.61377e+06 | 0.02783 |
| [glaze](https://github.com/stephenberry/glaze/commit/158ac74) | 973.939 | 6661897 | 6.84016e+06 | 0.01703 | 1204.42 | 6661897 | 5.53121e+06 | 0.007045 |
| [simdjson](https://github.com/simdjson/simdjson/commit/ba1819f) | 283.472 | 6661897 | 2.35011e+07 | 0.7417 | 

### Canada Test (Minified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/CanadaData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Canada%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Canada%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/s) | ReadLength (Bytes) | ReadTime (ns) | ReadMape (%) | Write (MB/s) | WriteLength (Bytes) | WriteTime (ns) | WriteMape (%) |
| ------------------------------------------------- | ---------- | ----------- | ---------- | ----------- | ---------- | ---------- | ---------- | ---------- |
| [jsonifier](https://github.com/RealTimeChris/Jsonifier/commit/) | 744.907 | 2090234 | 2.80603e+06 | 1.217 | 474.765 | 2090234 | 4.40267e+06 | 0.9915 |
| [glaze](https://github.com/stephenberry/glaze/commit/158ac74) | 411.055 | 2090234 | 5.08505e+06 | 1.053 | 387.376 | 2090234 | 5.39587e+06 | 2.275 |
| [simdjson](https://github.com/simdjson/simdjson/commit/ba1819f) | 101.505 | 2090234 | 2.05923e+07 | 1.488 | 

### Twitter Test (Prettified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/TwitterData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Twitter%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Twitter%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/s) | ReadLength (Bytes) | ReadTime (ns) | ReadMape (%) | Write (MB/s) | WriteLength (Bytes) | WriteTime (ns) | WriteMape (%) |
| ------------------------------------------------- | ---------- | ----------- | ---------- | ----------- | ---------- | ---------- | ---------- | ---------- |
| [glaze](https://github.com/stephenberry/glaze/commit/158ac74) | 910.849 | 722038 | 792709 | 0.09219 | 2706.91 | 721479 | 266533 | 0.08794 |
| [jsonifier](https://github.com/RealTimeChris/Jsonifier/commit/) | 250.383 | 722038 | 2.88373e+06 | 1.479 | 2765.26 | 722038 | 261111 | 0.3662 |
| [simdjson](https://github.com/simdjson/simdjson/commit/ba1819f) | 107.136 | 722038 | 6.73945e+06 | 1.019 | 

### Twitter Test (Minified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/TwitterData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Twitter%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Twitter%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/s) | ReadLength (Bytes) | ReadTime (ns) | ReadMape (%) | Write (MB/s) | WriteLength (Bytes) | WriteTime (ns) | WriteMape (%) |
| ------------------------------------------------- | ---------- | ----------- | ---------- | ----------- | ---------- | ---------- | ---------- | ---------- |
| [glaze](https://github.com/stephenberry/glaze/commit/158ac74) | 678.045 | 478986 | 706422 | 0.02562 | 2185.8 | 478427 | 218880 | 0.01604 |
| [jsonifier](https://github.com/RealTimeChris/Jsonifier/commit/) | 187.765 | 478986 | 2.55098e+06 | 0.9489 | 2506.77 | 478986 | 191077 | 0.9486 |
| [simdjson](https://github.com/simdjson/simdjson/commit/ba1819f) | 73.813 | 478986 | 6.48918e+06 | 0.8849 | 

### Minify Test Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Minify%20Test_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Minify%20Test_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Write (MB/s) | WriteLength (Bytes) | WriteTime (ns) | WriteMape (%) |
| ------------------------------------------------- | ---------- | ----------- | ---------- | ----------- |
| [jsonifier](https://github.com/RealTimeChris/Jsonifier/commit/) | 535.831 | 69037 | 128841 | 0.06712 |
| [glaze](https://github.com/stephenberry/glaze/commit/158ac74) | 507.442 | 69037 | 136049 | 0.001702 |
| [simdjson](https://github.com/simdjson/simdjson/commit/ba1819f) | 171.55 | 69037 | 402431 | 0.0322 |

### Prettify Test Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Prettify%20Test_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Prettify%20Test_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Write (MB/s) | WriteLength (Bytes) | WriteTime (ns) | WriteMape (%) |
| ------------------------------------------------- | ---------- | ----------- | ---------- | ----------- |
| [jsonifier](https://github.com/RealTimeChris/Jsonifier/commit/) | 1398.79 | 2811064 | 2.00964e+06 | 1.054 |
| [glaze](https://github.com/stephenberry/glaze/commit/158ac74) | 1304.15 | 2811064 | 2.15548e+06 | 0.6659 |

### Validation Test Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Validate%20Test_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Validate%20Test_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/s) | ReadLength (Bytes) | ReadTime (ns) | ReadMape (%) |
| ------------------------------------------------- | ---------- | ----------- | ---------- | ----------- |
| [glaze](https://github.com/stephenberry/glaze/commit/158ac74) | 907.201 | 138774 | 152969 | 0.03406 | 
| [jsonifier](https://github.com/RealTimeChris/Jsonifier/commit/) | 719.651 | 138774 | 192835 | 0.003601 | 