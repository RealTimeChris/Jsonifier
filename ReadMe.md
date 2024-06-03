# Json-Performance
Performance profiling of JSON libraries (Compiled and run on Ubuntu-22.04 using the Clang++18 compiler)

Latest Results: (Jun 03, 2024)

 > At least 100 iterations on a 6 core (Intel i7 9750h), until Median Absolute Percentage Error (Mape) reduced below 5.0%.


### Json Test (Prettified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/JsonData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Json%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Json%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/s) | ReadLength (Bytes) | ReadTime (ns) | ReadMape (%) | Write (MB/s) | WriteLength (Bytes) | WriteTime (ns) | WriteMape (%) |
| ------------------------------------------------- | ---------- | ----------- | ---------- | ----------- | ---------- | ---------- | ---------- | ---------- |
| [jsonifier](https://github.com/realtimechris/jsonifier) | 751.186 | 3506536 | 4.668e+06 | 0.5539 | 811.914 | 3501459 | 4.3126e+06 | 0.5226 |
| [glaze](https://github.com/stephenberry/glaze) | 701.602 | 3506536 | 4.9979e+06 | 0.1359 | 930.724 | 3505944 | 3.7669e+06 | 1.25 |
| [simdjson](https://github.com/simdjson/simdjson) | 231.51 | 3506536 | 1.51464e+07 | 4.73 | N/A | N/A | N/A | N/A |

### Json Test (Minified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/JsonData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Json%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Json%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/s) | ReadLength (Bytes) | ReadTime (ns) | ReadMape (%) | Write (MB/s) | WriteLength (Bytes) | WriteTime (ns) | WriteMape (%) |
| ------------------------------------------------- | ---------- | ----------- | ---------- | ----------- | ---------- | ---------- | ---------- | ---------- |
| [jsonifier](https://github.com/realtimechris/jsonifier) | 717.098 | 2399409 | 3.346e+06 | 4.136 | 697.162 | 2394332 | 3.4344e+06 | 3.14 |
| [glaze](https://github.com/stephenberry/glaze) | 590.871 | 2399409 | 4.0608e+06 | 2.033 | 698.325 | 2398817 | 3.4351e+06 | 0.1776 |
| [simdjson](https://github.com/simdjson/simdjson) | 158.644 | 2399409 | 1.51245e+07 | 3.597 | N/A | N/A | N/A | N/A |

### ABC Test (Out of Sequence Performance - Prettified) [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/JsonData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Abc%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Abc%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>

The JSON documents in the previous tests featured keys ranging from "a" to "z," where each key corresponds to an array of values. Notably, the documents in this test arrange these keys in reverse order, deviating from the typical "a" to "z" arrangement.

This test effectively demonstrates the challenges encountered when utilizing simdjson and iterative parsers that lack the ability to efficiently allocate memory locations through hashing. In cases where the keys are not in the expected sequence, performance is significantly compromised, with the severity escalating as the document size increases.

In contrast, hash-based solutions offer a viable alternative by circumventing these issues and maintaining optimal performance regardless of the JSON document's scale, or ordering of the keys being parsed.

| Library | Read (MB/s) | ReadLength (Bytes) | ReadTime (ns) | ReadMape (%) | Write (MB/s) | WriteLength (Bytes) | WriteTime (ns) | WriteMape (%) |
| ------------------------------------------------- | ---------- | ----------- | ---------- | ----------- | ---------- | ---------- | ---------- | ---------- |
| [jsonifier](https://github.com/realtimechris/jsonifier) | 588.612 | 3506536 | 5.9573e+06 | 2.813 | 610.659 | 3501459 | 5.7339e+06 | 1.763 |
| [glaze](https://github.com/stephenberry/glaze) | 550.658 | 3506536 | 6.3679e+06 | 0.8543 | 763.972 | 3505944 | 4.5891e+06 | 0.4891 |
| [simdjson](https://github.com/simdjson/simdjson) | 119.351 | 3506536 | 2.938e+07 | 0.9769 | N/A | N/A | N/A | N/A |

### ABC Test (Out of Sequence Performance - Minified) [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/JsonData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Abc%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Abc%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/s) | ReadLength (Bytes) | ReadTime (ns) | ReadMape (%) | Write (MB/s) | WriteLength (Bytes) | WriteTime (ns) | WriteMape (%) |
| ------------------------------------------------- | ---------- | ----------- | ---------- | ----------- | ---------- | ---------- | ---------- | ---------- |
| [jsonifier](https://github.com/realtimechris/jsonifier) | 894.501 | 2399409 | 2.6824e+06 | 0.5182 | 685.76 | 2394332 | 3.4915e+06 | 0.6692 |
| [glaze](https://github.com/stephenberry/glaze) | 648.84 | 2399409 | 3.698e+06 | 0.4413 | 673.031 | 2398817 | 3.5642e+06 | 4.346 |
| [simdjson](https://github.com/simdjson/simdjson) | 97.5381 | 2399409 | 2.45997e+07 | 2.427 | N/A | N/A | N/A | N/A |

### Discord Test (Prettified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Discord%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Discord%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/s) | ReadLength (Bytes) | ReadTime (ns) | ReadMape (%) | Write (MB/s) | WriteLength (Bytes) | WriteTime (ns) | WriteMape (%) |
| ------------------------------------------------- | ---------- | ----------- | ---------- | ----------- | ---------- | ---------- | ---------- | ---------- |
| [jsonifier](https://github.com/realtimechris/jsonifier) | 888.839 | 133948 | 150700 | 4.844 | 778.315 | 133948 | 172100 | 2.673 |
| [glaze](https://github.com/stephenberry/glaze) | 817.753 | 133948 | 163800 | 0.5047 | 1850.35 | 133965 | 72400 | 0.411 |
| [simdjson](https://github.com/simdjson/simdjson) | 301.481 | 133948 | 444300 | 0.3812 | N/A | N/A | N/A | N/A |

### Discord Test (Minified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Discord%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Discord%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/s) | ReadLength (Bytes) | ReadTime (ns) | ReadMape (%) | Write (MB/s) | WriteLength (Bytes) | WriteTime (ns) | WriteMape (%) |
| ------------------------------------------------- | ---------- | ----------- | ---------- | ----------- | ---------- | ---------- | ---------- | ---------- |
| [glaze](https://github.com/stephenberry/glaze) | 560.462 | 71571 | 127700 | 0 | 1219.27 | 71571 | 58700 | 1.495 |
| [jsonifier](https://github.com/realtimechris/jsonifier) | 330.582 | 71571 | 216500 | 1.565 | 636.187 | 71571 | 112500 | 0.08889 |
| [simdjson](https://github.com/simdjson/simdjson) | 189.091 | 71571 | 378500 | 1.693 | N/A | N/A | N/A | N/A |

### Canada Test (Prettified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/CanadaData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Canada%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Canada%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/s) | ReadLength (Bytes) | ReadTime (ns) | ReadMape (%) | Write (MB/s) | WriteLength (Bytes) | WriteTime (ns) | WriteMape (%) |
| ------------------------------------------------- | ---------- | ----------- | ---------- | ----------- | ---------- | ---------- | ---------- | ---------- |
| [jsonifier](https://github.com/realtimechris/jsonifier) | 1109.74 | 6661897 | 6.0031e+06 | 1.469 | 1029.04 | 6661897 | 6.4739e+06 | 4.034 |
| [glaze](https://github.com/stephenberry/glaze) | 1082.78 | 6661897 | 6.1526e+06 | 3.184 | 962.41 | 6661897 | 6.9221e+06 | 0.2199 |
| [simdjson](https://github.com/simdjson/simdjson) | 285.703 | 6661897 | 2.33176e+07 | 0.3871 | N/A | N/A | N/A | N/A |

### Canada Test (Minified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/CanadaData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Canada%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Canada%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/s) | ReadLength (Bytes) | ReadTime (ns) | ReadMape (%) | Write (MB/s) | WriteLength (Bytes) | WriteTime (ns) | WriteMape (%) |
| ------------------------------------------------- | ---------- | ----------- | ---------- | ----------- | ---------- | ---------- | ---------- | ---------- |
| [glaze](https://github.com/stephenberry/glaze) | 713.805 | 2090234 | 2.9283e+06 | 1.032 | 300.494 | 2090234 | 6.956e+06 | 1.42 |
| [jsonifier](https://github.com/realtimechris/jsonifier) | 668.297 | 2090234 | 3.1277e+06 | 1.103 | 387.26 | 2090234 | 5.3975e+06 | 0.717 |
| [simdjson](https://github.com/simdjson/simdjson) | 86.5733 | 2090234 | 2.41441e+07 | 0.9751 | N/A | N/A | N/A | N/A |

### Twitter Test (Prettified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/TwitterData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Twitter%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Twitter%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/s) | ReadLength (Bytes) | ReadTime (ns) | ReadMape (%) | Write (MB/s) | WriteLength (Bytes) | WriteTime (ns) | WriteMape (%) |
| ------------------------------------------------- | ---------- | ----------- | ---------- | ----------- | ---------- | ---------- | ---------- | ---------- |
| [glaze](https://github.com/stephenberry/glaze) | 679.217 | 408617 | 601600 | 8.777 | 1324.73 | 408547 | 308400 | 1.003 |
| [jsonifier](https://github.com/realtimechris/jsonifier) | 524.07 | 408617 | 779700 | 1.326 | 1871.81 | 408617 | 218300 | 1.909 |
| [simdjson](https://github.com/simdjson/simdjson) | 230.857 | 408617 | 1.77e+06 | 0.2094 | N/A | N/A | N/A | N/A |

### Twitter Test (Minified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/TwitterData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Twitter%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Twitter%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/s) | ReadLength (Bytes) | ReadTime (ns) | ReadMape (%) | Write (MB/s) | WriteLength (Bytes) | WriteTime (ns) | WriteMape (%) |
| ------------------------------------------------- | ---------- | ----------- | ---------- | ----------- | ---------- | ---------- | ---------- | ---------- |
| [jsonifier](https://github.com/realtimechris/jsonifier) | 1008.54 | 272710 | 270400 | 3.088 | 1784.75 | 272710 | 152800 | 0.1874 |
| [glaze](https://github.com/stephenberry/glaze) | 553.613 | 272710 | 492600 | 3.654 | 1708.27 | 272640 | 159600 | 4.855 |
| [simdjson](https://github.com/simdjson/simdjson) | 98.9191 | 272710 | 2.7569e+06 | 13.59 | N/A | N/A | N/A | N/A |

### Minify Test Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Minify%20Test_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Minify%20Test_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/s) | ReadLength (Bytes) | ReadTime (ns) | ReadMape (%) | Write (MB/s) | WriteLength (Bytes) | WriteTime (ns) | WriteMape (%) |
| ------------------------------------------------- | ---------- | ----------- | ---------- | ----------- | ---------- | ---------- | ---------- | ---------- |
| [jsonifier](https://github.com/realtimechris/jsonifier) | N/A | N/A | N/A | N/A | 489.207 | 71571 | 146300 | 0.8886 |
| [glaze](https://github.com/stephenberry/glaze) | N/A | N/A | N/A | N/A | 194.328 | 71571 | 368300 | 2.935 |
| [simdjson](https://github.com/simdjson/simdjson) | N/A | N/A | N/A | N/A | 152.441 | 71571 | 469500 | 0.3621 |

### Prettify Test Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Prettify%20Test_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Prettify%20Test_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/s) | ReadLength (Bytes) | ReadTime (ns) | ReadMape (%) | Write (MB/s) | WriteLength (Bytes) | WriteTime (ns) | WriteMape (%) |
| ------------------------------------------------- | ---------- | ----------- | ---------- | ----------- | ---------- | ---------- | ---------- | ---------- |
| [jsonifier](https://github.com/realtimechris/jsonifier) | N/A | N/A | N/A | N/A | 940.948 | 3506536 | 3.7266e+06 | 4.133 |
| [glaze](https://github.com/stephenberry/glaze) | N/A | N/A | N/A | N/A | 645.486 | 3506536 | 5.4324e+06 | 4.375 |

### Validation Test Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Validate%20Test_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Validate%20Test_Results.png?raw=true" 
alt="" width="400"/></p>


| Library | Read (MB/s) | ReadLength (Bytes) | ReadTime (ns) | ReadMape (%) | Write (MB/s) | WriteLength (Bytes) | WriteTime (ns) | WriteMape (%) |
| ------------------------------------------------- | ---------- | ----------- | ---------- | ----------- | ---------- | ---------- | ---------- | ---------- |
| [jsonifier](https://github.com/realtimechris/jsonifier) | 427.539 | 133948 | 313300 | 2.654 | N/A | N/A | N/A | N/A |