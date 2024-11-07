# Json-Performance
Performance profiling of JSON libraries (Compiled and run on Windows 10.0.22631 using the MSVC 19.41.34123.0 compiler).  

Latest Results: (Nov 11, 2024)
#### Using the following commits:
----
| Jsonifier: [](https://github.com/RealTimeChris/Jsonifier/commit/)  
| Glaze: [952e960](https://github.com/stephenberry/glaze/commit/952e960)  
| Simdjson: [9b31492](https://github.com/simdjson/simdjson/commit/9b31492)  

 > At least 30 iterations on a (Intel(R) Core(TM) i7-8700K CPU @ 3.70G), until coefficient of variance is at or below 1%.


### Json Test (Prettified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/Windows-MSVC/JsonData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Json%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Json%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 226.271 | 1.63342e+07 | 2049579 | 3.34783e+07 | 300 | 243.522 | 1.51771e+07 | 2049579 | 3.11068e+07 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/952e960) | 85.7118 | 4.31208e+07 | 2049579 | 8.83796e+07 | 300 | 190.843 | 1.93665e+07 | 2049579 | 3.96932e+07 | 300 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/9b31492) | 8.84722 | 4.17756e+08 | 2049579 | 8.56223e+08 | 300 | 

### Json Test (Minified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/Windows-MSVC/JsonData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Json%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Json%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [glaze](https://github.com/stephenberry/glaze/commit/952e960) | 131.256 | 2.81586e+07 | 1570846 | 4.42328e+07 | 300 | 182.994 | 2.01972e+07 | 1570846 | 3.17267e+07 | 300 | 
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 116.945 | 3.16044e+07 | 1570846 | 4.96457e+07 | 300 | 185.892 | 1.98823e+07 | 1570846 | 3.1232e+07 | 300 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/9b31492) | 8.15157 | 4.53405e+08 | 1570846 | 7.1223e+08 | 300 | 

### ABC Test (Out of Sequence Performance - Prettified) [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/Windows-MSVC/JsonData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Abc%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Abc%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>

The JSON documents in the previous tests featured keys ranging from "a" to "z," where each key corresponds to an array of values. Notably, the documents in this test arrange these keys in reverse order, deviating from the typical "a" to "z" arrangement.

This test effectively demonstrates the challenges encountered when utilizing simdjson and iterative parsers that lack the ability to efficiently allocate memory locations through hashing. In cases where the keys are not in the expected sequence, performance is significantly compromised, with the severity escalating as the document size increases.

In contrast, hash-based solutions offer a viable alternative by circumventing these issues and maintaining optimal performance regardless of the JSON document's scale, or ordering of the keys being parsed.

| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 221.008 | 1.67232e+07 | 2049579 | 3.42756e+07 | 300 | 255.157 | 1.4485e+07 | 2049579 | 2.96882e+07 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/952e960) | 146.581 | 2.52145e+07 | 2049579 | 5.16791e+07 | 300 | 225.238 | 1.64092e+07 | 2049579 | 3.3632e+07 | 300 | 


### ABC Test (Out of Sequence Performance - Minified) [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/Windows-MSVC/JsonData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Abc%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Abc%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 176.522 | 2.09378e+07 | 1570846 | 3.289e+07 | 300 | 188.901 | 1.95655e+07 | 1570846 | 3.07343e+07 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/952e960) | 121.441 | 3.04342e+07 | 1570846 | 4.78075e+07 | 300 | 172.659 | 2.14062e+07 | 1570846 | 3.36259e+07 | 300 | 


### Discord Test (Prettified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Discord%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Discord%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 242.288 | 1.52544e+07 | 138774 | 2.11692e+06 | 300 | 624.869 | 5.91479e+06 | 138774 | 820819 | 299 | 
| [glaze](https://github.com/stephenberry/glaze/commit/952e960) | 154.955 | 2.38519e+07 | 138774 | 3.31002e+06 | 300 | 177.529 | 2.08188e+07 | 138774 | 2.88911e+06 | 300 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/9b31492) | 14.6332 | 2.52576e+08 | 138482 | 3.49772e+07 | 300 | 

### Discord Test (Minified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Discord%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Discord%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 153.797 | 2.40316e+07 | 69037 | 1.65907e+06 | 300 | 440.087 | 8.39828e+06 | 69037 | 579792 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/952e960) | 65.4815 | 5.6443e+07 | 69037 | 3.89665e+06 | 300 | 112.904 | 3.27354e+07 | 69037 | 2.25995e+06 | 300 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/9b31492) | 8.4417 | 4.37823e+08 | 68745 | 3.00982e+07 | 300 | 

### Canada Test (Prettified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/CanadaData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Canada%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Canada%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 110.8 | 3.33569e+07 | 6661897 | 2.2222e+08 | 300 | 90.6949 | 4.07517e+07 | 6661897 | 2.71484e+08 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/952e960) | 57.305 | 6.44945e+07 | 6661897 | 4.29656e+08 | 300 | 78.2002 | 4.72629e+07 | 6661897 | 3.14861e+08 | 300 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/9b31492) | 17.2221 | 2.14604e+08 | 6661897 | 1.42967e+09 | 300 | 

### Canada Test (Minified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/CanadaData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Canada%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Canada%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 40.9571 | 9.024e+07 | 2090234 | 1.88623e+08 | 300 | 35.5677 | 1.03914e+08 | 2090234 | 2.17204e+08 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/952e960) | 26.6462 | 1.38703e+08 | 2090234 | 2.89921e+08 | 300 | 32.9362 | 1.12216e+08 | 2090234 | 2.34557e+08 | 300 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/9b31492) | 5.55199 | 6.65702e+08 | 2090234 | 1.39147e+09 | 300 | 

### CitmCatalog Test (Prettified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/CitmCatalogData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/CitmCatalog%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/CitmCatalog%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 324.172 | 1.14013e+07 | 1439562 | 1.64129e+07 | 300 | 628.386 | 5.88164e+06 | 1439562 | 8.46699e+06 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/952e960) | 155.211 | 2.38127e+07 | 1439584 | 3.42803e+07 | 300 | 339.805 | 1.08768e+07 | 1439584 | 1.56581e+07 | 300 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/9b31492) | 19.0664 | 1.93847e+08 | 1428054 | 2.76825e+08 | 300 | 

### CitmCatalog Test (Minified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/CitmCatalogData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/CitmCatalog%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/CitmCatalog%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 176.248 | 2.09703e+07 | 500299 | 1.04914e+07 | 300 | 272.349 | 1.35707e+07 | 500299 | 6.78943e+06 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/952e960) | 105.239 | 3.51199e+07 | 500299 | 1.75704e+07 | 300 | 160.117 | 2.3083e+07 | 500299 | 1.15484e+07 | 300 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/9b31492) | 7.43467 | 4.97128e+08 | 497527 | 2.47335e+08 | 300 | 

### Twitter Test (Prettified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/TwitterData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Twitter%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Twitter%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 225.2 | 1.6412e+07 | 719107 | 1.1802e+07 | 300 | 590.683 | 6.25711e+06 | 719107 | 4.49953e+06 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/952e960) | 172.277 | 2.14537e+07 | 719139 | 1.54282e+07 | 300 | 251.066 | 1.47211e+07 | 719139 | 1.05865e+07 | 300 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/9b31492) | 9.7724 | 3.78206e+08 | 516830 | 1.95468e+08 | 300 | 

### Twitter Test (Minified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/TwitterData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Twitter%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Twitter%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 179.122 | 2.06339e+07 | 477683 | 9.85645e+06 | 300 | 485.606 | 7.61102e+06 | 477683 | 3.63566e+06 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/952e960) | 125.52 | 2.94454e+07 | 477715 | 1.40665e+07 | 300 | 165.857 | 2.22841e+07 | 477715 | 1.06454e+07 | 300 | 


### Minify Test Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Minify%20Test_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Minify%20Test_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ------------ | ------------------| -------------------- | --------------- | --------------------- |   
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 137.82 | 2.68173e+07 | 69037 | 1.85139e+06 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/952e960) | 72.0782 | 5.12774e+07 | 69037 | 3.54004e+06 | 291 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/9b31492) | 8.29993 | 4.45299e+08 | 69037 | 3.07421e+07 | 300 | 

### Prettify Test Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Prettify%20Test_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Prettify%20Test_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ------------ | ------------------| -------------------- | --------------- | --------------------- |   
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 422.215 | 8.75579e+06 | 2049579 | 1.79457e+07 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/952e960) | 381.033 | 9.69983e+06 | 2049579 | 1.98806e+07 | 300 | 

### Validation Test Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Validate%20Test_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Validate%20Test_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- |   
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 301.379 | 1.22636e+07 | 123050 | 1.50903e+06 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/952e960) | 255.784 | 1.44496e+07 | 123050 | 1.77802e+06 | 300 | 