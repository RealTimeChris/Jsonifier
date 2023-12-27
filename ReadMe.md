# Json-Performance
Performance profiling of JSON libraries (Compiled and run on Ubuntu-22.04 using the Clang++18 compiler)

Latest Results: (Apr 25, 2024)

### Single Iteration Test (Prettified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/JsonData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Single%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Single%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>

> 1 iteration on a 6 core (Intel i7 8700k)

| Library | Read (MB/s) | Write (MB/s) |
| ------------------------------------------------- | ---------- | ----------- |
| [jsonifier](https://github.com/realtimechris/jsonifier) | 356.614 | 829.069 |
| [glaze](https://github.com/stephenberry/glaze) | 315.691 | 848.636 |
| [simdjson](https://github.com/simdjson/simdjson) | 239.192 | N/A |)

### Single Iteration Test (Minified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/JsonData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Single%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Single%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/s) | Write (MB/s) |
| ------------------------------------------------- | ---------- | ----------- |
| [glaze](https://github.com/stephenberry/glaze) | 195.53 | 615.55 |
| [jsonifier](https://github.com/realtimechris/jsonifier) | 189.846 | 623.526 |
| [simdjson](https://github.com/simdjson/simdjson) | 167.88 | N/A |
> 100 iterations on a 6 core (Intel i7 8700k)

### Multi Iteration Test (Prettified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/JsonData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Multi%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Multi%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/s) | Write (MB/s) |
| ------------------------------------------------- | ---------- | ----------- |
| [jsonifier](https://github.com/realtimechris/jsonifier) | 1003.18 | 924.876 |
| [glaze](https://github.com/stephenberry/glaze) | 853.278 | 865.395 |
| [simdjson](https://github.com/simdjson/simdjson) | 316.77 | N/A |
> 100 iterations on a 6 core (Intel i7 8700k)

### Multi Iteration Test (Minified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/JsonData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Multi%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Multi%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/s) | Write (MB/s) |
| ------------------------------------------------- | ---------- | ----------- |
| [jsonifier](https://github.com/realtimechris/jsonifier) | 692.182 | 900.03 |
| [glaze](https://github.com/stephenberry/glaze) | 670.069 | 889.348 |
| [simdjson](https://github.com/simdjson/simdjson) | 206.151 | N/A |
> 100 iterations on a 6 core (Intel i7 8700k)

### ABC Test (Out of Sequence Performance - Prettified) [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/JsonData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Abc%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Abc%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>

The JSON documents in the previous tests featured keys ranging from "a" to "z," where each key corresponds to an array of values. Notably, the documents in this test arrange these keys in reverse order, deviating from the typical "a" to "z" arrangement.

This test effectively demonstrates the challenges encountered when utilizing simdjson and iterative parsers that lack the ability to efficiently allocate memory locations through hashing. In cases where the keys are not in the expected sequence, performance is significantly compromised, with the severity escalating as the document size increases.

In contrast, hash-based solutions offer a viable alternative by circumventing these issues and maintaining optimal performance regardless of the JSON document's scale, or ordering of the keys being parsed.

| Library | Read (MB/s) | Write (MB/s) |
| ------------------------------------------------- | ---------- | ----------- |
| [jsonifier](https://github.com/realtimechris/jsonifier) | 1003.42 | 933.851 |
| [glaze](https://github.com/stephenberry/glaze) | 864.486 | 931.326 |
| [simdjson](https://github.com/simdjson/simdjson) | 191.226 | N/A |
> 100 iterations on a 6 core (Intel i7 8700k)

### ABC Test (Out of Sequence Performance - Minified) [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/JsonData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Abc%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Abc%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/s) | Write (MB/s) |
| ------------------------------------------------- | ---------- | ----------- |
| [jsonifier](https://github.com/realtimechris/jsonifier) | 686.182 | 921.49 |
| [glaze](https://github.com/stephenberry/glaze) | 675.719 | 919.979 |
| [simdjson](https://github.com/simdjson/simdjson) | 119.657 | N/A |
> 100 iterations on a 6 core (Intel i7 8700k)

### Discord Test (Prettified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Discord%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Discord%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/s) | Write (MB/s) |
| ------------------------------------------------- | ---------- | ----------- |
| [jsonifier](https://github.com/realtimechris/jsonifier) | 1223.81 | 1265 |
| [glaze](https://github.com/stephenberry/glaze) | 1038 | 1775.02 |
| [simdjson](https://github.com/simdjson/simdjson) | 120.254 | N/A |
> 100 iterations on a 6 core (Intel i7 8700k)

### Discord Test (Minified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Discord%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Discord%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/s) | Write (MB/s) |
| ------------------------------------------------- | ---------- | ----------- |
| [glaze](https://github.com/stephenberry/glaze) | 713.192 | 1741.84 |
| [jsonifier](https://github.com/realtimechris/jsonifier) | 708.098 | 1262.14 |
| [simdjson](https://github.com/simdjson/simdjson) | 58.8943 | N/A |
> 100 iterations on a 6 core (Intel i7 8700k)

### Twitter Test (Prettified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/TwitterData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Twitter%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Twitter%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/s) | Write (MB/s) |
| ------------------------------------------------- | ---------- | ----------- |
| [jsonifier](https://github.com/realtimechris/jsonifier) | 1296.84 | 1957.2 |
| [glaze](https://github.com/stephenberry/glaze) | 1228.36 | 2458.79 |
| [simdjson](https://github.com/simdjson/simdjson) | 478.763 | N/A |
> 100 iterations on a 6 core (Intel i7 8700k)

### Twitter Test (Minified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/TwitterData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Twitter%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Twitter%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/s) | Write (MB/s) |
| ------------------------------------------------- | ---------- | ----------- |
| [glaze](https://github.com/stephenberry/glaze) | 1080.05 | 2451.4 |
| [jsonifier](https://github.com/realtimechris/jsonifier) | 919.649 | 1961.91 |
| [simdjson](https://github.com/simdjson/simdjson) | 339.964 | N/A |
> 100 iterations on a 6 core (Intel i7 8700k)

### Minify Test Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Minify%20Test_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Minify%20Test_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/s) | Write (MB/s) |
| ------------------------------------------------- | ---------- | ----------- |
| [glaze](https://github.com/stephenberry/glaze) | N/A | 1515.87 |
| [jsonifier](https://github.com/realtimechris/jsonifier) | N/A | 1110.45 |
| [simdjson](https://github.com/simdjson/simdjson) | N/A | 427.504 |
> 100 iterations on a 6 core (Intel i7 8700k)

### Prettify Test Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Prettify%20Test_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Prettify%20Test_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/s) | Write (MB/s) |
| ------------------------------------------------- | ---------- | ----------- |
| [glaze](https://github.com/stephenberry/glaze) | N/A | 643.204 |
| [jsonifier](https://github.com/realtimechris/jsonifier) | N/A | 500.36 |
> 100 iterations on a 6 core (Intel i7 8700k)

### Validation Test Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Validate%20Test_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Validate%20Test_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/s) | Write (MB/s) |
| ------------------------------------------------- | ---------- | ----------- |
| [jsonifier](https://github.com/realtimechris/jsonifier) | 932.267 | N/A |