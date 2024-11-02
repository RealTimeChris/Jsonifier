# Json-Performance
Performance profiling of JSON libraries (Compiled and run on Windows 10.0.22631 using the MSVC 19.41.34123.0 compiler).  

Latest Results: (Nov 02, 2024)
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



### Discord Test (Prettified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Discord%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Discord%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [glaze](https://github.com/stephenberry/glaze/commit/890bf43) | 163.363 | 2.26244e+07 | 138774 | 3.13967e+06 | 300 | 299.061 | 1.23586e+07 | 138774 | 1.71505e+06 | 300 | 
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 134.006 | 2.75806e+07 | 138774 | 3.82748e+06 | 300 | 608.237 | 6.07654e+06 | 138774 | 843266 | 296 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/587beae) | 14.9687 | 2.46913e+08 | 138482 | 3.4193e+07 | 300 | 

### Discord Test (Minified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Discord%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Discord%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/S) | Read (Cycles/MB) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/S) | Write (Cycles/MB) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | -----------------| ------------------- | -------------- | -------------------- | ------------ | ------------------| -------------------- | ----------------| --------------------- |  
| [jsonifier](https://github.com/realtimechris/jsonifier/commit/) | 161.881 | 2.28314e+07 | 69037 | 1.57621e+06 | 300 | 262.171 | 1.40976e+07 | 69037 | 973254 | 300 | 
| [glaze](https://github.com/stephenberry/glaze/commit/890bf43) | 105.144 | 3.51516e+07 | 69037 | 2.42676e+06 | 300 | 195.502 | 1.8905e+07 | 69037 | 1.30514e+06 | 300 | 
| [simdjson](https://github.com/simdjson/simdjson/commit/587beae) | 8.0751 | 4.577e+08 | 68745 | 3.14646e+07 | 300 | 