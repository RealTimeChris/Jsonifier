## Installing Jsonifier

### Installation (Vcpkg)
----
- Requirements:
	- CMake 3.18 or later.
	- A C++20 or later compiler.
- Steps:   
	1. Install vcpkg, if need be.
	2. Make sure to run vcpkg integrate install.
	3. Enter within a terminal vcpkg install jsonifier:x64-windows_OR_linux.
	4. Set up a project in your IDE and make sure to set the C++ standard to C++20 or later - and include `<jsonifier/Index.hpp>`.
	5. Build and run!
	
### Installation (CMake-FetchContent)
----
- Requirements:
	- CMake 3.18 or later.
	- A C++20 or later compiler.
- Steps:   Add the following to your CMakeLists.txt build script.
```cpp
include(FetchContent)

FetchContent_Declare(
   Jsonifier
   GIT_REPOSITORY https://github.com/RealTimeChris/Jsonifier.git
   GIT_TAG main
)
FetchContent_MakeAvailable(Jsonifier)

target_link_libraries("${PROJECT_NAME}" PRIVATE jsonifier::Jsonifier)
```

### Installation (CMake)
----
- Requirements:
	- CMake 3.18 or later.
	- A C++20 or later compiler.
- Steps:   
	1. Clone this repo into a folder.
	2. Set the installation directory if you wish, using the `CMAKE_INSTALL_PREFIX` variable in CMakeLists.txt.
	3. Enter the directory in a terminal, and enter `cmake -S . --preset=Windows_OR_Linux-Release_OR_Debug`.
	4. Enter within the same terminal, `cmake --build --preset=Windows_OR_Linux-Release_OR_Debug`.
	5. Enter within the same terminal, `cmake --install ./Build/Release_OR_Debug`.
	6. Now within the CMakeLists.txt of the project you wish to use the library in, set Jsonifier_DIR to wherever you set the `CMAKE_INSTALL_PREFIX` to, and then use `find_package(Jsonifier CONFIG REQUIRED)` and then `target_link_libraries("${PROJECT_NAME}" PUBLIC/PRIVATE jsonifier::Jsonifier)`.

