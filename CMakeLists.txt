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
#	License along with this library; if not, Write to the Free Software
#	Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301
#	USA
#
# CMakeLists.txt - The CMake script for building this library.
# Dec 17, 2022
# https://github.com/RealTimeChris/Jsonifier

cmake_minimum_required(VERSION 3.18)

set(PROJECT_NAME "Jsonifier")

set(CMAKE_INSTALL_PREFIX "${CMAKE_SOURCE_DIR}/Output")

project(
    "${PROJECT_NAME}"
    VERSION 1.0.1
    LANGUAGES CXX
)

file(GLOB_RECURSE Headers CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/Include/jsonifier/*.hpp")
source_group(TREE "${CMAKE_SOURCE_DIR}/Include" PREFIX Headers FILES ${Headers})

file(GLOB_RECURSE Sources CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/Source/*.cpp")
source_group(TREE "${CMAKE_SOURCE_DIR}/Source" PREFIX Sources FILES ${Sources})

add_library(Jsonifier INTERFACE "${Sources}" "${Headers}")
add_library(Jsonifier::Jsonifier ALIAS Jsonifier)

if (MSVC)
   if (matched_cl)
      target_compile_options(Jsonifier INTERFACE "/Zc:preprocessor" /GL /permissive- /Zc:lambda /O3)
      target_link_options(Jsonifier INTERFACE /LTCG /INCREMENTAL:NO)
   endif()
else()
   target_compile_options(Jsonifier INTERFACE "-Wno-missing-braces")
endif()

set_target_properties(Jsonifier PROPERTIES EXPORT_NAME "${PROJECT_NAME}" CXX_STANDARD C++23)

target_compile_features(Jsonifier INTERFACE cxx_std_20)

set(CMAKE_INSTALL_INCLUDEDIR "Include/jsonifier" CACHE PATH "")

set(CMAKE_INSTALL_LIBDIR lib CACHE PATH "")

include(CMakePackageConfigHelpers)
include(GNUInstallDirs)

install(
    DIRECTORY Include/
    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
    COMPONENT Jsonifier_Development
)

install(
    TARGETS Jsonifier
    EXPORT JsonifierTargets
    INCLUDES DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
)

write_basic_package_version_file(
    "${PROJECT_NAME}ConfigVersion.cmake"
    COMPATIBILITY SameMajorVersion
    ARCH_INDEPENDENT
)

# Allow package maintainers to freely override the path for the configs
set(
    Jsonifier_INSTALL_CMAKEDIR "${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}"
    CACHE PATH "CMake package config location relative to the install prefix"
)
mark_as_advanced(Jsonifier_INSTALL_CMAKEDIR)

install(
    FILES cmake/install-config.cmake
    DESTINATION "${Jsonifier_INSTALL_CMAKEDIR}"
    RENAME "${PROJECT_NAME}Config.cmake"
    COMPONENT Jsonifier_Development
)

install(
    FILES "${PROJECT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
    DESTINATION "${Jsonifier_INSTALL_CMAKEDIR}"
    COMPONENT Jsonifier_Development
)

install(
    EXPORT JsonifierTargets
    NAMESPACE Jsonifier::
    DESTINATION "${Jsonifier_INSTALL_CMAKEDIR}"
    COMPONENT Jsonifier_Development
)

if(PROJECT_IS_TOP_LEVEL)
  include(CPack)
endif()

target_include_directories(
    Jsonifier INTERFACE "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/Include/>"
)