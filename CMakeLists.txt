#	MIT License
#
#	Copyright (c) 2023 RealTimeChris
#
#	Permission is hereby granted, free of charge, to any person obtaining a copy of this 
#	software and associated documentation files (the "Software"), to deal in the Software 
#	without restriction, including without limitation the rights to use, copy, modify, merge, 
#	publish, distribute, sublicense, and/or sell copies of the Software, and to permit 
#	persons to whom the Software is furnished to do so, subject to the following conditions:
#
#	The above copyright notice and this permission notice shall be included in all copies or 
#	substantial portions of the Software.
#
#	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, 
#	INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR 
#	PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE 
#	FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR 
#	OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER 
#	DEALINGS IN THE SOFTWARE.
#
# CMakeLists.txt - The CMake script for building this library.
# Dec 17, 2022
# https://github.com/RealTimeChris/Jsonifier

cmake_minimum_required(VERSION 3.18)

set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_CURRENT_SOURCE_DIR}/CMake;")
include("CollectVersion")
collect_version("jsonifier")

set(PROJECT_NAME "Jsonifier")
set(PRODUCT_VERSION "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}")

project(
  "${PROJECT_NAME}"
  VERSION "${PRODUCT_VERSION}"
  LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)

add_library("${PROJECT_NAME}" INTERFACE)

add_library("${PROJECT_NAME}::${PROJECT_NAME}" ALIAS "${PROJECT_NAME}")

file(GLOB_RECURSE HEADERS CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/Include/jsonifier/*.hpp")

set(JSONIFIER-POPCNT_EXITCODE 0 CACHE INTERNAL "For cross-compiling.")
set(JSONIFIER-LZCNT_EXITCODE 0 CACHE INTERNAL "For cross-compiling.")
set(JSONIFIER-BMI_EXITCODE 0 CACHE INTERNAL "For cross-compiling.")
set(JSONIFIER-BMI2_EXITCODE 0 CACHE INTERNAL "For cross-compiling.")
set(JSONIFIER-AVX_EXITCODE 0 CACHE INTERNAL "For cross-compiling.")
set(JSONIFIER-AVX2_EXITCODE 0 CACHE INTERNAL "For cross-compiling.")
set(JSONIFIER-AVX512_EXITCODE 0 CACHE INTERNAL "For cross-compiling.")

if (NOT JSONIFIER_CPU_INSTRUCTIONS)
	include("JsonifierDetectArchitecture")
endif()

 set_target_properties(
     "${PROJECT_NAME}" PROPERTIES
     OUTPUT_NAME "jsonifier"
     CXX_STANDARD_REQUIRED ON
     CXX_EXTENSIONS OFF
 )
 
 target_link_options(
     "${PROJECT_NAME}" INTERFACE
     "$<$<CXX_COMPILER_ID:CLANG>:$<$<STREQUAL:${ASAN_ENABLED},TRUE>:-faddress-sanitizer>>"
     "$<$<CXX_COMPILER_ID:GNU>:$<$<STREQUAL:${ASAN_ENABLED},TRUE>:-fsanitize=address>>"
     "$<$<CXX_COMPILER_ID:MSVC>:/INCREMENTAL:NO>"
     "$<$<CXX_COMPILER_ID:MSVC>:/LTCG>"
 )
 
 target_include_directories(
     "${PROJECT_NAME}" INTERFACE
     "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Include/>"
    "$<INSTALL_INTERFACE:include/>"
 )

target_compile_options(
	"${PROJECT_NAME}" INTERFACE
	"$<$<CXX_COMPILER_ID:MSVC>:$<$<STREQUAL:${ASAN_ENABLED},TRUE>:/fsanitize=address>>"
	"$<$<CXX_COMPILER_ID:MSVC>:/constexpr:steps 2097152>"
	"$<$<CXX_COMPILER_ID:GNU>:-Wno-ignored-attributes>"	
	"$<$<CXX_COMPILER_ID:MSVC>:/Zc:preprocessor>"
	"$<$<CXX_COMPILER_ID:CLANG>:-march=native>"
	"$<$<CXX_COMPILER_ID:MSVC>:/permissive->"
	"$<$<CXX_COMPILER_ID:GNU>:-march=native>"
	"$<$<CXX_COMPILER_ID:MSVC>:/Zc:lambda>"
	"$<$<CXX_COMPILER_ID:MSVC>:/fp:fast>"
	"$<$<CXX_COMPILER_ID:MSVC>:/bigobj>"
	"$<$<CXX_COMPILER_ID:MSVC>:/GL>"
	"$<$<CXX_COMPILER_ID:MSVC>:/Gy>"
	"${AVX_FLAG}"
)

target_compile_definitions(
	"${PROJECT_NAME}" INTERFACE 
	"$<$<STREQUAL:${ASAN_ENABLED},TRUE>:ASAN_ENABLED>"
)

set(CONFIG_FILE_NAME "${PROJECT_NAME}Config.cmake")
set(EXPORTED_TARGETS_NAME "${PROJECT_NAME}Targets")
set(EXPORTED_TARGETS_FILE_NAME "${EXPORTED_TARGETS_NAME}.cmake")
set(EXPORTED_TARGETS_FILE_PATH "share/jsonifier/${EXPORTED_TARGETS_FILE_NAME}")

include(CMakePackageConfigHelpers)
configure_package_config_file(
	"${CMAKE_CURRENT_SOURCE_DIR}/CMake/${CONFIG_FILE_NAME}.in"
	"${CMAKE_CURRENT_BINARY_DIR}/${CONFIG_FILE_NAME}"
	INSTALL_DESTINATION "share/jsonifier"
	PATH_VARS
	EXPORTED_TARGETS_FILE_PATH
)

set(VERSION_FILE_NAME "${PROJECT_NAME}ConfigVersion.cmake")

write_basic_package_version_file(
	"${CMAKE_CURRENT_BINARY_DIR}/${VERSION_FILE_NAME}"
	VERSION "${PRODUCT_VERSION}"
	COMPATIBILITY AnyNewerVersion
)

install(
	FILES
	"${CMAKE_CURRENT_BINARY_DIR}/${CONFIG_FILE_NAME}"
	"${CMAKE_CURRENT_BINARY_DIR}/${VERSION_FILE_NAME}"
	DESTINATION "share/jsonifier"
)

install(
	DIRECTORY
	"${CMAKE_CURRENT_SOURCE_DIR}/Include/"
	DESTINATION "include"
)

install(
	TARGETS "${PROJECT_NAME}"
	EXPORT "${EXPORTED_TARGETS_NAME}"
)

install(
	EXPORT "${EXPORTED_TARGETS_NAME}"
	FILE "${EXPORTED_TARGETS_FILE_NAME}"
	NAMESPACE "${PROJECT_NAME}::"
	DESTINATION "share/jsonifier"
)

if("${JSONIFIER_TEST}" STREQUAL "TRUE")
	add_subdirectory("./Tests")
endif()
