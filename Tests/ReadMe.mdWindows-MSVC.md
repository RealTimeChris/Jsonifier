# Json-Performance
Performance profiling of JSON libraries (Compiled and run on Windows 10.0.22631 using the MSVC 19.41.34123.0 compiler).  

Latest Results: (Nov 26, 2024)
#### Using the following commits:
----
| Jsonifier: [](https://github.com/RealTimeChris/Jsonifier/commit/)  
| Glaze: [8f35df4](https://github.com/stephenberry/glaze/commit/8f35df4)  
| Simdjson: [9b31492](https://github.com/simdjson/simdjson/commit/9b31492)  

 > 100 iterations on a (Intel(R) Core(TM) i7-8700K CPU @ 3.70G).


### Json Test (Prettified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/Windows-MSVC/JsonData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Json%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Json%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read Variation (+/-%) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Write (MB/S) | Write Variation (+/-%) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) |
| ------- | ----------- | --------------------- | -----------------| ------------------- | -------------- | ------------ | ---------------------- | ------------------| -------------------- | --------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 239.906 | 19.0559 | 4.91104e+08 | 2112777 | 1.03759e+09 | 260.306 | 11.57 | 4.1431e+08 | 2112777 | 8.75345e+08 | 
| [glaze](https://github.com/stephenberry/glaze/commit/8f35df4) | 147.004 | 17.2297 | 7.83782e+08 | 2112777 | 1.65596e+09 | 221.676 | 27.29 | 5.9168e+08 | 2112777 | 1.25009e+09 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/9b31492) | 12.2268 | 10.5814 | 8.72283e+09 | 2112777 | 1.84294e+10 | 

### Json Test (Minified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/Windows-MSVC/JsonData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Json%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Json%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read Variation (+/-%) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Write (MB/S) | Write Variation (+/-%) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) |
| ------- | ----------- | --------------------- | -----------------| ------------------- | -------------- | ------------ | ---------------------- | ------------------| -------------------- | --------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 196.822 | 15.5702 | 5.73893e+08 | 1631187 | 9.36127e+08 | 207.203 | 15.78 | 5.46489e+08 | 1631187 | 8.91426e+08 | 
| [glaze](https://github.com/stephenberry/glaze/commit/8f35df4) | 132.357 | 13.9082 | 8.36932e+08 | 1631187 | 1.36519e+09 | 188.17 | 8.384 | 5.53196e+08 | 1631187 | 9.02367e+08 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/9b31492) | 10.1775 | 12.2185 | 1.06747e+10 | 1631187 | 1.74124e+10 | 

### Partial Test (Prettified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/Windows-MSVC/JsonData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Partial%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Partial%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read Variation (+/-%) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Write (MB/S) | Write Variation (+/-%) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) |
| ------- | ----------- | --------------------- | -----------------| ------------------- | -------------- | ------------ | ---------------------- | ------------------| -------------------- | --------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 1213.36 | 29.9333 | 4.20852e+09 | 56315 | 2.37003e+08 | 279.283 | 14.15 | 3.97775e+08 | 56315 | 2.24007e+07 | 
| [glaze](https://github.com/stephenberry/glaze/commit/8f35df4) | 604.785 | 4.84748 | 6.2174e+09 | 56315 | 3.50133e+08 | 195.722 | 32.38 | 7.20588e+08 | 56315 | 4.05799e+07 | 


### Partial Test (Minified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/Windows-MSVC/JsonData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Partial%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Partial%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read Variation (+/-%) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Write (MB/S) | Write Variation (+/-%) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) |
| ------- | ----------- | --------------------- | -----------------| ------------------- | -------------- | ------------ | ---------------------- | ------------------| -------------------- | --------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 1346.62 | 21.9954 | 3.53286e+09 | 41919 | 1.48094e+08 | 211.519 | 19.24 | 5.58312e+08 | 41919 | 2.34039e+07 | 
| [glaze](https://github.com/stephenberry/glaze/commit/8f35df4) | 601.369 | 33.4917 | 9.27847e+09 | 41919 | 3.88944e+08 | 178.151 | 37.64 | 8.58422e+08 | 41919 | 3.59842e+07 | 


### Abc Test (Out of Sequence Performance - Prettified) [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/Windows-MSVC/JsonData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Abc%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Abc%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>

The JSON documents in the previous tests featured keys ranging from "a" to "z," where each key corresponds to an array of values. Notably, the documents in this test arrange these keys in reverse order, deviating from the typical "a" to "z" arrangement.

This test effectively demonstrates the challenges encountered when utilizing simdjson and iterative parsers that lack the ability to efficiently allocate memory locations through hashing. In cases where the keys are not in the expected sequence, performance is significantly compromised, with the severity escalating as the document size increases.

In contrast, hash-based solutions offer a viable alternative by circumventing these issues and maintaining optimal performance regardless of the JSON document's scale, or ordering of the keys being parsed.

| Library | Read (MB/S) | Read Variation (+/-%) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Write (MB/S) | Write Variation (+/-%) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) |
| ------- | ----------- | --------------------- | -----------------| ------------------- | -------------- | ------------ | ---------------------- | ------------------| -------------------- | --------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 186.618 | 17.0048 | 6.15733e+08 | 2112777 | 1.30091e+09 | 250.183 | 23.19 | 4.963e+08 | 2112777 | 1.04857e+09 | 
| [glaze](https://github.com/stephenberry/glaze/commit/8f35df4) | 145.433 | 22.2992 | 8.43941e+08 | 2112777 | 1.78306e+09 | 222.981 | 25.19 | 5.71742e+08 | 2112777 | 1.20796e+09 | 


### ABC Test (Out of Sequence Performance - Minified) [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/Windows-MSVC/JsonData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Abc%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Abc%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read Variation (+/-%) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Write (MB/S) | Write Variation (+/-%) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) |
| ------- | ----------- | --------------------- | -----------------| ------------------- | -------------- | ------------ | ---------------------- | ------------------| -------------------- | --------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 151.393 | 19.8916 | 7.86352e+08 | 1631187 | 1.28269e+09 | 205.939 | 29.72 | 6.58879e+08 | 1631187 | 1.07475e+09 | 
| [glaze](https://github.com/stephenberry/glaze/commit/8f35df4) | 128.23 | 40.6439 | 1.25298e+09 | 1631187 | 2.04385e+09 | 182.255 | 34.41 | 7.9773e+08 | 1631187 | 1.30125e+09 | 


### Discord Test (Prettified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Discord%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Discord%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read Variation (+/-%) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Write (MB/S) | Write Variation (+/-%) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) |
| ------- | ----------- | --------------------- | -----------------| ------------------- | -------------- | ------------ | ---------------------- | ------------------| -------------------- | --------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 221.373 | 41.8224 | 6.56589e+08 | 138774 | 9.11175e+07 | 643.702 | 13.09 | 1.70465e+08 | 138774 | 2.36561e+07 | 
| [glaze](https://github.com/stephenberry/glaze/commit/8f35df4) | 139.139 | 41.4086 | 1.03727e+09 | 138774 | 1.43946e+08 | 288.083 | 40.26 | 5.5411e+08 | 138774 | 7.6896e+07 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/9b31492) | 12.4552 | 19.9842 | 8.50276e+09 | 138482 | 1.17748e+09 | 

### Discord Test (Minified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Discord%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Discord%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read Variation (+/-%) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Write (MB/S) | Write Variation (+/-%) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) |
| ------- | ----------- | --------------------- | -----------------| ------------------- | -------------- | ------------ | ---------------------- | ------------------| -------------------- | --------------- |  
| [glaze](https://github.com/stephenberry/glaze/commit/8f35df4) | 117.716 | 31.9773 | 1.19099e+09 | 69037 | 8.22226e+07 | 185.461 | 39.51 | 8.50134e+08 | 69037 | 5.86907e+07 | 
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 87.8906 | 27.2837 | 1.4922e+09 | 69037 | 1.03017e+08 | 347.068 | 34.72 | 4.20922e+08 | 69037 | 2.90592e+07 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/9b31492) | 7.7291 | 25.1235 | 1.65488e+10 | 68745 | 1.13765e+09 | 

### Canada Test (Prettified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/CanadaData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Canada%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Canada%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read Variation (+/-%) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Write (MB/S) | Write Variation (+/-%) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) |
| ------- | ----------- | --------------------- | -----------------| ------------------- | -------------- | ------------ | ---------------------- | ------------------| -------------------- | --------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 49.2329 | 29.5729 | 9.2938e+08 | 6661897 | 6.19144e+09 | 108.942 | 29.99 | 1.25034e+09 | 6661897 | 8.32967e+09 | 
| [glaze](https://github.com/stephenberry/glaze/commit/8f35df4) | 28.4238 | 24.9967 | 1.51157e+09 | 6661897 | 1.00699e+10 | 94.8388 | 28.91 | 1.41443e+09 | 6661897 | 9.42276e+09 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/9b31492) | 7.24358 | 25.1844 | 5.94625e+09 | 6661897 | 3.96133e+10 | 

### Canada Test (Minified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/CanadaData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Canada%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Canada%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read Variation (+/-%) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Write (MB/S) | Write Variation (+/-%) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) |
| ------- | ----------- | --------------------- | -----------------| ------------------- | -------------- | ------------ | ---------------------- | ------------------| -------------------- | --------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 51.0962 | 28.2428 | 2.60103e+09 | 2090234 | 5.43677e+09 | 36.4709 | 33.81 | 3.95068e+09 | 2090234 | 8.25784e+09 | 
| [glaze](https://github.com/stephenberry/glaze/commit/8f35df4) | 27.314 | 21.9664 | 4.47438e+09 | 2090234 | 9.35249e+09 | 33.9116 | 18.8 | 3.46328e+09 | 2090234 | 7.23907e+09 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/9b31492) | 6.61996 | 26.4235 | 1.95797e+10 | 2090234 | 4.09261e+10 | 

### CitmCatalog Test (Prettified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/CitmCatalogData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/CitmCatalog%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/CitmCatalog%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read Variation (+/-%) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Write (MB/S) | Write Variation (+/-%) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) |
| ------- | ----------- | --------------------- | -----------------| ------------------- | -------------- | ------------ | ---------------------- | ------------------| -------------------- | --------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 377.132 | 22.4017 | 4.02417e+08 | 1439562 | 5.79304e+08 | 671.693 | 24.61 | 1.88339e+08 | 1439562 | 2.71125e+08 | 
| [glaze](https://github.com/stephenberry/glaze/commit/8f35df4) | 212.186 | 25.0768 | 7.40768e+08 | 1439584 | 1.0664e+09 | 338.276 | 23.98 | 3.70841e+08 | 1439584 | 5.33856e+08 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/9b31492) | 20.1492 | 24.8138 | 7.86171e+09 | 1423437 | 1.11906e+10 | 

### CitmCatalog Test (Minified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/CitmCatalogData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/CitmCatalog%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/CitmCatalog%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read Variation (+/-%) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Write (MB/S) | Write Variation (+/-%) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) |
| ------- | ----------- | --------------------- | -----------------| ------------------- | -------------- | ------------ | ---------------------- | ------------------| -------------------- | --------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 169.294 | 35.7209 | 8.7637e+08 | 500299 | 4.38447e+08 | 276.801 | 22.74 | 4.45941e+08 | 500299 | 2.23104e+08 | 
| [glaze](https://github.com/stephenberry/glaze/commit/8f35df4) | 107.883 | 35.1329 | 1.36277e+09 | 500299 | 6.81794e+08 | 154.699 | 22.38 | 7.94262e+08 | 500299 | 3.97369e+08 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/9b31492) | 8.02111 | 20.1208 | 1.51075e+10 | 492910 | 7.44665e+09 | 

### Twitter Test (Prettified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/TwitterData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Twitter%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Twitter%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read Variation (+/-%) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Write (MB/S) | Write Variation (+/-%) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) |
| ------- | ----------- | --------------------- | -----------------| ------------------- | -------------- | ------------ | ---------------------- | ------------------| -------------------- | --------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 204.536 | 29.9102 | 5.98527e+08 | 719107 | 4.30405e+08 | 562.68 | 29.4 | 2.40056e+08 | 719107 | 1.72626e+08 | 
| [glaze](https://github.com/stephenberry/glaze/commit/8f35df4) | 151.995 | 20.3801 | 7.08987e+08 | 719139 | 5.0986e+08 | 317.688 | 21.18 | 3.80876e+08 | 719139 | 2.73903e+08 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/9b31492) | 11.7754 | 16.3422 | 9.49549e+09 | 659630 | 6.26351e+09 | 

### Twitter Test (Minified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/TwitterData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Twitter%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Twitter%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read Variation (+/-%) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Write (MB/S) | Write Variation (+/-%) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) |
| ------- | ----------- | --------------------- | -----------------| ------------------- | -------------- | ------------ | ---------------------- | ------------------| -------------------- | --------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 185.779 | 23.9129 | 6.59461e+08 | 477674 | 3.15008e+08 | 477.661 | 24.48 | 2.6439e+08 | 477674 | 1.26292e+08 | 
| [glaze](https://github.com/stephenberry/glaze/commit/8f35df4) | 149.985 | 24.0267 | 8.18011e+08 | 477706 | 3.90769e+08 | 279.666 | 18.81 | 4.19983e+08 | 477706 | 2.00628e+08 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/9b31492) | 9.03172 | 16.5256 | 1.38759e+10 | 425643 | 5.90617e+09 | 

### Minify Test Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Minify%20Test_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Minify%20Test_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Write (MB/S) | Write Variation (+/-%) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) |
| ------- | ------------ | ---------------------- | ----------------- | -------------------- | --------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 133.601 | 29.81 | 1.01703e+09 | 69037 | 7.02127e+07 | 
| [glaze](https://github.com/stephenberry/glaze/commit/8f35df4) | 69.3551 | 21.04 | 1.74144e+09 | 69037 | 1.20224e+08 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/9b31492) | 14.5877 | 24.18 | 1.53677e+10 | 69037 | 1.06094e+09 | 

### Prettify Test Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Prettify%20Test_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Prettify%20Test_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Write (MB/S) | Write Variation (+/-%) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) |
| ------- | ------------ | ---------------------- | ----------------- | -------------------- | --------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 435.758 | 21.32 | 2.78168e+08 | 2112777 | 5.87707e+08 | 
| [glaze](https://github.com/stephenberry/glaze/commit/8f35df4) | 376.772 | 20.73 | 3.19296e+08 | 2112777 | 6.74602e+08 | 

### Validation Test Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Validate%20Test_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Validate%20Test_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read Variation (+/-%) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) |
| ------- | ----------- | --------------------- | ---------------- | ------------------- | -------------- |   
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 339.259 | 23.2493 | 3.66258e+08 | 123050 | 4.5068e+07 | 
| [glaze](https://github.com/stephenberry/glaze/commit/8f35df4) | 284.898 | 22.3343 | 4.31004e+08 | 123050 | 5.3035e+07 | 