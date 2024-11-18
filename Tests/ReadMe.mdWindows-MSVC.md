# Json-Performance
Performance profiling of JSON libraries (Compiled and run on Windows 10.0.22631 using the MSVC 19.41.34123.0 compiler).  

Latest Results: (Nov 21, 2024)
#### Using the following commits:
----
| Jsonifier: [](https://github.com/RealTimeChris/Jsonifier/commit/)  
| Glaze: [41ab7ce](https://github.com/stephenberry/glaze/commit/41ab7ce)  
| Simdjson: [9b31492](https://github.com/simdjson/simdjson/commit/9b31492)  

 > At least 30 iterations on a (Intel(R) Core(TM) i7-8700K CPU @ 3.70G), until coefficient of variance is at or below 1%.


### Json Test (Prettified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/Windows-MSVC/JsonData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Json%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Json%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>



### Json Test (Minified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/Windows-MSVC/JsonData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Json%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Json%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 1470.46 | 2.51345e+06 | 1639047 | 4.11966e+06 | 163 | 1916.72 | 1.92829e+06 | 1639047 | 3.16055e+06 | 296 | 
| [glaze](https://github.com/stephenberry/glaze/commit/41ab7ce) | 1215.22 | 3.04141e+06 | 1639047 | 4.98502e+06 | 99 | 1749.52 | 2.11257e+06 | 1639047 | 3.4626e+06 | 297 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/9b31492) | 332.009 | 1.11324e+07 | 1639047 | 1.82465e+07 | 300 | 

### Partial Test (Prettified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/Windows-MSVC/JsonData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Partial%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Partial%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 178.771 | 2.06744e+07 | 82853 | 1.71294e+06 | 300 | 2799.4 | 1.32027e+06 | 82853 | 109388 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/41ab7ce) | 156.608 | 2.36002e+07 | 82853 | 1.95535e+06 | 300 | 2508.39 | 1.47343e+06 | 82853 | 122078 | 300 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/9b31492) | 101.47 | 3.64242e+07 | 82853 | 3.01786e+06 | 169 | 

### Partial Test (Minified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/Windows-MSVC/JsonData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Partial%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Partial%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 184.378 | 2.00457e+07 | 63477 | 1.27244e+06 | 160 | 2779.44 | 1.32976e+06 | 63477 | 84409 | 97 | 
| [glaze](https://github.com/stephenberry/glaze/commit/41ab7ce) | 148.423 | 2.49017e+07 | 63477 | 1.58069e+06 | 299 | 2396.21 | 1.5424e+06 | 63477 | 97907 | 300 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/9b31492) | 82.244 | 4.49391e+07 | 63477 | 2.8526e+06 | 299 | 

### Abc Test (Out of Sequence Performance - Prettified) [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/Windows-MSVC/JsonData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Abc%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Abc%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>

The JSON documents in the previous tests featured keys ranging from "a" to "z," where each key corresponds to an array of values. Notably, the documents in this test arrange these keys in reverse order, deviating from the typical "a" to "z" arrangement.

This test effectively demonstrates the challenges encountered when utilizing simdjson and iterative parsers that lack the ability to efficiently allocate memory locations through hashing. In cases where the keys are not in the expected sequence, performance is significantly compromised, with the severity escalating as the document size increases.

In contrast, hash-based solutions offer a viable alternative by circumventing these issues and maintaining optimal performance regardless of the JSON document's scale, or ordering of the keys being parsed.

| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 1688.76 | 2.18856e+06 | 2132849 | 4.66787e+06 | 300 | 2214.99 | 1.66863e+06 | 2132849 | 3.55893e+06 | 296 | 
| [glaze](https://github.com/stephenberry/glaze/commit/41ab7ce) | 1339.41 | 2.7594e+06 | 2132849 | 5.88538e+06 | 99 | 1840.99 | 2.00757e+06 | 2132849 | 4.28184e+06 | 300 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/9b31492) | 143.865 | 2.56905e+07 | 2132849 | 5.47939e+07 | 180 | 

### ABC Test (Out of Sequence Performance - Minified) [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/Windows-MSVC/JsonData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Abc%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Abc%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 1475.33 | 2.50519e+06 | 1639047 | 4.10613e+06 | 98 | 1940.37 | 1.90479e+06 | 1639047 | 3.12204e+06 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/41ab7ce) | 1210.15 | 3.05414e+06 | 1639047 | 5.00587e+06 | 300 | 1637.21 | 2.25749e+06 | 1639047 | 3.70013e+06 | 300 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/9b31492) | 112.92 | 3.2731e+07 | 1639047 | 5.36477e+07 | 298 | 

### Discord Test (Prettified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Discord%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Discord%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 2252.51 | 1.64083e+06 | 138774 | 227704 | 300 | 3455.31 | 1.06964e+06 | 138774 | 148438 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/41ab7ce) | 1173.75 | 3.14885e+06 | 138774 | 436979 | 300 | 2424.02 | 1.52472e+06 | 138774 | 211591 | 300 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/9b31492) | 472.547 | 7.8214e+06 | 138482 | 1.08312e+06 | 158 | 

### Discord Test (Minified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Discord%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Discord%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 1541.62 | 2.39747e+06 | 69037 | 165514 | 300 | 3147.9 | 1.1741e+06 | 69037 | 81056 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/41ab7ce) | 975.935 | 3.78707e+06 | 69037 | 261448 | 300 | 2164.71 | 1.70736e+06 | 69037 | 117871 | 300 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/9b31492) | 241.173 | 1.5325e+07 | 68745 | 1.05352e+06 | 296 | 

### Canada Test (Prettified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/CanadaData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Canada%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Canada%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 2029.57 | 1.82106e+06 | 6661897 | 1.21317e+07 | 296 | 1532.82 | 2.41124e+06 | 6661897 | 1.60634e+07 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/41ab7ce) | 1105.16 | 3.3443e+06 | 6661897 | 2.22794e+07 | 300 | 1291.01 | 2.86282e+06 | 6661897 | 1.90718e+07 | 300 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/9b31492) | 411.374 | 8.98443e+06 | 6661897 | 5.98534e+07 | 300 | 

### Canada Test (Minified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/CanadaData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Canada%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Canada%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 677.085 | 5.45863e+06 | 2090234 | 1.14098e+07 | 298 | 536.694 | 6.88658e+06 | 2090234 | 1.43946e+07 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/41ab7ce) | 443.226 | 8.33881e+06 | 2090234 | 1.74301e+07 | 99 | 487.724 | 7.57796e+06 | 2090234 | 1.58397e+07 | 300 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/9b31492) | 133.39 | 2.7708e+07 | 2090234 | 5.79162e+07 | 300 | 

### CitmCatalog Test (Prettified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/CitmCatalogData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/CitmCatalog%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/CitmCatalog%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 3102.4 | 1.19133e+06 | 1439562 | 1.71499e+06 | 300 | 3873.78 | 954101 | 1439562 | 1.37349e+06 | 293 | 
| [glaze](https://github.com/stephenberry/glaze/commit/41ab7ce) | 2082.07 | 1.77514e+06 | 1439584 | 2.55547e+06 | 185 | 3138.45 | 1.17764e+06 | 1439584 | 1.69531e+06 | 300 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/9b31492) | 680.164 | 5.43395e+06 | 1423437 | 7.73488e+06 | 300 | 

### CitmCatalog Test (Minified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/CitmCatalogData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/CitmCatalog%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/CitmCatalog%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 2011.94 | 1.83702e+06 | 500299 | 919059 | 299 | 2443.05 | 1.51283e+06 | 500299 | 756866 | 299 | 
| [glaze](https://github.com/stephenberry/glaze/commit/41ab7ce) | 1268.23 | 2.91427e+06 | 500299 | 1.458e+06 | 186 | 1787.93 | 2.06715e+06 | 500299 | 1.03419e+06 | 184 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/9b31492) | 230.542 | 1.60316e+07 | 492910 | 7.90214e+06 | 300 | 

### Twitter Test (Prettified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/TwitterData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Twitter%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Twitter%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 2346.7 | 1.57497e+06 | 719107 | 1.13257e+06 | 300 | 3139.16 | 1.17737e+06 | 719107 | 846653 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/41ab7ce) | 1163.79 | 3.17579e+06 | 719139 | 2.28384e+06 | 297 | 3094.88 | 1.19423e+06 | 719139 | 858816 | 300 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/9b31492) | 383.126 | 9.64682e+06 | 659630 | 6.36333e+06 | 300 | 

### Twitter Test (Minified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/TwitterData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Twitter%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Twitter%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 1976.95 | 1.86953e+06 | 477683 | 893041 | 265 | 3716.07 | 994588 | 477683 | 475098 | 211 | 
| [glaze](https://github.com/stephenberry/glaze/commit/41ab7ce) | 968.887 | 3.81466e+06 | 477715 | 1.82232e+06 | 298 | 2877.31 | 1.28451e+06 | 477715 | 613631 | 300 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/9b31492) | 248.561 | 1.48695e+07 | 425652 | 6.32923e+06 | 300 | 

### Minify Test Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Minify%20Test_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Minify%20Test_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ------------ | ------------------| -------------------- | --------------- | --------------------- |   
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 713.28 | 5.18164e+06 | 69037 | 357725 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/41ab7ce) | 574.618 | 6.43184e+06 | 69037 | 444035 | 300 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/9b31492) | 338.679 | 1.09129e+07 | 69037 | 753392 | 296 | 

### Prettify Test Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Prettify%20Test_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Prettify%20Test_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ------------ | ------------------| -------------------- | --------------- | --------------------- |   
| [glaze](https://github.com/stephenberry/glaze/commit/41ab7ce) | 1687.22 | 2.19057e+06 | 2132849 | 4.67216e+06 | 299 | 
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 1347.91 | 2.74199e+06 | 2132849 | 5.84825e+06 | 300 | 

### Validation Test Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Validate%20Test_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Validate%20Test_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- |   
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 3098.43 | 1.19285e+06 | 123050 | 146780 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/41ab7ce) | 1355.65 | 2.72633e+06 | 123050 | 335475 | 299 | 