#
#	Jsonifier, A few classes for parsing and serializing objects from/into JSON or ETF strings - very rapidly.
#
#	Copyright 2022 Chris M. (RealTimeChris)
#
#	This library is free software; you can redistribute it and/or
#	modify it under the terms of the GNU Lesser General Public
#	License as published by the Free Software Foundation; either
#	version 2.1 of the License, or (at your option) any later version.
#
#	This library is distributed in the hope that it will be useful,
#	but WITHOUT ANY WARRANTY; without even the implied warranty of
#	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#	Lesser General Public License for more details.
#
#	You should have received a copy of the GNU Lesser General Public
#	License along with this library; if not, write to the Free Software
#	Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301
#	USA
#
# CMakeLists.txt - The CMake script for building this library.
# Dec 17, 2022
# https://github.com/RealTimeChris/Jsonifier

cmake_minimum_required(VERSION 3.18)

project(
    "Jsonifier"
    VERSION 1.0.1
    LANGUAGES CXX C
)

set(CXX_STANDARD 23)

file(GLOB HEADERS CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/Include/jsonifier/*.hpp")

add_library("${PROJECT_NAME}" INTERFACE "${HEADERS}")

add_library("${PROJECT_NAME}::${PROJECT_NAME}" ALIAS "${PROJECT_NAME}")

set_target_properties(
	"${PROJECT_NAME}" PROPERTIES
	PUBLIC_HEADER "${HEADERS}"
	OUTPUT_NAME "jsonifier"
	CXX_STANDARD_REQUIRED ON
	CXX_EXTENSIONS OFF
	CXX_STANDARD 23
)

if (NOT "${VCPKG_TARGET_TRIPLET}" STREQUAL "x64_uwp")
	include(CMake/FindAVX.cmake)
endif()

if (CXX_AVX512_FOUND)
	target_compile_definitions("${PROJECT_NAME}" INTERFACE INSTRUCTION_SET_TYPE_AVX512)
	if (WIN32)
		set(AVX_TYPE "AVX512")
	else()
		set(AVX_TYPE "march=skylake-avx512")
	endif()
elseif (CXX_AVX2_FOUND)
	target_compile_definitions("${PROJECT_NAME}" INTERFACE INSTRUCTION_SET_TYPE_AVX2)
	if (WIN32)
		set(AVX_TYPE "AVX2")
	else()
		set(AVX_TYPE "march=core-avx2")
	endif()
elseif(CXX_AVX_FOUND)
	target_compile_definitions("${PROJECT_NAME}" INTERFACE INSTRUCTION_SET_TYPE_AVX)
	if (WIN32)
		set(AVX_TYPE "AVX")
	else()
		set(AVX_TYPE "march=core-i-avx")
	endif()
else()	
	target_compile_definitions("${PROJECT_NAME}" INTERFACE INSTRUCTION_SET_TYPE_X64)
	set(AVX_TYPE "march=native")
endif()

target_include_directories(
	"${PROJECT_NAME}" INTERFACE
	"$<INSTALL_INTERFACE:$<INSTALL_PREFIX>/include>"
	INTERFACE
	"$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Include/>"
)

target_compile_options(
	"${PROJECT_NAME}" INTERFACE 
	"$<$<CXX_COMPILER_ID:MSVC>:$<$<STREQUAL:${ASAN_ENABLED},TRUE>:/fsanitize=address>>"
	"$<$<CXX_COMPILER_ID:MSVC>:$<$<STREQUAL:${ASAN_ENABLED},TRUE>:/Zi>>"
	"$<$<CXX_COMPILER_ID:MSVC>:/arch:${AVX_TYPE}>"
	"$<$<CXX_COMPILER_ID:GNU>:-${AVX_TYPE}>"
	"$<$<CXX_COMPILER_ID:Clang>:-${AVX_TYPE}>"
	"$<$<CXX_COMPILER_ID:Clang>:-std=c++2b>"
)

target_link_options(
	"${PROJECT_NAME}" INTERFACE 
	"$<$<CXX_COMPILER_ID:GNU>:$<$<STREQUAL:${ASAN_ENABLED},TRUE>:-fsanitize=address>>"
)

set_target_properties(
	"${PROJECT_NAME}" PROPERTIES 
	"$<$<CXX_COMPILER_ID:MSVC>:$<$<CONFIG:Release>:COMPILE_PDB_NAME ${PROJECT_NAME}>>"
	"$<$<CXX_COMPILER_ID:MSVC>:$<$<CONFIG:Release>:COMPILE_PDB_OUTPUT_DIR ${CMAKE_BINARY_DIR}>>"
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
	PUBLIC_HEADER DESTINATION "include/jsonifier"
)

install(
	EXPORT "${EXPORTED_TARGETS_NAME}"
	FILE "${EXPORTED_TARGETS_FILE_NAME}"
	NAMESPACE "${PROJECT_NAME}::"
	DESTINATION "share/jsonifier"
)
