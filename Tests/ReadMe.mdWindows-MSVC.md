# Json-Performance
Performance profiling of JSON libraries (Compiled and run on Windows 10.0.22631 using the MSVC 19.41.34123.0 compiler).  

Latest Results: (Nov 13, 2024)
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
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 222.118 | 1.66397e+07 | 2078942 | 3.4593e+07 | 300 | 257.932 | 1.43292e+07 | 2078942 | 2.97895e+07 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/952e960) | 152.669 | 2.4209e+07 | 2078942 | 5.03291e+07 | 300 | 233.422 | 1.58339e+07 | 2078942 | 3.29177e+07 | 300 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/9b31492) | 12.7504 | 2.89872e+08 | 2078942 | 6.02626e+08 | 300 | 

### Json Test (Minified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/Windows-MSVC/JsonData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Json%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Json%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [glaze](https://github.com/stephenberry/glaze/commit/952e960) | 134.233 | 2.75368e+07 | 1594818 | 4.39162e+07 | 300 | 194.241 | 1.90277e+07 | 1594818 | 3.03457e+07 | 300 | 
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 129.255 | 2.85944e+07 | 1594818 | 4.56029e+07 | 300 | 205.495 | 1.79857e+07 | 1594818 | 2.86839e+07 | 300 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/9b31492) | 10.4477 | 3.53761e+08 | 1594818 | 5.64185e+08 | 300 | 

### Partial Test (Prettified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/Windows-MSVC/JsonData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Partial%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Partial%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 39.2159 | 9.42541e+07 | 76764 | 7.23532e+06 | 300 | 290.813 | 1.27089e+07 | 76764 | 975585 | 299 | 
| [glaze](https://github.com/stephenberry/glaze/commit/952e960) | 22.142 | 1.66921e+08 | 76764 | 1.28136e+07 | 298 | 255.547 | 1.4463e+07 | 76764 | 1.11024e+06 | 168 | 


### Partial Test (Minified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/Windows-MSVC/JsonData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Partial%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Partial%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 41.2557 | 8.9587e+07 | 58391 | 5.23108e+06 | 300 | 228.587 | 1.61688e+07 | 58391 | 944114 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/952e960) | 21.9288 | 1.68545e+08 | 58391 | 9.84149e+06 | 298 | 209.101 | 1.76757e+07 | 58391 | 1.0321e+06 | 291 | 
