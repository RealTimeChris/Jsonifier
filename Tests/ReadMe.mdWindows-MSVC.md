# Json-Performance
Performance profiling of JSON libraries (Compiled and run on Windows 10.0.22631 using the MSVC 19.41.34123.0 compiler).  

Latest Results: (Nov 01, 2024)
#### Using the following commits:
----
| Jsonifier: [](https://github.com/RealTimeChris/Jsonifier/commit/)  
| Glaze: [06533c7](https://github.com/stephenberry/glaze/commit/06533c7)  
| Simdjson: [6cd8a6d](https://github.com/simdjson/simdjson/commit/6cd8a6d)  

 > At least 30 iterations on a (Intel(R) Core(TM) i7-8700K CPU @ 3.70G), until coefficient of variance is at or below 1%.


### Json Test (Prettified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/Windows-MSVC/JsonData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Json%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Json%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 176.41 | 2.0951e+07 | 828877 | 1.73658e+07 | 299 | 201.001 | 1.83878e+07 | 828877 | 1.52412e+07 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/06533c7) | 117.469 | 3.14634e+07 | 828877 | 2.60793e+07 | 246 | 172.544 | 2.14205e+07 | 828877 | 1.7755e+07 | 299 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/6cd8a6d) | 11.0597 | 3.34184e+08 | 828877 | 2.76997e+08 | 296 | 

### Json Test (Minified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/Windows-MSVC/JsonData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Json%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Json%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 123.342 | 2.99651e+07 | 547928 | 1.64187e+07 | 300 | 133.465 | 2.7692e+07 | 547928 | 1.51732e+07 | 299 | 
| [glaze](https://github.com/stephenberry/glaze/commit/06533c7) | 83.8555 | 4.40755e+07 | 547928 | 2.41502e+07 | 300 | 121.343 | 3.04588e+07 | 547928 | 1.66892e+07 | 300 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/6cd8a6d) | 7.87225 | 4.69492e+08 | 547928 | 2.57248e+08 | 300 | 

### ABC Test (Out of Sequence Performance - Prettified) [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/Windows-MSVC/JsonData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Abc%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Abc%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>

The JSON documents in the previous tests featured keys ranging from "a" to "z," where each key corresponds to an array of values. Notably, the documents in this test arrange these keys in reverse order, deviating from the typical "a" to "z" arrangement.

This test effectively demonstrates the challenges encountered when utilizing simdjson and iterative parsers that lack the ability to efficiently allocate memory locations through hashing. In cases where the keys are not in the expected sequence, performance is significantly compromised, with the severity escalating as the document size increases.

In contrast, hash-based solutions offer a viable alternative by circumventing these issues and maintaining optimal performance regardless of the JSON document's scale, or ordering of the keys being parsed.

| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 174.096 | 2.12295e+07 | 828877 | 1.75967e+07 | 300 | 201.149 | 1.83742e+07 | 828877 | 1.52299e+07 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/06533c7) | 113.179 | 3.26693e+07 | 828877 | 2.70788e+07 | 300 | 174.26 | 2.12096e+07 | 828877 | 1.75801e+07 | 300 | 


### ABC Test (Out of Sequence Performance - Minified) [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/Windows-MSVC/JsonData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Abc%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Abc%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 117.139 | 3.1552e+07 | 547928 | 1.72882e+07 | 300 | 130.1 | 2.84086e+07 | 547928 | 1.55658e+07 | 270 | 
| [glaze](https://github.com/stephenberry/glaze/commit/06533c7) | 88.6714 | 4.16815e+07 | 547928 | 2.28385e+07 | 300 | 120.498 | 3.06725e+07 | 547928 | 1.68063e+07 | 300 | 


### Discord Test (Prettified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Discord%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Discord%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 267.26 | 1.38291e+07 | 138774 | 1.91913e+06 | 300 | 518.568 | 7.12729e+06 | 138774 | 989082 | 161 | 
| [glaze](https://github.com/stephenberry/glaze/commit/06533c7) | 162.36 | 2.27639e+07 | 138774 | 3.15904e+06 | 261 | 302.909 | 1.22016e+07 | 138774 | 1.69326e+06 | 97 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/6cd8a6d) | 15.2112 | 2.42978e+08 | 138482 | 3.3648e+07 | 300 | 

### Discord Test (Minified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Discord%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Discord%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 171.696 | 2.15262e+07 | 69037 | 1.4861e+06 | 300 | 350.836 | 1.05348e+07 | 69037 | 727291 | 212 | 
| [glaze](https://github.com/stephenberry/glaze/commit/06533c7) | 119.098 | 3.1033e+07 | 69037 | 2.14242e+06 | 295 | 192.135 | 1.92362e+07 | 69037 | 1.32801e+06 | 300 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/6cd8a6d) | 8.43834 | 4.37998e+08 | 68745 | 3.01102e+07 | 299 | 

### Canada Test (Prettified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/CanadaData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Canada%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Canada%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 123.054 | 3.00351e+07 | 6661897 | 2.00091e+08 | 300 | 120.089 | 3.07771e+07 | 6661897 | 2.05034e+08 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/06533c7) | 88.2967 | 4.18584e+07 | 6661897 | 2.78857e+08 | 300 | 108.677 | 3.40085e+07 | 6661897 | 2.26561e+08 | 300 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/6cd8a6d) | 20.2631 | 1.82399e+08 | 6661897 | 1.21512e+09 | 298 | 

### Canada Test (Minified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/CanadaData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Canada%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Canada%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 43.49 | 8.49834e+07 | 2090234 | 1.77635e+08 | 279 | 39.2025 | 9.42791e+07 | 2090234 | 1.97065e+08 | 187 | 
| [glaze](https://github.com/stephenberry/glaze/commit/06533c7) | 29.7918 | 1.2406e+08 | 2090234 | 2.59314e+08 | 243 | 36.7723 | 1.0051e+08 | 2090234 | 2.10089e+08 | 300 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/6cd8a6d) | 6.50071 | 5.68546e+08 | 2090234 | 1.1884e+09 | 300 | 

### CitmCatalog Test (Prettified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/CitmCatalogData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/CitmCatalog%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/CitmCatalog%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 358.357 | 1.03137e+07 | 1439562 | 1.48472e+07 | 300 | 680.135 | 5.4342e+06 | 1439562 | 7.82287e+06 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/06533c7) | 214.881 | 1.72001e+07 | 1439584 | 2.4761e+07 | 192 | 373.866 | 9.88581e+06 | 1439584 | 1.42315e+07 | 299 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/6cd8a6d) | 21.6314 | 1.70861e+08 | 1428054 | 2.43999e+08 | 169 | 

### CitmCatalog Test (Minified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/CitmCatalogData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/CitmCatalog%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/CitmCatalog%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 175.608 | 2.10467e+07 | 500299 | 1.05296e+07 | 300 | 285.224 | 1.29582e+07 | 500299 | 6.48295e+06 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/06533c7) | 109.096 | 3.3878e+07 | 500299 | 1.69491e+07 | 300 | 165.491 | 2.23334e+07 | 500299 | 1.11734e+07 | 300 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/6cd8a6d) | 9.27653 | 3.98422e+08 | 497527 | 1.98226e+08 | 298 | 

### Twitter Test (Prettified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/TwitterData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Twitter%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Twitter%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 250.016 | 1.47829e+07 | 719107 | 1.06305e+07 | 300 | 489.185 | 7.55533e+06 | 719107 | 5.43309e+06 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/06533c7) | 141.467 | 2.6126e+07 | 719139 | 1.87882e+07 | 300 | 321.189 | 1.15072e+07 | 719139 | 8.27525e+06 | 300 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/6cd8a6d) | 8.9146 | 4.14598e+08 | 516830 | 2.14277e+08 | 300 | 

### Twitter Test (Minified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/TwitterData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Twitter%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Twitter%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 203.214 | 1.81875e+07 | 477683 | 8.68788e+06 | 300 | 483.565 | 7.64315e+06 | 477683 | 3.651e+06 | 294 | 
| [glaze](https://github.com/stephenberry/glaze/commit/06533c7) | 158.967 | 2.32499e+07 | 477715 | 1.11068e+07 | 300 | 294.567 | 1.25471e+07 | 477715 | 5.99396e+06 | 300 | 


### Minify Test Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Minify%20Test_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Minify%20Test_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ------------ | ------------------| -------------------- | --------------- | --------------------- |   
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 131.481 | 2.81103e+07 | 69037 | 1.94065e+06 | 97 | 
| [glaze](https://github.com/stephenberry/glaze/commit/06533c7) | 73.0238 | 5.06134e+07 | 69037 | 3.4942e+06 | 300 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/6cd8a6d) | 8.55288 | 4.32129e+08 | 69037 | 2.98329e+07 | 296 | 

### Prettify Test Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Prettify%20Test_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Prettify%20Test_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ------------ | ------------------| -------------------- | --------------- | --------------------- |   
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 444.547 | 8.31394e+06 | 828877 | 6.89124e+06 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/06533c7) | 302.043 | 1.22366e+07 | 828877 | 1.01427e+07 | 296 | 

### Validation Test Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Validate%20Test_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Validate%20Test_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- |   
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 350.602 | 1.05418e+07 | 123050 | 1.29716e+06 | 98 | 
| [glaze](https://github.com/stephenberry/glaze/commit/06533c7) | 303.501 | 1.21778e+07 | 123050 | 1.49848e+06 | 185 | 