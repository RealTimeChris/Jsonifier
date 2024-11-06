# Json-Performance
Performance profiling of JSON libraries (Compiled and run on Windows 10.0.22631 using the MSVC 19.41.34123.0 compiler).  

Latest Results: (Nov 06, 2024)
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
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 197.946 | 1.86717e+07 | 1602853 | 2.9928e+07 | 299 | 220.164 | 1.67873e+07 | 1602853 | 2.69076e+07 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/890bf43) | 129.39 | 2.85646e+07 | 1602853 | 4.57849e+07 | 300 | 197.131 | 1.87489e+07 | 1602853 | 3.00517e+07 | 300 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/587beae) | 11.7887 | 3.13519e+08 | 1602853 | 5.02524e+08 | 299 | 

### Json Test (Minified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/Windows-MSVC/JsonData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Json%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Json%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 149.552 | 2.47137e+07 | 1145214 | 2.83024e+07 | 292 | 164.637 | 2.24493e+07 | 1145214 | 2.57092e+07 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/890bf43) | 107.428 | 3.44039e+07 | 1145214 | 3.93998e+07 | 300 | 145.342 | 2.54292e+07 | 1145214 | 2.91219e+07 | 300 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/587beae) | 9.00283 | 4.10535e+08 | 1145214 | 4.7015e+08 | 300 | 

### ABC Test (Out of Sequence Performance - Prettified) [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/Windows-MSVC/JsonData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Abc%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Abc%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>

The JSON documents in the previous tests featured keys ranging from "a" to "z," where each key corresponds to an array of values. Notably, the documents in this test arrange these keys in reverse order, deviating from the typical "a" to "z" arrangement.

This test effectively demonstrates the challenges encountered when utilizing simdjson and iterative parsers that lack the ability to efficiently allocate memory locations through hashing. In cases where the keys are not in the expected sequence, performance is significantly compromised, with the severity escalating as the document size increases.

In contrast, hash-based solutions offer a viable alternative by circumventing these issues and maintaining optimal performance regardless of the JSON document's scale, or ordering of the keys being parsed.

| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 194.139 | 1.90378e+07 | 1602853 | 3.05147e+07 | 299 | 218.288 | 1.69317e+07 | 1602853 | 2.7139e+07 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/890bf43) | 125.715 | 2.93997e+07 | 1602853 | 4.71234e+07 | 300 | 190.611 | 1.939e+07 | 1602853 | 3.10793e+07 | 296 | 


### ABC Test (Out of Sequence Performance - Minified) [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/Windows-MSVC/JsonData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Abc%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Abc%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 146.909 | 2.51584e+07 | 1145214 | 2.88118e+07 | 300 | 166.26 | 2.22301e+07 | 1145214 | 2.54582e+07 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/890bf43) | 106.291 | 3.47722e+07 | 1145214 | 3.98216e+07 | 300 | 146.332 | 2.52574e+07 | 1145214 | 2.89251e+07 | 300 | 


### Discord Test (Prettified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Discord%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Discord%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 269.575 | 1.37104e+07 | 138774 | 1.90264e+06 | 295 | 677.036 | 5.45906e+06 | 138774 | 757576 | 297 | 
| [glaze](https://github.com/stephenberry/glaze/commit/890bf43) | 161.793 | 2.28438e+07 | 138774 | 3.17013e+06 | 257 | 298.505 | 1.23816e+07 | 138774 | 1.71825e+06 | 300 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/587beae) | 15.0592 | 2.4543e+08 | 138482 | 3.39876e+07 | 300 | 

### Discord Test (Minified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Discord%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Discord%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 168.804 | 2.1895e+07 | 69037 | 1.51156e+06 | 300 | 471.908 | 7.83192e+06 | 69037 | 540692 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/890bf43) | 109.018 | 3.39026e+07 | 69037 | 2.34053e+06 | 300 | 195.082 | 1.89457e+07 | 69037 | 1.30795e+06 | 96 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/587beae) | 8.33679 | 4.43331e+08 | 68745 | 3.04768e+07 | 300 | 

### Canada Test (Prettified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/CanadaData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Canada%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Canada%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 135.784 | 2.72192e+07 | 6661897 | 1.81331e+08 | 300 | 120.367 | 3.07059e+07 | 6661897 | 2.0456e+08 | 283 | 
| [glaze](https://github.com/stephenberry/glaze/commit/890bf43) | 88.0257 | 4.19874e+07 | 6661897 | 2.79716e+08 | 286 | 107.427 | 3.44045e+07 | 6661897 | 2.29199e+08 | 98 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/587beae) | 20.4793 | 1.80472e+08 | 6661897 | 1.20228e+09 | 300 | 

### Canada Test (Minified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/CanadaData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Canada%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Canada%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 45.031 | 8.20762e+07 | 2090234 | 1.71559e+08 | 232 | 40.4112 | 9.14589e+07 | 2090234 | 1.9117e+08 | 295 | 
| [glaze](https://github.com/stephenberry/glaze/commit/890bf43) | 30.3618 | 1.21731e+08 | 2090234 | 2.54446e+08 | 98 | 36.309 | 1.01792e+08 | 2090234 | 2.1277e+08 | 97 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/587beae) | 6.97421 | 5.29948e+08 | 2090234 | 1.10772e+09 | 300 | 

### CitmCatalog Test (Prettified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/CitmCatalogData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/CitmCatalog%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/CitmCatalog%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 359.798 | 1.02723e+07 | 1439562 | 1.47875e+07 | 300 | 753.92 | 4.90235e+06 | 1439562 | 7.05724e+06 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/890bf43) | 212.68 | 1.73779e+07 | 1439584 | 2.5017e+07 | 222 | 377.229 | 9.79769e+06 | 1439584 | 1.41046e+07 | 300 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/587beae) | 21.8114 | 1.69452e+08 | 1428054 | 2.41986e+08 | 98 | 

### CitmCatalog Test (Minified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/CitmCatalogData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/CitmCatalog%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/CitmCatalog%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 181.532 | 2.03599e+07 | 500299 | 1.0186e+07 | 300 | 343.122 | 1.07717e+07 | 500299 | 5.38905e+06 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/890bf43) | 113.538 | 3.25526e+07 | 500299 | 1.6286e+07 | 300 | 178.891 | 2.06605e+07 | 500299 | 1.03364e+07 | 300 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/587beae) | 9.4139 | 3.92608e+08 | 497527 | 1.95333e+08 | 288 | 

### Twitter Test (Prettified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/TwitterData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Twitter%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Twitter%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 260.716 | 1.41763e+07 | 719107 | 1.01942e+07 | 299 | 617.665 | 5.98378e+06 | 719107 | 4.30298e+06 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/890bf43) | 175.73 | 2.10321e+07 | 719139 | 1.5125e+07 | 300 | 355.817 | 1.03873e+07 | 719139 | 7.46993e+06 | 300 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/587beae) | 10.5126 | 3.51574e+08 | 516830 | 1.81704e+08 | 97 | 

### Twitter Test (Minified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/TwitterData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Twitter%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Twitter%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 221.426 | 1.66916e+07 | 477683 | 7.9733e+06 | 299 | 522.025 | 7.08009e+06 | 477683 | 3.38204e+06 | 298 | 
| [glaze](https://github.com/stephenberry/glaze/commit/890bf43) | 157.493 | 2.34673e+07 | 477715 | 1.12107e+07 | 300 | 310.163 | 1.19162e+07 | 477715 | 5.69255e+06 | 300 | 


### Minify Test Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Minify%20Test_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Minify%20Test_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ------------ | ------------------| -------------------- | --------------- | --------------------- |   
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 134.836 | 2.74108e+07 | 69037 | 1.89236e+06 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/890bf43) | 74.3798 | 4.96923e+07 | 69037 | 3.4306e+06 | 297 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/587beae) | 8.87595 | 4.16457e+08 | 69037 | 2.87509e+07 | 195 | 

### Prettify Test Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Prettify%20Test_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Prettify%20Test_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ------------ | ------------------| -------------------- | --------------- | --------------------- |   
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 456.946 | 8.08841e+06 | 1602853 | 1.29645e+07 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/890bf43) | 346.916 | 1.06538e+07 | 1602853 | 1.70764e+07 | 244 | 

### Validation Test Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Validate%20Test_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Validate%20Test_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- |   
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 352.538 | 1.04839e+07 | 123050 | 1.29004e+06 | 100 | 
| [glaze](https://github.com/stephenberry/glaze/commit/890bf43) | 300.502 | 1.22993e+07 | 123050 | 1.51343e+06 | 300 | 