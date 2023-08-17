cmake_minimum_required(VERSION 3.18)

project(
  "Json-Performance"
  VERSION 0.1
  LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)

if (DEFINED GITHUB_BRANCH_TYPE)
    string (REPLACE "/" ";" GITHUB_BRANCH_TYPE "${GITHUB_BRANCH_TYPE}")
    list(GET GITHUB_BRANCH_TYPE 2 BRANCH)
else()
    set(BRANCH "dev")
endif()

include(FetchContent)

FetchContent_Declare(
    Jsonifier
    GIT_REPOSITORY https://github.com/RealTimeChris/Jsonifier.git
    GIT_TAG "${BRANCH}"
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(Jsonifier)

FetchContent_Declare(
    glaze
    GIT_REPOSITORY https://github.com/stephenberry/glaze.git
    GIT_TAG main
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(glaze)

FetchContent_Declare(
    simdjson
    GIT_REPOSITORY https://github.com/RealTimeChris/simdjson.git
    GIT_TAG master
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(simdjson)

FetchContent_Declare(
    fmt
    GIT_REPOSITORY https://github.com/fmtlib/fmt.git
    GIT_TAG master
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(fmt)

add_executable(
    "${PROJECT_NAME}" 
    "main.cpp"
)

target_compile_options(
    "${PROJECT_NAME}" PRIVATE 
    "$<$<CXX_COMPILER_ID:CLANG>:-Wextra>"
	"$<$<CXX_COMPILER_ID:CLANG>:-Wall>"
	"$<$<CXX_COMPILER_ID:GNU>:-Wextra>"
	"$<$<CXX_COMPILER_ID:GNU>:-Wall>"
	"$<$<CXX_COMPILER_ID:MSVC>:/W4>"
)

target_link_options(
	"${PROJECT_NAME}" INTERFACE	
    "$<$<CXX_COMPILER_ID:CLANG>>>:-Wl,-rpath,/usr/local/opt/llvm/lib/c++>"
	"$<$<CXX_COMPILER_ID:CLANG>>>:-L/usr/local/opt/llvm/lib/c++>"
)

target_link_libraries(
	"${PROJECT_NAME}" PRIVATE
	"$<$<NOT:$<BOOL:$<STREQUAL:$<PLATFORM_ID>,Windows>>>:c++abi>"
    "$<$<NOT:$<BOOL:$<STREQUAL:$<PLATFORM_ID>,Windows>>>:c++>"
    Jsonifier glaze::glaze simdjson fmt::fmt
)
