# Jsonifier
[![Build Jsonifier](https://img.shields.io/github/actions/workflow/status/RealTimeChris/Jsonifier/Release.yml?branch=main&style=plastic&color=purple)](https://github.com/RealTimeChris/Jsonifier/actions/workflows/Release.yml)
![Commit Activity](https://img.shields.io/github/commit-activity/m/realtimechris/Jsonifier?color=green&label=Commits&style=plastic)
![Lines of code](https://img.shields.io/tokei/lines/github/RealTimeChris/Jsonifier?&style=plastic&label=Lines%20of%20Code)


## A few classes for serializing and parsing objects into/from JSON strings - very rapidly (more rapidly than any other library).
---
## [Benchmarks](https://github.com/RealTimeChris/Json-Benchmarks)
----
## Usage - Serialization
- Use the square bracket operator with the desired key names, to create objects. Also, use `Jsonifier::Serializer::emplaceBack()` to create and add to arrays.
- Alternatively use the square bracket operator with numbers as keys to access array-fields. If you try to access fields that don't exist, it will append the missing number of fields as "null".
----
```cpp

class UpdatePresenceData {
	std::string status{};
	int64_t since{ 0 };
	bool afk{ false };
	operator Jsonifier::Serializer();
	};

UpdatePresenceData::operator Jsonifier::Serializer() {
	Jsonifier::Serializer serializer{};
	serializer["status"] = status;
	serializer["since"] = since;
	serializer["afk"] = afk;
	return serializer;
}

WebSocketIdentifyData::operator std::string() {
	Jsonifier::Serializer serializer{};
	serializer["d"]["intents"] = intents;
	std::map<std::string, DiscordCoreAPI::ChannelType> map{};
	serializer["d"]["large_threshold"] = map;

	UpdatePresenceData data{};
	serializer["d"]["presence"]["activities"].emplaceBack(data);
	serializer["d"]["presence"]["activities"].emplaceBack(data);
	serializer["d"]["presence"]["activities"].emplaceBack(std::move(data));
	serializer["d"]["afk"] = presence.afk;
	if (presence.since != 0) {
		serializer["since"] = presence.since;
	}
	serializer["d"]["status"] = presence.status;
	serializer["d"]["properties"]["browser"] = "DiscordCoreAPI";
	serializer["d"]["properties"]["device"] = "DiscordCoreAPI";
#ifdef _WIN32
	serializer["d"]["properties"]["os"] = "Windows";
#else
	serializer["d"]["properties"]["os"] = "Linux";
#endif
	serializer["d"]["shard"].emplaceBack(0);
	serializer["d"]["shard"].emplaceBack(1);
	serializer["d"]["token"] = botToken;
	serializer["op"] = 2;
	serializer.refreshString(Jsonifier::SerializeType::Json);
	return serializer.operator std::string&&();
	}

```
- To generate the string, call the `Jsonifier::Serializer::refreshString()` method with an argument of type `Jsonifier::JsonifierSerializeType`, set to either Json or Etf, depending on which one you would like to generate, and then call the `std::string` operator or the `std::string&&` operator of the `Jsonifier::Serializer` class to acquire the string. **(Note: The `std::string` operator copies the string out of the `Jsonifier::Serializer` class, while the `std::string&&` operator moves it out of the `Jsonifier::Serializer` class.)**
```cpp
serializer.refreshString(Jsonifier::JsonifierSerializeType::Json);
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
- Also note that the `Jsonifier::Serializer` class can accept arguments of type `Jsonifier::Serializer` to be concatenated into the string - as the `UpdatePresenceData` class above shows.
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

