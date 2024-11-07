# Json-Performance
Performance profiling of JSON libraries (Compiled and run on Windows 10.0.22631 using the MSVC 19.41.34123.0 compiler).  

Latest Results: (Nov 07, 2024)
#### Using the following commits:
----
| Jsonifier: [](https://github.com/RealTimeChris/Jsonifier/commit/)  
| Glaze: [665a0c7](https://github.com/stephenberry/glaze/commit/665a0c7)  
| Simdjson: [587beae](https://github.com/simdjson/simdjson/commit/587beae)  

 > At least 30 iterations on a (Intel(R) Core(TM) i7-8700K CPU @ 3.70G), until coefficient of variance is at or below 1%.


### Json Test (Prettified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/Windows-MSVC/JsonData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Json%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Json%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 179.977 | 2.05357e+07 | 1705228 | 3.50181e+07 | 300 | 219.663 | 1.68256e+07 | 1705228 | 2.86915e+07 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/665a0c7) | 125.599 | 2.94268e+07 | 1705228 | 5.01795e+07 | 300 | 156.082 | 2.36797e+07 | 1705228 | 4.03793e+07 | 300 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/587beae) | 10.209 | 3.62032e+08 | 1705228 | 6.17346e+08 | 300 | 

### Json Test (Minified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/Windows-MSVC/JsonData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Json%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Json%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 148.686 | 2.48575e+07 | 1224748 | 3.04442e+07 | 300 | 165.436 | 2.23408e+07 | 1224748 | 2.73619e+07 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/665a0c7) | 108.062 | 3.42022e+07 | 1224748 | 4.1889e+07 | 300 | 125.008 | 2.95657e+07 | 1224748 | 3.62106e+07 | 300 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/587beae) | 8.78786 | 4.20576e+08 | 1224748 | 5.151e+08 | 300 | 

### ABC Test (Out of Sequence Performance - Prettified) [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/Windows-MSVC/JsonData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Abc%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Abc%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>

The JSON documents in the previous tests featured keys ranging from "a" to "z," where each key corresponds to an array of values. Notably, the documents in this test arrange these keys in reverse order, deviating from the typical "a" to "z" arrangement.

This test effectively demonstrates the challenges encountered when utilizing simdjson and iterative parsers that lack the ability to efficiently allocate memory locations through hashing. In cases where the keys are not in the expected sequence, performance is significantly compromised, with the severity escalating as the document size increases.

In contrast, hash-based solutions offer a viable alternative by circumventing these issues and maintaining optimal performance regardless of the JSON document's scale, or ordering of the keys being parsed.

| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 183.375 | 2.01553e+07 | 1705228 | 3.43693e+07 | 300 | 208.954 | 1.7688e+07 | 1705228 | 3.0162e+07 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/665a0c7) | 113.73 | 3.24975e+07 | 1705228 | 5.54156e+07 | 300 | 134.065 | 2.75682e+07 | 1705228 | 4.70102e+07 | 300 | 


### ABC Test (Out of Sequence Performance - Minified) [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/Windows-MSVC/JsonData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Abc%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Abc%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 137.875 | 2.68066e+07 | 1224748 | 3.28313e+07 | 300 | 151.792 | 2.43488e+07 | 1224748 | 2.98212e+07 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/665a0c7) | 103.965 | 3.55499e+07 | 1224748 | 4.35397e+07 | 300 | 107.938 | 3.42418e+07 | 1224748 | 4.19375e+07 | 300 | 


### Discord Test (Prettified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Discord%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Discord%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 270.44 | 1.36665e+07 | 138774 | 1.89656e+06 | 300 | 658.296 | 5.61444e+06 | 138774 | 779138 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/665a0c7) | 157.523 | 2.3463e+07 | 138774 | 3.25606e+06 | 300 | 267.788 | 1.38018e+07 | 138774 | 1.91533e+06 | 300 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/587beae) | 14.4428 | 2.55904e+08 | 138482 | 3.54381e+07 | 300 | 

### Discord Test (Minified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Discord%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Discord%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 162.901 | 2.26884e+07 | 69037 | 1.56634e+06 | 300 | 449.649 | 8.21965e+06 | 69037 | 567460 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/665a0c7) | 112.79 | 3.27685e+07 | 69037 | 2.26224e+06 | 300 | 166.336 | 2.22195e+07 | 69037 | 1.53397e+06 | 300 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/587beae) | 8.31302 | 4.44601e+08 | 68745 | 3.05641e+07 | 300 | 

### Canada Test (Prettified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/CanadaData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Canada%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Canada%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 134.019 | 2.75779e+07 | 6661897 | 1.83721e+08 | 300 | 116.747 | 3.16581e+07 | 6661897 | 2.10903e+08 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/665a0c7) | 81.3317 | 4.54421e+07 | 6661897 | 3.02731e+08 | 300 | 85.83 | 4.30615e+07 | 6661897 | 2.86871e+08 | 300 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/587beae) | 18.3841 | 2.01041e+08 | 6661897 | 1.33931e+09 | 300 | 

### Canada Test (Minified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/CanadaData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Canada%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Canada%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 42.4767 | 8.7012e+07 | 2090234 | 1.81875e+08 | 300 | 27.9134 | 1.32409e+08 | 2090234 | 2.76765e+08 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/665a0c7) | 21.8621 | 1.69056e+08 | 2090234 | 3.53366e+08 | 300 | 34.7493 | 1.0636e+08 | 2090234 | 2.22317e+08 | 300 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/587beae) | 6.28618 | 5.87951e+08 | 2090234 | 1.22895e+09 | 300 | 

### CitmCatalog Test (Prettified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/CitmCatalogData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/CitmCatalog%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/CitmCatalog%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 312.36 | 1.18324e+07 | 1439562 | 1.70334e+07 | 300 | 371.635 | 9.94521e+06 | 1439562 | 1.43168e+07 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/665a0c7) | 118.789 | 3.11137e+07 | 1439584 | 4.47908e+07 | 300 | 161.818 | 2.28403e+07 | 1439584 | 3.28805e+07 | 300 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/587beae) | 19.2528 | 1.9197e+08 | 1428054 | 2.74144e+08 | 300 | 

### CitmCatalog Test (Minified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/CitmCatalogData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/CitmCatalog%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/CitmCatalog%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 171.709 | 2.15245e+07 | 500299 | 1.07687e+07 | 300 | 277.635 | 1.33123e+07 | 500299 | 6.66014e+06 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/665a0c7) | 85.4174 | 4.32694e+07 | 500299 | 2.16476e+07 | 300 | 130.23 | 2.83802e+07 | 500299 | 1.41986e+07 | 300 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/587beae) | 6.9092 | 5.35107e+08 | 497527 | 2.6623e+08 | 300 | 

### Twitter Test (Prettified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/TwitterData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Twitter%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Twitter%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 164.587 | 2.2456e+07 | 719107 | 1.61483e+07 | 300 | 506.91 | 7.29116e+06 | 719107 | 5.24312e+06 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/665a0c7) | 164.154 | 2.25153e+07 | 719139 | 1.61916e+07 | 300 | 207.457 | 1.78156e+07 | 719139 | 1.28119e+07 | 300 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/587beae) | 9.48933 | 3.89486e+08 | 516830 | 2.01298e+08 | 300 | 

### Twitter Test (Minified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/TwitterData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Twitter%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Twitter%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 134.982 | 2.73811e+07 | 477683 | 1.30795e+07 | 300 | 415.802 | 8.88873e+06 | 477683 | 4.24599e+06 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/665a0c7) | 109.899 | 3.36305e+07 | 477715 | 1.60658e+07 | 300 | 241.806 | 1.52848e+07 | 477715 | 7.30178e+06 | 300 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/587beae) | 5.65954 | 6.53051e+08 | 282852 | 1.84717e+08 | 300 | 

### Minify Test Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Minify%20Test_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Minify%20Test_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ------------ | ------------------| -------------------- | --------------- | --------------------- |   
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 90.6271 | 4.73229e+07 | 69037 | 3.26703e+06 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/665a0c7) | 70.4994 | 5.24257e+07 | 69037 | 3.61932e+06 | 300 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/587beae) | 7.54286 | 4.8999e+08 | 69037 | 3.38274e+07 | 300 | 

### Prettify Test Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Prettify%20Test_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Prettify%20Test_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ------------ | ------------------| -------------------- | --------------- | --------------------- |   
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 406.262 | 9.09752e+06 | 1705228 | 1.55134e+07 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/665a0c7) | 297.017 | 1.24434e+07 | 1705228 | 2.12189e+07 | 300 | 

### Validation Test Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Validate%20Test_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Validate%20Test_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- |   
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 292.937 | 1.26169e+07 | 123050 | 1.55251e+06 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/665a0c7) | 188.201 | 1.96381e+07 | 123050 | 2.41647e+06 | 300 | 