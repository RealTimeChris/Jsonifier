# Jsonifier
[![Codacy Badge](https://img.shields.io/codacy/grade/cfeb498652bb4f269a9d7287c2acaed3?color=lightblue&label=Code%20Quality&style=plastic)](https://www.codacy.com/gh/RealTimeChris/Jsonifier/dashboard?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=RealTimeChris/Jsonifier&amp;utm_campaign=Badge_Grade)
[![Build Jsonifier](https://img.shields.io/github/workflow/status/realtimechris/Jsonifier/Release?label=Release&style=plastic&color=purple)](https://github.com/RealTimeChris/Jsonifier/actions/workflows/Release.yml)
![Commit Activity](https://img.shields.io/github/commit-activity/m/realtimechris/Jsonifier?color=green&label=Commits&style=plastic)
![Lines of code](https://img.shields.io/tokei/lines/github/realtimechris/Jsonifier?&style=plastic&label=Lines%20of%20Code)


## A class Jsonifier_Dll for serializing and parsing objects into/from JSON or ETF strings - very rapidly.
## Benchmark
### Serializing the following data into a json string:
```cpp
{"d":{"intents":131071,"large_threshold":250,"presence":{"afk":false,"since":0,"status":""},"properties":{"browser":"DiscordCoreAPI","device":"DiscordCoreAPI","os":"Windows"},"shard":[0,1],"token":""},"op":2}
```
### Using the following setup:
```cpp
Jsonifier::StopWatch<std::chrono::milliseconds> stopWatch{ std::chrono::milliseconds{ 1 } };
std::vector<std::string> vector{};
uint64_t totalTime{};
size_t size{};
WebSocketIdentifyData data{};
auto serializer = data.operator Jsonifier::Jsonifier();
stopWatch.resetTimer();

for (uint32_t x = 0; x < 50; ++x) {
	stopWatch.resetTimer();
	for (uint32_t x = 0; x < 1024 * 128; ++x) {
		serializer["d"]["intents"] = x;
		serializer.refreshString(Jsonifier::JsonifierSerializeType::Json);
		vector.push_back(serializer.operator std::string());
		size += vector.back().size();
	}

	totalTime += stopWatch.totalTimePassed();
}
std::cout << "The time it took (In milliseconds, on average): " << totalTime / 50 << ", with a total number of bytes serialized: " << size << std::endl;
		
vector.clear();
totalTime = 0;
size = 0;
WebSocketIdentifyDataTwo dataOne{};
nlohmann::json stringBufferTwo = dataOne;
stopWatch.resetTimer();
for (uint32_t x = 0; x < 50; ++x) {
	stopWatch.resetTimer();
	for (uint32_t x = 0; x < 1024 * 128; ++x) {
		stringBufferTwo["d"]["intents"] = x;
		vector.push_back(stringBufferTwo.dump());
		size += vector.back().size();
	}
	totalTime += stopWatch.totalTimePassed();
}
std::cout << "The time it took (In milliseconds, on average): " << totalTime / 50 << ", with a total number of bytes serialized: " << size << std::endl;

vector.clear();
totalTime = 0;
size = 0;
WebSocketIdentifyDataThree dataTwo{};
rapidjson::StringBuffer stringBuffer = dataTwo;
stopWatch.resetTimer();
for (uint32_t x = 0; x < 50; ++x) {			
	stopWatch.resetTimer();
	for (uint32_t x = 0; x < 1024 * 128 ; ++x) {
		dataTwo.intents = x;
		stringBuffer = dataTwo;
		std::string string{ stringBuffer.GetString() };
		vector.push_back(string);
		size += vector.back().size();
	}
	totalTime += stopWatch.totalTimePassed();
}
std::cout << "The time it took (In milliseconds, on average): " << totalTime / 50 << ", with a total number of bytes serialized: " << size << std::endl;
```
### Windows 11 Results:
----
### A total of 131,072 times, over 50 iterations, for a total number of serialized bytes of 1357593300:
- Jsonifier = 85 milliseconds average per iteration.   
- rapidjson = 113 milliseconds average per iteration.   
- Nlohmann-Json = 234 milliseconds average per iteration.   
### Ubuntu 22.04 Results:
----
### A total of 131,072 times, over 50 iterations, for a total number of serialized bytes of 2091596500:  
- Jsonifier = 238 milliseconds average per iteration.   
- Nlohmann-Json = 451 milliseconds average per iteration.   
## Usage
- Use the square bracket operator with the desired key names, to create objects. Also, use `Jsonifier::emplaceBack()` to create and add to arrays.
- Alternatively use the square bracket operator with numbers as keys to access array-fields. If you try to access fields that don't exist, it will append the missing number of fields as "null".
----
```cpp

struct Jsonifier_Dll UpdatePresenceData {
	std::string status{};
	int64_t since{ 0 };
	bool afk{ false };
	operator Jsonifier();
};

UpdatePresenceData::operator Jsonifier() {
	Jsonifier serializer{};
	serializer["status"] = this->status;
	serializer["since"] = this->since;
	serializer["afk"] = this->afk;
	return serializer;
}

WebSocketIdentifyData::operator std::string() {
	Jsonifier serializer{};
	serializer["d"]["intents"] = this->intents;
	std::map<std::string, DiscordCoreAPI::ChannelType> map{};
	serializer["d"]["large_threshold"] = map;

	UpdatePresenceData data{};
	serializer["d"]["presence"]["activities"].emplaceBack(data);
	serializer["d"]["presence"]["activities"].emplaceBack(data);
	serializer["d"]["presence"]["activities"].emplaceBack(std::move(data));
	serializer["d"]["afk"] = this->presence.afk;
	if (this->presence.since != 0) {
		serializer["since"] = this->presence.since;
	}
	serializer["d"]["status"] = this->presence.status;
	serializer["d"]["properties"]["browser"] = "DiscordCoreAPI";
	serializer["d"]["properties"]["device"] = "DiscordCoreAPI";
#ifdef _WIN32
	serializer["d"]["properties"]["os"] = "Windows";
#else
	serializer["d"]["properties"]["os"] = "Linux";
#endif
	serializer["d"]["shard"].emplaceBack(0);
	serializer["d"]["shard"].emplaceBack(1);
	serializer["d"]["token"] = this->botToken;
	serializer["op"] = 2;
	serializer.refreshString(JsonifierSerializeType::Json);
	return serializer.operator std::string&&();
}

```
- To generate the string, call the `Jsonifier::refreshString()` method with an argument of type `JsonifierSerializeType`, set to either Json or Etf, depending on which one you would like to generate, and then call the `std::string` operator or the `std::string&&` operator of the Jsonifier class Jsonifier_Dll to acquire the string. **(Note: The `std::string` operator copies the string out of the `Jsonifier` class Jsonifier_Dll, while the `std::string&&` operator moves it out of the `Jsonifier` class Jsonifier_Dll.)**
```cpp
serializer.refreshString(JsonifierSerializeType::Json);
return serializer.operator std::string&&();
```
- The previous inputs will generate the following output, in Json-generating mode.
```cpp
{"d":{"afk":false,"intents":0,"large_threshold":{},"presence":{"activities":[{"afk":false,"since":0,"status":""},{"afk":false,"since":0,"status":""},{"afk":false,"since":0,"status":""}]},"properties":{"browser":"DiscordCoreAPI","device":"DiscordCoreAPI","os":"Windows"},"shard":[0,1],"status":"","token":""},"op":2}
```   
- Or the following, in Etf-generating mode.
```cpp
ât☻m☺dm♥afks♣falsemintentsamlarge_thresholdtpresencet☺m
activitiesl♥t♥m♥afks♣falsem♣sinceam♠statusmt♥m♥afks♣falsem♣sinceam♠statusmt♥m♥afks♣falsem♣sinceam♠statusmjm
propertiest♥mbrowsermDiscordCoreAPIm♠devicemDiscordCoreAPIm☻osmWindowsm♣shardl☻aa☺jm♠statusmm♣tokenmm☻opa☻
```
- Also note that the `Jsonifier` class Jsonifier_Dll can accept arguments of type `Jsonifier` to be concatenated into the string - as the `UpdatePresenceData` class Jsonifier_Dll above shows.
## Installation (CMake)
- Requirements:
	- CMake 3.20 or later.
	- A C++20 or later compiler.
- Steps:   
	1. Clone this repo into a folder.
	2. Set the installation directory if you wish, using the `CMAKE_INSTALL_PREFIX` variable in CMakeLists.txt.
	3. Enter the directory in a terminal, and enter `cmake -S . --preset=Windows_OR_Linux-Release_OR_Debug`.
	4. Enter within the same terminal, `cmake --build --preset=Windows_OR_Linux-Release_OR_Debug`.
	5. Enter within the same terminal, `cmake --install ./Build/Release_OR_Debug`.
	6. Now within the CMakeLists.txt of the project you wish to use the library in, set Jsonifier_DIR to wherever you set the `CMAKE_INSTALL_PREFIX` to, and then use `find_package(Jsonifier CONFIG REQUIRED)` and then `target_link_libraries("${PROJECT_NAME}" PUBLIC/PRIVATE Jsonifier::Jsonifier)`.

