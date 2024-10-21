# Json-Performance
Performance profiling of JSON libraries (Compiled and run on Windows 10.0.22631 using the MSVC 19.41.34123.0 compiler).  

Latest Results: (Oct 22, 2024)
#### Using the following commits:
----
| Jsonifier: [](https://github.com/RealTimeChris/Jsonifier/commit/)  
| Glaze: [65d45d5](https://github.com/stephenberry/glaze/commit/65d45d5)  
| Simdjson: [3c0d032](https://github.com/simdjson/simdjson/commit/3c0d032)  

 > At least 30 iterations on a (Intel(R) Core(TM) i7-9750H CPU @ 2.60G), until coefficient of variance is at or below 1%.


### Json Test (Prettified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/Windows-MSVC/JsonData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Json%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Json%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 149.547 | 1.73322e+07 | 914273 | 1.58463e+07 | 300 | 139.29 | 1.86085e+07 | 914273 | 1.70132e+07 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/65d45d5) | 94.5079 | 2.7426e+07 | 914142 | 2.50712e+07 | 300 | 163.538 | 1.58494e+07 | 914142 | 1.44886e+07 | 300 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/3c0d032) | 9.79863 | 2.64524e+08 | 914142 | 2.41813e+08 | 300 | 

### Json Test (Minified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/Windows-MSVC/JsonData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Json%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Json%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 111.028 | 2.33452e+07 | 616766 | 1.43985e+07 | 300 | 105.555 | 2.45556e+07 | 616766 | 1.51451e+07 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/65d45d5) | 76.6829 | 3.38011e+07 | 616635 | 2.08429e+07 | 300 | 107.887 | 2.40359e+07 | 616635 | 1.48214e+07 | 300 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/3c0d032) | 7.00168 | 3.70192e+08 | 616635 | 2.28273e+08 | 300 | 

### ABC Test (Out of Sequence Performance - Prettified) [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/Windows-MSVC/JsonData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Abc%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Abc%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>

The JSON documents in the previous tests featured keys ranging from "a" to "z," where each key corresponds to an array of values. Notably, the documents in this test arrange these keys in reverse order, deviating from the typical "a" to "z" arrangement.

This test effectively demonstrates the challenges encountered when utilizing simdjson and iterative parsers that lack the ability to efficiently allocate memory locations through hashing. In cases where the keys are not in the expected sequence, performance is significantly compromised, with the severity escalating as the document size increases.

In contrast, hash-based solutions offer a viable alternative by circumventing these issues and maintaining optimal performance regardless of the JSON document's scale, or ordering of the keys being parsed.

| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 144.463 | 1.7942e+07 | 914273 | 1.64039e+07 | 300 | 130.103 | 1.99225e+07 | 914273 | 1.82146e+07 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/65d45d5) | 102.154 | 2.53731e+07 | 914142 | 2.31946e+07 | 300 | 158.196 | 1.63845e+07 | 914142 | 1.49777e+07 | 300 | 


### ABC Test (Out of Sequence Performance - Minified) [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/Windows-MSVC/JsonData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Abc%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Abc%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 105.395 | 2.45928e+07 | 616766 | 1.5168e+07 | 300 | 111.933 | 2.31565e+07 | 616766 | 1.42822e+07 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/65d45d5) | 74.944 | 3.45855e+07 | 616635 | 2.13266e+07 | 300 | 107.02 | 2.42196e+07 | 616635 | 1.49346e+07 | 300 | 


### Discord Test (Prettified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Discord%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Discord%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 187.982 | 1.37884e+07 | 138774 | 1.91348e+06 | 300 | 178.739 | 1.45015e+07 | 138774 | 2.01243e+06 | 292 | 
| [glaze](https://github.com/stephenberry/glaze/commit/65d45d5) | 125.413 | 2.06675e+07 | 138774 | 2.86811e+06 | 300 | 226.078 | 1.14649e+07 | 138774 | 1.59103e+06 | 300 | 


### Discord Test (Minified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Discord%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Discord%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 105.519 | 2.45639e+07 | 69037 | 1.69582e+06 | 300 | 123.269 | 2.10268e+07 | 69037 | 1.45163e+06 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/65d45d5) | 92.0544 | 2.8157e+07 | 69037 | 1.94387e+06 | 300 | 96.0746 | 2.69786e+07 | 69037 | 1.86252e+06 | 300 | 


### Canada Test (Prettified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/CanadaData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Canada%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Canada%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 110.346 | 2.34892e+07 | 6661897 | 1.56482e+08 | 300 | 88.9654 | 2.91347e+07 | 6661897 | 1.94092e+08 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/65d45d5) | 73.3118 | 3.53555e+07 | 6661897 | 2.35534e+08 | 300 | 94.8729 | 2.73205e+07 | 6661897 | 1.82006e+08 | 300 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/3c0d032) | 17.7406 | 1.46102e+08 | 6661897 | 9.73316e+08 | 300 | 

### Canada Test (Minified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/CanadaData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Canada%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Canada%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 38.0642 | 6.80946e+07 | 2090234 | 1.42334e+08 | 300 | 31.9251 | 8.11893e+07 | 2090234 | 1.69705e+08 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/65d45d5) | 27.9041 | 9.28875e+07 | 2090234 | 1.94157e+08 | 300 | 33.1469 | 7.81966e+07 | 2090234 | 1.63449e+08 | 300 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/3c0d032) | 5.99913 | 4.32059e+08 | 2090234 | 9.03105e+08 | 300 | 

### CitmCatalog Test (Prettified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/CitmCatalogData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/CitmCatalog%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/CitmCatalog%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 269.113 | 9.63156e+06 | 1439556 | 1.38652e+07 | 300 | 237.834 | 1.08983e+07 | 1439556 | 1.56887e+07 | 300 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/3c0d032) | 19.4533 | 1.33241e+08 | 1428057 | 1.90276e+08 | 300 | 


### CitmCatalog Test (Minified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/CitmCatalogData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/CitmCatalog%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/CitmCatalog%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 126.132 | 2.05497e+07 | 500293 | 1.02809e+07 | 300 | 99.5884 | 2.60268e+07 | 500293 | 1.3021e+07 | 300 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/3c0d032) | 8.26794 | 3.13498e+08 | 497530 | 1.55975e+08 | 300 | 


### Twitter Test (Prettified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/TwitterData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Twitter%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Twitter%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [glaze](https://github.com/stephenberry/glaze/commit/65d45d5) | 113.765 | 2.27837e+07 | 719148 | 1.63848e+07 | 300 | 264.906 | 9.78444e+06 | 719148 | 7.03646e+06 | 300 | 
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 99.9088 | 2.59433e+07 | 719239 | 1.86594e+07 | 300 | 203.893 | 1.27123e+07 | 719239 | 9.14321e+06 | 300 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/3c0d032) | 7.95805 | 3.25699e+08 | 516839 | 1.68334e+08 | 300 | 

### Twitter Test (Minified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/TwitterData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Twitter%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Twitter%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 142.704 | 1.81633e+07 | 477806 | 8.67852e+06 | 300 | 203.858 | 1.27145e+07 | 477806 | 6.07507e+06 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/65d45d5) | 69.3926 | 3.73522e+07 | 477715 | 1.78437e+07 | 300 | 144.572 | 1.79311e+07 | 477715 | 8.56594e+06 | 300 | 


### Minify Test Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Minify%20Test_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Minify%20Test_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ------------ | ------------------| -------------------- | --------------- | --------------------- |   
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 65.327 | 3.9682e+07 | 69037 | 2.73952e+06 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/65d45d5) | 59.6788 | 4.3432e+07 | 69037 | 2.99841e+06 | 300 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/3c0d032) | 7.02537 | 3.68939e+08 | 69037 | 2.54705e+07 | 300 | 

### Prettify Test Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Prettify%20Test_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Prettify%20Test_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ------------ | ------------------| -------------------- | --------------- | --------------------- |   
| [glaze](https://github.com/stephenberry/glaze/commit/65d45d5) | 249.003 | 1.04094e+07 | 914273 | 9.51699e+06 | 300 | 
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 210.882 | 1.22911e+07 | 914273 | 1.12374e+07 | 300 | 

### Validation Test Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Validate%20Test_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Validate%20Test_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- |   
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 272.505 | 9.51165e+06 | 123050 | 1.17041e+06 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/65d45d5) | 247.992 | 1.04518e+07 | 123050 | 1.2861e+06 | 300 | 