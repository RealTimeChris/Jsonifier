# Json-Performance
Performance profiling of JSON libraries (Compiled and run on Windows 10.0.22631 using the MSVC 19.41.34123.0 compiler).  

Latest Results: (Nov 06, 2024)
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
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 201.099 | 1.8379e+07 | 1648499 | 3.02977e+07 | 300 | 225.627 | 1.63809e+07 | 1648499 | 2.70038e+07 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/890bf43) | 134.593 | 2.74603e+07 | 1648499 | 4.52683e+07 | 254 | 202.897 | 1.82161e+07 | 1648499 | 3.00292e+07 | 297 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/587beae) | 12.0584 | 3.06506e+08 | 1648499 | 5.05276e+08 | 300 | 

### Json Test (Minified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/Windows-MSVC/JsonData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Json%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Json%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 136.256 | 2.71253e+07 | 1183512 | 3.21031e+07 | 300 | 163.009 | 2.26732e+07 | 1183512 | 2.6834e+07 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/890bf43) | 109.978 | 3.36064e+07 | 1183512 | 3.97736e+07 | 300 | 151.842 | 2.43409e+07 | 1183512 | 2.88078e+07 | 300 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/587beae) | 9.06438 | 4.07747e+08 | 1183512 | 4.82574e+08 | 294 | 

### ABC Test (Out of Sequence Performance - Prettified) [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/Windows-MSVC/JsonData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Abc%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Abc%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>

The JSON documents in the previous tests featured keys ranging from "a" to "z," where each key corresponds to an array of values. Notably, the documents in this test arrange these keys in reverse order, deviating from the typical "a" to "z" arrangement.

This test effectively demonstrates the challenges encountered when utilizing simdjson and iterative parsers that lack the ability to efficiently allocate memory locations through hashing. In cases where the keys are not in the expected sequence, performance is significantly compromised, with the severity escalating as the document size increases.

In contrast, hash-based solutions offer a viable alternative by circumventing these issues and maintaining optimal performance regardless of the JSON document's scale, or ordering of the keys being parsed.

| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 200.364 | 1.84463e+07 | 1648499 | 3.04088e+07 | 300 | 212.223 | 1.74155e+07 | 1648499 | 2.87095e+07 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/890bf43) | 131.378 | 2.81324e+07 | 1648499 | 4.63763e+07 | 300 | 195.981 | 1.88588e+07 | 1648499 | 3.10887e+07 | 300 | 


### ABC Test (Out of Sequence Performance - Minified) [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/Windows-MSVC/JsonData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Abc%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Abc%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 139.562 | 2.64825e+07 | 1183512 | 3.13424e+07 | 300 | 163.488 | 2.26066e+07 | 1183512 | 2.67552e+07 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/890bf43) | 109.202 | 3.38454e+07 | 1183512 | 4.00564e+07 | 300 | 149.669 | 2.46942e+07 | 1183512 | 2.92259e+07 | 300 | 


### Discord Test (Prettified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Discord%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Discord%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 271.897 | 1.35933e+07 | 138774 | 1.88639e+06 | 300 | 682.242 | 5.41737e+06 | 138774 | 751790 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/890bf43) | 154.129 | 2.39797e+07 | 138774 | 3.32776e+06 | 300 | 261.725 | 1.41215e+07 | 138774 | 1.95969e+06 | 300 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/587beae) | 14.8773 | 2.48431e+08 | 138482 | 3.44032e+07 | 300 | 

### Discord Test (Minified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Discord%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Discord%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 168.42 | 2.1945e+07 | 69037 | 1.51502e+06 | 259 | 481.18 | 7.6812e+06 | 69037 | 530287 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/890bf43) | 111.987 | 3.30035e+07 | 69037 | 2.27846e+06 | 295 | 193.647 | 1.90861e+07 | 69037 | 1.31765e+06 | 188 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/587beae) | 8.1098 | 4.5574e+08 | 68745 | 3.13298e+07 | 300 | 

### Canada Test (Prettified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/CanadaData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Canada%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Canada%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 137.862 | 2.6809e+07 | 6661897 | 1.78599e+08 | 252 | 121.756 | 3.03558e+07 | 6661897 | 2.02227e+08 | 296 | 
| [glaze](https://github.com/stephenberry/glaze/commit/890bf43) | 89.5087 | 4.1291e+07 | 6661897 | 2.75076e+08 | 298 | 107.225 | 3.44692e+07 | 6661897 | 2.2963e+08 | 298 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/587beae) | 19.826 | 1.8642e+08 | 6661897 | 1.24191e+09 | 300 | 

### Canada Test (Minified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/CanadaData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Canada%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Canada%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 44.1448 | 8.37275e+07 | 2090234 | 1.7501e+08 | 300 | 39.1474 | 9.44113e+07 | 2090234 | 1.97342e+08 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/890bf43) | 28.3696 | 1.30279e+08 | 2090234 | 2.72314e+08 | 300 | 35.6607 | 1.03643e+08 | 2090234 | 2.16639e+08 | 172 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/587beae) | 6.56366 | 5.63097e+08 | 2090234 | 1.177e+09 | 300 | 

### CitmCatalog Test (Prettified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/CitmCatalogData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/CitmCatalog%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/CitmCatalog%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 339.661 | 1.08813e+07 | 1439562 | 1.56643e+07 | 300 | 450.954 | 8.19589e+06 | 1439562 | 1.17985e+07 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/890bf43) | 206.67 | 1.78835e+07 | 1439584 | 2.57448e+07 | 300 | 358.014 | 1.03235e+07 | 1439584 | 1.48616e+07 | 300 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/587beae) | 21.3694 | 1.72956e+08 | 1428054 | 2.46991e+08 | 300 | 

### CitmCatalog Test (Minified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/CitmCatalogData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/CitmCatalog%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/CitmCatalog%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 168.658 | 2.19141e+07 | 500299 | 1.09636e+07 | 300 | 335.227 | 1.10252e+07 | 500299 | 5.51592e+06 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/890bf43) | 112.409 | 3.28796e+07 | 500299 | 1.64496e+07 | 300 | 172.853 | 2.1382e+07 | 500299 | 1.06974e+07 | 300 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/587beae) | 9.08547 | 4.068e+08 | 497527 | 2.02394e+08 | 300 | 

### Twitter Test (Prettified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/TwitterData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Twitter%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Twitter%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 256.76 | 1.43946e+07 | 719107 | 1.03513e+07 | 300 | 619.246 | 5.96853e+06 | 719107 | 4.29201e+06 | 246 | 
| [glaze](https://github.com/stephenberry/glaze/commit/890bf43) | 174.027 | 2.12381e+07 | 719139 | 1.52731e+07 | 300 | 358.527 | 1.03087e+07 | 719139 | 7.41342e+06 | 299 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/587beae) | 10.4708 | 3.5298e+08 | 516830 | 1.8243e+08 | 240 | 

### Twitter Test (Minified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/TwitterData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Twitter%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Twitter%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 211.114 | 1.75069e+07 | 477683 | 8.36277e+06 | 297 | 517.954 | 7.13574e+06 | 477683 | 3.40862e+06 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/890bf43) | 157.975 | 2.33958e+07 | 477715 | 1.11765e+07 | 300 | 299.459 | 1.23422e+07 | 477715 | 5.89604e+06 | 300 | 


### Minify Test Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Minify%20Test_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Minify%20Test_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ------------ | ------------------| -------------------- | --------------- | --------------------- |   
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 131.924 | 2.80159e+07 | 69037 | 1.93414e+06 | 296 | 
| [glaze](https://github.com/stephenberry/glaze/commit/890bf43) | 74.9709 | 4.9711e+07 | 69037 | 3.4319e+06 | 300 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/587beae) | 8.83232 | 4.18457e+08 | 69037 | 2.8889e+07 | 190 | 

### Prettify Test Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Prettify%20Test_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Prettify%20Test_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ------------ | ------------------| -------------------- | --------------- | --------------------- |   
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 459.902 | 8.03642e+06 | 1648499 | 1.3248e+07 | 297 | 
| [glaze](https://github.com/stephenberry/glaze/commit/890bf43) | 349.982 | 1.05604e+07 | 1648499 | 1.74089e+07 | 299 | 

### Validation Test Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Validate%20Test_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Validate%20Test_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- |   
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 346.368 | 1.06706e+07 | 123050 | 1.31302e+06 | 296 | 
| [glaze](https://github.com/stephenberry/glaze/commit/890bf43) | 300.722 | 1.22903e+07 | 123050 | 1.51233e+06 | 299 | 