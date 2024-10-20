# Json-Performance
Performance profiling of JSON libraries (Compiled and run on Windows 10.0.22631 using the MSVC 19.41.34123.0 compiler).  

Latest Results: (Oct 20, 2024)
#### Using the following commits:
----
| Jsonifier: [](https://github.com/RealTimeChris/Jsonifier/commit/)  
| Glaze: [df960ee](https://github.com/stephenberry/glaze/commit/df960ee)  
| Simdjson: [3c0d032](https://github.com/simdjson/simdjson/commit/3c0d032)  

 > At least 30 iterations on a (Intel(R) Core(TM) i7-9750H CPU @ 2.60G), until coefficient of variance is at or below 1%.


### Json Test (Prettified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/Windows-MSVC/JsonData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Json%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Json%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 84.5471 | 3.06571e+07 | 868304 | 2.66197e+07 | 300 | 124.19 | 2.0871e+07 | 868304 | 1.81224e+07 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/df960ee) | 70.4746 | 3.67789e+07 | 868208 | 3.19317e+07 | 300 | 76.5703 | 3.38508e+07 | 868208 | 2.93896e+07 | 300 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/3c0d032) | 8.11636 | 3.19352e+08 | 868208 | 2.77264e+08 | 300 | 

### Json Test (Minified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/Windows-MSVC/JsonData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Json%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Json%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 111.331 | 2.32817e+07 | 582585 | 1.35636e+07 | 300 | 112.651 | 2.30089e+07 | 582585 | 1.34046e+07 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/df960ee) | 76.7295 | 3.37808e+07 | 582489 | 1.96769e+07 | 300 | 105.124 | 2.46564e+07 | 582489 | 1.43621e+07 | 300 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/3c0d032) | 6.84971 | 3.78407e+08 | 582489 | 2.20418e+08 | 300 | 

### ABC Test (Out of Sequence Performance - Prettified) [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/Windows-MSVC/JsonData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Abc%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Abc%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>

The JSON documents in the previous tests featured keys ranging from "a" to "z," where each key corresponds to an array of values. Notably, the documents in this test arrange these keys in reverse order, deviating from the typical "a" to "z" arrangement.

This test effectively demonstrates the challenges encountered when utilizing simdjson and iterative parsers that lack the ability to efficiently allocate memory locations through hashing. In cases where the keys are not in the expected sequence, performance is significantly compromised, with the severity escalating as the document size increases.

In contrast, hash-based solutions offer a viable alternative by circumventing these issues and maintaining optimal performance regardless of the JSON document's scale, or ordering of the keys being parsed.

| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 141.465 | 1.83224e+07 | 868304 | 1.59094e+07 | 300 | 119.67 | 2.16593e+07 | 868304 | 1.88069e+07 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/df960ee) | 95.59 | 2.71155e+07 | 868208 | 2.35419e+07 | 300 | 154.296 | 1.67986e+07 | 868208 | 1.45847e+07 | 300 | 


### ABC Test (Out of Sequence Performance - Minified) [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/Windows-MSVC/JsonData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Abc%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Abc%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [glaze](https://github.com/stephenberry/glaze/commit/df960ee) | 77.7883 | 3.33207e+07 | 582489 | 1.9409e+07 | 300 | 109.382 | 2.36964e+07 | 582489 | 1.38029e+07 | 300 | 
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 67.3498 | 3.84853e+07 | 582585 | 2.2421e+07 | 300 | 117.583 | 2.20437e+07 | 582585 | 1.28423e+07 | 300 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/3c0d032) | 2.71187 | 9.55773e+08 | 582489 | 5.56727e+08 | 300 | 

### Discord Test (Prettified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Discord%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Discord%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 189.295 | 1.36928e+07 | 138774 | 1.90021e+06 | 300 | 172.549 | 1.50216e+07 | 138774 | 2.08461e+06 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/df960ee) | 147.437 | 1.75801e+07 | 138774 | 2.43966e+06 | 300 | 140.545 | 1.84422e+07 | 138774 | 2.55929e+06 | 300 | 


### Discord Test (Minified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Discord%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Discord%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [glaze](https://github.com/stephenberry/glaze/commit/df960ee) | 105.994 | 2.44538e+07 | 69037 | 1.68822e+06 | 300 | 176.821 | 1.46587e+07 | 69037 | 1.012e+06 | 300 | 
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 87.684 | 2.95604e+07 | 69037 | 2.04076e+06 | 300 | 127.021 | 2.04058e+07 | 69037 | 1.40876e+06 | 300 | 


### Canada Test (Prettified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/CanadaData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Canada%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Canada%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 85.3034 | 3.03849e+07 | 6661897 | 2.02421e+08 | 300 | 71.8441 | 3.60776e+07 | 6661897 | 2.40345e+08 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/df960ee) | 77.4481 | 3.34673e+07 | 6661897 | 2.22956e+08 | 300 | 87.0526 | 2.97747e+07 | 6661897 | 1.98356e+08 | 300 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/3c0d032) | 17.2211 | 1.50511e+08 | 6661897 | 1.00269e+09 | 300 | 

### Canada Test (Minified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/CanadaData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Canada%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Canada%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 34.1045 | 7.6001e+07 | 2090234 | 1.5886e+08 | 300 | 32.1852 | 8.05332e+07 | 2090234 | 1.68333e+08 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/df960ee) | 24.9486 | 1.03893e+08 | 2090234 | 2.1716e+08 | 300 | 28.8298 | 8.99059e+07 | 2090234 | 1.87924e+08 | 300 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/3c0d032) | 4.80338 | 5.39607e+08 | 2090234 | 1.1279e+09 | 300 | 

### CitmCatalog Test (Prettified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/CitmCatalogData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/CitmCatalog%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/CitmCatalog%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [glaze](https://github.com/stephenberry/glaze/commit/df960ee) | 160.002 | 1.61997e+07 | 1439584 | 2.33208e+07 | 300 | 306.914 | 8.44527e+06 | 1439584 | 1.21577e+07 | 300 | 
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 140.528 | 1.84446e+07 | 1439556 | 2.6552e+07 | 300 | 202.878 | 1.2776e+07 | 1439556 | 1.83917e+07 | 300 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/3c0d032) | 17.8755 | 1.45002e+08 | 1428054 | 2.0707e+08 | 300 | 

### CitmCatalog Test (Minified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/CitmCatalogData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/CitmCatalog%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/CitmCatalog%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 119.497 | 2.16908e+07 | 500293 | 1.08518e+07 | 300 | 140.433 | 1.8457e+07 | 500293 | 9.23391e+06 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/df960ee) | 90.4552 | 2.86549e+07 | 500299 | 1.4336e+07 | 300 | 146.938 | 1.76399e+07 | 500299 | 8.82524e+06 | 300 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/3c0d032) | 7.78339 | 3.33014e+08 | 497527 | 1.65683e+08 | 300 | 

### Twitter Test (Prettified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/TwitterData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Twitter%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Twitter%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 180.619 | 1.43505e+07 | 719239 | 1.03214e+07 | 300 | 206.319 | 1.2563e+07 | 719239 | 9.03576e+06 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/df960ee) | 130.965 | 1.97914e+07 | 719148 | 1.42329e+07 | 300 | 286.529 | 9.04607e+06 | 719148 | 6.50546e+06 | 300 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/3c0d032) | 8.81438 | 2.94066e+08 | 516839 | 1.51985e+08 | 300 | 

### Twitter Test (Minified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/TwitterData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Twitter%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Twitter%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 158.712 | 1.63312e+07 | 477806 | 7.80317e+06 | 300 | 218.225 | 1.18775e+07 | 477806 | 5.67515e+06 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/df960ee) | 112.195 | 2.31024e+07 | 477715 | 1.10364e+07 | 300 | 250.228 | 1.03584e+07 | 477715 | 4.94837e+06 | 300 | 


### Minify Test Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Minify%20Test_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Minify%20Test_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ------------ | ------------------| -------------------- | --------------- | --------------------- |   
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 67.4726 | 3.84151e+07 | 69037 | 2.65206e+06 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/df960ee) | 52.3994 | 4.94656e+07 | 69037 | 3.41496e+06 | 300 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/3c0d032) | 7.50274 | 3.4547e+08 | 69037 | 2.38502e+07 | 300 | 

### Prettify Test Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Prettify%20Test_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Prettify%20Test_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ------------ | ------------------| -------------------- | --------------- | --------------------- |   
| [glaze](https://github.com/stephenberry/glaze/commit/df960ee) | 267.43 | 9.69214e+06 | 868304 | 8.41573e+06 | 300 | 
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 228.944 | 1.13214e+07 | 868304 | 9.83045e+06 | 300 | 

### Validation Test Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Validate%20Test_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Validate%20Test_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- |   
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 295.097 | 8.78348e+06 | 123050 | 1.08081e+06 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/df960ee) | 238.892 | 1.085e+07 | 123050 | 1.33509e+06 | 300 | 