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

# Set this Primitive to install the output files to a particular location.
#set(CMAKE_INSTALL_PREFIX)

cmake_minimum_required(VERSION 3.20)
set(PROJECT_NAME "Jsonifier")
set(PRODUCT_VERSION "1.0.0")
set(DESCRIPTION "A few classes for serializing objects into JSON or ETF strings - very rapidly.")

project(
	"${PROJECT_NAME}"
	VERSION "${PRODUCT_VERSION}"
	LANGUAGES CXX
	HOMEPAGE_URL "https://github/com/RealTimeChris/Jsonifier"
	DESCRIPTION "${DESCRIPTION}"
)

if (UNIX)
	set(CMAKE_CXX_FLAGS_RELEASE "-DNDEBUG -Ofast")
endif()

if (WIN32)
	if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
		cmake_path(GET CMAKE_ROOT ROOT_PATH ROOT_DRIVE)
		if (DEFINED ENV{EnableASAN})
			set(CMAKE_INSTALL_PREFIX "${ROOT_DRIVE}Users/$ENV{USERNAME}/CMake/${PROJECT_NAME}/ASAN" CACHE PATH "The CMake install prefix." FORCE)
		else()
			set(CMAKE_INSTALL_PREFIX "${ROOT_DRIVE}Users/$ENV{USERNAME}/CMake/${PROJECT_NAME}" CACHE PATH "The CMake install prefix." FORCE)
		endif()
	endif()
elseif(UNIX)
	if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
		if (DEFINED ENV{EnableASAN})
			set(CMAKE_INSTALL_PREFIX "/home/$ENV{USERNAME}/CMake/${PROJECT_NAME}/ASAN" CACHE PATH "The CMake install prefix." FORCE)
		else()
			set(CMAKE_INSTALL_PREFIX "/home/$ENV{USERNAME}/CMake/${PROJECT_NAME}" CACHE PATH "The CMake install prefix." FORCE)
		endif()
	endif()
endif()
 
set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_SOURCE_DIR}/CMake/;")
set(CMAKE_CONFIGURATION_TYPES "${CMAKE_BUILD_TYPE}")

include("${CMAKE_CURRENT_SOURCE_DIR}/CMake/GenerateProductVersion.cmake")
generate_product_version(
	PRODUCT_INFO
	NAME "${PROJECT_NAME}"
	FILE_DESCRIPTION "${DESCRIPTION}"
	ORIGINAL_FILENAME "jsonifier.dll"
	COMPANY_NAME "RealTimeChris Inc."
	VERSION_MAJOR "1"
	VERSION_MINOR "0"
	VERSION_PATCH "0"
	VERSION_REVISION "0"
)

file(GLOB PACKAGE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/CMake/*.in" "${CMAKE_CURRENT_SOURCE_DIR}/CMake/*.rc")

file(GLOB SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/Source/*.cpp")

if (WIN32)
	add_library("${PROJECT_NAME}" SHARED "${SOURCES}" "${PRODUCT_INFO}")
elseif (UNIX)
	add_library("${PROJECT_NAME}" STATIC "${SOURCES}")
endif()

add_library("${PROJECT_NAME}::${PROJECT_NAME}" ALIAS "${PROJECT_NAME}")

file(GLOB HEADERS CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/Include/jsonifier/*.hpp")

target_compile_options(
	"${PROJECT_NAME}" PUBLIC
	"$<$<NOT:$<PLATFORM_ID:Linux>>:/bigobj>"
	"$<$<CXX_COMPILER_ID:MSVC>:$<$<STREQUAL:$ENV{EnableASAN},true>:/fsanitize=address>>"
	"$<$<CXX_COMPILER_ID:MSVC>:$<$<STREQUAL:$ENV{EnableASAN},true>:/Zi>>"
	"$<$<CXX_COMPILER_ID:MSVC>:/MP>"
	"$<$<CXX_COMPILER_ID:MSVC>:/arch:AVX2>"
	"$<$<CXX_COMPILER_ID:GNU>:$<$<STREQUAL:$ENV{EnableASAN},true>:-fsanitize=address>>"
	"$<$<CXX_COMPILER_ID:GNU>:-fpermissive>"
	"$<$<CXX_COMPILER_ID:GNU>:-mavx512f>"
	"$<$<CXX_COMPILER_ID:GNU>:-march=native>"
)

target_link_options(
	"${PROJECT_NAME}" PUBLIC
	"$<$<CXX_COMPILER_ID:MSVC>:/DEBUG>"
	"$<$<CXX_COMPILER_ID:GNU>:$<$<STREQUAL:$ENV{EnableASAN},true>:-fsanitize=address>>"
)

set_target_properties(
	"${PROJECT_NAME}" PROPERTIES
	PUBLIC_HEADER "${HEADERS}"
	OUTPUT_NAME "jsonifier"
	CXX_STANDARD_REQUIRED ON
	CXX_EXTENSIONS OFF
)

target_include_directories(
	"${PROJECT_NAME}" PUBLIC
	"$<INSTALL_INTERFACE:${CMAKE_INSTALL_PREFIX}/Include/>"
	"$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Include>"
)

target_compile_features(
	"${PROJECT_NAME}" PUBLIC
	"cxx_std_20"
)

set(CONFIG_FILE_NAME "${PROJECT_NAME}Config.cmake")
set(EXPORTED_TARGETS_NAME "${PROJECT_NAME}Targets")
set(EXPORTED_TARGETS_FILE_NAME "${EXPORTED_TARGETS_NAME}.cmake")
set(EXPORTED_TARGETS_FILE_PATH "share/jsonifier/${EXPORTED_TARGETS_FILE_NAME}")
set(RELEASE_PDB_FILE_PATH "bin/jsonifier.pdb")
set(DEBUG_PDB_FILE_PATH "debug/bin/jsonifier.pdb")

include(CMakePackageConfigHelpers)
configure_package_config_file(
	"${CMAKE_SOURCE_DIR}/CMake/${CONFIG_FILE_NAME}.in"
	"${CMAKE_CURRENT_BINARY_DIR}/${CONFIG_FILE_NAME}"
	INSTALL_DESTINATION "share/jsonifier"
	PATH_VARS
	EXPORTED_TARGETS_FILE_PATH
	RELEASE_PDB_FILE_PATH
	DEBUG_PDB_FILE_PATH
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

if (WIN32)
	install(
		FILES 
		"$<TARGET_PDB_FILE:${PROJECT_NAME}>"
		DESTINATION "$<IF:$<CONFIG:Debug>,${DEBUG_PREFIX}bin,bin>"
		OPTIONAL
	)
endif()


if (BUILDING_THE_FETCH_PACKAGE)
	set(package_files "${SOURCES}" "${HEADERS}" "${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt" "${CMAKE_CURRENT_SOURCE_DIR}/CMake/GenerateProductVersion.cmake" "${CMAKE_CURRENT_SOURCE_DIR}/CMake/FetchContent.cmake" "${PACKAGE_FILES}")
	add_custom_command(
		OUTPUT 
		"${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-src.zip"
	    COMMAND "${CMAKE_COMMAND}" -E tar c "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-src.zip" --format=zip -- ${package_files}
	    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
	    DEPENDS ${package_files}
	)
	add_custom_target(
		"${PROJECT_NAME}_package" 
		ALL DEPENDS 
		"${PROJECT_NAME}" 
		"${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-src.zip"
		${package_files}
	)
 endif()

