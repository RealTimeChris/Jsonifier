# Json-Performance
Performance profiling of JSON libraries (Compiled and run on Windows 10.0.22631 using the MSVC 19.41.34123.0 compiler).  

Latest Results: (Oct 28, 2024)
#### Using the following commits:
----
| Jsonifier: [](https://github.com/RealTimeChris/Jsonifier/commit/)  
| Glaze: [a1e3237](https://github.com/stephenberry/glaze/commit/a1e3237)  
| Simdjson: [3c0d032](https://github.com/simdjson/simdjson/commit/3c0d032)  

 > At least 30 iterations on a (Intel(R) Core(TM) i7-8700K CPU @ 3.70G), until coefficient of variance is at or below 1%.


### Json Test (Prettified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/Windows-MSVC/JsonData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Json%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Json%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 130.465 | 2.83291e+07 | 1032256 | 2.92429e+07 | 300 | 104.929 | 3.52231e+07 | 1032256 | 3.63593e+07 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/a1e3237) | 70.297 | 5.25762e+07 | 1032102 | 5.4264e+07 | 300 | 93.7608 | 3.9419e+07 | 1032102 | 4.06845e+07 | 300 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/3c0d032) | 9.7719 | 3.78224e+08 | 1032102 | 3.90365e+08 | 300 | 

### Json Test (Minified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/Windows-MSVC/JsonData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Json%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Json%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [glaze](https://github.com/stephenberry/glaze/commit/a1e3237) | 91.1027 | 4.05692e+07 | 702143 | 2.84854e+07 | 300 | 124.584 | 2.96666e+07 | 702143 | 2.08302e+07 | 300 | 
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 86.8955 | 4.25335e+07 | 702297 | 2.98712e+07 | 300 | 143.918 | 2.56811e+07 | 702297 | 1.80358e+07 | 300 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/3c0d032) | 7.8795 | 4.69062e+08 | 702143 | 3.29349e+08 | 300 | 

### ABC Test (Out of Sequence Performance - Prettified) [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/Windows-MSVC/JsonData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Abc%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Abc%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>

The JSON documents in the previous tests featured keys ranging from "a" to "z," where each key corresponds to an array of values. Notably, the documents in this test arrange these keys in reverse order, deviating from the typical "a" to "z" arrangement.

This test effectively demonstrates the challenges encountered when utilizing simdjson and iterative parsers that lack the ability to efficiently allocate memory locations through hashing. In cases where the keys are not in the expected sequence, performance is significantly compromised, with the severity escalating as the document size increases.

In contrast, hash-based solutions offer a viable alternative by circumventing these issues and maintaining optimal performance regardless of the JSON document's scale, or ordering of the keys being parsed.

| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 141.813 | 2.60623e+07 | 1032256 | 2.6903e+07 | 300 | 196.265 | 1.88316e+07 | 1032256 | 1.9439e+07 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/a1e3237) | 116.068 | 3.1843e+07 | 1032102 | 3.28652e+07 | 300 | 160.126 | 2.30817e+07 | 1032102 | 2.38226e+07 | 300 | 


### ABC Test (Out of Sequence Performance - Minified) [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/Windows-MSVC/JsonData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Abc%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Abc%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 129.396 | 2.85733e+07 | 702297 | 2.00669e+07 | 300 | 146.013 | 2.53126e+07 | 702297 | 1.7777e+07 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/a1e3237) | 90.0301 | 4.10526e+07 | 702143 | 2.88248e+07 | 300 | 90.2692 | 4.09438e+07 | 702143 | 2.87484e+07 | 300 | 


### Discord Test (Prettified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Discord%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Discord%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 169.891 | 2.17549e+07 | 138774 | 3.01901e+06 | 300 | 438.847 | 8.42202e+06 | 138774 | 1.16876e+06 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/a1e3237) | 152.037 | 2.43096e+07 | 138774 | 3.37354e+06 | 300 | 169.155 | 2.18495e+07 | 138774 | 3.03214e+06 | 300 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/3c0d032) | 13.7758 | 2.68294e+08 | 138482 | 3.7154e+07 | 300 | 

### Discord Test (Minified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Discord%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Discord%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 137.555 | 2.68689e+07 | 69037 | 1.85495e+06 | 300 | 326.19 | 1.13307e+07 | 69037 | 782236 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/a1e3237) | 113.056 | 3.26915e+07 | 69037 | 2.25692e+06 | 300 | 182.556 | 2.02456e+07 | 69037 | 1.3977e+06 | 300 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/3c0d032) | 7.83197 | 4.71907e+08 | 68745 | 3.24412e+07 | 300 | 

### Canada Test (Prettified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/CanadaData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Canada%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Canada%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 131.949 | 2.80105e+07 | 6661897 | 1.86603e+08 | 300 | 104.526 | 3.53596e+07 | 6661897 | 2.35562e+08 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/a1e3237) | 83.0825 | 4.44849e+07 | 6661897 | 2.96354e+08 | 300 | 100.093 | 3.69254e+07 | 6661897 | 2.45993e+08 | 300 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/3c0d032) | 16.4936 | 2.24085e+08 | 6661897 | 1.49283e+09 | 300 | 

### Canada Test (Minified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/CanadaData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Canada%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Canada%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 37.9581 | 9.73695e+07 | 2090234 | 2.03525e+08 | 300 | 34.9766 | 1.05669e+08 | 2090234 | 2.20872e+08 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/a1e3237) | 28.6498 | 1.29003e+08 | 2090234 | 2.69647e+08 | 300 | 31.0969 | 1.18853e+08 | 2090234 | 2.48431e+08 | 300 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/3c0d032) | 6.53811 | 5.65293e+08 | 2090234 | 1.1816e+09 | 300 | 

### CitmCatalog Test (Prettified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/CitmCatalogData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/CitmCatalog%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/CitmCatalog%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 343.847 | 1.07489e+07 | 1439562 | 1.54736e+07 | 300 | 598.227 | 6.17824e+06 | 1439562 | 8.89396e+06 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/a1e3237) | 209.304 | 1.76584e+07 | 1439584 | 2.54207e+07 | 300 | 354.847 | 1.04157e+07 | 1439584 | 1.49943e+07 | 300 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/3c0d032) | 19.2143 | 1.92354e+08 | 1428054 | 2.74693e+08 | 300 | 

### CitmCatalog Test (Minified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/CitmCatalogData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/CitmCatalog%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/CitmCatalog%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 180.597 | 2.04653e+07 | 500299 | 1.02388e+07 | 300 | 287.576 | 1.28521e+07 | 500299 | 6.42989e+06 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/a1e3237) | 114.951 | 3.21524e+07 | 500299 | 1.60858e+07 | 300 | 168.928 | 2.1879e+07 | 500299 | 1.0946e+07 | 300 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/3c0d032) | 9.14873 | 4.03988e+08 | 497527 | 2.00995e+08 | 300 | 

### Twitter Test (Prettified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/TwitterData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Twitter%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Twitter%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [glaze](https://github.com/stephenberry/glaze/commit/a1e3237) | 178.379 | 2.07198e+07 | 719139 | 1.49004e+07 | 300 | 338.546 | 1.09171e+07 | 719139 | 7.85093e+06 | 300 | 
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 162.566 | 2.27352e+07 | 719198 | 1.63511e+07 | 300 | 520.3 | 7.10353e+06 | 719198 | 5.10884e+06 | 300 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/3c0d032) | 10.4812 | 3.52627e+08 | 516830 | 1.82248e+08 | 300 | 

### Twitter Test (Minified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/TwitterData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Twitter%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Twitter%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 184.564 | 2.00255e+07 | 477774 | 9.56765e+06 | 300 | 485.212 | 7.61723e+06 | 477774 | 3.63932e+06 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/a1e3237) | 147.306 | 2.50904e+07 | 477715 | 1.19861e+07 | 300 | 299.135 | 1.23555e+07 | 477715 | 5.9024e+06 | 300 | 


### Minify Test Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Minify%20Test_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Minify%20Test_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ------------ | ------------------| -------------------- | --------------- | --------------------- |   
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 128.546 | 2.87523e+07 | 69037 | 1.98497e+06 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/a1e3237) | 74.2417 | 4.97822e+07 | 69037 | 3.43682e+06 | 300 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/3c0d032) | 8.6019 | 4.29666e+08 | 69037 | 2.96628e+07 | 300 | 

### Prettify Test Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Prettify%20Test_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Prettify%20Test_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ------------ | ------------------| -------------------- | --------------- | --------------------- |   
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 395.058 | 9.35553e+06 | 1032256 | 9.65731e+06 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/a1e3237) | 319.84 | 1.15557e+07 | 1032256 | 1.19284e+07 | 300 | 

### Validation Test Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Validate%20Test_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Validate%20Test_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- |   
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 344.528 | 1.07276e+07 | 123050 | 1.32003e+06 | 97 | 
| [glaze](https://github.com/stephenberry/glaze/commit/a1e3237) | 293.723 | 1.25832e+07 | 123050 | 1.54836e+06 | 300 | 