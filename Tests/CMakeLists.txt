cmake_minimum_required(VERSION 3.18)

project(
  "Json-Performance"
  VERSION 0.1
  LANGUAGES CXX
)

include(FetchContent)

FetchContent_Declare(
   Jsonifier
   GIT_REPOSITORY https://github.com/RealTimeChris/Jsonifier.git
   GIT_TAG main
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

add_executable("${PROJECT_NAME}" "main.cpp")
target_link_libraries("${PROJECT_NAME}" PRIVATE Jsonifier::Jsonifier glaze::glaze simdjson fmt::fmt)
target_compile_features("${PROJECT_NAME}" PRIVATE cxx_std_23)
