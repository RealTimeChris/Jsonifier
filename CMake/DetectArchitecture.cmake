include(CheckCXXSourceRuns)

function(check_instruction_set INSTRUCTION_SET_NAME INSTRUCTION_SET_FLAG INSTRUCTION_SET_INTRINSIC)

    set(INSTRUCTION_SET_CODE "
        #include <immintrin.h>
        #include <stdint.h>
        int main()
        {
            ${INSTRUCTION_SET_INTRINSIC};
            return 0;
        }
    ")

    set(CMAKE_REQUIRED_FLAGS "${INSTRUCTION_SET_FLAG}")
    CHECK_CXX_SOURCE_RUNS("${INSTRUCTION_SET_CODE}" "${INSTRUCTION_SET_NAME}")
    if(${INSTRUCTION_SET_NAME})
        math(EXPR AVX_TYPE "${AVX_TYPE} + 1")
        set(AVX_TYPE "${AVX_TYPE}" PARENT_SCOPE)
        set(AVX_FLAG "${INSTRUCTION_SET_FLAG}" PARENT_SCOPE)
        set(AVX_NAME "${INSTRUCTION_SET_NAME}" PARENT_SCOPE)
    else()
        message(STATUS "Instruction set ${INSTRUCTION_SET_NAME} not supported. Falling back to the previous instruction set.")
        return()
    endif()
endfunction()

if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    set(INSTRUCTION_SETS
        "AVX?/arch:AVX?auto result = _mm_testz_ps(__m128{}, __m128{})"
        "AVX2?/arch:AVX2?auto result = _mm256_extract_epi64(__m256i{}, 0)"
        "AVX512?/arch:AVX512?auto result = _mm512_add_ps(__m512i{}, __m512i{}).auto result2 = _mm512_cmplt_epu8_mask(__m512i{}, __m512i{})"
    )
else()
    set(INSTRUCTION_SETS
        "AVX?-mavx.-mpclmul.-mbmi?auto result = _mm_testz_ps(__m128{}, __m128{})"
        "AVX2?-mavx2.-mavx.-mpclmul.-mbmi?auto result = _mm256_extract_epi64(__m256i{}, 0)"
        "AVX512?-mavx512bw.-mavx512f.-mavx2.-mavx.-mpclmul.-mbmi?auto result = _mm512_add_ps(__m512i{}, __m512i{}).auto result2 = _mm512_cmplt_epu8_mask(__m512i{}, __m512i{})"
    )
endif()

set(CMAKE_REQUIRED_FLAGS_SAVE "${CMAKE_REQUIRED_FLAGS}")

set(AVX_NAME "Fallback")
set(AVX_TYPE 124)

foreach(INSTRUCTION_SET IN LISTS INSTRUCTION_SETS)
    string(REPLACE "?" ";" CURRENT_LIST "${INSTRUCTION_SET}")
    list(GET CURRENT_LIST 0 INSTRUCTION_SET_NAME)
    list(GET CURRENT_LIST 1 INSTRUCTION_SET_FLAG)
    string(REPLACE "." ";" INSTRUCTION_SET_FLAG "${INSTRUCTION_SET_FLAG}")
    list(GET CURRENT_LIST 2 INSTRUCTION_SET_INTRINSIC)
    string(REPLACE "." ";" INSTRUCTION_SET_INTRINSIC "${INSTRUCTION_SET_INTRINSIC}")
    check_instruction_set("${INSTRUCTION_SET_NAME}" "${INSTRUCTION_SET_FLAG}" "${INSTRUCTION_SET_INTRINSIC}")
endforeach()

message(STATUS "Detected CPU Architecture: ${AVX_NAME}")
set(CMAKE_REQUIRED_FLAGS "${CMAKE_REQUIRED_FLAGS_SAVE}")
