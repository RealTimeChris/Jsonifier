# Json-Performance
Performance profiling of JSON libraries (Compiled and run on Windows 10.0.22631 using the MSVC 19.41.34123.0 compiler).  

Latest Results: (Nov 04, 2024)
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
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 175.69 | 2.10369e+07 | 1668206 | 3.50939e+07 | 300 | 197.98 | 1.86684e+07 | 1668206 | 3.11427e+07 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/890bf43) | 105.872 | 3.49096e+07 | 1668206 | 5.82365e+07 | 300 | 181.983 | 2.03094e+07 | 1668206 | 3.38803e+07 | 300 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/587beae) | 10.1868 | 3.6282e+08 | 1668206 | 6.05259e+08 | 300 | 

### Json Test (Minified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/Windows-MSVC/JsonData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Json%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Json%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 128.45 | 2.87736e+07 | 1189438 | 3.42244e+07 | 300 | 150.072 | 2.4628e+07 | 1189438 | 2.92935e+07 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/890bf43) | 105.355 | 3.5081e+07 | 1189438 | 4.17266e+07 | 300 | 136.533 | 2.707e+07 | 1189438 | 3.21981e+07 | 300 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/587beae) | 8.70783 | 4.24442e+08 | 1189438 | 5.04848e+08 | 300 | 

### ABC Test (Out of Sequence Performance - Prettified) [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/Windows-MSVC/JsonData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Abc%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Abc%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>

The JSON documents in the previous tests featured keys ranging from "a" to "z," where each key corresponds to an array of values. Notably, the documents in this test arrange these keys in reverse order, deviating from the typical "a" to "z" arrangement.

This test effectively demonstrates the challenges encountered when utilizing simdjson and iterative parsers that lack the ability to efficiently allocate memory locations through hashing. In cases where the keys are not in the expected sequence, performance is significantly compromised, with the severity escalating as the document size increases.

In contrast, hash-based solutions offer a viable alternative by circumventing these issues and maintaining optimal performance regardless of the JSON document's scale, or ordering of the keys being parsed.

| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 176.568 | 2.09321e+07 | 1668206 | 3.49191e+07 | 300 | 192.845 | 1.91654e+07 | 1668206 | 3.19718e+07 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/890bf43) | 120.3 | 3.0723e+07 | 1668206 | 5.12523e+07 | 300 | 185.56 | 1.99178e+07 | 1668206 | 3.3227e+07 | 300 | 


### ABC Test (Out of Sequence Performance - Minified) [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/Windows-MSVC/JsonData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Abc%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Abc%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 133.904 | 2.76017e+07 | 1189438 | 3.28305e+07 | 300 | 152.672 | 2.42085e+07 | 1189438 | 2.87945e+07 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/890bf43) | 104.862 | 3.5246e+07 | 1189438 | 4.19229e+07 | 300 | 140.736 | 2.62618e+07 | 1189438 | 3.12368e+07 | 300 | 


### Discord Test (Prettified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Discord%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Discord%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 254.792 | 1.45059e+07 | 138774 | 2.01304e+06 | 97 | 661.584 | 5.58654e+06 | 138774 | 775266 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/890bf43) | 161.199 | 2.29281e+07 | 138774 | 3.18182e+06 | 99 | 298.589 | 1.23781e+07 | 138774 | 1.71776e+06 | 162 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/587beae) | 15.2585 | 2.42224e+08 | 138482 | 3.35437e+07 | 300 | 

### Discord Test (Minified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Discord%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Discord%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 160.813 | 2.2983e+07 | 69037 | 1.58668e+06 | 300 | 475.73 | 7.76904e+06 | 69037 | 536351 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/890bf43) | 114.365 | 3.23174e+07 | 69037 | 2.2311e+06 | 275 | 189.754 | 1.94776e+07 | 69037 | 1.34468e+06 | 300 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/587beae) | 8.50183 | 4.34726e+08 | 68745 | 2.98852e+07 | 300 | 

### Canada Test (Prettified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/CanadaData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Canada%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Canada%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 136.377 | 2.71011e+07 | 6661897 | 1.80545e+08 | 300 | 119.552 | 3.0915e+07 | 6661897 | 2.05953e+08 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/890bf43) | 80.7897 | 4.5748e+07 | 6661897 | 3.04768e+08 | 300 | 105.837 | 3.4921e+07 | 6661897 | 2.3264e+08 | 300 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/587beae) | 19.4514 | 1.90011e+08 | 6661897 | 1.26583e+09 | 300 | 

### Canada Test (Minified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/CanadaData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Canada%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Canada%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 44.19 | 8.36384e+07 | 2090234 | 1.74824e+08 | 300 | 39.1641 | 9.43716e+07 | 2090234 | 1.97259e+08 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/890bf43) | 28.84 | 1.28155e+08 | 2090234 | 2.67873e+08 | 300 | 35.7553 | 1.03369e+08 | 2090234 | 2.16065e+08 | 300 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/587beae) | 6.71418 | 5.50472e+08 | 2090234 | 1.15062e+09 | 300 | 

### CitmCatalog Test (Prettified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/CitmCatalogData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/CitmCatalog%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/CitmCatalog%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 329.602 | 1.12134e+07 | 1439562 | 1.61423e+07 | 300 | 677.947 | 5.45172e+06 | 1439562 | 7.84808e+06 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/890bf43) | 209.764 | 1.76197e+07 | 1439584 | 2.5365e+07 | 300 | 359.762 | 1.02734e+07 | 1439584 | 1.47894e+07 | 300 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/587beae) | 21.2801 | 1.73682e+08 | 1428054 | 2.48027e+08 | 300 | 

### CitmCatalog Test (Minified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/CitmCatalogData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/CitmCatalog%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/CitmCatalog%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 182.131 | 2.02929e+07 | 500299 | 1.01525e+07 | 300 | 320.539 | 1.15305e+07 | 500299 | 5.76872e+06 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/890bf43) | 110.215 | 3.35343e+07 | 500299 | 1.67772e+07 | 300 | 162.889 | 2.26902e+07 | 500299 | 1.13519e+07 | 300 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/587beae) | 9.14092 | 4.04333e+08 | 497527 | 2.01166e+08 | 293 | 

### Twitter Test (Prettified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/TwitterData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Twitter%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Twitter%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 246.191 | 1.5018e+07 | 719107 | 1.07995e+07 | 300 | 553.525 | 6.67714e+06 | 719107 | 4.80158e+06 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/890bf43) | 171.176 | 2.15916e+07 | 719139 | 1.55273e+07 | 300 | 343.181 | 1.07697e+07 | 719139 | 7.74494e+06 | 300 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/587beae) | 10.2699 | 3.59885e+08 | 516830 | 1.85999e+08 | 300 | 

### Twitter Test (Minified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/TwitterData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Twitter%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Twitter%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 192.016 | 1.92482e+07 | 477683 | 9.19455e+06 | 300 | 452.754 | 8.16333e+06 | 477683 | 3.89949e+06 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/890bf43) | 153.374 | 2.40978e+07 | 477715 | 1.15119e+07 | 300 | 300.323 | 1.23067e+07 | 477715 | 5.8791e+06 | 300 | 


### Minify Test Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Minify%20Test_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Minify%20Test_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ------------ | ------------------| -------------------- | --------------- | --------------------- |   
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 131.324 | 2.81436e+07 | 69037 | 1.94295e+06 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/890bf43) | 73.0783 | 5.05756e+07 | 69037 | 3.49159e+06 | 293 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/587beae) | 8.56896 | 4.3132e+08 | 69037 | 2.9777e+07 | 300 | 

### Prettify Test Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Prettify%20Test_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Prettify%20Test_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ------------ | ------------------| -------------------- | --------------- | --------------------- |   
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 440.249 | 8.39519e+06 | 1668206 | 1.40049e+07 | 297 | 
| [glaze](https://github.com/stephenberry/glaze/commit/890bf43) | 336.653 | 1.09785e+07 | 1668206 | 1.83144e+07 | 300 | 

### Validation Test Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Validate%20Test_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Validate%20Test_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- |   
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 353.772 | 1.04474e+07 | 123050 | 1.28555e+06 | 178 | 
| [glaze](https://github.com/stephenberry/glaze/commit/890bf43) | 296.2 | 1.2478e+07 | 123050 | 1.53542e+06 | 246 | 