# Json-Performance
Performance profiling of JSON libraries (Compiled and run on Windows 10.0.22631 using the MSVC 19.41.34123.0 compiler).  

Latest Results: (Oct 17, 2024)
#### Using the following commits:
----
| Jsonifier: [](https://github.com/RealTimeChris/Jsonifier/commit/)  
| Glaze: [2f372e7](https://github.com/stephenberry/glaze/commit/2f372e7)  
| Simdjson: [36f5dbc](https://github.com/simdjson/simdjson/commit/36f5dbc)  

 > At least 30 iterations on a (Intel(R) Core(TM) i7-9750H CPU @ 2.60G), until coefficient of variance is at or below 1%.


### Json Test (Prettified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/Windows-MSVC/JsonData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Json%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Json%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 189.633 | 1.36683e+07 | 1800511 | 2.461e+07 | 300 | 185.276 | 1.39897e+07 | 1800511 | 2.51886e+07 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/2f372e7) | 113.794 | 2.27777e+07 | 1800440 | 4.10099e+07 | 300 | 199.3 | 1.30053e+07 | 1800440 | 2.34152e+07 | 300 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/36f5dbc) | 12.6768 | 2.04465e+08 | 1800440 | 3.68127e+08 | 300 | 

### Json Test (Minified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/Windows-MSVC/JsonData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Json%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Json%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 189.188 | 1.37005e+07 | 1490326 | 2.04182e+07 | 300 | 176.292 | 1.47026e+07 | 1490326 | 2.19117e+07 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/2f372e7) | 147.539 | 1.7568e+07 | 1490255 | 2.61808e+07 | 300 | 194.605 | 1.3319e+07 | 1490255 | 1.98488e+07 | 300 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/36f5dbc) | 8.64975 | 2.99659e+08 | 1490255 | 4.46568e+08 | 300 | 

### ABC Test (Out of Sequence Performance - Prettified) [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/Windows-MSVC/JsonData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Abc%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Abc%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>

The JSON documents in the previous tests featured keys ranging from "a" to "z," where each key corresponds to an array of values. Notably, the documents in this test arrange these keys in reverse order, deviating from the typical "a" to "z" arrangement.

This test effectively demonstrates the challenges encountered when utilizing simdjson and iterative parsers that lack the ability to efficiently allocate memory locations through hashing. In cases where the keys are not in the expected sequence, performance is significantly compromised, with the severity escalating as the document size increases.

In contrast, hash-based solutions offer a viable alternative by circumventing these issues and maintaining optimal performance regardless of the JSON document's scale, or ordering of the keys being parsed.

| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 178.298 | 1.45371e+07 | 1800511 | 2.61743e+07 | 300 | 111.008 | 2.33494e+07 | 1800511 | 4.20409e+07 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/2f372e7) | 122.129 | 2.12232e+07 | 1800440 | 3.82111e+07 | 300 | 196.262 | 1.32066e+07 | 1800440 | 2.37778e+07 | 300 | 


### ABC Test (Out of Sequence Performance - Minified) [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/Windows-MSVC/JsonData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Abc%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Abc%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 233.182 | 1.11157e+07 | 1490326 | 1.6566e+07 | 300 | 231.224 | 1.12098e+07 | 1490326 | 1.67063e+07 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/2f372e7) | 142.301 | 1.82145e+07 | 1490255 | 2.71442e+07 | 300 | 230.241 | 1.12577e+07 | 1490255 | 1.67768e+07 | 300 | 


### Discord Test (Prettified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Discord%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Discord%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 169.063 | 1.53314e+07 | 138774 | 2.1276e+06 | 300 | 156.91 | 1.65188e+07 | 138774 | 2.29238e+06 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/2f372e7) | 138.262 | 1.87468e+07 | 138774 | 2.60158e+06 | 300 | 230.517 | 1.12441e+07 | 138774 | 1.56039e+06 | 300 | 


### Discord Test (Minified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Discord%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Discord%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [glaze](https://github.com/stephenberry/glaze/commit/2f372e7) | 108.71 | 2.38429e+07 | 69037 | 1.64604e+06 | 300 | 157.266 | 1.64815e+07 | 69037 | 1.13783e+06 | 300 | 
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 98.8828 | 2.62126e+07 | 69037 | 1.80964e+06 | 300 | 132.155 | 1.96131e+07 | 69037 | 1.35403e+06 | 300 | 


### Canada Test (Prettified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/CanadaData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Canada%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Canada%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 112.866 | 2.29647e+07 | 6661897 | 1.52989e+08 | 300 | 79.3783 | 3.26533e+07 | 6661897 | 2.17533e+08 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/2f372e7) | 71.282 | 3.63621e+07 | 6661897 | 2.42241e+08 | 300 | 69.4873 | 3.73014e+07 | 6661897 | 2.48498e+08 | 300 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/36f5dbc) | 16.7944 | 1.54333e+08 | 6661897 | 1.02815e+09 | 300 | 

### Canada Test (Minified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/CanadaData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Canada%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Canada%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 32.3791 | 8.00508e+07 | 2090234 | 1.67325e+08 | 300 | 30.8266 | 8.40822e+07 | 2090234 | 1.75751e+08 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/2f372e7) | 21.6707 | 1.19605e+08 | 2090234 | 2.50002e+08 | 300 | 25.8365 | 1.00322e+08 | 2090234 | 2.09696e+08 | 300 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/36f5dbc) | 5.16221 | 5.02106e+08 | 2090234 | 1.04952e+09 | 300 | 

### CitmCatalog Test (Prettified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/CitmCatalogData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/CitmCatalog%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/CitmCatalog%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [glaze](https://github.com/stephenberry/glaze/commit/2f372e7) | 187.504 | 1.38236e+07 | 1439584 | 1.99002e+07 | 300 | 292.581 | 8.85899e+06 | 1439584 | 1.27533e+07 | 300 | 
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 123.784 | 2.09394e+07 | 1439556 | 3.01435e+07 | 300 | 239.294 | 1.08317e+07 | 1439556 | 1.55928e+07 | 300 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/36f5dbc) | 17.4595 | 1.48457e+08 | 1428054 | 2.12004e+08 | 300 | 

### CitmCatalog Test (Minified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/CitmCatalogData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/CitmCatalog%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/CitmCatalog%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 128.81 | 2.01225e+07 | 500293 | 1.00671e+07 | 300 | 69.415 | 3.73401e+07 | 500293 | 1.8681e+07 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/2f372e7) | 93.94 | 2.75917e+07 | 500299 | 1.38041e+07 | 300 | 142.345 | 1.8209e+07 | 500299 | 9.10993e+06 | 300 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/36f5dbc) | 7.95011 | 3.26031e+08 | 497527 | 1.62209e+08 | 300 | 

### Twitter Test (Prettified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/TwitterData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Twitter%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Twitter%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [glaze](https://github.com/stephenberry/glaze/commit/2f372e7) | 125.515 | 2.06507e+07 | 719148 | 1.48509e+07 | 300 | 256.646 | 1.00994e+07 | 719148 | 7.26294e+06 | 300 | 
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 68.7529 | 3.76998e+07 | 719239 | 2.71152e+07 | 300 | 230.568 | 1.12417e+07 | 719239 | 8.08548e+06 | 300 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/36f5dbc) | 8.51315 | 3.04467e+08 | 516839 | 1.5736e+08 | 300 | 

### Twitter Test (Minified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/TwitterData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Twitter%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Twitter%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [glaze](https://github.com/stephenberry/glaze/commit/2f372e7) | 134.087 | 1.93305e+07 | 477715 | 9.23449e+06 | 300 | 219.411 | 1.18133e+07 | 477715 | 5.64339e+06 | 300 | 
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 43.9332 | 5.8998e+07 | 477806 | 2.81896e+07 | 300 | 200.551 | 1.29241e+07 | 477806 | 6.1752e+06 | 300 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/36f5dbc) | 4.88356 | 5.30748e+08 | 282852 | 1.50123e+08 | 300 | 

### Minify Test Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Minify%20Test_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Minify%20Test_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ------------ | ------------------| -------------------- | --------------- | --------------------- |   
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 65.5045 | 3.95692e+07 | 69037 | 2.73174e+06 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/2f372e7) | 32.843 | 7.89204e+07 | 69037 | 5.44842e+06 | 300 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/36f5dbc) | 7.22076 | 3.58961e+08 | 69037 | 2.47816e+07 | 300 | 

### Prettify Test Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Prettify%20Test_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Prettify%20Test_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ------------ | ------------------| -------------------- | --------------- | --------------------- |   
| [glaze](https://github.com/stephenberry/glaze/commit/2f372e7) | 377.334 | 6.86916e+06 | 1800511 | 1.2368e+07 | 300 | 
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 168.251 | 1.54057e+07 | 1800511 | 2.77382e+07 | 300 | 

### Validation Test Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Validate%20Test_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Validate%20Test_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- |   
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 286.272 | 9.05423e+06 | 123050 | 1.11412e+06 | 96 | 
| [glaze](https://github.com/stephenberry/glaze/commit/2f372e7) | 133.936 | 1.93522e+07 | 123050 | 2.38129e+06 | 300 | 