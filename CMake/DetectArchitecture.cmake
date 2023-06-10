INCLUDE(CheckCXXSourceRuns)

SET(BMI_CODE "
  #include <immintrin.h>
  #include <stdint.h>
  int main()
  {
    int64_t a{};
    auto result = _blsmsk_u64(a);
    return 0;
 }
")

set(PCLMULQDQ_CODE "
  #include <immintrin.h>
  #include <stdint.h>
  int main()
  {
    const int8_t c{};
    __m128i a{};
    __m128i b{};
    auto result = _mm_clmulepi64_si128(a, b, c);
    return 0;
 }
")
 
SET(AVX_CODE "
  #include <immintrin.h>

  int main()
  {
    auto result = _mm_testz_ps(__m128{}, __m128{});
    return 0;
 }
")

SET(AVX2_CODE "
  #include <immintrin.h>

  int main()
  {
    auto result = _mm256_extract_epi64(__m256i{}, 0);
    return 0;
 }
")

SET(AVX512F_CODE "
  #include <immintrin.h>

  int main()
  {
    auto result = _mm512_add_ps(__m512i{}, __m512i{});
    return 0;
 }
")

SET(AVX512BW_CODE "
  #include <immintrin.h>

  int main()
  {
    auto result = _mm512_cmplt_epu8_mask(__m512i{}, __m512i{});
    return 0;
 }
")

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    set(PCLMULQDQ_FLAG "/arch:AVX")
    set(BMI_FLAG "/arch:AVX")  
    set(AVX_FLAG "/arch:AVX")
    set(AVX2_FLAG "/arch:AVX2")
    set(AVX512F_FLAG "/arch:AVX512")
    set(AVX512BW_FLAG "/arch:AVX512")
else()
    set(PCLMULQDQ_FLAG "-mpclmul")
    set(BMI_FLAG "-mbmi")  
    set(AVX_FLAG "-mavx")
    set(AVX2_FLAG "-mavx2")
    set(AVX512F_FLAG "-mavx512f")
    set(AVX512BW_FLAG "-mavx512bw")
endif()

SET(CMAKE_REQUIRED_FLAGS_SAVE "${CMAKE_REQUIRED_FLAGS}")
SET(CMAKE_REQUIRED_FLAGS "${PCLMULQDQ_FLAG}")
CHECK_CXX_SOURCE_RUNS("${PCLMULQDQ_CODE}" HAS_PCLMULQDQ)
SET(CMAKE_REQUIRED_FLAGS "${BMI_FLAG}")
CHECK_CXX_SOURCE_RUNS("${BMI_CODE}" HAS_BMI)
SET(CMAKE_REQUIRED_FLAGS "${AVX_FLAG}")
CHECK_CXX_SOURCE_RUNS("${AVX_CODE}" HAS_AVX)
SET(CMAKE_REQUIRED_FLAGS "${AVX2_FLAG}")
CHECK_CXX_SOURCE_RUNS("${AVX2_CODE}" HAS_AVX2)
SET(CMAKE_REQUIRED_FLAGS "${AVX512F_FLAG}")
CHECK_CXX_SOURCE_RUNS("${AVX512F_CODE}" HAS_AVX512F)
SET(CMAKE_REQUIRED_FLAGS "${AVX512BW_FLAG}")
CHECK_CXX_SOURCE_RUNS("${AVX512BW_CODE}" HAS_AVX512BW)
SET(CMAKE_REQUIRED_FLAGS "${CMAKE_REQUIRED_FLAGS_SAVE}")

if (NOT HAS_AVX OR NOT HAS_BMI OR NOT HAS_PCLMULQDQ)
    set(AVX_TYPE 127 PARENT_DIRECTORY)
    return()
endif()
if(NOT HAS_AVX2)
    set(AVX_TYPE 126 PARENT_DIRECTORY)
    return()
endif()
if(NOT HAS_AVX512F OR NOT HAS_AVX512BW)
    set(AVX_TYPE 125 PARENT_DIRECTORY)
    return()
endif()
set(AVX_TYPE 124 PARENT_DIRECTORY)
return()