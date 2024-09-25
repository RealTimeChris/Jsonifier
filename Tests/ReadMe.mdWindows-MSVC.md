# Json-Performance
Performance profiling of JSON libraries (Compiled and run on Windows 10.0.22631 using the MSVC 19.41.34120.0 compiler).  

Latest Results: (Sep 26, 2024)
#### Using the following commits:
----
| Jsonifier: [](https://github.com/RealTimeChris/Jsonifier/commit/)  
| Glaze: [6d249bf](https://github.com/stephenberry/glaze/commit/6d249bf)  
| Simdjson: [3ef3078](https://github.com/simdjson/simdjson/commit/3ef3078)  

 > At least 30 iterations on a (Intel(R) Core(TM) i7-8700K CPU @ 3.70G), until coefficient of variance is at or below 1%.


### Json Test (Prettified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/Windows-MSVC/JsonData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Json%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Json%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 321.259 | 1.15047e+07 | 1518463 | 1.74694e+07 | 300 | 266.071 | 1.38909e+07 | 1518463 | 2.10929e+07 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/6d249bf) | 182.316 | 2.02724e+07 | 1519967 | 3.08134e+07 | 300 | 290.579 | 1.27193e+07 | 1519967 | 1.93329e+07 | 300 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/3ef3078) | 11.2201 | 3.29406e+08 | 1267853 | 4.17638e+08 | 300 | 

### Json Test (Minified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/Windows-MSVC/JsonData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Json%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Json%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 285.815 | 1.29313e+07 | 1266349 | 1.63756e+07 | 295 | 271.209 | 1.36278e+07 | 1266349 | 1.72575e+07 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/6d249bf) | 193.317 | 1.91187e+07 | 1267853 | 2.42397e+07 | 295 | 269.198 | 1.37296e+07 | 1267853 | 1.74071e+07 | 300 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/3ef3078) | 12.2014 | 3.02913e+08 | 1267853 | 3.84049e+08 | 300 | 

### ABC Test (Out of Sequence Performance - Prettified) [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/Windows-MSVC/JsonData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Abc%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Abc%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>

The JSON documents in the previous tests featured keys ranging from "a" to "z," where each key corresponds to an array of values. Notably, the documents in this test arrange these keys in reverse order, deviating from the typical "a" to "z" arrangement.

This test effectively demonstrates the challenges encountered when utilizing simdjson and iterative parsers that lack the ability to efficiently allocate memory locations through hashing. In cases where the keys are not in the expected sequence, performance is significantly compromised, with the severity escalating as the document size increases.

In contrast, hash-based solutions offer a viable alternative by circumventing these issues and maintaining optimal performance regardless of the JSON document's scale, or ordering of the keys being parsed.

| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 317.511 | 1.16405e+07 | 1518463 | 1.76756e+07 | 300 | 269.926 | 1.36926e+07 | 1518463 | 2.07917e+07 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/6d249bf) | 197.733 | 1.86918e+07 | 1519967 | 2.84109e+07 | 300 | 203.352 | 1.81753e+07 | 1519967 | 2.76259e+07 | 300 | 


### ABC Test (Out of Sequence Performance - Minified) [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/Windows-MSVC/JsonData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Abc%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Abc%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [glaze](https://github.com/stephenberry/glaze/commit/6d249bf) | 187.7 | 1.96909e+07 | 1267853 | 2.49651e+07 | 300 | 253.446 | 1.45829e+07 | 1267853 | 1.84889e+07 | 300 | 
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 175.91 | 2.10105e+07 | 1266349 | 2.66067e+07 | 300 | 252.429 | 1.46416e+07 | 1266349 | 1.85414e+07 | 300 | 


### Discord Test (Prettified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Discord%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Discord%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [glaze](https://github.com/stephenberry/glaze/commit/6d249bf) | 164.581 | 2.2457e+07 | 138774 | 3.11644e+06 | 300 | 321.802 | 1.14852e+07 | 138774 | 1.59385e+06 | 294 | 
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 157.407 | 2.34805e+07 | 138774 | 3.25848e+06 | 300 | 358.41 | 1.03121e+07 | 138774 | 1.43106e+06 | 300 | 


### Discord Test (Minified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Discord%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Discord%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [glaze](https://github.com/stephenberry/glaze/commit/6d249bf) | 79.8999 | 4.62574e+07 | 69037 | 3.19347e+06 | 300 | 126.029 | 2.93263e+07 | 69037 | 2.0246e+06 | 300 | 
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 65.2963 | 5.66032e+07 | 69037 | 3.90772e+06 | 300 | 209.196 | 1.76675e+07 | 69037 | 1.21971e+06 | 297 | 


### Canada Test (Prettified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/CanadaData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Canada%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Canada%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 124.805 | 2.96141e+07 | 6661897 | 1.97286e+08 | 300 | 89.8777 | 4.11222e+07 | 6661897 | 2.73952e+08 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/6d249bf) | 88.468 | 4.17768e+07 | 6661897 | 2.78313e+08 | 300 | 102.121 | 3.61922e+07 | 6661897 | 2.41109e+08 | 300 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/3ef3078) | 4.95027 | 7.46615e+08 | 2090234 | 1.5606e+09 | 300 | 

### Canada Test (Minified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/CanadaData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Canada%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Canada%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 38.8803 | 9.50594e+07 | 2090234 | 1.98696e+08 | 300 | 31.7818 | 1.16293e+08 | 2090234 | 2.43079e+08 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/6d249bf) | 24.7637 | 1.49247e+08 | 2090234 | 3.1196e+08 | 300 | 34.404 | 1.07429e+08 | 2090234 | 2.24551e+08 | 300 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/3ef3078) | 6.00152 | 6.15839e+08 | 2090234 | 1.28725e+09 | 300 | 

### CitmCatalog Test (Prettified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/CitmCatalogData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/CitmCatalog%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/CitmCatalog%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [glaze](https://github.com/stephenberry/glaze/commit/6d249bf) | 205.376 | 1.79962e+07 | 1439584 | 2.5907e+07 | 300 | 222.557 | 1.6607e+07 | 1439584 | 2.39071e+07 | 300 | 
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 196.946 | 1.87664e+07 | 1439556 | 2.70153e+07 | 300 | 324.759 | 1.13806e+07 | 1439556 | 1.63831e+07 | 300 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/3ef3078) | 7.13183 | 5.18237e+08 | 496411 | 2.57258e+08 | 300 | 

### CitmCatalog Test (Minified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/CitmCatalogData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/CitmCatalog%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/CitmCatalog%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 138.601 | 2.66663e+07 | 500293 | 1.3341e+07 | 300 | 178.926 | 2.06563e+07 | 500293 | 1.03342e+07 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/6d249bf) | 70.0873 | 5.27337e+07 | 500299 | 2.63826e+07 | 300 | 146.496 | 2.52291e+07 | 500299 | 1.26221e+07 | 300 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/3ef3078) | 7.43591 | 4.97043e+08 | 496411 | 2.46738e+08 | 300 | 

### Twitter Test (Prettified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/TwitterData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Twitter%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Twitter%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [glaze](https://github.com/stephenberry/glaze/commit/6d249bf) | 177.621 | 2.08081e+07 | 719139 | 1.49639e+07 | 300 | 351.641 | 1.05107e+07 | 719139 | 7.55863e+06 | 300 | 
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 135.535 | 2.72694e+07 | 719230 | 1.9613e+07 | 300 | 351.553 | 1.05133e+07 | 719230 | 7.56146e+06 | 300 | 


### Twitter Test (Minified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/TwitterData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Twitter%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Twitter%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 175.87 | 2.10155e+07 | 477797 | 1.00412e+07 | 300 | 315.514 | 1.17142e+07 | 477797 | 5.59699e+06 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/6d249bf) | 150.904 | 2.44923e+07 | 477706 | 1.17001e+07 | 300 | 200.6 | 1.84245e+07 | 477706 | 8.80151e+06 | 300 | 


### Minify Test Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Minify%20Test_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Minify%20Test_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ------------ | ------------------| -------------------- | --------------- | --------------------- |   
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 79.7316 | 4.63551e+07 | 69037 | 3.20022e+06 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/6d249bf) | 47.3683 | 7.80263e+07 | 69037 | 5.3867e+06 | 300 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/3ef3078) | 7.07972 | 5.22048e+08 | 69037 | 3.60407e+07 | 300 | 

### Prettify Test Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Prettify%20Test_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Prettify%20Test_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ------------ | ------------------| -------------------- | --------------- | --------------------- |   
| [glaze](https://github.com/stephenberry/glaze/commit/6d249bf) | 498.295 | 7.41722e+06 | 1520031 | 1.12744e+07 | 300 | 
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 246.055 | 1.5021e+07 | 1520031 | 2.28324e+07 | 300 | 

### Validation Test Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Validate%20Test_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Validate%20Test_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- |   
| [glaze](https://github.com/stephenberry/glaze/commit/6d249bf) | 304.85 | 1.21239e+07 | 123050 | 1.49184e+06 | 300 | 
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 233.442 | 1.58325e+07 | 123050 | 1.94819e+06 | 300 | 