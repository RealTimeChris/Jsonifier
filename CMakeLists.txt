#	MIT License
#
#	Copyright (c) 2024 RealTimeChris
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
# https://github.com/RealTimeChris/Jsonifier
cmake_minimum_required(VERSION 3.28)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(cmake/get_version.cmake)

set(JSONIFIER_TAPE_MAX "8" CACHE STRING "Max tape step size")
set(JSONIFIER_TAPE_STEP "2" CACHE STRING "Tape step increment")
set(JSONIFIER_BLOCKS_PER_STEP "2" CACHE STRING "Pipeline depth")

project(Jsonifier
    VERSION ${CLEAN_VERSION}
    DESCRIPTION "A few classes for parsing and serializing json - very rapidly."
    LANGUAGES CXX
)

include(cmake/library_setup.cmake)

include(cmake/installation_setup.cmake)

if(JSONIFIER_UNIT_TESTS)
    add_subdirectory(unit-tests)
endif()