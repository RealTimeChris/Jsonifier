# JsonifierDetectArchitecture.cmake - Script for detecting the CPU architecture.
# MIT License
# Copyright (c) 2023 RealTimeChris
# https://discordcoreapi.com

if (UNIX OR APPLE)
    file(WRITE "${CMAKE_CURRENT_SOURCE_DIR}/CMake/BuildFeatureTester.sh" "#!/bin/bash
\"${CMAKE_COMMAND}\" -S ./ -B ./Build -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
\"${CMAKE_COMMAND}\" --build ./Build --config=Release")
    execute_process(
        COMMAND chmod +x "${CMAKE_CURRENT_SOURCE_DIR}/CMake/BuildFeatureTester.sh"
        RESULT_VARIABLE CHMOD_RESULT
    )
    if(NOT ${CHMOD_RESULT} EQUAL 0)
        message(FATAL_ERROR "Failed to set executable permissions for BuildFeatureTester.sh")
    endif()
    execute_process(
        COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/CMake/BuildFeatureTester.sh"
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/CMake"
    )
    set(FEATURE_TESTER_FILE "${CMAKE_CURRENT_SOURCE_DIR}/CMake/Build/feature_detector")
elseif(WIN32)
    file(WRITE "${CMAKE_CURRENT_SOURCE_DIR}/CMake/BuildFeatureTester.bat" "\"${CMAKE_COMMAND}\" -S ./ -B ./Build -DCMAKE_BUILD_TYPE=Release
\"${CMAKE_COMMAND}\" --build ./Build --config=Release")
    execute_process(
        COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/CMake/BuildFeatureTester.bat"
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/CMake"
    )
    set(FEATURE_TESTER_FILE "${CMAKE_CURRENT_SOURCE_DIR}/CMake/Build/Release/feature_detector.exe")
endif()

execute_process(
    COMMAND "${FEATURE_TESTER_FILE}"
    RESULT_VARIABLE JSONIFIER_CPU_INSTRUCTIONS_NEW
)

set(AVX_FLAG "")

math(EXPR JSONIFIER_CPU_INSTRUCTIONS_NUMERIC "${JSONIFIER_CPU_INSTRUCTIONS_NEW}")
math(EXPR JSONIFIER_CPU_INSTRUCTIONS 0)

function(check_instruction_set INSTRUCTION_SET_NAME INSTRUCTION_SET_FLAG INSTRUCTION_SET_NUMERIC_VALUE)
    math(EXPR INSTRUCTION_PRESENT "( ${JSONIFIER_CPU_INSTRUCTIONS_NUMERIC} & ${INSTRUCTION_SET_NUMERIC_VALUE} )")
    if(INSTRUCTION_PRESENT)
        message(STATUS "Instruction Set Found: ${INSTRUCTION_SET_NAME}")
        math(EXPR JSONIFIER_CPU_INSTRUCTIONS "( ${JSONIFIER_CPU_INSTRUCTIONS} | ${INSTRUCTION_SET_NUMERIC_VALUE} )")
        set(AVX_FLAG "${AVX_FLAG};${INSTRUCTION_SET_FLAG}" PARENT_SCOPE)
    endif()
endfunction()

math(EXPR INSTRUCTION_PRESENT "( ${JSONIFIER_CPU_INSTRUCTIONS_NUMERIC} & 0x1 )")
if(INSTRUCTION_PRESENT)
    math(EXPR JSONIFIER_CPU_INSTRUCTIONS "${JSONIFIER_CPU_INSTRUCTIONS} | 1 << 0" OUTPUT_FORMAT DECIMAL)
endif()
math(EXPR INSTRUCTION_PRESENT "( ${JSONIFIER_CPU_INSTRUCTIONS_NUMERIC} & 0x2 )")
if(INSTRUCTION_PRESENT)
    math(EXPR JSONIFIER_CPU_INSTRUCTIONS "${JSONIFIER_CPU_INSTRUCTIONS} | 1 << 1" OUTPUT_FORMAT DECIMAL)
endif()
math(EXPR INSTRUCTION_PRESENT "( ${JSONIFIER_CPU_INSTRUCTIONS_NUMERIC} & 0x4 )")
if(INSTRUCTION_PRESENT)
    math(EXPR JSONIFIER_CPU_INSTRUCTIONS "${JSONIFIER_CPU_INSTRUCTIONS} | 1 << 2" OUTPUT_FORMAT DECIMAL)
endif()

math(EXPR INSTRUCTION_PRESENT "( ${JSONIFIER_CPU_INSTRUCTIONS_NUMERIC} & 0x8 )")
if(INSTRUCTION_PRESENT)
    math(EXPR JSONIFIER_CPU_INSTRUCTIONS "${JSONIFIER_CPU_INSTRUCTIONS} | 1 << 3" OUTPUT_FORMAT DECIMAL)
endif()
math(EXPR INSTRUCTION_PRESENT128 "( ${JSONIFIER_CPU_INSTRUCTIONS_NUMERIC} & 0x10 )")
math(EXPR INSTRUCTION_PRESENT256 "( ${JSONIFIER_CPU_INSTRUCTIONS_NUMERIC} & 0x20 )")
math(EXPR INSTRUCTION_PRESENT512 "( ${JSONIFIER_CPU_INSTRUCTIONS_NUMERIC} & 0x40 )")
if(INSTRUCTION_PRESENT512)
    math(EXPR JSONIFIER_CPU_INSTRUCTIONS "${JSONIFIER_CPU_INSTRUCTIONS} | 1 << 6" OUTPUT_FORMAT DECIMAL)
elseif(INSTRUCTION_PRESENT256)
    math(EXPR JSONIFIER_CPU_INSTRUCTIONS "${JSONIFIER_CPU_INSTRUCTIONS} | 1 << 5" OUTPUT_FORMAT DECIMAL)
elseif(INSTRUCTION_PRESENT128)
    math(EXPR JSONIFIER_CPU_INSTRUCTIONS "${JSONIFIER_CPU_INSTRUCTIONS} | 1 << 4" OUTPUT_FORMAT DECIMAL)
endif() 

if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    check_instruction_set("LzCnt" "" 0x1)
    check_instruction_set("PopCnt" "" 0x2)
    check_instruction_set("Bmi" "" 0x4)
    check_instruction_set("Neon" "" 0x8)
    check_instruction_set("Avx" "/arch:AVX" 0x10)
    check_instruction_set("Avx2" "/arch:AVX2" 0x20)
    check_instruction_set("Avx512" "/arch:AVX512" 0x40)
else()
    check_instruction_set("LzCnt" "-mlzcnt" 0x1)
    check_instruction_set("PopCnt" "-mpopcnt" 0x2)
    check_instruction_set("Bmi" "-mbmi" 0x4)
    check_instruction_set("Neon" "" 0x8)
    check_instruction_set("Avx" "-mavx;-mlzcnt;-mpopcnt;-mbmi;-mbmi2" 0x10)
    check_instruction_set("Avx2" "-mavx2;-mavx;-mlzcnt;-mpopcnt;-mbmi;-mbmi2" 0x20)
    check_instruction_set("Avx512" "-mavx512f;-mavx2;-mavx;-mlzcnt;-mpopcnt;-mbmi;-mbmi2" 0x40)
endif()

set(AVX_FLAG "${AVX_FLAG}" CACHE STRING "AVX flags" FORCE)
set(JSONIFIER_CPU_INSTRUCTIONS "${JSONIFIER_CPU_INSTRUCTIONS}" CACHE STRING "CPU Instruction Sets" FORCE)

# Generate JsonifierCPUInstructions.hpp
file(WRITE "${CMAKE_CURRENT_SOURCE_DIR}/Include/jsonifier/ISA/JsonifierCPUInstructions.hpp" "/*
	MIT License

	Copyright (c) 2023 RealTimeChris

	Permission is hereby granted, free of charge, to any person obtaining a copy of this
	software and associated documentation files (the \"Software\"), to deal in the Software
	without restriction, including without limitation the rights to use, copy, modify, merge,
	publish, distribute, sublicense, and/or sell copies of the Software, and to permit
	persons to whom the Software is furnished to do so, subject to the following conditions:

	The above copyright notice and this permission notice shall be included in all copies or
	substantial portions of the Software.

	THE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
	INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
	PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE
	FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
	OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
	DEALINGS IN THE SOFTWARE.
*/
/// https://github.com/RealTimeChris/jsonifier
/// Updated: Sep 3, 2024
#pragma once

#undef JSONIFIER_CPU_INSTRUCTIONS
#define JSONIFIER_CPU_INSTRUCTIONS ${JSONIFIER_CPU_INSTRUCTIONS}

#if (JSONIFIER_CPU_INSTRUCTIONS & (1 << 0))
	#define JSONIFIER_POPCNT 
#endif
#if (JSONIFIER_CPU_INSTRUCTIONS & (1 << 1))
	#define JSONIFIER_LZCNT
#endif
#if (JSONIFIER_CPU_INSTRUCTIONS & (1 << 2))
	#define JSONIFIER_BMI
#endif
#if (JSONIFIER_CPU_INSTRUCTIONS & (1 << 3))
	#define JSONIFIER_NEON
#endif
#if (JSONIFIER_CPU_INSTRUCTIONS & (1 << 4))
	#define JSONIFIER_AVX
#endif
#if (JSONIFIER_CPU_INSTRUCTIONS & (1 << 5))
	#define JSONIFIER_AVX
	#define JSONIFIER_AVX2
#endif
#if (JSONIFIER_CPU_INSTRUCTIONS & (1 << 6))
	#define JSONIFIER_AVX
	#define JSONIFIER_AVX2
	#define JSONIFIER_AVX512
#endif

#if (defined(JSONIFIER_AVX) | defined(JSONIFIER_AVX2) || defined(JSONIFIER_AVX512) || defined(JSONIFIER_POPCNT) || defined(JSONIFIER_BMI) || defined(JSONIFIER_LZCNT))
	#define JSONIFIER_ANY 
#endif

#if (defined(JSONIFIER_AVX) || defined(JSONIFIER_AVX2) || defined(JSONIFIER_AVX512))
	#define JSONIFIER_ANY_AVX
#endif")