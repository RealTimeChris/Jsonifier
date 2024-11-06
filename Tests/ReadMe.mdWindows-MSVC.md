# Json-Performance
Performance profiling of JSON libraries (Compiled and run on Windows 10.0.22631 using the MSVC 19.41.34123.0 compiler).  

Latest Results: (Nov 06, 2024)
#### Using the following commits:
----
| Jsonifier: [](https://github.com/RealTimeChris/Jsonifier/commit/)  
| Glaze: [665a0c7](https://github.com/stephenberry/glaze/commit/665a0c7)  
| Simdjson: [587beae](https://github.com/simdjson/simdjson/commit/587beae)  

 > At least 30 iterations on a (Intel(R) Core(TM) i7-8700K CPU @ 3.70G), until coefficient of variance is at or below 1%.


### Json Test (Prettified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/Windows-MSVC/JsonData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Json%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Json%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 196.417 | 1.8817e+07 | 1587898 | 2.98794e+07 | 173 | 215.234 | 1.71719e+07 | 1587898 | 2.72672e+07 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/665a0c7) | 122.479 | 3.01764e+07 | 1587898 | 4.79171e+07 | 300 | 193.963 | 1.90551e+07 | 1587898 | 3.02575e+07 | 98 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/587beae) | 11.841 | 3.12134e+08 | 1587898 | 4.95637e+08 | 300 | 

### Json Test (Minified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/Windows-MSVC/JsonData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Json%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Json%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 148.835 | 2.48326e+07 | 1138195 | 2.82643e+07 | 300 | 169.153 | 2.18499e+07 | 1138195 | 2.48695e+07 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/665a0c7) | 99.1073 | 3.72927e+07 | 1138195 | 4.24463e+07 | 300 | 147.219 | 2.51052e+07 | 1138195 | 2.85747e+07 | 300 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/587beae) | 8.90641 | 4.14978e+08 | 1138195 | 4.72326e+08 | 300 | 

### ABC Test (Out of Sequence Performance - Prettified) [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/Windows-MSVC/JsonData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Abc%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Abc%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>

The JSON documents in the previous tests featured keys ranging from "a" to "z," where each key corresponds to an array of values. Notably, the documents in this test arrange these keys in reverse order, deviating from the typical "a" to "z" arrangement.

This test effectively demonstrates the challenges encountered when utilizing simdjson and iterative parsers that lack the ability to efficiently allocate memory locations through hashing. In cases where the keys are not in the expected sequence, performance is significantly compromised, with the severity escalating as the document size increases.

In contrast, hash-based solutions offer a viable alternative by circumventing these issues and maintaining optimal performance regardless of the JSON document's scale, or ordering of the keys being parsed.

| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 153.638 | 2.40562e+07 | 1587898 | 3.81988e+07 | 300 | 221.125 | 1.67144e+07 | 1587898 | 2.65407e+07 | 292 | 
| [glaze](https://github.com/stephenberry/glaze/commit/665a0c7) | 125.471 | 2.94569e+07 | 1587898 | 4.67746e+07 | 300 | 191.437 | 1.93063e+07 | 1587898 | 3.06565e+07 | 300 | 


### ABC Test (Out of Sequence Performance - Minified) [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/Windows-MSVC/JsonData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Abc%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Abc%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 145.791 | 2.53511e+07 | 1138195 | 2.88545e+07 | 297 | 167.19 | 2.21063e+07 | 1138195 | 2.51613e+07 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/665a0c7) | 105.122 | 3.51587e+07 | 1138195 | 4.00174e+07 | 300 | 146.68 | 2.51974e+07 | 1138195 | 2.86796e+07 | 300 | 


### Discord Test (Prettified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Discord%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Discord%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 273.979 | 1.349e+07 | 138774 | 1.87206e+06 | 300 | 378.956 | 9.75336e+06 | 138774 | 1.35351e+06 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/665a0c7) | 161.956 | 2.28209e+07 | 138774 | 3.16695e+06 | 300 | 298.215 | 1.23936e+07 | 138774 | 1.71991e+06 | 296 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/587beae) | 15.1338 | 2.44221e+08 | 138482 | 3.38202e+07 | 297 | 

### Discord Test (Minified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Discord%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Discord%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 158.556 | 2.33102e+07 | 69037 | 1.60926e+06 | 300 | 477.351 | 7.74266e+06 | 69037 | 534530 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/665a0c7) | 113.756 | 3.24904e+07 | 69037 | 2.24304e+06 | 300 | 187.831 | 1.96771e+07 | 69037 | 1.35845e+06 | 300 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/587beae) | 8.48752 | 4.3546e+08 | 68745 | 2.99357e+07 | 300 | 

### Canada Test (Prettified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/CanadaData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Canada%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Canada%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 136.332 | 2.71099e+07 | 6661897 | 1.80603e+08 | 300 | 120.116 | 3.077e+07 | 6661897 | 2.04987e+08 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/665a0c7) | 88.1762 | 4.19151e+07 | 6661897 | 2.79234e+08 | 300 | 106.36 | 3.47497e+07 | 6661897 | 2.31499e+08 | 300 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/587beae) | 19.7811 | 1.86842e+08 | 6661897 | 1.24472e+09 | 300 | 

### Canada Test (Minified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/CanadaData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Canada%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Canada%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 44.1893 | 8.36386e+07 | 2090234 | 1.74824e+08 | 300 | 39.8603 | 9.27236e+07 | 2090234 | 1.93814e+08 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/665a0c7) | 28.9462 | 1.27684e+08 | 2090234 | 2.6689e+08 | 300 | 34.6648 | 1.06619e+08 | 2090234 | 2.22858e+08 | 300 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/587beae) | 6.09585 | 6.0631e+08 | 2090234 | 1.26733e+09 | 300 | 

### CitmCatalog Test (Prettified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/CitmCatalogData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/CitmCatalog%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/CitmCatalog%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 364.323 | 1.01448e+07 | 1439562 | 1.46041e+07 | 300 | 695.247 | 5.31606e+06 | 1439562 | 7.6528e+06 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/665a0c7) | 204.751 | 1.8051e+07 | 1439584 | 2.5986e+07 | 300 | 352.51 | 1.04848e+07 | 1439584 | 1.50937e+07 | 300 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/587beae) | 21.3429 | 1.73171e+08 | 1428054 | 2.47298e+08 | 300 | 

### CitmCatalog Test (Minified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/CitmCatalogData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/CitmCatalog%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/CitmCatalog%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 175.88 | 2.10142e+07 | 500299 | 1.05134e+07 | 300 | 330.812 | 1.11725e+07 | 500299 | 5.58958e+06 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/665a0c7) | 109.979 | 3.36061e+07 | 500299 | 1.68131e+07 | 294 | 169.1 | 2.18567e+07 | 500299 | 1.09349e+07 | 300 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/587beae) | 9.04753 | 4.08506e+08 | 497527 | 2.03243e+08 | 300 | 

### Twitter Test (Prettified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/TwitterData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Twitter%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Twitter%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 233.31 | 1.58414e+07 | 719107 | 1.13917e+07 | 300 | 563.949 | 6.55374e+06 | 719107 | 4.71284e+06 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/665a0c7) | 174.463 | 2.11848e+07 | 719139 | 1.52348e+07 | 300 | 352.194 | 1.04942e+07 | 719139 | 7.54676e+06 | 298 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/587beae) | 10.3447 | 3.57283e+08 | 516830 | 1.84654e+08 | 300 | 

### Twitter Test (Minified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/TwitterData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Twitter%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Twitter%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 200.367 | 1.8446e+07 | 477683 | 8.81134e+06 | 300 | 510.051 | 7.24629e+06 | 477683 | 3.46143e+06 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/665a0c7) | 154.91 | 2.38589e+07 | 477715 | 1.13977e+07 | 300 | 266.066 | 1.38912e+07 | 477715 | 6.63602e+06 | 300 | 


### Minify Test Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Minify%20Test_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Minify%20Test_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ------------ | ------------------| -------------------- | --------------- | --------------------- |   
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 130.972 | 2.82196e+07 | 69037 | 1.9482e+06 | 198 | 
| [glaze](https://github.com/stephenberry/glaze/commit/665a0c7) | 72.4623 | 5.10054e+07 | 69037 | 3.52126e+06 | 257 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/587beae) | 8.24685 | 4.48169e+08 | 69037 | 3.09402e+07 | 300 | 

### Prettify Test Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Prettify%20Test_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Prettify%20Test_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ------------ | ------------------| -------------------- | --------------- | --------------------- |   
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 445.653 | 8.29331e+06 | 1587898 | 1.31689e+07 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/665a0c7) | 333.326 | 1.10878e+07 | 1587898 | 1.76064e+07 | 300 | 

### Validation Test Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Validate%20Test_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Validate%20Test_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- |   
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 349.167 | 1.05851e+07 | 123050 | 1.3025e+06 | 297 | 
| [glaze](https://github.com/stephenberry/glaze/commit/665a0c7) | 302.327 | 1.22251e+07 | 123050 | 1.5043e+06 | 300 | 