# Json-Performance
Performance profiling of JSON libraries (Compiled and run on Windows 10.0.22631 using the MSVC 19.41.34123.0 compiler).  

Latest Results: (Nov 14, 2024)
#### Using the following commits:
----
| Jsonifier: [](https://github.com/RealTimeChris/Jsonifier/commit/)  
| Glaze: [e65db35](https://github.com/stephenberry/glaze/commit/e65db35)  
| Simdjson: [9b31492](https://github.com/simdjson/simdjson/commit/9b31492)  

 > At least 30 iterations on a (Intel(R) Core(TM) i7-8700K CPU @ 3.70G), until coefficient of variance is at or below 1%.


### Json Test (Prettified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/Windows-MSVC/JsonData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Json%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Json%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 238.206 | 1.55158e+07 | 2268663 | 3.52002e+07 | 300 | 271.671 | 1.36046e+07 | 2268663 | 3.08643e+07 | 300 | 

### Json Test (Minified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/Windows-MSVC/JsonData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Json%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Json%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 185.292 | 1.99467e+07 | 1755374 | 3.50139e+07 | 300 | 210.605 | 1.75492e+07 | 1755374 | 3.08054e+07 | 300 | 

### Partial Test (Prettified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/Windows-MSVC/JsonData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Partial%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Partial%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 82.3942 | 4.48571e+07 | 14 | 628 | 300 | 100.668 | 3.67143e+07 | 14 | 514 | 300 | 

### Partial Test (Minified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/Windows-MSVC/JsonData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Partial%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Partial%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 40.1939 | 9.19534e+07 | 64268 | 5.90966e+06 | 300 | 202.304 | 1.82694e+07 | 64268 | 1.17414e+06 | 299 | 

### Abc Test (Out of Sequence Performance - Prettified) [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/Windows-MSVC/JsonData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Abc%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Abc%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>

The JSON documents in the previous tests featured keys ranging from "a" to "z," where each key corresponds to an array of values. Notably, the documents in this test arrange these keys in reverse order, deviating from the typical "a" to "z" arrangement.

This test effectively demonstrates the challenges encountered when utilizing simdjson and iterative parsers that lack the ability to efficiently allocate memory locations through hashing. In cases where the keys are not in the expected sequence, performance is significantly compromised, with the severity escalating as the document size increases.

In contrast, hash-based solutions offer a viable alternative by circumventing these issues and maintaining optimal performance regardless of the JSON document's scale, or ordering of the keys being parsed.

| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 0.0407833 | 9.06248e+10 | 314 | 2.84562e+07 | 298 | 145.943 | 2.53248e+07 | 314 | 7952 | 300 | 

### ABC Test (Out of Sequence Performance - Minified) [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/Windows-MSVC/JsonData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Abc%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Abc%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 189.625 | 1.9491e+07 | 1755374 | 3.42139e+07 | 300 | 220.047 | 1.67963e+07 | 1755374 | 2.94837e+07 | 300 | 

### Discord Test (Prettified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Discord%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Discord%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 282.659 | 1.30757e+07 | 138774 | 1.81457e+06 | 300 | 600.476 | 6.15507e+06 | 138774 | 854163 | 300 | 

### Discord Test (Minified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Discord%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Discord%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 155.367 | 2.37886e+07 | 69037 | 1.6423e+06 | 300 | 367.559 | 1.00554e+07 | 69037 | 694198 | 300 | 

### Canada Test (Prettified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/CanadaData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Canada%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Canada%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 127.67 | 2.89494e+07 | 6661897 | 1.92858e+08 | 300 | 115.734 | 3.19346e+07 | 6661897 | 2.12745e+08 | 300 | 

### Canada Test (Minified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/CanadaData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Canada%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Canada%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 44.2385 | 8.35464e+07 | 2090234 | 1.74631e+08 | 300 | 39.1813 | 9.433e+07 | 2090234 | 1.97172e+08 | 296 | 

### CitmCatalog Test (Prettified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/CitmCatalogData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/CitmCatalog%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/CitmCatalog%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 372 | 9.93544e+06 | 1439562 | 1.43027e+07 | 300 | 666.918 | 5.54184e+06 | 1439562 | 7.97782e+06 | 300 | 

### CitmCatalog Test (Minified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/CitmCatalogData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/CitmCatalog%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/CitmCatalog%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 172.108 | 2.14748e+07 | 500299 | 1.07438e+07 | 300 | 243.22 | 1.5196e+07 | 500299 | 7.60255e+06 | 300 | 

### Twitter Test (Prettified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/TwitterData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Twitter%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Twitter%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 1.07363 | 3.44252e+09 | 7225 | 2.48722e+07 | 298 | 512.699 | 7.20886e+06 | 7225 | 52084 | 300 | 

### Twitter Test (Minified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/TwitterData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Twitter%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Twitter%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 210.178 | 1.75849e+07 | 477683 | 8.40001e+06 | 295 | 485.163 | 7.618e+06 | 477683 | 3.63899e+06 | 300 | 

### Minify Test Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Minify%20Test_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Minify%20Test_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ------------ | ------------------| -------------------- | --------------- | --------------------- |   
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 133.456 | 2.76944e+07 | 69037 | 1.91194e+06 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/e65db35) | 72.3337 | 5.10961e+07 | 69037 | 3.52752e+06 | 300 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/9b31492) | 8.59433 | 4.30046e+08 | 69037 | 2.96891e+07 | 98 | 

### Prettify Test Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Prettify%20Test_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Prettify%20Test_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ------------ | ------------------| -------------------- | --------------- | --------------------- |   
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 447.436 | 8.26033e+06 | 2268663 | 1.87399e+07 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/e65db35) | 404.922 | 9.12761e+06 | 2268663 | 2.07075e+07 | 300 | 

### Validation Test Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Validate%20Test_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Validate%20Test_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- |   
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 321.847 | 1.14835e+07 | 123050 | 1.41305e+06 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/e65db35) | 295.175 | 1.25212e+07 | 123050 | 1.54074e+06 | 300 | 