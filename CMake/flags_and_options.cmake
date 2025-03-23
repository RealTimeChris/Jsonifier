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
# Dec 17, 2022
# https://github.com/RealTimeChris/Jsonifier

option(JSONIFIER_ASAN  "Enable AddressSanitizer"            OFF)
option(JSONIFIER_UBSAN "Enable UndefinedBehaviorSanitizer"  OFF)

set(JSONIFIER_ASAN_EFFECTIVE  ${JSONIFIER_ASAN})
set(JSONIFIER_UBSAN_EFFECTIVE ${JSONIFIER_UBSAN})

if(APPLE AND CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    if(JSONIFIER_ASAN)
        message(WARNING
            "JSONIFIER_ASAN is not supported with GCC on macOS -- ignoring for this "
            "configure. Re-run with a Clang/AppleClang toolchain to enable.")
        set(JSONIFIER_ASAN_EFFECTIVE OFF)
    endif()
    if(JSONIFIER_UBSAN)
        message(WARNING
            "JSONIFIER_UBSAN is not supported with GCC on macOS -- ignoring for this "
            "configure. Re-run with a Clang/AppleClang toolchain to enable.")
        set(JSONIFIER_UBSAN_EFFECTIVE OFF)
    endif()
endif()

if(JSONIFIER_UBSAN_EFFECTIVE AND CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    message(WARNING
        "JSONIFIER_UBSAN has no effect with MSVC (no equivalent runtime). "
        "Use Clang-cl or a Clang/GCC build to get UBSan coverage.")
    set(JSONIFIER_UBSAN_EFFECTIVE OFF)
endif()

set(JSONIFIER_HOMEBREW_GCC_LIBDIR "")
if(APPLE AND CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    find_program(JSONIFIER_BREW_EXE brew)
    if(JSONIFIER_BREW_EXE)
        execute_process(
            COMMAND ${JSONIFIER_BREW_EXE} --prefix gcc
            OUTPUT_VARIABLE JSONIFIER_BREW_GCC_PREFIX
            OUTPUT_STRIP_TRAILING_WHITESPACE
            RESULT_VARIABLE JSONIFIER_BREW_RC
        )
        if(JSONIFIER_BREW_RC EQUAL 0 AND JSONIFIER_BREW_GCC_PREFIX)
            file(GLOB JSONIFIER_GCC_LIB_CANDIDATES
                "${JSONIFIER_BREW_GCC_PREFIX}/lib/gcc/*")
            if(JSONIFIER_GCC_LIB_CANDIDATES)
                list(SORT JSONIFIER_GCC_LIB_CANDIDATES)
                list(REVERSE JSONIFIER_GCC_LIB_CANDIDATES)
                list(GET JSONIFIER_GCC_LIB_CANDIDATES 0 JSONIFIER_HOMEBREW_GCC_LIBDIR)
            endif()
        endif()
    endif()
    if(NOT JSONIFIER_HOMEBREW_GCC_LIBDIR)
        message(STATUS
            "Could not auto-detect Homebrew GCC runtime path; link-time "
            "rpath for libstdc++ will not be added. Set "
            "JSONIFIER_HOMEBREW_GCC_LIBDIR manually if needed.")
    endif()
endif()

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
    "JSONIFIER_INLINE=$<IF:$<CUDA_COMPILER_ID:NVIDIA>,$<IF:$<CONFIG:Release>,__forceinline__ __host__,__noinline__ __host__>,$<IF:$<CONFIG:Release>,$<IF:$<CXX_COMPILER_ID:MSVC>,[[msvc::forceinline]] inline,inline __attribute__((always_inline))>,$<IF:$<CXX_COMPILER_ID:MSVC>,[[msvc::noinline]],__attribute__((noinline))>>>"
    "JSONIFIER_NOINLINE=$<IF:$<CUDA_COMPILER_ID:NVIDIA>,__noinline__,$<IF:$<CXX_COMPILER_ID:MSVC>,[[msvc::noinline]],__attribute__((noinline))>>"
    $<$<CXX_COMPILER_ID:MSVC>:NOMINMAX;WIN32_LEAN_AND_MEAN>
    $<$<CONFIG:Release>:NDEBUG>
    $<$<CONFIG:Debug>:DEBUG;_DEBUG>
    JSONIFIER_DEBUG=$<IF:$<CONFIG:Debug>,1,0>
)

target_compile_definitions(${PROJECT_NAME}
    INTERFACE ${JSONIFIER_COMPILE_DEFINITIONS}
)

target_include_directories(${PROJECT_NAME}
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

set(JSONIFIER_CLANG_COMPILE_OPTIONS
    $<$<AND:$<CONFIG:Release>,$<NOT:$<BOOL:${JSONIFIER_ASAN_EFFECTIVE}>>>:
        -O3
        -march=native
        -finline-functions
        -fomit-frame-pointer
        -fno-rtti
        -fno-vectorize
        -fno-slp-vectorize
        -fno-stack-protector
    >
    $<$<AND:$<CONFIG:Release>,$<BOOL:${JSONIFIER_ASAN_EFFECTIVE}>>:
        -O1
        -fno-rtti
    >
    $<$<CONFIG:Debug>:
        -O0
        -g
        -fno-omit-frame-pointer
    >
    $<$<BOOL:${JSONIFIER_ASAN_EFFECTIVE}>:
        -fsanitize=address
        -fno-omit-frame-pointer
        -fno-optimize-sibling-calls
        -fsanitize-address-use-after-scope
        -U_FORTIFY_SOURCE
        -D_FORTIFY_SOURCE=0
    >
    $<$<BOOL:${JSONIFIER_UBSAN_EFFECTIVE}>:
        -fsanitize=undefined
        -fno-sanitize-recover=all
    >
)

set(JSONIFIER_GNU_COMPILE_OPTIONS
    $<$<CONFIG:Debug>:-O0;-g>
    $<$<NOT:$<CONFIG:Debug>>:-fwhole-program>
    $<$<BOOL:${JSONIFIER_ASAN_EFFECTIVE}>:
        -fsanitize=address
        -fsanitize-address-use-after-scope
        -U_FORTIFY_SOURCE
        -D_FORTIFY_SOURCE=0
    >
    $<$<BOOL:${JSONIFIER_UBSAN_EFFECTIVE}>:
        -fsanitize=undefined
        -fno-sanitize-recover=all
    >
)

set(JSONIFIER_MSVC_COMPILE_OPTIONS
    $<$<AND:$<CONFIG:Debug>,$<NOT:$<BOOL:${JSONIFIER_ASAN_EFFECTIVE}>>>:/Od;/Zi;/RTC1>
    $<$<AND:$<CONFIG:Debug>,$<BOOL:${JSONIFIER_ASAN_EFFECTIVE}>>:/Od;/Zi>
    $<$<CONFIG:Release>:/O2;/Ob2;/GL;/GS-;/Gy>
    $<$<BOOL:${JSONIFIER_ASAN_EFFECTIVE}>:/fsanitize=address>
    /arch:AVX2
    /bigobj
)

set(JSONIFIER_COMPILE_OPTIONS
    $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>>:${JSONIFIER_CLANG_COMPILE_OPTIONS}>
    $<$<CXX_COMPILER_ID:MSVC>:${JSONIFIER_MSVC_COMPILE_OPTIONS}>
    $<$<CXX_COMPILER_ID:GNU>:${JSONIFIER_GNU_COMPILE_OPTIONS}>
    ${JSONIFIER_SIMD_FLAGS}
)

target_compile_options(${PROJECT_NAME}
    INTERFACE ${JSONIFIER_COMPILE_OPTIONS}
)

set(JSONIFIER_CLANG_LINK_OPTIONS
    $<$<BOOL:${JSONIFIER_ASAN_EFFECTIVE}>:-fsanitize=address>
    $<$<BOOL:${JSONIFIER_UBSAN_EFFECTIVE}>:-fsanitize=undefined>
    $<$<NOT:$<PLATFORM_ID:Darwin>>:-s>
    $<$<PLATFORM_ID:Darwin>:-Wl,-x>
)

set(JSONIFIER_GNU_LINK_OPTIONS
    $<$<BOOL:${JSONIFIER_ASAN_EFFECTIVE}>:-fsanitize=address>
    $<$<BOOL:${JSONIFIER_UBSAN_EFFECTIVE}>:-fsanitize=undefined>
    $<$<AND:$<PLATFORM_ID:Linux>,$<BOOL:${JSONIFIER_ASAN_EFFECTIVE}>>:-static-libasan>
    $<$<NOT:$<PLATFORM_ID:Darwin>>:-s>
    $<$<PLATFORM_ID:Darwin>:-Wl,-x>
    $<$<CONFIG:Release>:-flto>
)

set(JSONIFIER_MSVC_LINK_OPTIONS
    $<$<CONFIG:Release>:/LTCG;/OPT:REF;/OPT:ICF>
)

set(JSONIFIER_LINK_OPTIONS
    $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>>:${JSONIFIER_CLANG_LINK_OPTIONS}>
    $<$<CXX_COMPILER_ID:MSVC>:${JSONIFIER_MSVC_LINK_OPTIONS}>
    $<$<CXX_COMPILER_ID:GNU>:${JSONIFIER_GNU_LINK_OPTIONS}>
)

target_link_options(${PROJECT_NAME}
    INTERFACE ${JSONIFIER_LINK_OPTIONS}
)

if(JSONIFIER_HOMEBREW_GCC_LIBDIR)
    target_link_options(${PROJECT_NAME}
        INTERFACE
            -L${JSONIFIER_HOMEBREW_GCC_LIBDIR}
            -Wl,-rpath,${JSONIFIER_HOMEBREW_GCC_LIBDIR}
    )
endif()