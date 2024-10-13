# Json-Performance
Performance profiling of JSON libraries (Compiled and run on Windows 10.0.22631 using the MSVC 19.41.34120.0 compiler).  

Latest Results: (Oct 13, 2024)
#### Using the following commits:
----
| Jsonifier: [](https://github.com/RealTimeChris/Jsonifier/commit/)  
| Glaze: [e85dc2e](https://github.com/stephenberry/glaze/commit/e85dc2e)  
| Simdjson: [36f5dbc](https://github.com/simdjson/simdjson/commit/36f5dbc)  

 > At least 30 iterations on a (Intel(R) Core(TM) i7-9750H CPU @ 2.60G), until coefficient of variance is at or below 1%.


### Json Test (Prettified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/Windows-MSVC/JsonData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Json%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Json%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 193.992 | 1.33613e+07 | 1612444 | 2.15443e+07 | 300 | 117.494 | 2.20608e+07 | 1612444 | 3.55718e+07 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/e85dc2e) | 120.098 | 2.15821e+07 | 1612394 | 3.47989e+07 | 300 | 116.575 | 2.22346e+07 | 1612394 | 3.58509e+07 | 300 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/36f5dbc) | 10.5068 | 2.46698e+08 | 1612394 | 3.97774e+08 | 300 | 

### Json Test (Minified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/Windows-MSVC/JsonData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Json%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Json%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 140.826 | 1.84057e+07 | 1327611 | 2.44356e+07 | 300 | 130.507 | 1.98611e+07 | 1327611 | 2.63678e+07 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/e85dc2e) | 118.906 | 2.17986e+07 | 1327561 | 2.8939e+07 | 300 | 178.238 | 1.45423e+07 | 1327561 | 1.93059e+07 | 300 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/36f5dbc) | 7.68771 | 3.37162e+08 | 1327561 | 4.47604e+08 | 300 | 

### ABC Test (Out of Sequence Performance - Prettified) [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/Windows-MSVC/JsonData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Abc%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Abc%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>

The JSON documents in the previous tests featured keys ranging from "a" to "z," where each key corresponds to an array of values. Notably, the documents in this test arrange these keys in reverse order, deviating from the typical "a" to "z" arrangement.

This test effectively demonstrates the challenges encountered when utilizing simdjson and iterative parsers that lack the ability to efficiently allocate memory locations through hashing. In cases where the keys are not in the expected sequence, performance is significantly compromised, with the severity escalating as the document size increases.

In contrast, hash-based solutions offer a viable alternative by circumventing these issues and maintaining optimal performance regardless of the JSON document's scale, or ordering of the keys being parsed.

| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 156.215 | 1.65923e+07 | 1612444 | 2.67542e+07 | 300 | 151.199 | 1.7143e+07 | 1612444 | 2.76422e+07 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/e85dc2e) | 124.377 | 2.08393e+07 | 1612394 | 3.36011e+07 | 300 | 180.206 | 1.43835e+07 | 1612394 | 2.31919e+07 | 300 | 


### ABC Test (Out of Sequence Performance - Minified) [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/Windows-MSVC/JsonData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Abc%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Abc%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 180.675 | 1.43901e+07 | 1327611 | 1.91044e+07 | 300 | 105.909 | 2.44736e+07 | 1327611 | 3.24914e+07 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/e85dc2e) | 133.052 | 1.94809e+07 | 1327561 | 2.58621e+07 | 300 | 201.544 | 1.28608e+07 | 1327561 | 1.70735e+07 | 300 | 


### Discord Test (Prettified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Discord%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Discord%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 142.833 | 1.81472e+07 | 138774 | 2.51836e+06 | 300 | 115.733 | 2.23964e+07 | 138774 | 3.10804e+06 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/e85dc2e) | 72.8116 | 3.55989e+07 | 138774 | 4.9402e+06 | 300 | 206.184 | 1.25713e+07 | 138774 | 1.74457e+06 | 300 | 


### Discord Test (Minified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Discord%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Discord%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 104.023 | 2.49177e+07 | 69037 | 1.72025e+06 | 300 | 125.937 | 2.05819e+07 | 69037 | 1.42091e+06 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/e85dc2e) | 70.8108 | 3.66048e+07 | 69037 | 2.52708e+06 | 300 | 150.856 | 1.71819e+07 | 69037 | 1.18618e+06 | 300 | 


### Canada Test (Prettified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/CanadaData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Canada%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Canada%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [glaze](https://github.com/stephenberry/glaze/commit/e85dc2e) | 52.9596 | 4.89419e+07 | 6661897 | 3.26046e+08 | 300 | 63.877 | 4.05781e+07 | 6661897 | 2.70327e+08 | 300 | 
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 50.5654 | 5.12598e+07 | 6661897 | 3.41488e+08 | 300 | 69.6357 | 3.72222e+07 | 6661897 | 2.47971e+08 | 300 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/36f5dbc) | 12.3491 | 2.09893e+08 | 6661897 | 1.39828e+09 | 300 | 

### Canada Test (Minified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/CanadaData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Canada%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Canada%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 26.4356 | 9.80482e+07 | 2090234 | 2.04944e+08 | 300 | 21.7531 | 1.19155e+08 | 2090234 | 2.49062e+08 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/e85dc2e) | 21.9184 | 1.18254e+08 | 2090234 | 2.47179e+08 | 300 | 16.9399 | 1.53011e+08 | 2090234 | 3.19828e+08 | 300 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/36f5dbc) | 3.04988 | 8.49856e+08 | 2090234 | 1.7764e+09 | 300 | 

### CitmCatalog Test (Prettified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/CitmCatalogData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/CitmCatalog%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/CitmCatalog%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 196.377 | 1.31992e+07 | 1439556 | 1.9001e+07 | 300 | 120.176 | 2.15684e+07 | 1439556 | 3.10489e+07 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/e85dc2e) | 105.298 | 2.46159e+07 | 1439584 | 3.54366e+07 | 300 | 302.322 | 8.57367e+06 | 1439584 | 1.23425e+07 | 300 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/36f5dbc) | 15.3471 | 1.68892e+08 | 1428054 | 2.41187e+08 | 300 | 

### CitmCatalog Test (Minified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/CitmCatalogData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/CitmCatalog%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/CitmCatalog%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 60.2951 | 4.29888e+07 | 500293 | 2.1507e+07 | 300 | 110.253 | 2.35096e+07 | 500293 | 1.17617e+07 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/e85dc2e) | 56.9148 | 4.55417e+07 | 500299 | 2.27845e+07 | 300 | 129.255 | 2.00533e+07 | 500299 | 1.00326e+07 | 300 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/36f5dbc) | 7.08385 | 3.659e+08 | 497527 | 1.82045e+08 | 300 | 

### Twitter Test (Prettified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/TwitterData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Twitter%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Twitter%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 136.982 | 1.89222e+07 | 719239 | 1.36096e+07 | 300 | 101.713 | 2.54835e+07 | 719239 | 1.83287e+07 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/e85dc2e) | 96.6422 | 2.68207e+07 | 719148 | 1.9288e+07 | 300 | 143.99 | 1.80013e+07 | 719148 | 1.29456e+07 | 300 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/36f5dbc) | 8.60977 | 3.01052e+08 | 516839 | 1.55595e+08 | 300 | 

### Twitter Test (Minified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/TwitterData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Twitter%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Twitter%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 133.737 | 1.93813e+07 | 477806 | 9.26052e+06 | 300 | 200.137 | 1.29512e+07 | 477806 | 6.18816e+06 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/e85dc2e) | 130.708 | 1.98306e+07 | 477715 | 9.47339e+06 | 300 | 127.649 | 2.03058e+07 | 477715 | 9.7004e+06 | 300 | 


### Minify Test Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Minify%20Test_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Minify%20Test_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ------------ | ------------------| -------------------- | --------------- | --------------------- |   
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 63.4341 | 4.08612e+07 | 69037 | 2.82094e+06 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/e85dc2e) | 62.0759 | 4.17553e+07 | 69037 | 2.88266e+06 | 300 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/36f5dbc) | 6.42276 | 4.03562e+08 | 69037 | 2.78607e+07 | 300 | 

### Prettify Test Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Prettify%20Test_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Prettify%20Test_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ------------ | ------------------| -------------------- | --------------- | --------------------- |   
| [glaze](https://github.com/stephenberry/glaze/commit/e85dc2e) | 318.99 | 8.12562e+06 | 1612444 | 1.31021e+07 | 300 | 
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 135.343 | 1.91514e+07 | 1612444 | 3.08806e+07 | 300 | 

### Validation Test Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Validate%20Test_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Validate%20Test_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- |   
| [glaze](https://github.com/stephenberry/glaze/commit/e85dc2e) | 244.446 | 1.06036e+07 | 123050 | 1.30477e+06 | 300 | 
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 174.216 | 1.48781e+07 | 123050 | 1.83075e+06 | 300 | 