cmake_minimum_required(VERSION 3.10)
project(FeatureDetection)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(feature_detector main.cpp)

target_compile_options(feature_detector PUBLIC
    $<$<CXX_COMPILER_ID:MSVC>:/arch:AVX512>
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-march=native>
)