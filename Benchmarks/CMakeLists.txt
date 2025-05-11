cmake_minimum_required(VERSION 3.18)

project(
  "Json-Performance"
  VERSION "${PRODUCT_VERSION}"
  LANGUAGES CXX
)

set(CMAKE_INSTALL_PREFIX "${CMAKE_CURRENT_SOURCE_DIR}/Install/")

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

include(FetchContent)
include(GetCommitHash.cmake)

FetchContent_Declare(
  glaze
  GIT_REPOSITORY https://github.com/stephenberry/glaze.git
  GIT_TAG main
  GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(glaze)
getCommitHash("${glaze_SOURCE_DIR}" GLAZE_COMMIT_HASH)

set(BENCHMARK_ENABLE_GTEST_TESTS OFF CACHE BOOL "" FORCE)
set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE)
FetchContent_Declare(
        benchmark
        GIT_REPOSITORY "https://github.com/google/benchmark.git"
        GIT_TAG main
)
FetchContent_MakeAvailable(benchmark)
FetchContent_GetProperties(benchmark SOURCE_DIR BENCHMARK_INCLUDE_DIR)
include_directories(${BENCHMARK_INCLUDE_DIR})

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
	Jsonifier::Jsonifier glaze::glaze simdjson BenchmarkSuite::BenchmarkSuite benchmark::benchmark
)

target_compile_options(
	"Json-Performance" PUBLIC
	"$<$<CXX_COMPILER_ID:MSVC>:$<$<STREQUAL:${ASAN_ENABLED},TRUE>:/fsanitize=address>>"
	"$<$<CXX_COMPILER_ID:CLANG>:-Wnull-dereference>"
	"$<$<CXX_COMPILER_ID:CLANG>:-Wuninitialized>"
	"$<$<CXX_COMPILER_ID:CLANG>:-Wconversion>"
	"$<$<CXX_COMPILER_ID:CLANG>:-Wpedantic>"
	"$<$<CXX_COMPILER_ID:CLANG>:-Wshadow>"
	"$<$<CXX_COMPILER_ID:CLANG>:-Wextra>"
	"$<$<CXX_COMPILER_ID:CLANG>:-Wall>"
	
	"$<$<CXX_COMPILER_ID:GNU>:-Wnull-dereference>"
	"$<$<CXX_COMPILER_ID:GNU>:-Wuninitialized>"
	"$<$<CXX_COMPILER_ID:GNU>:-Wconversion>"
	"$<$<CXX_COMPILER_ID:GNU>:-Wpedantic>"
	"$<$<CXX_COMPILER_ID:GNU>:-Wshadow>"
	"$<$<CXX_COMPILER_ID:GNU>:-Wextra>"
	"$<$<CXX_COMPILER_ID:GNU>:-Wall>"

	"$<$<CXX_COMPILER_ID:MSVC>:/Wall>"
	"$<$<CXX_COMPILER_ID:MSVC>:/W4>"
)

target_link_options(
	"Json-Performance" PUBLIC
	"$<$<CXX_COMPILER_ID:GNU>:$<$<STREQUAL:${ASAN_ENABLED},TRUE>:-fsanitize=address>>"
)

target_compile_definitions(
	"Json-Performance" PUBLIC 
	"BASE_PATH=\"${CMAKE_CURRENT_SOURCE_DIR}/../\""
	"JSONIFIER_COMMIT=\"${JSONIFIER_COMMIT_HASH}\""
	"SIMDJSON_COMMIT=\"${SIMDJSON_COMMIT_HASH}\""
	"GLAZE_COMMIT=\"${GLAZE_COMMIT_HASH}\""
	"OPERATING_SYSTEM_NAME=\"${CMAKE_SYSTEM_NAME}\""
	"OPERATING_SYSTEM_VERSION=\"${CMAKE_SYSTEM_VERSION}\""
	"COMPILER_ID=\"${CMAKE_CXX_COMPILER_ID}\""
	"COMPILER_VERSION=\"${CMAKE_CXX_COMPILER_VERSION}\""
)

if (WIN32)
	install(
		FILES 
		"$<TARGET_PDB_FILE:Json-Performance>"
		DESTINATION "bin"
		OPTIONAL
	)
endif()

install(
	FILES 
	"$<TARGET_FILE:Json-Performance>"
	DESTINATION "bin"
	OPTIONAL
)
