# Json-Performance
Performance profiling of JSON libraries (Compiled and run on Windows 10.0.22631 using the MSVC 19.41.34123.0 compiler).  

Latest Results: (Nov 03, 2024)
#### Using the following commits:
----
| Jsonifier: [](https://github.com/RealTimeChris/Jsonifier/commit/)  
| Glaze: [890bf43](https://github.com/stephenberry/glaze/commit/890bf43)  
| Simdjson: [587beae](https://github.com/simdjson/simdjson/commit/587beae)  

 > At least 30 iterations on a (Intel(R) Core(TM) i7-8700K CPU @ 3.70G), until coefficient of variance is at or below 1%.


### Json Test (Prettified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/Windows-MSVC/JsonData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Json%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Json%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 165.293 | 2.23602e+07 | 911045 | 2.03712e+07 | 300 | 189.786 | 1.94744e+07 | 911045 | 1.77421e+07 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/890bf43) | 114.461 | 3.22901e+07 | 911045 | 2.94178e+07 | 300 | 181.595 | 2.03529e+07 | 911045 | 1.85424e+07 | 300 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/587beae) | 10.3536 | 3.56974e+08 | 911045 | 3.25219e+08 | 300 | 

### Json Test (Minified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/Windows-MSVC/JsonData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Json%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Json%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 123.204 | 2.99989e+07 | 618629 | 1.85582e+07 | 300 | 140.418 | 2.63212e+07 | 618629 | 1.62831e+07 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/890bf43) | 91.0692 | 4.05841e+07 | 618629 | 2.51065e+07 | 300 | 122.803 | 3.00967e+07 | 618629 | 1.86187e+07 | 300 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/587beae) | 7.94112 | 4.65423e+08 | 618629 | 2.87924e+08 | 300 | 

### ABC Test (Out of Sequence Performance - Prettified) [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/Windows-MSVC/JsonData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Abc%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Abc%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>

The JSON documents in the previous tests featured keys ranging from "a" to "z," where each key corresponds to an array of values. Notably, the documents in this test arrange these keys in reverse order, deviating from the typical "a" to "z" arrangement.

This test effectively demonstrates the challenges encountered when utilizing simdjson and iterative parsers that lack the ability to efficiently allocate memory locations through hashing. In cases where the keys are not in the expected sequence, performance is significantly compromised, with the severity escalating as the document size increases.

In contrast, hash-based solutions offer a viable alternative by circumventing these issues and maintaining optimal performance regardless of the JSON document's scale, or ordering of the keys being parsed.

| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 154.172 | 2.3973e+07 | 911045 | 2.18405e+07 | 300 | 191.681 | 1.92818e+07 | 911045 | 1.75666e+07 | 294 | 
| [glaze](https://github.com/stephenberry/glaze/commit/890bf43) | 116.378 | 3.17584e+07 | 911045 | 2.89333e+07 | 300 | 166.128 | 2.22478e+07 | 911045 | 2.02687e+07 | 300 | 


### ABC Test (Out of Sequence Performance - Minified) [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/Windows-MSVC/JsonData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Abc%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Abc%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 107.89 | 3.4257e+07 | 618629 | 2.11924e+07 | 300 | 138.612 | 2.66642e+07 | 618629 | 1.64952e+07 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/890bf43) | 91.2451 | 4.0506e+07 | 618629 | 2.50582e+07 | 300 | 134.479 | 2.74836e+07 | 618629 | 1.70022e+07 | 300 | 


### Discord Test (Prettified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Discord%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Discord%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 248.794 | 1.48556e+07 | 138774 | 2.06157e+06 | 300 | 639.374 | 5.78061e+06 | 138774 | 802198 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/890bf43) | 165.974 | 2.22683e+07 | 138774 | 3.09026e+06 | 300 | 299.98 | 1.23208e+07 | 138774 | 1.7098e+06 | 297 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/587beae) | 14.1334 | 2.61506e+08 | 138482 | 3.62139e+07 | 300 | 

### Discord Test (Minified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Discord%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Discord%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [glaze](https://github.com/stephenberry/glaze/commit/890bf43) | 115.077 | 3.21173e+07 | 69037 | 2.21728e+06 | 300 | 196.999 | 1.87614e+07 | 69037 | 1.29523e+06 | 294 | 
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 99.4807 | 3.71527e+07 | 69037 | 2.56491e+06 | 300 | 435.288 | 8.49081e+06 | 69037 | 586180 | 300 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/587beae) | 8.16211 | 4.52821e+08 | 68745 | 3.11292e+07 | 300 | 

### Canada Test (Prettified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/CanadaData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Canada%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Canada%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 126.396 | 2.92411e+07 | 6661897 | 1.94801e+08 | 300 | 80.6951 | 4.58018e+07 | 6661897 | 3.05127e+08 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/890bf43) | 87.9907 | 4.20037e+07 | 6661897 | 2.79824e+08 | 300 | 87.8566 | 4.20682e+07 | 6661897 | 2.80254e+08 | 300 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/587beae) | 17.92 | 2.06248e+08 | 6661897 | 1.374e+09 | 300 | 

### Canada Test (Minified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/CanadaData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Canada%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Canada%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 40.6536 | 9.09138e+07 | 2090234 | 1.90031e+08 | 300 | 34.2068 | 1.08047e+08 | 2090234 | 2.25844e+08 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/890bf43) | 27.8316 | 1.32796e+08 | 2090234 | 2.77574e+08 | 300 | 36.1283 | 1.02301e+08 | 2090234 | 2.13832e+08 | 300 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/587beae) | 6.14671 | 6.01294e+08 | 2090234 | 1.25685e+09 | 300 | 

### CitmCatalog Test (Prettified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/CitmCatalogData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/CitmCatalog%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/CitmCatalog%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [glaze](https://github.com/stephenberry/glaze/commit/890bf43) | 210.041 | 1.75965e+07 | 1439584 | 2.53316e+07 | 300 | 323.017 | 1.1442e+07 | 1439584 | 1.64717e+07 | 300 | 
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 206.517 | 1.78967e+07 | 1439562 | 2.57635e+07 | 300 | 633.266 | 5.83633e+06 | 1439562 | 8.40175e+06 | 300 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/587beae) | 21.2375 | 1.7403e+08 | 1428054 | 2.48525e+08 | 300 | 

### CitmCatalog Test (Minified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/CitmCatalogData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/CitmCatalog%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/CitmCatalog%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 174.363 | 2.12055e+07 | 500299 | 1.06091e+07 | 300 | 298.027 | 1.24014e+07 | 500299 | 6.20442e+06 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/890bf43) | 103.815 | 3.56015e+07 | 500299 | 1.78114e+07 | 300 | 141.603 | 2.61028e+07 | 500299 | 1.30592e+07 | 300 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/587beae) | 7.36658 | 5.0172e+08 | 497527 | 2.49619e+08 | 300 | 

### Twitter Test (Prettified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/TwitterData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Twitter%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Twitter%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 227.587 | 1.62398e+07 | 719107 | 1.16781e+07 | 300 | 514.483 | 7.18385e+06 | 719107 | 5.16596e+06 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/890bf43) | 157.84 | 2.34158e+07 | 719139 | 1.68392e+07 | 300 | 331.111 | 1.11623e+07 | 719139 | 8.02722e+06 | 300 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/587beae) | 10.3295 | 3.57809e+08 | 516830 | 1.84926e+08 | 300 | 

### Twitter Test (Minified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/TwitterData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Twitter%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Twitter%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 196.988 | 1.87624e+07 | 477683 | 8.96247e+06 | 300 | 443.852 | 8.32699e+06 | 477683 | 3.97766e+06 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/890bf43) | 100.964 | 3.66066e+07 | 477715 | 1.74875e+07 | 300 | 313.829 | 1.17771e+07 | 477715 | 5.62608e+06 | 300 | 


### Minify Test Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Minify%20Test_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Minify%20Test_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ------------ | ------------------| -------------------- | --------------- | --------------------- |   
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 120.983 | 3.05495e+07 | 69037 | 2.10905e+06 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/890bf43) | 70.7027 | 5.22748e+07 | 69037 | 3.60889e+06 | 300 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/587beae) | 7.50172 | 4.92682e+08 | 69037 | 3.40133e+07 | 300 | 

### Prettify Test Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Prettify%20Test_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Prettify%20Test_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ------------ | ------------------| -------------------- | --------------- | --------------------- |   
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 383.929 | 9.62667e+06 | 911045 | 8.77033e+06 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/890bf43) | 216.436 | 1.70765e+07 | 911045 | 1.55574e+07 | 300 | 

### Validation Test Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Validate%20Test_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Validate%20Test_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- |   
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 346.489 | 1.06669e+07 | 123050 | 1.31256e+06 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/890bf43) | 293.775 | 1.2581e+07 | 123050 | 1.54809e+06 | 300 | 