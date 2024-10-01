# Json-Performance
Performance profiling of JSON libraries (Compiled and run on Windows 10.0.22631 using the MSVC 19.41.34120.0 compiler).  

Latest Results: (Oct 01, 2024)
#### Using the following commits:
----
| Jsonifier: [](https://github.com/RealTimeChris/Jsonifier/commit/)  
| Glaze: [76fbda3](https://github.com/stephenberry/glaze/commit/76fbda3)  
| Simdjson: [3ef3078](https://github.com/simdjson/simdjson/commit/3ef3078)  

 > At least 30 iterations on a (Intel(R) Core(TM) i7-8700K CPU @ 3.70G), until coefficient of variance is at or below 1%.


### Json Test (Prettified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/Windows-MSVC/JsonData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Json%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Json%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 256.799 | 1.43924e+07 | 1842974 | 2.65248e+07 | 300 | 222.301 | 1.6626e+07 | 1842974 | 3.06412e+07 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/76fbda3) | 182.223 | 2.02827e+07 | 1843990 | 3.7401e+07 | 300 | 288.723 | 1.28011e+07 | 1843990 | 2.36051e+07 | 300 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/3ef3078) | 12.0236 | 3.07391e+08 | 1531548 | 4.70785e+08 | 298 | 

### Json Test (Minified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/Windows-MSVC/JsonData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Json%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Json%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 228.511 | 1.61741e+07 | 1530532 | 2.4755e+07 | 300 | 211.451 | 1.7479e+07 | 1530532 | 2.67522e+07 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/76fbda3) | 174.511 | 2.11791e+07 | 1531548 | 3.24368e+07 | 300 | 244.669 | 1.51127e+07 | 1531548 | 2.31458e+07 | 300 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/3ef3078) | 12.6593 | 2.91954e+08 | 1531548 | 4.47142e+08 | 300 | 

### ABC Test (Out of Sequence Performance - Prettified) [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/Windows-MSVC/JsonData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Abc%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Abc%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>

The JSON documents in the previous tests featured keys ranging from "a" to "z," where each key corresponds to an array of values. Notably, the documents in this test arrange these keys in reverse order, deviating from the typical "a" to "z" arrangement.

This test effectively demonstrates the challenges encountered when utilizing simdjson and iterative parsers that lack the ability to efficiently allocate memory locations through hashing. In cases where the keys are not in the expected sequence, performance is significantly compromised, with the severity escalating as the document size increases.

In contrast, hash-based solutions offer a viable alternative by circumventing these issues and maintaining optimal performance regardless of the JSON document's scale, or ordering of the keys being parsed.

| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 259.43 | 1.42465e+07 | 1842974 | 2.6256e+07 | 300 | 213.707 | 1.72946e+07 | 1842974 | 3.18735e+07 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/76fbda3) | 186.951 | 1.97697e+07 | 1843990 | 3.64551e+07 | 300 | 290.211 | 1.27355e+07 | 1843990 | 2.34841e+07 | 300 | 


### ABC Test (Out of Sequence Performance - Minified) [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/Windows-MSVC/JsonData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Abc%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Abc%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 226.712 | 1.63025e+07 | 1530532 | 2.49515e+07 | 300 | 219.661 | 1.68257e+07 | 1530532 | 2.57523e+07 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/76fbda3) | 172.202 | 2.1463e+07 | 1531548 | 3.28716e+07 | 300 | 256.996 | 1.43814e+07 | 1531548 | 2.20259e+07 | 300 | 


### Discord Test (Prettified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Discord%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Discord%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 197.758 | 1.86892e+07 | 138897 | 2.59588e+06 | 300 | 313.393 | 1.17934e+07 | 138897 | 1.63807e+06 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/76fbda3) | 159.487 | 2.31741e+07 | 138774 | 3.21596e+06 | 300 | 290.709 | 1.27137e+07 | 138774 | 1.76433e+06 | 294 | 


### Discord Test (Minified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Discord%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Discord%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 125.078 | 2.95494e+07 | 69160 | 2.04364e+06 | 300 | 199.527 | 1.85237e+07 | 69160 | 1.2811e+06 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/76fbda3) | 115.596 | 3.19732e+07 | 69037 | 2.20734e+06 | 300 | 192.299 | 1.92198e+07 | 69037 | 1.32688e+06 | 300 | 


### Canada Test (Prettified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/CanadaData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Canada%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Canada%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 122.526 | 3.01648e+07 | 6661897 | 2.00955e+08 | 300 | 93.7475 | 3.94248e+07 | 6661897 | 2.62644e+08 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/76fbda3) | 84.3777 | 4.38022e+07 | 6661897 | 2.91806e+08 | 300 | 98.836 | 3.73948e+07 | 6661897 | 2.4912e+08 | 300 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/3ef3078) | 6.16746 | 5.99272e+08 | 2090234 | 1.25262e+09 | 300 | 

### Canada Test (Minified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/CanadaData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Canada%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Canada%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 40.8919 | 9.0384e+07 | 2090234 | 1.88924e+08 | 300 | 35.0019 | 1.05593e+08 | 2090234 | 2.20714e+08 | 298 | 
| [glaze](https://github.com/stephenberry/glaze/commit/76fbda3) | 29.4633 | 1.25442e+08 | 2090234 | 2.62203e+08 | 300 | 33.961 | 1.0883e+08 | 2090234 | 2.2748e+08 | 225 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/3ef3078) | 6.28746 | 5.87833e+08 | 2090234 | 1.22871e+09 | 300 | 

### CitmCatalog Test (Prettified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/CitmCatalogData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/CitmCatalog%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/CitmCatalog%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 249.57 | 1.48094e+07 | 1453948 | 2.15321e+07 | 300 | 320.571 | 1.15317e+07 | 1453948 | 1.67664e+07 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/76fbda3) | 202.043 | 1.82931e+07 | 1439584 | 2.63344e+07 | 300 | 338.371 | 1.09227e+07 | 1439584 | 1.57241e+07 | 300 | 


### CitmCatalog Test (Minified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/CitmCatalogData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/CitmCatalog%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/CitmCatalog%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 119.225 | 3.09998e+07 | 514685 | 1.59551e+07 | 300 | 202.527 | 1.8249e+07 | 514685 | 9.39249e+06 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/76fbda3) | 105.631 | 3.49896e+07 | 500299 | 1.75052e+07 | 300 | 159.457 | 2.31786e+07 | 500299 | 1.15962e+07 | 300 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/3ef3078) | 8.35194 | 4.4253e+08 | 496411 | 2.19677e+08 | 300 | 

### Twitter Test (Prettified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/TwitterData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Twitter%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Twitter%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 196.616 | 1.8798e+07 | 720885 | 1.35512e+07 | 300 | 345.038 | 1.07117e+07 | 720885 | 7.72194e+06 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/76fbda3) | 167.066 | 2.21227e+07 | 719148 | 1.59095e+07 | 300 | 274.638 | 1.34576e+07 | 719148 | 9.67802e+06 | 300 | 


### Twitter Test (Minified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/TwitterData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Twitter%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Twitter%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [glaze](https://github.com/stephenberry/glaze/commit/76fbda3) | 156.689 | 2.35879e+07 | 477715 | 1.12683e+07 | 300 | 259.511 | 1.42439e+07 | 477715 | 6.8045e+06 | 300 | 
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 154.997 | 2.38454e+07 | 479452 | 1.14327e+07 | 300 | 272.328 | 1.35717e+07 | 479452 | 6.50699e+06 | 300 | 


### Minify Test Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Minify%20Test_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Minify%20Test_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ------------ | ------------------| -------------------- | --------------- | --------------------- |   
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 114.375 | 3.23365e+07 | 69037 | 2.23241e+06 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/76fbda3) | 73.0324 | 5.06072e+07 | 69037 | 3.49377e+06 | 300 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/3ef3078) | 8.49219 | 4.35221e+08 | 69037 | 3.00464e+07 | 300 | 

### Prettify Test Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Prettify%20Test_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Prettify%20Test_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ------------ | ------------------| -------------------- | --------------- | --------------------- |   
| [glaze](https://github.com/stephenberry/glaze/commit/76fbda3) | 477.665 | 7.73755e+06 | 1844061 | 1.42685e+07 | 300 | 
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 331.148 | 1.11611e+07 | 1844061 | 2.05817e+07 | 300 | 

### Validation Test Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Validate%20Test_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Validate%20Test_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- |   
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 333.947 | 1.10676e+07 | 123050 | 1.36186e+06 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/76fbda3) | 293.734 | 1.25827e+07 | 123050 | 1.5483e+06 | 300 | 