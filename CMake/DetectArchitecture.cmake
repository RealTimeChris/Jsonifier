include(CheckCXXSourceRuns)

if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    set(AVX "AVX" "/arch:AVX" "__m128 a{}.__m128 b{}.auto result = _mm_testz_ps(a, b)")
    set(AVX2 "AVX2" "/arch:AVX2" "auto result = _mm256_extract_epi64(__m256i{}, 0)")
    set(AVX512F "AVX512F" "/arch:AVX512" "auto result = _mm512_add_ps(__m512i{}, __m512i{})")
    set(AVX512BW "AVX512BW" "/arch:AVX512" "auto result = _mm512_cmplt_epu8_mask(__m512i{}, __m512i{})")
else()
    set(AVX "AVX" "-mavx" "__m128 a{}.__m128 b{}.auto result = _mm_testz_ps(a, b)")
    set(AVX2 "AVX2" "-mavx2" "auto result = _mm256_extract_epi64(__m256i{}, 0)")
    set(AVX512F "AVX512F" "-mavx512f" "auto result = _mm512_add_ps(__m512i{}, __m512i{})")
    set(AVX512BW "AVX512BW" "-mavx512bw" "auto result = _mm512_cmplt_epu8_mask(__m512i{}, __m512i{})")
endif()

set(AVX_TYPE 0)

list(APPEND INSTRUCTION_SETS AVX)
list(APPEND INSTRUCTION_SETS AVX2)
list(APPEND INSTRUCTION_SETS AVX512F)
list(APPEND INSTRUCTION_SETS AVX512BW)

foreach(INSTRUCTION_SET ${INSTRUCTION_SETS})
    set(CURRENT_LIST ${${INSTRUCTION_SET}})
    list(GET CURRENT_LIST 0 INSTRUCTION_SET_NAME)
    list(GET CURRENT_LIST 1 INSTRUCTION_SET_FLAG)
    list(GET CURRENT_LIST 2 INSTRUCTION_SET_INTRINSIC)

    string(REPLACE "." ";" INSTRUCTION_SET_INTRINSIC "${INSTRUCTION_SET_INTRINSIC}")

    set(INSTRUCTION_SET_CODE "
        #include <immintrin.h>
        #include <stdint.h>
        int main()
        {
            ${INSTRUCTION_SET_INTRINSIC};
            return 0;
        }
    ")

    unset(INSTRUCTION_SET_SUPPORTED CACHE)
    set(CMAKE_REQUIRED_FLAGS "")
    set(CMAKE_REQUIRED_FLAGS "${INSTRUCTION_SET_FLAG}")
    CHECK_CXX_SOURCE_RUNS("${INSTRUCTION_SET_CODE}" INSTRUCTION_SET_SUPPORTED)
    if(INSTRUCTION_SET_NAME STREQUAL "AVX" AND NOT INSTRUCTION_SET_SUPPORTED)
        set(AVX_TYPE 127)
        return()
    elseif (INSTRUCTION_SET_NAME STREQUAL "AVX2" AND NOT INSTRUCTION_SET_SUPPORTED)
        set(AVX_TYPE 126)
        return()
    elseif ((INSTRUCTION_SET_NAME STREQUAL "AVX512F" AND NOT INSTRUCTION_SET_SUPPORTED) OR (INSTRUCTION_SET_NAME STREQUAL "AVX512BW" AND NOT INSTRUCTION_SET_SUPPORTED))
        set(AVX_TYPE 125)
        return()
    endif()
endforeach()

set(AVX_TYPE 124)