# Json-Performance
Performance profiling of JSON libraries (Compiled and run on Windows 10.0.22631 using the MSVC 19.41.34123.0 compiler).  

Latest Results: (Nov 26, 2024)
#### Using the following commits:
----
| Jsonifier: [](https://github.com/RealTimeChris/Jsonifier/commit/)  
| Glaze: [8f35df4](https://github.com/stephenberry/glaze/commit/8f35df4)  
| Simdjson: [9b31492](https://github.com/simdjson/simdjson/commit/9b31492)  

 > " + 100 iterations on a (Intel(R) Core(TM) i7-8700K CPU @ 3.70G).


### Json Test (Prettified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/Windows-MSVC/JsonData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Json%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Json%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read Variation (+/-%) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write Variation (+/-%) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | --------------------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ---------------------- | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 251.971 | 8.35728 | 4.13001e+08 | 2194697 | 9.06411e+08 | 100 | 276.555 | 10.91 | 3.87091e+08 | 2194697 | 8.49547e+08 | 100 | 
| [glaze](https://github.com/stephenberry/glaze/commit/8f35df4) | 159.766 | 10.6878 | 6.68353e+08 | 2194697 | 1.46683e+09 | 100 | 245.79 | 8.402 | 4.23596e+08 | 2194697 | 9.29664e+08 | 100 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/9b31492) | 12.9506 | 3.8788 | 7.66111e+09 | 2194697 | 1.68138e+10 | 100 | 

### Json Test (Minified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/Windows-MSVC/JsonData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Json%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Json%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read Variation (+/-%) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write Variation (+/-%) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | --------------------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ---------------------- | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 206.223 | 8.24359 | 5.03995e+08 | 1701663 | 8.5763e+08 | 100 | 230.182 | 9.617 | 4.58396e+08 | 1701663 | 7.80036e+08 | 100 | 
| [glaze](https://github.com/stephenberry/glaze/commit/8f35df4) | 143.408 | 8.46254 | 7.26488e+08 | 1701663 | 1.23624e+09 | 100 | 203.309 | 5.407 | 4.95892e+08 | 1701663 | 8.4384e+08 | 100 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/9b31492) | 10.7846 | 3.57869 | 9.17116e+09 | 1701663 | 1.56062e+10 | 100 | 