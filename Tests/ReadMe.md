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
| [glaze](https://github.com/stephenberry/glaze/commit/158ac74) | 636.285 | 4304672 | 6.76532e+06 | 1.646 | 874.698 | 4304028 | 4.92059e+06 | 2.482 |
| [jsonifier](https://github.com/RealTimeChris/Jsonifier/commit/) | 570.592 | 4304672 | 7.54422e+06 | 2.735 | 1058.35 | 4298467 | 4.06147e+06 | 1.916 |
| [simdjson](https://github.com/simdjson/simdjson/commit/ba1819f) | 208.466 | 4304672 | 2.06493e+07 | 0.9633 | 

### Json Test (Minified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/JsonData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Json%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Json%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/s) | ReadLength (Bytes) | ReadTime (ns) | ReadMape (%) | Write (MB/s) | WriteLength (Bytes) | WriteTime (ns) | WriteMape (%) |
| ------------------------------------------------- | ---------- | ----------- | ---------- | ----------- | ---------- | ---------- | ---------- | ---------- |
| [jsonifier](https://github.com/RealTimeChris/Jsonifier/commit/) | 721.703 | 2958273 | 4.09902e+06 | 0.8628 | 888.673 | 2952068 | 3.32188e+06 | 1.376 |
| [glaze](https://github.com/stephenberry/glaze/commit/158ac74) | 578.656 | 2958273 | 5.11232e+06 | 1.925 | 762.332 | 2957629 | 3.87971e+06 | 0.0287 |
| [simdjson](https://github.com/simdjson/simdjson/commit/ba1819f) | 158.036 | 2958273 | 1.8719e+07 | 1.515 | 

### ABC Test (Out of Sequence Performance - Prettified) [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/JsonData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Abc%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Abc%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>

The JSON documents in the previous tests featured keys ranging from "a" to "z," where each key corresponds to an array of values. Notably, the documents in this test arrange these keys in reverse order, deviating from the typical "a" to "z" arrangement.

This test effectively demonstrates the challenges encountered when utilizing simdjson and iterative parsers that lack the ability to efficiently allocate memory locations through hashing. In cases where the keys are not in the expected sequence, performance is significantly compromised, with the severity escalating as the document size increases.

In contrast, hash-based solutions offer a viable alternative by circumventing these issues and maintaining optimal performance regardless of the JSON document's scale, or ordering of the keys being parsed.

| Library | Read (MB/s) | ReadLength (Bytes) | ReadTime (ns) | ReadMape (%) | Write (MB/s) | WriteLength (Bytes) | WriteTime (ns) | WriteMape (%) |
| ------------------------------------------------- | ---------- | ----------- | ---------- | ----------- | ---------- | ---------- | ---------- | ---------- |
| [glaze](https://github.com/stephenberry/glaze/commit/158ac74) | 719.769 | 4304672 | 5.98063e+06 | 1.324 | 1016.75 | 4304028 | 4.23314e+06 | 1.754 |
| [jsonifier](https://github.com/RealTimeChris/Jsonifier/commit/) | 682.086 | 4304672 | 6.31104e+06 | 1.099 | 1196.84 | 4298467 | 3.59152e+06 | 0.5928 |
| [simdjson](https://github.com/simdjson/simdjson/commit/ba1819f) | 128.597 | 4304672 | 3.34741e+07 | 1.454 | 

### ABC Test (Out of Sequence Performance - Minified) [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/JsonData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Abc%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Abc%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/s) | ReadLength (Bytes) | ReadTime (ns) | ReadMape (%) | Write (MB/s) | WriteLength (Bytes) | WriteTime (ns) | WriteMape (%) |
| ------------------------------------------------- | ---------- | ----------- | ---------- | ----------- | ---------- | ---------- | ---------- | ---------- |
| [jsonifier](https://github.com/RealTimeChris/Jsonifier/commit/) | 730.477 | 2958273 | 4.04978e+06 | 1.411 | 945.615 | 2952068 | 3.12185e+06 | 0.9725 |
| [glaze](https://github.com/stephenberry/glaze/commit/158ac74) | 613.909 | 2958273 | 4.81875e+06 | 0.02542 | 610.453 | 2957629 | 4.84497e+06 | 3.124 |
| [simdjson](https://github.com/simdjson/simdjson/commit/ba1819f) | 101.589 | 2958273 | 2.912e+07 | 0.628 | 

### Discord Test (Prettified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Discord%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Discord%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/s) | ReadLength (Bytes) | ReadTime (ns) | ReadMape (%) | Write (MB/s) | WriteLength (Bytes) | WriteTime (ns) | WriteMape (%) |
| ------------------------------------------------- | ---------- | ----------- | ---------- | ----------- | ---------- | ---------- | ---------- | ---------- |
| [glaze](https://github.com/stephenberry/glaze/commit/158ac74) | 767.516 | 138774 | 180809 | 0.06726 | 2461.23 | 138774 | 56384 | 0.03829 |
| [jsonifier](https://github.com/RealTimeChris/Jsonifier/commit/) | 171.759 | 138774 | 807956 | 0.1015 | 2106.57 | 138774 | 65876.9 | 0.0586 |
| [simdjson](https://github.com/simdjson/simdjson/commit/ba1819f) | 93.8168 | 138774 | 1.4792e+06 | 0.5008 | 

### Discord Test (Minified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Discord%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Discord%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/s) | ReadLength (Bytes) | ReadTime (ns) | ReadMape (%) | Write (MB/s) | WriteLength (Bytes) | WriteTime (ns) | WriteMape (%) |
| ------------------------------------------------- | ---------- | ----------- | ---------- | ----------- | ---------- | ---------- | ---------- | ---------- |
| [glaze](https://github.com/stephenberry/glaze/commit/158ac74) | 568.805 | 69037 | 121372 | 0.08421 | 1699.27 | 69037 | 40627.5 | 0.2571 |
| [jsonifier](https://github.com/RealTimeChris/Jsonifier/commit/) | 99.2486 | 69037 | 695596 | 0.01592 | 1661.95 | 69037 | 41539.9 | 0.1669 |
| [simdjson](https://github.com/simdjson/simdjson/commit/ba1819f) | 47.5967 | 69037 | 1.45046e+06 | 0.0339 | 

### Canada Test (Prettified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/CanadaData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Canada%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Canada%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/s) | ReadLength (Bytes) | ReadTime (ns) | ReadMape (%) | Write (MB/s) | WriteLength (Bytes) | WriteTime (ns) | WriteMape (%) |
| ------------------------------------------------- | ---------- | ----------- | ---------- | ----------- | ---------- | ---------- | ---------- | ---------- |
| [jsonifier](https://github.com/RealTimeChris/Jsonifier/commit/) | 1148.88 | 6661897 | 5.79862e+06 | 1.41 | 1123.21 | 6661897 | 5.93115e+06 | 0.03465 |
| [glaze](https://github.com/stephenberry/glaze/commit/158ac74) | 813.314 | 6661897 | 8.19106e+06 | 1.666 | 1112.69 | 6661897 | 5.98718e+06 | 0.02401 |
| [simdjson](https://github.com/simdjson/simdjson/commit/ba1819f) | 270.063 | 6661897 | 2.4668e+07 | 0.9882 | 

### Canada Test (Minified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/CanadaData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Canada%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Canada%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/s) | ReadLength (Bytes) | ReadTime (ns) | ReadMape (%) | Write (MB/s) | WriteLength (Bytes) | WriteTime (ns) | WriteMape (%) |
| ------------------------------------------------- | ---------- | ----------- | ---------- | ----------- | ---------- | ---------- | ---------- | ---------- |
| [jsonifier](https://github.com/RealTimeChris/Jsonifier/commit/) | 735.993 | 2090234 | 2.84002e+06 | 1.053 | 427.224 | 2090234 | 4.89259e+06 | 0.01254 |
| [glaze](https://github.com/stephenberry/glaze/commit/158ac74) | 416.65 | 2090234 | 5.01676e+06 | 1.014 | 406.393 | 2090234 | 5.14338e+06 | 1.119 |
| [simdjson](https://github.com/simdjson/simdjson/commit/ba1819f) | 102.234 | 2090234 | 2.04456e+07 | 0.7817 | 

### Twitter Test (Prettified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/TwitterData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Twitter%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Twitter%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/s) | ReadLength (Bytes) | ReadTime (ns) | ReadMape (%) | Write (MB/s) | WriteLength (Bytes) | WriteTime (ns) | WriteMape (%) |
| ------------------------------------------------- | ---------- | ----------- | ---------- | ----------- | ---------- | ---------- | ---------- | ---------- |
| [glaze](https://github.com/stephenberry/glaze/commit/158ac74) | 758.233 | 722038 | 952264 | 0.8063 | 2291.85 | 721479 | 314803 | 0.09077 |
| [jsonifier](https://github.com/RealTimeChris/Jsonifier/commit/) | 245.988 | 722038 | 2.93526e+06 | 0.7119 | 2645.52 | 722038 | 272929 | 0.02857 |
| [simdjson](https://github.com/simdjson/simdjson/commit/ba1819f) | 92.1911 | 722038 | 7.83197e+06 | 1.297 | 

### Twitter Test (Minified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/TwitterData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Twitter%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Twitter%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/s) | ReadLength (Bytes) | ReadTime (ns) | ReadMape (%) | Write (MB/s) | WriteLength (Bytes) | WriteTime (ns) | WriteMape (%) |
| ------------------------------------------------- | ---------- | ----------- | ---------- | ----------- | ---------- | ---------- | ---------- | ---------- |
| [glaze](https://github.com/stephenberry/glaze/commit/158ac74) | 766.904 | 478986 | 624571 | 0.06821 | 2201.28 | 478427 | 217341 | 0.01616 |
| [jsonifier](https://github.com/RealTimeChris/Jsonifier/commit/) | 167.383 | 478986 | 2.86162e+06 | 1.414 | 2399.49 | 478986 | 199620 | 1.058 |
| [simdjson](https://github.com/simdjson/simdjson/commit/ba1819f) | 65.4495 | 478986 | 7.31841e+06 | 1.33 | 

### Minify Test Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Minify%20Test_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Minify%20Test_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Write (MB/s) | WriteLength (Bytes) | WriteTime (ns) | WriteMape (%) |
| ------------------------------------------------- | ---------- | ----------- | ---------- | ----------- |
| [jsonifier](https://github.com/RealTimeChris/Jsonifier/commit/) | 533.046 | 69037 | 129514 | 0.02026 |
| [glaze](https://github.com/stephenberry/glaze/commit/158ac74) | 484.267 | 69037 | 142560 | 0.05682 |
| [simdjson](https://github.com/simdjson/simdjson/commit/ba1819f) | 162.825 | 69037 | 423995 | 0.1009 |

### Prettify Test Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Prettify%20Test_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Prettify%20Test_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Write (MB/s) | WriteLength (Bytes) | WriteTime (ns) | WriteMape (%) |
| ------------------------------------------------- | ---------- | ----------- | ---------- | ----------- |
| [jsonifier](https://github.com/RealTimeChris/Jsonifier/commit/) | 1293.55 | 4304672 | 3.32781e+06 | 2.548 |
| [glaze](https://github.com/stephenberry/glaze/commit/158ac74) | 1196.33 | 4304672 | 3.59825e+06 | 0.09311 |

### Validation Test Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Validate%20Test_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Validate%20Test_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/s) | ReadLength (Bytes) | ReadTime (ns) | ReadMape (%) |
| ------------------------------------------------- | ---------- | ----------- | ---------- | ----------- |
| [glaze](https://github.com/stephenberry/glaze/commit/158ac74) | 861.838 | 138774 | 161021 | 0.0849 | 
| [jsonifier](https://github.com/RealTimeChris/Jsonifier/commit/) | 787.231 | 138774 | 176281 | 0.01028 | 