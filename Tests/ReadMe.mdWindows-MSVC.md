# Json-Performance
Performance profiling of JSON libraries (Compiled and run on Windows 10.0.22631 using the MSVC 19.41.34120.0 compiler).  

Latest Results: (Sep 29, 2024)
#### Using the following commits:
----
| Jsonifier: [](https://github.com/RealTimeChris/Jsonifier/commit/)  
| Glaze: [469f381](https://github.com/stephenberry/glaze/commit/469f381)  
| Simdjson: [3ef3078](https://github.com/simdjson/simdjson/commit/3ef3078)  

 > At least 30 iterations on a (Intel(R) Core(TM) i7-9750H CPU @ 2.60G), until coefficient of variance is at or below 1%.


### Json Test (Prettified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/Windows-MSVC/JsonData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Json%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Json%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 187.822 | 1.38003e+07 | 1660770 | 2.29191e+07 | 300 | 212.384 | 1.22043e+07 | 1660770 | 2.02685e+07 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/469f381) | 125.163 | 2.0709e+07 | 1660712 | 3.43917e+07 | 300 | 220.283 | 1.17667e+07 | 1660712 | 1.9541e+07 | 300 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/3ef3078) | 9.49317 | 2.73038e+08 | 1376847 | 3.75932e+08 | 300 | 

### Json Test (Minified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/Windows-MSVC/JsonData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Json%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Json%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 150.626 | 1.72078e+07 | 1376905 | 2.36935e+07 | 300 | 190.253 | 1.3624e+07 | 1376905 | 1.87589e+07 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/469f381) | 129.956 | 1.99451e+07 | 1376847 | 2.74613e+07 | 300 | 197.519 | 1.31228e+07 | 1376847 | 1.8068e+07 | 300 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/3ef3078) | 10.9732 | 2.3621e+08 | 1376847 | 3.25226e+08 | 300 | 

### ABC Test (Out of Sequence Performance - Prettified) [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/Windows-MSVC/JsonData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Abc%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Abc%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>

The JSON documents in the previous tests featured keys ranging from "a" to "z," where each key corresponds to an array of values. Notably, the documents in this test arrange these keys in reverse order, deviating from the typical "a" to "z" arrangement.

This test effectively demonstrates the challenges encountered when utilizing simdjson and iterative parsers that lack the ability to efficiently allocate memory locations through hashing. In cases where the keys are not in the expected sequence, performance is significantly compromised, with the severity escalating as the document size increases.

In contrast, hash-based solutions offer a viable alternative by circumventing these issues and maintaining optimal performance regardless of the JSON document's scale, or ordering of the keys being parsed.

| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [glaze](https://github.com/stephenberry/glaze/commit/469f381) | 154.981 | 1.67245e+07 | 1660712 | 2.77747e+07 | 300 | 233.445 | 1.11032e+07 | 1660712 | 1.84393e+07 | 300 | 
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 128.969 | 2.00979e+07 | 1660770 | 3.3378e+07 | 300 | 160.836 | 1.61158e+07 | 1660770 | 2.67646e+07 | 300 | 


### ABC Test (Out of Sequence Performance - Minified) [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/Windows-MSVC/JsonData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Abc%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Abc%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 172.696 | 1.50089e+07 | 1376905 | 2.06659e+07 | 300 | 232.019 | 1.11714e+07 | 1376905 | 1.5382e+07 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/469f381) | 158.362 | 1.63675e+07 | 1376847 | 2.25355e+07 | 300 | 212.094 | 1.22209e+07 | 1376847 | 1.68264e+07 | 300 | 


### Discord Test (Prettified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Discord%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Discord%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 190.755 | 1.3588e+07 | 138774 | 1.88567e+06 | 300 | 311.4 | 8.32365e+06 | 138774 | 1.15511e+06 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/469f381) | 136.225 | 1.90272e+07 | 138774 | 2.64049e+06 | 300 | 280.461 | 9.2419e+06 | 138774 | 1.28254e+06 | 300 | 


### Discord Test (Minified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Discord%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Discord%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 116.726 | 2.22056e+07 | 69037 | 1.53301e+06 | 300 | 174.534 | 1.48509e+07 | 69037 | 1.02526e+06 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/469f381) | 105.644 | 2.45351e+07 | 69037 | 1.69383e+06 | 300 | 169.591 | 1.52836e+07 | 69037 | 1.05514e+06 | 300 | 


### Canada Test (Prettified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/CanadaData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Canada%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Canada%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 83.1519 | 3.11718e+07 | 6661897 | 2.07663e+08 | 300 | 81.2955 | 3.18834e+07 | 6661897 | 2.12404e+08 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/469f381) | 61.8086 | 4.1935e+07 | 6661897 | 2.79367e+08 | 300 | 76.7474 | 3.3773e+07 | 6661897 | 2.24992e+08 | 300 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/3ef3078) | 5.2817 | 4.90749e+08 | 2090234 | 1.02578e+09 | 300 | 

### Canada Test (Minified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/CanadaData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Canada%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Canada%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 35.5882 | 7.28313e+07 | 2090234 | 1.52234e+08 | 300 | 30.8247 | 8.4088e+07 | 2090234 | 1.75763e+08 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/469f381) | 21.6364 | 1.19798e+08 | 2090234 | 2.50405e+08 | 300 | 27.3012 | 9.49397e+07 | 2090234 | 1.98446e+08 | 300 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/3ef3078) | 4.63801 | 5.58842e+08 | 2090234 | 1.16811e+09 | 300 | 

### CitmCatalog Test (Prettified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/CitmCatalogData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/CitmCatalog%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/CitmCatalog%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 208.303 | 1.24433e+07 | 1439556 | 1.79128e+07 | 300 | 269.428 | 9.62033e+06 | 1439556 | 1.3849e+07 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/469f381) | 188.367 | 1.37603e+07 | 1439584 | 1.98091e+07 | 300 | 310.681 | 8.34294e+06 | 1439584 | 1.20104e+07 | 300 | 


### CitmCatalog Test (Minified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/CitmCatalogData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/CitmCatalog%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/CitmCatalog%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 77.446 | 3.34684e+07 | 500293 | 1.6744e+07 | 300 | 171.911 | 1.50775e+07 | 500293 | 7.54318e+06 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/469f381) | 67.5024 | 3.83985e+07 | 500299 | 1.92107e+07 | 300 | 123.402 | 2.10044e+07 | 500299 | 1.05085e+07 | 300 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/3ef3078) | 8.02335 | 3.23056e+08 | 496411 | 1.60368e+08 | 300 | 

### Twitter Test (Prettified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/TwitterData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Twitter%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Twitter%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 184.703 | 1.40333e+07 | 719239 | 1.00933e+07 | 300 | 363.94 | 7.12575e+06 | 719239 | 5.12512e+06 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/469f381) | 139.542 | 1.85751e+07 | 719148 | 1.33582e+07 | 300 | 284.77 | 9.10204e+06 | 719148 | 6.54572e+06 | 300 | 


### Twitter Test (Minified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/TwitterData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Twitter%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Twitter%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [glaze](https://github.com/stephenberry/glaze/commit/469f381) | 129.777 | 1.99725e+07 | 477715 | 9.54118e+06 | 300 | 242.241 | 1.07001e+07 | 477715 | 5.11158e+06 | 300 | 
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 107.629 | 2.40827e+07 | 477806 | 1.15069e+07 | 300 | 267.72 | 9.68169e+06 | 477806 | 4.62597e+06 | 300 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/3ef3078) | 4.58665 | 5.65109e+08 | 257279 | 1.45391e+08 | 300 | 

### Minify Test Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Minify%20Test_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Minify%20Test_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ------------ | ------------------| -------------------- | --------------- | --------------------- |   
| [glaze](https://github.com/stephenberry/glaze/commit/469f381) | 62.3359 | 4.15808e+07 | 69037 | 2.87062e+06 | 300 | 
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 59.9998 | 4.31999e+07 | 69037 | 2.98239e+06 | 300 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/3ef3078) | 6.71459 | 3.86023e+08 | 69037 | 2.66499e+07 | 300 | 

### Prettify Test Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Prettify%20Test_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Prettify%20Test_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ------------ | ------------------| -------------------- | --------------- | --------------------- |   
| [glaze](https://github.com/stephenberry/glaze/commit/469f381) | 386.024 | 6.71456e+06 | 1660770 | 1.11513e+07 | 300 | 
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 234.563 | 1.10501e+07 | 1660770 | 1.83516e+07 | 300 | 

### Validation Test Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Validate%20Test_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Validate%20Test_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- |   
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 265.433 | 9.76506e+06 | 123050 | 1.20159e+06 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/469f381) | 247.684 | 1.04647e+07 | 123050 | 1.28768e+06 | 300 | 