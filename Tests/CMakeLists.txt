#
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
# https://github.com/RealTimeChris/jsonifier
set(CMAKE_CXX_STANDARD 23)

include(FetchContent)
include(GetCommitHash.cmake)

FetchContent_Declare(
  glaze
  GIT_REPOSITORY https://github.com/stephenberry/glaze
  GIT_TAG main
  GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(glaze)
getCommitHash("${glaze_SOURCE_DIR}" GLAZE_COMMIT_HASH)

set(JSONIFIER_LIBRARY "TRUE" CACHE STRING "To avoid circular dependency issues." FORCE)
FetchContent_Declare(
  BenchmarkSuite
  GIT_REPOSITORY https://github.com/RealTimeChris/BenchmarkSuite.git
  GIT_TAG main
  GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(BenchmarkSuite)

FetchContent_Declare(
  simdjson
  GIT_REPOSITORY https://github.com/simdjson/simdjson.git
  GIT_TAG master
  GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(simdjson)
getCommitHash("${simdjson_SOURCE_DIR}" SIMDJSON_COMMIT_HASH)

add_executable(
  "Json-Performance" 
  "./main.cpp"
)

target_link_libraries(
	"Json-Performance" PUBLIC 
	Jsonifier::Jsonifier glaze::glaze simdjson BenchmarkSuite::BenchmarkSuite
)

target_link_options(
	"Json-Performance" PUBLIC
	"$<$<AND:$<STREQUAL:$<UPPER_CASE:${UBSAN}>,TRUE>,$<NOT:$<CXX_COMPILER_ID:MSVC>>>:-fsanitize=undefined>"
	"$<$<CXX_COMPILER_ID:MSVC>:/Debug>"
)

target_compile_options(
	"Json-Performance" PUBLIC
	"$<$<AND:$<STREQUAL:$<UPPER_CASE:${UBSAN}>,TRUE>,$<NOT:$<CXX_COMPILER_ID:MSVC>>>:-fsanitize=undefined>"
	"$<$<CXX_COMPILER_ID:MSVC>:$<$<STREQUAL:${ASAN_ENABLED},TRUE>:/fsanitize=address>>"
	"$<$<CXX_COMPILER_ID:CLANG>:-Wextra>"
	"$<$<CXX_COMPILER_ID:CLANG>:-Wall>"
	"$<$<CXX_COMPILER_ID:GNU>:-Wextra>"
	"$<$<CXX_COMPILER_ID:MSVC>:/Wall>"
	"$<$<CXX_COMPILER_ID:GNU>:-Wall>"
	"$<$<CXX_COMPILER_ID:MSVC>:/Zi>"
)

if (WIN32)
	install(
		FILES 
		"$<TARGET_PDB_FILE:Json-Performance>"
		DESTINATION "$<IF:$<CONFIG:Debug>,bin,bin>"
		OPTIONAL
	)
endif()

target_compile_definitions(
	"Json-Performance" PUBLIC 
	"JSON_TEST_PATH=\"${CMAKE_CURRENT_SOURCE_DIR}/\""
	"JSON_PATH=\"${CMAKE_CURRENT_SOURCE_DIR}/../Json\""
	"README_PATH=\"${CMAKE_CURRENT_SOURCE_DIR}/ReadMe.md\""
	"GRAPHS_PATH=\"${CMAKE_CURRENT_SOURCE_DIR}/Graphs\""
	"BASE_PATH=\"${CMAKE_CURRENT_SOURCE_DIR}/\""
	"JSONIFIER_COMMIT=\"${JSONIFIER_COMMIT_HASH}\""
	"SIMDJSON_COMMIT=\"${SIMDJSON_COMMIT_HASH}\""
	"GLAZE_COMMIT=\"${GLAZE_COMMIT_HASH}\""
	"OPERATING_SYSTEM_NAME=\"${CMAKE_SYSTEM_NAME}\""
	"OPERATING_SYSTEM_VERSION=\"${CMAKE_SYSTEM_VERSION}\""
	"COMPILER_ID=\"${CMAKE_CXX_COMPILER_ID}\""
	"COMPILER_VERSION=\"${CMAKE_CXX_COMPILER_VERSION}\""
)

install(
	FILES 
	"$<TARGET_FILE:Json-Performance>"
	DESTINATION "$<IF:$<CONFIG:Debug>,bin,bin>"
	OPTIONAL
)
