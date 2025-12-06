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

set(JSONIFIER_COMPILE_DEFINITIONS
    JSONIFIER_ARCH_X64=$<IF:$<OR:$<STREQUAL:${CMAKE_SYSTEM_PROCESSOR},x86_64>,$<STREQUAL:${CMAKE_SYSTEM_PROCESSOR},AMD64>>,1,0>
    JSONIFIER_ARCH_ARM64=$<IF:$<OR:$<STREQUAL:${CMAKE_SYSTEM_PROCESSOR},aarch64>,$<STREQUAL:${CMAKE_SYSTEM_PROCESSOR},ARM64>,$<STREQUAL:${CMAKE_SYSTEM_PROCESSOR},arm64>>,1,0>
    JSONIFIER_PLATFORM_WINDOWS=$<IF:$<PLATFORM_ID:Windows>,1,0>
    JSONIFIER_PLATFORM_LINUX=$<IF:$<PLATFORM_ID:Linux>,1,0>
    JSONIFIER_PLATFORM_MAC=$<IF:$<PLATFORM_ID:Darwin>,1,0>
    JSONIFIER_COMPILER_CLANG=$<IF:$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>>,1,0>
    JSONIFIER_COMPILER_MSVC=$<IF:$<CXX_COMPILER_ID:MSVC>,1,0>
    JSONIFIER_COMPILER_GCC=$<IF:$<CXX_COMPILER_ID:GNU>,1,0>
    JSONIFIER_DEV=$<IF:$<STREQUAL:${JSONIFIER_DEV},TRUE>,1,0>
    "JSONIFIER_INLINE=$<IF:$<CUDA_COMPILER_ID:NVIDIA>,$<IF:$<CONFIG:Release>,__forceinline__ __host__,__noinline__ __host__>,$<IF:$<CONFIG:Release>,$<IF:$<CXX_COMPILER_ID:MSVC>,[[msvc::forceinline]] inline,inline __attribute__((always_inline))>,$<IF:$<CXX_COMPILER_ID:MSVC>,[[msvc::noinline]],__attribute__((noinline))>>>"
    "JSONIFIER_NOINLINE=$<IF:$<CUDA_COMPILER_ID:NVIDIA>,$<IF:$<CONFIG:Release>,__noinline__,__noinline__>,$<IF:$<CONFIG:Release>,$<IF:$<CXX_COMPILER_ID:MSVC>,[[msvc::noinline]],__attribute__((noinline))>,$<IF:$<CXX_COMPILER_ID:MSVC>,[[msvc::noinline]],__attribute__((noinline))>>>"
    $<$<CXX_COMPILER_ID:MSVC>:NOMINMAX;WIN32_LEAN_AND_MEAN>
    ${JSONIFIER_SIMD_DEFINITIONS}
)

set(JSONIFIER_CLANG_COMPILE_OPTIONS
    -O3
    -funroll-loops
    -fvectorize
    -fslp-vectorize
    -finline-functions
    -fomit-frame-pointer
    -fmerge-all-constants
    -ffunction-sections
    -fdata-sections
    -falign-functions=32
    -fno-math-errno
    -ffp-contract=on
    -fvisibility=hidden
    -fvisibility-inlines-hidden
    -fno-rtti
    -fno-asynchronous-unwind-tables
    -fno-unwind-tables
    -fno-ident
    -pipe
    -fno-common
    -fwrapv
    -Weverything
    -Wnon-virtual-dtor
    -Wno-c++98-compat
    -Wno-c++98-compat-pedantic
    -Wno-unsafe-buffer-usage
    -Wno-padded
    -Wno-c++20-compat
    -Wno-exit-time-destructors
    -Wno-c++20-extensions
)

set(JSONIFIER_APPLECLANG_COMPILE_OPTIONS 
    -O3
    -funroll-loops
    -fvectorize
    -fslp-vectorize
    -finline-functions
    -fomit-frame-pointer
    -fmerge-all-constants
    -ffunction-sections
    -fdata-sections
    -falign-functions=32
    -fno-math-errno
    -ffp-contract=on
    -fvisibility=hidden
    -fvisibility-inlines-hidden
    -fno-rtti
    -fno-asynchronous-unwind-tables
    -fno-unwind-tables
    -fno-ident
    -pipe
    -fno-common
    -fwrapv
    -Weverything
    -Wnon-virtual-dtor
    -Wno-c++98-compat
    -Wno-c++98-compat-pedantic
    -Wno-unsafe-buffer-usage
    -Wno-padded
    -Wno-c++20-compat
    -Wno-exit-time-destructors
    -Wno-poison-system-directories
    -Wno-c++20-extensions
)

set(JSONIFIER_GNU_COMPILE_OPTIONS 
    -O3
    -funroll-loops
    -finline-functions
    -fomit-frame-pointer
    -fno-math-errno
    -falign-functions=32
    -falign-loops=32
    -fprefetch-loop-arrays
    -ftree-vectorize
    -fstrict-aliasing
    -ffunction-sections
    -fdata-sections
    -fvisibility=hidden
    -fvisibility-inlines-hidden
    -fno-keep-inline-functions
    -fno-ident
    -fmerge-all-constants
    -fno-rtti
    -fgcse-after-reload
    -ftree-loop-distribute-patterns
    -fpredictive-commoning
    -funswitch-loops
    -ftree-loop-vectorize
    -ftree-slp-vectorize
    -Wall
    -Wextra
    -Wpedantic
    -Wnon-virtual-dtor
    -Wlogical-op
    -Wduplicated-cond
    -Wduplicated-branches
    -Wnull-dereference
    -Wdouble-promotion
)

set(JSONIFIER_MSVC_RELEASE_FLAGS
    /Ob3
    /Ot
    /Oy
    /GT
    $<$<NOT:$<CUDA_COMPILER_ID:NVIDIA>>:/GL>
    /fp:precise
    /Qpar
    /GS-
)

set(JSONIFIER_MSVC_COMPILE_OPTIONS
    /Gy    
    /Gw
    $<$<NOT:$<CUDA_COMPILER_ID:NVIDIA>>:/Zc:inline>    
    /Zc:throwingNew
    /W4
    $<$<NOT:$<CUDA_COMPILER_ID:NVIDIA>>:/bigobj>
    /permissive-
    /Zc:__cplusplus
    /wd4820
    /wd4324
    /wd5002
    /Zc:alignedNew
    /Zc:auto
    /Zc:forScope
    /Zc:implicitNoexcept
    /Zc:noexceptTypes
    /Zc:referenceBinding
    /Zc:rvalueCast
    /Zc:sizedDealloc
    /Zc:strictStrings
    /Zc:ternary
    /Zc:wchar_t
    $<$<CONFIG:Release>:${JSONIFIER_MSVC_RELEASE_FLAGS}>
)

set(JSONIFIER_CXX_COMPILE_OPTIONS
    $<$<CXX_COMPILER_ID:Clang>:${JSONIFIER_CLANG_COMPILE_OPTIONS}>
    $<$<CXX_COMPILER_ID:AppleClang>:${JSONIFIER_APPLECLANG_COMPILE_OPTIONS}>
    $<$<CXX_COMPILER_ID:GNU>:${JSONIFIER_GNU_COMPILE_OPTIONS}>
    $<$<CXX_COMPILER_ID:MSVC>:${JSONIFIER_MSVC_COMPILE_OPTIONS}>
)

set(JSONIFIER_COMPILE_OPTIONS
    $<$<COMPILE_LANGUAGE:CXX>:${JSONIFIER_CXX_COMPILE_OPTIONS}>
    ${JSONIFIER_SIMD_FLAGS}
)

set(JSONIFIER_LINK_OPTIONS
    $<$<AND:$<CXX_COMPILER_ID:Clang>,$<PLATFORM_ID:Darwin>>:
        -Wl,-dead_strip
        -Wl,-x
        -Wl,-S
    >
    $<$<AND:$<CXX_COMPILER_ID:AppleClang>,$<PLATFORM_ID:Darwin>>:
        -Wl,-dead_strip
        -Wl,-x
        -Wl,-S
    >
    $<$<AND:$<CXX_COMPILER_ID:GNU>,$<PLATFORM_ID:Darwin>>:
        -Wl,-dead_strip
        -Wl,-x
        -Wl,-S
    >
    $<$<AND:$<CXX_COMPILER_ID:Clang>,$<PLATFORM_ID:Linux>>:
        -Wl,--gc-sections
        -Wl,--strip-all
        -Wl,--build-id=none
        -Wl,--hash-style=gnu
        -Wl,-z,now
        -Wl,-z,relro
        -flto=thin
        -fwhole-program-vtables
    >
    $<$<AND:$<CXX_COMPILER_ID:GNU>,$<PLATFORM_ID:Linux>>:
        -Wl,--gc-sections
        -Wl,--strip-all
        -Wl,--as-needed
        -Wl,-O3
    >
    $<$<AND:$<CXX_COMPILER_ID:MSVC>,$<PLATFORM_ID:Windows>>:
        /DYNAMICBASE:NO
        /OPT:REF
        /OPT:ICF
        /INCREMENTAL:NO
        /MACHINE:X64
        /LTCG
    >
)