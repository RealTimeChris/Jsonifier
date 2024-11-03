# Json-Performance
Performance profiling of JSON libraries (Compiled and run on Windows 10.0.22631 using the MSVC 19.41.34123.0 compiler).  

Latest Results: (Nov 03, 2024)
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
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 140.957 | 2.62205e+07 | 776604 | 2.0363e+07 | 300 | 176.278 | 2.09667e+07 | 776604 | 1.62828e+07 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/890bf43) | 106.717 | 3.46332e+07 | 776604 | 2.68963e+07 | 300 | 144.513 | 2.55753e+07 | 776604 | 1.98618e+07 | 300 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/587beae) | 10.6525 | 3.46958e+08 | 776604 | 2.69449e+08 | 300 | 

### Json Test (Minified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/Windows-MSVC/JsonData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Json%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Json%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 116.613 | 3.16939e+07 | 505004 | 1.60056e+07 | 300 | 89.3316 | 4.13737e+07 | 505004 | 2.08939e+07 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/890bf43) | 80.597 | 4.58571e+07 | 505004 | 2.3158e+07 | 300 | 75.6505 | 4.88557e+07 | 505004 | 2.46723e+07 | 300 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/587beae) | 7.2133 | 5.12383e+08 | 505004 | 2.58755e+08 | 300 | 

### ABC Test (Out of Sequence Performance - Prettified) [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/Windows-MSVC/JsonData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Abc%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Abc%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>

The JSON documents in the previous tests featured keys ranging from "a" to "z," where each key corresponds to an array of values. Notably, the documents in this test arrange these keys in reverse order, deviating from the typical "a" to "z" arrangement.

This test effectively demonstrates the challenges encountered when utilizing simdjson and iterative parsers that lack the ability to efficiently allocate memory locations through hashing. In cases where the keys are not in the expected sequence, performance is significantly compromised, with the severity escalating as the document size increases.

In contrast, hash-based solutions offer a viable alternative by circumventing these issues and maintaining optimal performance regardless of the JSON document's scale, or ordering of the keys being parsed.

| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 164.887 | 2.2415e+07 | 776604 | 1.74076e+07 | 300 | 186.381 | 1.98303e+07 | 776604 | 1.54003e+07 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/890bf43) | 104.466 | 3.53796e+07 | 776604 | 2.7476e+07 | 300 | 134.318 | 2.75165e+07 | 776604 | 2.13694e+07 | 300 | 


### ABC Test (Out of Sequence Performance - Minified) [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/Windows-MSVC/JsonData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Abc%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Abc%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 111.751 | 3.30731e+07 | 505004 | 1.67021e+07 | 300 | 119.627 | 3.08957e+07 | 505004 | 1.56024e+07 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/890bf43) | 81.8882 | 4.51345e+07 | 505004 | 2.27931e+07 | 300 | 115.349 | 3.20417e+07 | 505004 | 1.61812e+07 | 300 | 


### Discord Test (Prettified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Discord%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Discord%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 271.782 | 1.3599e+07 | 138774 | 1.88719e+06 | 300 | 386.029 | 9.57433e+06 | 138774 | 1.32867e+06 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/890bf43) | 160.285 | 2.30588e+07 | 138774 | 3.19996e+06 | 300 | 282.053 | 1.31038e+07 | 138774 | 1.81847e+06 | 300 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/587beae) | 14.8463 | 2.4895e+08 | 138482 | 3.4475e+07 | 300 | 

### Discord Test (Minified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Discord%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Discord%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 171.089 | 2.16025e+07 | 69037 | 1.49137e+06 | 297 | 272.371 | 1.35696e+07 | 69037 | 936805 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/890bf43) | 113.305 | 3.26196e+07 | 69037 | 2.25196e+06 | 300 | 194.829 | 1.89703e+07 | 69037 | 1.30965e+06 | 300 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/587beae) | 8.04146 | 4.59613e+08 | 68745 | 3.15961e+07 | 300 | 

### Canada Test (Prettified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/CanadaData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Canada%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Canada%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 135.633 | 2.72493e+07 | 6661897 | 1.81532e+08 | 300 | 114.439 | 3.22966e+07 | 6661897 | 2.15157e+08 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/890bf43) | 75.5192 | 4.89409e+07 | 6661897 | 3.26039e+08 | 300 | 95.5424 | 3.8684e+07 | 6661897 | 2.57709e+08 | 300 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/587beae) | 18.9848 | 1.94681e+08 | 6661897 | 1.29695e+09 | 300 | 

### Canada Test (Minified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/CanadaData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Canada%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Canada%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 42.9451 | 8.60628e+07 | 2090234 | 1.79891e+08 | 300 | 34.4092 | 1.07411e+08 | 2090234 | 2.24515e+08 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/890bf43) | 28.5654 | 1.29385e+08 | 2090234 | 2.70445e+08 | 300 | 32.2666 | 1.14543e+08 | 2090234 | 2.39422e+08 | 300 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/587beae) | 6.63332 | 5.57184e+08 | 2090234 | 1.16464e+09 | 300 | 

### CitmCatalog Test (Prettified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/CitmCatalogData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/CitmCatalog%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/CitmCatalog%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 341.157 | 1.08337e+07 | 1439562 | 1.55957e+07 | 300 | 669.591 | 5.51976e+06 | 1439562 | 7.94603e+06 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/890bf43) | 186.331 | 1.98355e+07 | 1439584 | 2.85548e+07 | 300 | 210.624 | 1.75477e+07 | 1439584 | 2.52614e+07 | 300 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/587beae) | 19.5853 | 1.88711e+08 | 1428054 | 2.6949e+08 | 300 | 

### CitmCatalog Test (Minified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/CitmCatalogData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/CitmCatalog%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/CitmCatalog%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 189.876 | 1.94651e+07 | 500299 | 9.73839e+06 | 300 | 306.241 | 1.20688e+07 | 500299 | 6.03799e+06 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/890bf43) | 95.1547 | 3.88414e+07 | 500299 | 1.94323e+07 | 300 | 166.747 | 2.21651e+07 | 500299 | 1.10892e+07 | 300 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/587beae) | 9.04073 | 4.08811e+08 | 497527 | 2.03395e+08 | 300 | 

### Twitter Test (Prettified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/TwitterData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Twitter%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Twitter%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 238.915 | 1.54695e+07 | 719107 | 1.11242e+07 | 300 | 323.561 | 1.14228e+07 | 719107 | 8.21419e+06 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/890bf43) | 169.431 | 2.1814e+07 | 719139 | 1.56873e+07 | 300 | 298.311 | 1.23895e+07 | 719139 | 8.90981e+06 | 300 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/587beae) | 9.68322 | 3.81683e+08 | 516830 | 1.97265e+08 | 300 | 

### Twitter Test (Minified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/TwitterData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Twitter%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Twitter%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 176.178 | 2.09787e+07 | 477683 | 1.00212e+07 | 300 | 480.281 | 7.6954e+06 | 477683 | 3.67596e+06 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/890bf43) | 153.837 | 2.40252e+07 | 477715 | 1.14772e+07 | 300 | 169.825 | 2.17635e+07 | 477715 | 1.03967e+07 | 300 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/587beae) | 5.94866 | 6.21312e+08 | 282852 | 1.75739e+08 | 300 | 

### Minify Test Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Minify%20Test_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Minify%20Test_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ------------ | ------------------| -------------------- | --------------- | --------------------- |   
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 106.101 | 3.48342e+07 | 69037 | 2.40485e+06 | 96 | 
| [glaze](https://github.com/stephenberry/glaze/commit/890bf43) | 70.9281 | 5.21089e+07 | 69037 | 3.59744e+06 | 300 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/587beae) | 8.59483 | 4.30022e+08 | 69037 | 2.96874e+07 | 300 | 

### Prettify Test Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Prettify%20Test_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Prettify%20Test_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ------------ | ------------------| -------------------- | --------------- | --------------------- |   
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 354.091 | 1.04377e+07 | 776604 | 8.10599e+06 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/890bf43) | 192.628 | 1.9187e+07 | 776604 | 1.49007e+07 | 300 | 

### Validation Test Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Validate%20Test_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Validate%20Test_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- |   
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 194.263 | 1.90251e+07 | 123050 | 2.34104e+06 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/890bf43) | 179.085 | 2.0638e+07 | 123050 | 2.53951e+06 | 300 | 