cmake_minimum_required(VERSION 3.18)

project(
  "Json-Performance"
  VERSION 0.1
  LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 23)

if (GITHUB_BRANCH_TYPE STREQUAL "refs/heads/main")
    set(BRANCH "main")
else()
    set(BRANCH "dev")
endif()
include(FetchContent)

FetchContent_Declare(
    Jsonifier
    GIT_REPOSITORY https://github.com/RealTimeChris/Jsonifier.git
    GIT_TAG "${BRANCH}"
)
FetchContent_MakeAvailable(Jsonifier)

FetchContent_Declare(
    glaze
    GIT_REPOSITORY https://github.com/stephenberry/glaze.git
    GIT_TAG main
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
    SYSTEM TRUE
    GIT_TAG 9.1.0
    GIT_REPOSITORY https://github.com/fmtlib/fmt.git
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

target_link_libraries(
    "${PROJECT_NAME}" PRIVATE 
    Jsonifier glaze::glaze simdjson fmt::fmt
)
