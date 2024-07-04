# Json-Performance
Performance profiling of JSON libraries (Compiled and run on Windows 10.0.22631 using the MSVC 19.41.34120.0 compiler).  

Latest Results: (Oct 03, 2024)
#### Using the following commits:
----
| Jsonifier: [](https://github.com/RealTimeChris/Jsonifier/commit/)  
| Glaze: [4b04364](https://github.com/stephenberry/glaze/commit/4b04364)  
| Simdjson: [3ef3078](https://github.com/simdjson/simdjson/commit/3ef3078)  

 > At least 30 iterations on a (Intel(R) Core(TM) i7-8700K CPU @ 3.70G), until coefficient of variance is at or below 1%.


### Json Test (Prettified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/Windows-MSVC/JsonData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Json%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Json%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [glaze](https://github.com/stephenberry/glaze/commit/4b04364) | 177.166 | 2.08616e+07 | 1741119 | 3.63226e+07 | 300 | 297.665 | 1.24166e+07 | 1741119 | 2.16187e+07 | 300 | 
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 176.707 | 2.09159e+07 | 1741183 | 3.64184e+07 | 300 | 254.255 | 1.45364e+07 | 1741183 | 2.53106e+07 | 300 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/3ef3078) | 11.77 | 3.14017e+08 | 1435736 | 4.50845e+08 | 300 | 

### Json Test (Minified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/Windows-MSVC/JsonData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Json%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Json%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 193.746 | 1.90763e+07 | 1435800 | 2.73898e+07 | 300 | 251.643 | 1.46874e+07 | 1435800 | 2.10881e+07 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/4b04364) | 181.065 | 2.04125e+07 | 1435736 | 2.93069e+07 | 300 | 257.992 | 1.43259e+07 | 1435736 | 2.05682e+07 | 300 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/3ef3078) | 12.5163 | 2.95291e+08 | 1435736 | 4.2396e+08 | 300 | 

### ABC Test (Out of Sequence Performance - Prettified) [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/Windows-MSVC/JsonData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Abc%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Abc%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>

The JSON documents in the previous tests featured keys ranging from "a" to "z," where each key corresponds to an array of values. Notably, the documents in this test arrange these keys in reverse order, deviating from the typical "a" to "z" arrangement.

This test effectively demonstrates the challenges encountered when utilizing simdjson and iterative parsers that lack the ability to efficiently allocate memory locations through hashing. In cases where the keys are not in the expected sequence, performance is significantly compromised, with the severity escalating as the document size increases.

In contrast, hash-based solutions offer a viable alternative by circumventing these issues and maintaining optimal performance regardless of the JSON document's scale, or ordering of the keys being parsed.

| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 212.506 | 1.73923e+07 | 1741183 | 3.02832e+07 | 300 | 251.229 | 1.47116e+07 | 1741183 | 2.56155e+07 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/4b04364) | 186.998 | 1.97647e+07 | 1741119 | 3.44128e+07 | 300 | 290.426 | 1.2726e+07 | 1741119 | 2.21575e+07 | 300 | 


### ABC Test (Out of Sequence Performance - Minified) [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/Windows-MSVC/JsonData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Abc%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Abc%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 178.549 | 2.07001e+07 | 1435800 | 2.97211e+07 | 300 | 271.456 | 1.36154e+07 | 1435800 | 1.9549e+07 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/4b04364) | 114.63 | 3.22426e+07 | 1435736 | 4.62919e+07 | 300 | 268.689 | 1.37556e+07 | 1435736 | 1.97494e+07 | 300 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/3ef3078) | 6.15961 | 6.00034e+08 | 1435736 | 8.61491e+08 | 300 | 

### Discord Test (Prettified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Discord%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Discord%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 211.289 | 1.74923e+07 | 138774 | 2.42747e+06 | 300 | 365.579 | 1.01099e+07 | 138774 | 1.40299e+06 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/4b04364) | 163.666 | 2.25825e+07 | 138774 | 3.13386e+06 | 300 | 303.405 | 1.21817e+07 | 138774 | 1.6905e+06 | 300 | 


### Discord Test (Minified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Discord%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Discord%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 128.663 | 2.87339e+07 | 69037 | 1.9837e+06 | 300 | 239.454 | 1.5435e+07 | 69037 | 1.06559e+06 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/4b04364) | 112.946 | 3.27233e+07 | 69037 | 2.25912e+06 | 300 | 198.685 | 1.86022e+07 | 69037 | 1.28424e+06 | 300 | 


### Canada Test (Prettified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/CanadaData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Canada%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Canada%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 134.668 | 2.74452e+07 | 6661897 | 1.82837e+08 | 300 | 98.2088 | 3.76335e+07 | 6661897 | 2.5071e+08 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/4b04364) | 90.5867 | 4.07996e+07 | 6661897 | 2.71803e+08 | 300 | 104.958 | 3.52163e+07 | 6661897 | 2.34608e+08 | 300 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/3ef3078) | 6.58408 | 5.6135e+08 | 2090234 | 1.17335e+09 | 300 | 

### Canada Test (Minified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/CanadaData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Canada%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Canada%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 43.0556 | 8.58421e+07 | 2090234 | 1.7943e+08 | 300 | 35.5348 | 1.04009e+08 | 2090234 | 2.17403e+08 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/4b04364) | 29.8584 | 1.23782e+08 | 2090234 | 2.58734e+08 | 300 | 34.3476 | 1.07605e+08 | 2090234 | 2.24919e+08 | 300 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/3ef3078) | 6.90999 | 5.34866e+08 | 2090234 | 1.11799e+09 | 300 | 

### CitmCatalog Test (Prettified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/CitmCatalogData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/CitmCatalog%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/CitmCatalog%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 224.426 | 1.64685e+07 | 1439556 | 2.37074e+07 | 300 | 364.112 | 1.01506e+07 | 1439556 | 1.46124e+07 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/4b04364) | 213.425 | 1.73174e+07 | 1439584 | 2.49299e+07 | 300 | 340.685 | 1.08485e+07 | 1439584 | 1.56174e+07 | 300 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/3ef3078) | 7.48337 | 4.9389e+08 | 496411 | 2.45172e+08 | 300 | 

### CitmCatalog Test (Minified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/CitmCatalogData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/CitmCatalog%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/CitmCatalog%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [glaze](https://github.com/stephenberry/glaze/commit/4b04364) | 108.68 | 3.40079e+07 | 500299 | 1.70141e+07 | 300 | 175.781 | 2.10257e+07 | 500299 | 1.05191e+07 | 300 | 
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 99.39 | 3.71867e+07 | 500293 | 1.86042e+07 | 300 | 130.511 | 2.83193e+07 | 500293 | 1.41679e+07 | 300 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/3ef3078) | 9.31512 | 3.9677e+08 | 496411 | 1.96961e+08 | 300 | 

### Twitter Test (Prettified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/TwitterData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Twitter%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Twitter%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 212.124 | 1.74237e+07 | 719239 | 1.25318e+07 | 300 | 439.362 | 8.41218e+06 | 719239 | 6.05037e+06 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/4b04364) | 180.99 | 2.04208e+07 | 719148 | 1.46856e+07 | 300 | 332.398 | 1.11191e+07 | 719148 | 7.99627e+06 | 300 | 


### Twitter Test (Minified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/TwitterData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Twitter%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Twitter%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [glaze](https://github.com/stephenberry/glaze/commit/4b04364) | 156.815 | 2.35691e+07 | 477715 | 1.12593e+07 | 300 | 311.169 | 1.18776e+07 | 477715 | 5.67412e+06 | 300 | 
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 111.282 | 3.32127e+07 | 477806 | 1.58692e+07 | 300 | 350.055 | 1.05583e+07 | 477806 | 5.0448e+06 | 300 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/3ef3078) | 5.29873 | 6.9752e+08 | 257279 | 1.79457e+08 | 300 | 

### Minify Test Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Minify%20Test_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Minify%20Test_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ------------ | ------------------| -------------------- | --------------- | --------------------- |   
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 83.6036 | 4.42082e+07 | 69037 | 3.052e+06 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/4b04364) | 75.4547 | 4.89825e+07 | 69037 | 3.3816e+06 | 97 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/3ef3078) | 8.80916 | 4.19561e+08 | 69037 | 2.89653e+07 | 297 | 

### Prettify Test Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Prettify%20Test_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Prettify%20Test_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ------------ | ------------------| -------------------- | --------------- | --------------------- |   
| [glaze](https://github.com/stephenberry/glaze/commit/4b04364) | 472.812 | 7.81702e+06 | 1741183 | 1.36109e+07 | 300 | 
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 344.777 | 1.07198e+07 | 1741183 | 1.86652e+07 | 300 | 

### Validation Test Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Validate%20Test_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Validate%20Test_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- |   
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 341.119 | 1.08348e+07 | 123050 | 1.33322e+06 | 98 | 
| [glaze](https://github.com/stephenberry/glaze/commit/4b04364) | 191.993 | 1.92505e+07 | 123050 | 2.36877e+06 | 300 | 