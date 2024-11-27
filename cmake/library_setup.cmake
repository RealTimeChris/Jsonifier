# library_setup.cmake - Script for detecting the CPU architecture.
# MIT License
# Copyright (c) 2026 RealTimeChris

add_library(${PROJECT_NAME} INTERFACE)
add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

set(JSONIFIER_COMPILE_DEFINITIONS
    JSONIFIER_ARCH_X64=$<IF:$<OR:$<STREQUAL:${CMAKE_SYSTEM_PROCESSOR},x86_64>,$<STREQUAL:${CMAKE_SYSTEM_PROCESSOR},AMD64>>,1,0>
    JSONIFIER_ARCH_ARM64=$<IF:$<OR:$<STREQUAL:${CMAKE_SYSTEM_PROCESSOR},aarch64>,$<STREQUAL:${CMAKE_SYSTEM_PROCESSOR},ARM64>,$<STREQUAL:${CMAKE_SYSTEM_PROCESSOR},arm64>>,1,0>
    JSONIFIER_PLATFORM_WINDOWS=$<IF:$<PLATFORM_ID:Windows>,1,0>
    JSONIFIER_PLATFORM_LINUX=$<IF:$<PLATFORM_ID:Linux>,1,0>
    JSONIFIER_PLATFORM_MAC=$<IF:$<PLATFORM_ID:Darwin>,1,0>
    JSONIFIER_COMPILER_CLANG=$<IF:$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>>,1,0>
    JSONIFIER_COMPILER_MSVC=$<IF:$<CXX_COMPILER_ID:MSVC>,1,0>
    JSONIFIER_COMPILER_GCC=$<IF:$<CXX_COMPILER_ID:GNU>,1,0>
    JSONIFIER_ASAN=$<IF:$<BOOL:${JSONIFIER_ASAN_EFFECTIVE}>,1,0>
    JSONIFIER_UBSAN=$<IF:$<BOOL:${JSONIFIER_UBSAN_EFFECTIVE}>,1,0>
    JSONIFIER_DEV=$<IF:$<BOOL:${JSONIFIER_DEV}>,1,0>
    "JSONIFIER_INLINE=$<IF:$<CONFIG:Release>,$<IF:$<CXX_COMPILER_ID:MSVC>,[[msvc::forceinline]] inline,inline __attribute__((always_inline))>,$<IF:$<CXX_COMPILER_ID:MSVC>,[[msvc::noinline]],__attribute__((noinline))>>"
    "JSONIFIER_CLANG_INLINE=$<IF:$<CONFIG:Release>,$<IF:$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>>,inline __attribute__((always_inline)),$<IF:$<CXX_COMPILER_ID:MSVC>,inline,inline>>,$<IF:$<CXX_COMPILER_ID:MSVC>,[[msvc::noinline]],__attribute__((noinline))>>"
    "JSONIFIER_LIFETIME_BOUND=$<IF:$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>>,[[clang::lifetimebound]],$<IF:$<CXX_COMPILER_ID:MSVC>,[[msvc::lifetimebound]],>>"
    $<$<CXX_COMPILER_ID:MSVC>:NOMINMAX;WIN32_LEAN_AND_MEAN>
    $<$<CONFIG:Release>:NDEBUG>
    $<$<CONFIG:Debug>:DEBUG;_DEBUG>
)

if(NOT DEFINED JSONIFIER_CPU_INSTRUCTIONS)
    include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/JsonifierDetectArchitecture.cmake)
endif()

target_include_directories(${PROJECT_NAME}
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_compile_options(${PROJECT_NAME}
    INTERFACE ${JSONIFIER_SIMD_FLAGS}
)

target_compile_definitions(${PROJECT_NAME}
    INTERFACE ${JSONIFIER_COMPILE_DEFINITIONS}
)
