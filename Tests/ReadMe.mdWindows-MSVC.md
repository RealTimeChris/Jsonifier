# Json-Performance
Performance profiling of JSON libraries (Compiled and run on Windows 10.0.22631 using the MSVC 19.41.34123.0 compiler).  

Latest Results: (Oct 25, 2024)
#### Using the following commits:
----
| Jsonifier: [](https://github.com/RealTimeChris/Jsonifier/commit/)  
| Glaze: [98fa3bc](https://github.com/stephenberry/glaze/commit/98fa3bc)  
| Simdjson: [3c0d032](https://github.com/simdjson/simdjson/commit/3c0d032)  

 > At least 30 iterations on a (Intel(R) Core(TM) i7-8700K CPU @ 3.70G), until coefficient of variance is at or below 1%.


### Json Test (Prettified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/Windows-MSVC/JsonData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Json%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Json%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 178.014 | 2.07623e+07 | 960534 | 1.99429e+07 | 300 | 189.948 | 1.94578e+07 | 960534 | 1.86899e+07 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/98fa3bc) | 115.065 | 3.21207e+07 | 960420 | 3.08494e+07 | 300 | 175.363 | 2.10761e+07 | 960420 | 2.02419e+07 | 300 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/3c0d032) | 11.2153 | 3.29548e+08 | 960420 | 3.16504e+08 | 294 | 

### Json Test (Minified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/Windows-MSVC/JsonData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Json%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Json%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 128.853 | 2.86837e+07 | 646944 | 1.85568e+07 | 300 | 137.377 | 2.69039e+07 | 646944 | 1.74053e+07 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/98fa3bc) | 92.7482 | 3.98495e+07 | 646830 | 2.57758e+07 | 300 | 131.694 | 2.80647e+07 | 646830 | 1.81531e+07 | 300 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/3c0d032) | 8.11376 | 4.5552e+08 | 646830 | 2.94644e+08 | 300 | 

### ABC Test (Out of Sequence Performance - Prettified) [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/Windows-MSVC/JsonData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Abc%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Abc%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>

The JSON documents in the previous tests featured keys ranging from "a" to "z," where each key corresponds to an array of values. Notably, the documents in this test arrange these keys in reverse order, deviating from the typical "a" to "z" arrangement.

This test effectively demonstrates the challenges encountered when utilizing simdjson and iterative parsers that lack the ability to efficiently allocate memory locations through hashing. In cases where the keys are not in the expected sequence, performance is significantly compromised, with the severity escalating as the document size increases.

In contrast, hash-based solutions offer a viable alternative by circumventing these issues and maintaining optimal performance regardless of the JSON document's scale, or ordering of the keys being parsed.

| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 174.764 | 2.11483e+07 | 960534 | 2.03136e+07 | 300 | 188.347 | 1.96233e+07 | 960534 | 1.88489e+07 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/98fa3bc) | 113.905 | 3.24478e+07 | 960420 | 3.11635e+07 | 295 | 174.804 | 2.11436e+07 | 960420 | 2.03068e+07 | 300 | 


### ABC Test (Out of Sequence Performance - Minified) [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/Windows-MSVC/JsonData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Abc%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Abc%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 124.193 | 2.97599e+07 | 646944 | 1.9253e+07 | 300 | 134.977 | 2.73822e+07 | 646944 | 1.77147e+07 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/98fa3bc) | 82.8343 | 4.46189e+07 | 646830 | 2.88609e+07 | 300 | 122.271 | 3.0224e+07 | 646830 | 1.95498e+07 | 300 | 


### Discord Test (Prettified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Discord%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Discord%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 203.484 | 1.81634e+07 | 138774 | 2.52061e+06 | 300 | 428.616 | 8.62303e+06 | 138774 | 1.19665e+06 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/98fa3bc) | 160.802 | 2.29846e+07 | 138774 | 3.18967e+06 | 164 | 294.458 | 1.25516e+07 | 138774 | 1.74184e+06 | 300 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/3c0d032) | 14.6932 | 2.51544e+08 | 138482 | 3.48343e+07 | 300 | 

### Discord Test (Minified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Discord%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Discord%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 131.207 | 2.8172e+07 | 69037 | 1.94491e+06 | 300 | 290.35 | 1.27293e+07 | 69037 | 878793 | 291 | 
| [glaze](https://github.com/stephenberry/glaze/commit/98fa3bc) | 111.646 | 3.31043e+07 | 69037 | 2.28542e+06 | 300 | 187.888 | 1.96711e+07 | 69037 | 1.35803e+06 | 300 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/3c0d032) | 8.26141 | 4.47378e+08 | 68745 | 3.0755e+07 | 300 | 

### Canada Test (Prettified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/CanadaData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Canada%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Canada%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 132.324 | 2.79304e+07 | 6661897 | 1.86069e+08 | 300 | 115.239 | 3.20724e+07 | 6661897 | 2.13663e+08 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/98fa3bc) | 88.4189 | 4.18007e+07 | 6661897 | 2.78472e+08 | 300 | 103.629 | 3.56654e+07 | 6661897 | 2.37599e+08 | 300 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/3c0d032) | 19.7218 | 1.87404e+08 | 6661897 | 1.24847e+09 | 300 | 

### Canada Test (Minified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/CanadaData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Canada%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Canada%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 42.5272 | 8.6927e+07 | 2090234 | 1.81698e+08 | 261 | 37.951 | 9.73879e+07 | 2090234 | 2.03563e+08 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/98fa3bc) | 29.8987 | 1.23616e+08 | 2090234 | 2.58387e+08 | 169 | 35.9988 | 1.0267e+08 | 2090234 | 2.14603e+08 | 300 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/3c0d032) | 6.82979 | 5.41155e+08 | 2090234 | 1.13114e+09 | 159 | 

### CitmCatalog Test (Prettified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/CitmCatalogData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/CitmCatalog%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/CitmCatalog%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 306.093 | 1.20746e+07 | 1439562 | 1.73822e+07 | 300 | 579.091 | 6.38236e+06 | 1439562 | 9.1878e+06 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/98fa3bc) | 206.949 | 1.78593e+07 | 1439584 | 2.571e+07 | 300 | 363.913 | 1.01561e+07 | 1439584 | 1.46206e+07 | 300 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/3c0d032) | 21.5452 | 1.71544e+08 | 1428054 | 2.44975e+08 | 300 | 

### CitmCatalog Test (Minified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/CitmCatalogData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/CitmCatalog%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/CitmCatalog%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 144.803 | 2.55241e+07 | 500299 | 1.27697e+07 | 300 | 272.414 | 1.35674e+07 | 500299 | 6.78776e+06 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/98fa3bc) | 109.266 | 3.3825e+07 | 500299 | 1.69226e+07 | 300 | 172.317 | 2.14487e+07 | 500299 | 1.07308e+07 | 300 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/3c0d032) | 9.20958 | 4.01315e+08 | 497527 | 1.99665e+08 | 300 | 

### Twitter Test (Prettified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/TwitterData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Twitter%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Twitter%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 216.751 | 1.70516e+07 | 719230 | 1.2264e+07 | 300 | 504.84 | 7.32107e+06 | 719230 | 5.26554e+06 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/98fa3bc) | 171.239 | 2.15836e+07 | 719139 | 1.55216e+07 | 300 | 324.664 | 1.1384e+07 | 719139 | 8.18669e+06 | 300 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/3c0d032) | 10.358 | 3.56817e+08 | 516830 | 1.84414e+08 | 296 | 

### Twitter Test (Minified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/TwitterData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Twitter%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Twitter%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 176.075 | 2.0991e+07 | 477806 | 1.00296e+07 | 300 | 435.408 | 8.48852e+06 | 477806 | 4.05586e+06 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/98fa3bc) | 155.685 | 2.37401e+07 | 477715 | 1.1341e+07 | 300 | 296.137 | 1.24807e+07 | 477715 | 5.9622e+06 | 300 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/3c0d032) | 6.03772 | 6.12147e+08 | 282852 | 1.73147e+08 | 223 | 

### Minify Test Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Minify%20Test_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Minify%20Test_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ------------ | ------------------| -------------------- | --------------- | --------------------- |   
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 108.462 | 3.40764e+07 | 69037 | 2.35253e+06 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/98fa3bc) | 73.8194 | 5.00678e+07 | 69037 | 3.45653e+06 | 300 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/3c0d032) | 8.71631 | 4.24031e+08 | 69037 | 2.92738e+07 | 300 | 

### Prettify Test Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Prettify%20Test_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Prettify%20Test_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ------------ | ------------------| -------------------- | --------------- | --------------------- |   
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 398.023 | 9.28573e+06 | 960534 | 8.91926e+06 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/98fa3bc) | 312.345 | 1.1833e+07 | 960534 | 1.1366e+07 | 300 | 

### Validation Test Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Validate%20Test_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Validate%20Test_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- |   
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 342.855 | 1.07799e+07 | 123050 | 1.32647e+06 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/98fa3bc) | 296.476 | 1.24663e+07 | 123050 | 1.53398e+06 | 300 | 