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

set(PROJECT_NAME  "Jsonifier")
set(PRODUCT_VERSION 0.9.4)

project(
    "${PROJECT_NAME}"
    VERSION "${PRODUCT_VERSION}"
    LANGUAGES CXX
)

add_library("${PROJECT_NAME}" INTERFACE)

add_library("${PROJECT_NAME}::${PROJECT_NAME}" ALIAS "${PROJECT_NAME}")

file(GLOB_RECURSE HEADERS CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/Include/jsonifier/*.hpp")

include("${CMAKE_CURRENT_SOURCE_DIR}/CMake/DetectArchitecture.cmake")

set_target_properties(
	"${PROJECT_NAME}" PROPERTIES
	PUBLIC_HEADER "${HEADERS}"
	OUTPUT_NAME "jsonifier"
	CXX_STANDARD_REQUIRED ON
	CXX_EXTENSIONS OFF
)

target_include_directories(
	"${PROJECT_NAME}" INTERFACE
	"$<INSTALL_INTERFACE:$<INSTALL_PREFIX>/include>"
	"$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Include/>"
)

target_compile_definitions(
	"${PROJECT_NAME}" INTERFACE 
	"AVX_TYPE=${AVX_TYPE}"
)

target_compile_options(
	"${PROJECT_NAME}" INTERFACE 
	"$<$<CXX_COMPILER_ID:MSVC>:$<$<STREQUAL:${ASAN_ENABLED},TRUE>:/fsanitize=address>>"
	"$<$<CXX_COMPILER_ID:MSVC>:$<$<STREQUAL:${ASAN_ENABLED},TRUE>:/Zi>>"
	"$<$<CXX_COMPILER_ID:MSVC>:$<$<STREQUAL:${ASAN_ENABLED},TRUE>:/EHsc>>"
	"${AVX_FLAG}"
)

target_link_options(
	"${PROJECT_NAME}" INTERFACE 
	"$<$<CXX_COMPILER_ID:GNU>:$<$<STREQUAL:${ASAN_ENABLED},TRUE>:-fsanitize=address>>"
)

target_compile_features(
	"${PROJECT_NAME}" INTERFACE
	"$<$<CXX_COMPILER_ID:MSVC>:cxx_std_23>"
	"$<$<CXX_COMPILER_ID:GNU>:cxx_std_23>"
	"$<$<CXX_COMPILER_ID:CLANG>:cxx_std_23>"
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
	TARGETS "${PROJECT_NAME}"
	EXPORT "${EXPORTED_TARGETS_NAME}"
	RUNTIME DESTINATION "$<$<NOT:$<PLATFORM_ID:Linux>>:$<IF:$<CONFIG:Debug>,${DEBUG_PREFIX}bin,bin>>"
	ARCHIVE DESTINATION "$<IF:$<CONFIG:Debug>,${DEBUG_PREFIX}lib,lib>"
	PUBLIC_HEADER DESTINATION "include/jsonifier"
)

install(
	EXPORT "${EXPORTED_TARGETS_NAME}"
	FILE "${EXPORTED_TARGETS_FILE_NAME}"
	NAMESPACE "${PROJECT_NAME}::"
	DESTINATION "share/jsonifier"
)