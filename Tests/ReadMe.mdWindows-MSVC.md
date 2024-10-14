# Json-Performance
Performance profiling of JSON libraries (Compiled and run on Windows 10.0.22631 using the MSVC 19.41.34120.0 compiler).  

Latest Results: (Oct 14, 2024)
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
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 224.226 | 1.15597e+07 | 1832476 | 2.11828e+07 | 300 | 117.186 | 2.21188e+07 | 1832476 | 4.05322e+07 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/e85dc2e) | 57.7954 | 4.48481e+07 | 1832401 | 8.21797e+07 | 300 | 153.723 | 1.68613e+07 | 1832401 | 3.08967e+07 | 300 | 


### Json Test (Minified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/Windows-MSVC/JsonData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Json%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Json%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 215.655 | 1.20192e+07 | 1532864 | 1.84239e+07 | 300 | 165.803 | 1.5633e+07 | 1532864 | 2.39632e+07 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/e85dc2e) | 77.7053 | 3.3357e+07 | 1532789 | 5.11292e+07 | 300 | 135.761 | 1.90925e+07 | 1532789 | 2.92648e+07 | 300 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/36f5dbc) | 10.1374 | 2.55684e+08 | 1532789 | 3.91909e+08 | 300 | 

### ABC Test (Out of Sequence Performance - Prettified) [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/Windows-MSVC/JsonData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Abc%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Abc%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>

The JSON documents in the previous tests featured keys ranging from "a" to "z," where each key corresponds to an array of values. Notably, the documents in this test arrange these keys in reverse order, deviating from the typical "a" to "z" arrangement.

This test effectively demonstrates the challenges encountered when utilizing simdjson and iterative parsers that lack the ability to efficiently allocate memory locations through hashing. In cases where the keys are not in the expected sequence, performance is significantly compromised, with the severity escalating as the document size increases.

In contrast, hash-based solutions offer a viable alternative by circumventing these issues and maintaining optimal performance regardless of the JSON document's scale, or ordering of the keys being parsed.

| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 198.196 | 1.3078e+07 | 1832476 | 2.39651e+07 | 300 | 153.851 | 1.68476e+07 | 1832476 | 3.08728e+07 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/e85dc2e) | 99.7925 | 2.5974e+07 | 1832401 | 4.75947e+07 | 300 | 179.037 | 1.44774e+07 | 1832401 | 2.65283e+07 | 300 | 


### ABC Test (Out of Sequence Performance - Minified) [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/Windows-MSVC/JsonData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Abc%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Abc%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 137.172 | 1.88957e+07 | 1532864 | 2.89646e+07 | 300 | 218.642 | 1.1855e+07 | 1532864 | 1.81722e+07 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/e85dc2e) | 100.285 | 2.58464e+07 | 1532789 | 3.96171e+07 | 300 | 163.358 | 1.5867e+07 | 1532789 | 2.43208e+07 | 300 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/36f5dbc) | 4.90055 | 5.28916e+08 | 1532789 | 8.10717e+08 | 300 | 

### Discord Test (Prettified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Discord%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Discord%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 181.15 | 1.43087e+07 | 138774 | 1.98567e+06 | 300 | 84.1901 | 3.07875e+07 | 138774 | 4.27251e+06 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/e85dc2e) | 135.045 | 1.91936e+07 | 138774 | 2.66357e+06 | 300 | 264.02 | 9.81747e+06 | 138774 | 1.36241e+06 | 300 | 


### Discord Test (Minified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Discord%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Discord%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 108.844 | 2.38139e+07 | 69037 | 1.64404e+06 | 300 | 133.369 | 1.94349e+07 | 69037 | 1.34173e+06 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/e85dc2e) | 82.5303 | 3.14066e+07 | 69037 | 2.16822e+06 | 300 | 125.211 | 2.0701e+07 | 69037 | 1.42914e+06 | 300 | 


### Canada Test (Prettified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/CanadaData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Canada%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Canada%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [glaze](https://github.com/stephenberry/glaze/commit/e85dc2e) | 76.928 | 3.36941e+07 | 6661897 | 2.24466e+08 | 300 | 80.3471 | 3.22597e+07 | 6661897 | 2.14911e+08 | 300 | 
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 76.2899 | 3.39757e+07 | 6661897 | 2.26342e+08 | 300 | 86.5354 | 2.99531e+07 | 6661897 | 1.99544e+08 | 300 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/36f5dbc) | 13.4937 | 1.92088e+08 | 6661897 | 1.27967e+09 | 300 | 

### Canada Test (Minified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/CanadaData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Canada%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Canada%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 28.8868 | 8.97281e+07 | 2090234 | 1.87553e+08 | 300 | 17.7037 | 1.4641e+08 | 2090234 | 3.06032e+08 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/e85dc2e) | 19.4872 | 1.33011e+08 | 2090234 | 2.78023e+08 | 300 | 14.9923 | 1.72889e+08 | 2090234 | 3.61378e+08 | 300 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/36f5dbc) | 3.65856 | 7.08463e+08 | 2090234 | 1.48085e+09 | 300 | 

### CitmCatalog Test (Prettified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/CitmCatalogData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/CitmCatalog%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/CitmCatalog%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 191.33 | 1.35511e+07 | 1439556 | 1.95076e+07 | 300 | 214.432 | 1.20877e+07 | 1439556 | 1.7401e+07 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/e85dc2e) | 177.111 | 1.46349e+07 | 1439584 | 2.10682e+07 | 300 | 139.191 | 1.86219e+07 | 1439584 | 2.68078e+07 | 300 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/36f5dbc) | 16.4383 | 1.57678e+08 | 1428054 | 2.25173e+08 | 300 | 

### CitmCatalog Test (Minified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/CitmCatalogData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/CitmCatalog%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/CitmCatalog%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 123.089 | 2.10581e+07 | 500293 | 1.05352e+07 | 300 | 131.55 | 1.97035e+07 | 500293 | 9.85753e+06 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/e85dc2e) | 93.9394 | 2.75923e+07 | 500299 | 1.38044e+07 | 300 | 140.047 | 1.85079e+07 | 500299 | 9.2595e+06 | 300 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/36f5dbc) | 7.75788 | 3.34105e+08 | 497527 | 1.66226e+08 | 300 | 

### Twitter Test (Prettified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/TwitterData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Twitter%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Twitter%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [glaze](https://github.com/stephenberry/glaze/commit/e85dc2e) | 146.081 | 1.77466e+07 | 719148 | 1.27624e+07 | 300 | 258.05 | 1.00446e+07 | 719148 | 7.22354e+06 | 300 | 
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 68.3976 | 3.78958e+07 | 719239 | 2.72561e+07 | 300 | 197.155 | 1.3147e+07 | 719239 | 9.45587e+06 | 300 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/36f5dbc) | 9.13767 | 2.83661e+08 | 516839 | 1.46607e+08 | 300 | 

### Twitter Test (Minified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/TwitterData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Twitter%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Twitter%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [glaze](https://github.com/stephenberry/glaze/commit/e85dc2e) | 131.067 | 1.97763e+07 | 477715 | 9.44741e+06 | 300 | 240.892 | 1.076e+07 | 477715 | 5.14024e+06 | 300 | 
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 47.9344 | 5.40742e+07 | 477806 | 2.5837e+07 | 300 | 222.249 | 1.16626e+07 | 477806 | 5.57246e+06 | 300 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/36f5dbc) | 5.51721 | 4.69806e+08 | 282852 | 1.32886e+08 | 300 | 

### Minify Test Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Minify%20Test_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Minify%20Test_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ------------ | ------------------| -------------------- | --------------- | --------------------- |   
| [glaze](https://github.com/stephenberry/glaze/commit/e85dc2e) | 62.2241 | 4.1656e+07 | 69037 | 2.8758e+06 | 300 | 
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 53.1173 | 4.87977e+07 | 69037 | 3.36885e+06 | 300 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/36f5dbc) | 7.46075 | 3.47419e+08 | 69037 | 2.39848e+07 | 300 | 

### Prettify Test Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Prettify%20Test_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Prettify%20Test_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ------------ | ------------------| -------------------- | --------------- | --------------------- |   
| [glaze](https://github.com/stephenberry/glaze/commit/e85dc2e) | 353.237 | 7.33785e+06 | 1832476 | 1.34464e+07 | 300 | 
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 260.769 | 9.93986e+06 | 1832476 | 1.82146e+07 | 300 | 

### Validation Test Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Validate%20Test_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Validate%20Test_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- |   
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 287.205 | 9.02493e+06 | 123050 | 1.11052e+06 | 98 | 
| [glaze](https://github.com/stephenberry/glaze/commit/e85dc2e) | 269.572 | 9.6153e+06 | 123050 | 1.18316e+06 | 300 | 