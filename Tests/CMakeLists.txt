set(CMAKE_CXX_STANDARD 20)

set(CMAKE_INSTALL_PREFIX "${CMAKE_SOURCE_DIR}/Install/")

if (ASAN_ENABLED STREQUAL "TRUE")
	message(STATUS "Building with address-sanitizer enabled.")
endif()

include(FetchContent)

FetchContent_Declare(
    glaze
    GIT_REPOSITORY https://github.com/stephenberry/glaze.git
    GIT_TAG main
    GIT_SHALLOW TRUE
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
    GIT_REPOSITORY https://github.com/fmtlib/fmt.git
    GIT_TAG master
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(fmt)

add_executable(
    "Json-Performance" 
    "main.cpp"
)

target_link_libraries(
	"Json-Performance" PUBLIC 
    Jsonifier::Jsonifier glaze::glaze simdjson fmt::fmt
)

target_link_options(
	"Json-Performance" PUBLIC
	"$<$<CXX_COMPILER_ID:GNU>:$<$<STREQUAL:${ASAN_ENABLED},TRUE>:-fsanitize=address>>"
    "$<$<CXX_COMPILER_ID:MSVC>:/DEBUG>"
)

target_compile_options(
	"Json-Performance" PUBLIC
	"$<$<CXX_COMPILER_ID:MSVC>:$<$<STREQUAL:${ASAN_ENABLED},TRUE>:/fsanitize=address>>"
	"$<$<CXX_COMPILER_ID:CLANG>:$<$<STREQUAL:${ASAN_ENABLED},TRUE>:-fsanitize=address>>"
	"$<$<CXX_COMPILER_ID:GNU>:$<$<STREQUAL:${ASAN_ENABLED},TRUE>:-fsanitize=address>>"
	"$<$<CXX_COMPILER_ID:CLANG>:-pedantic>"
	"$<$<CXX_COMPILER_ID:GNU>:-pedantic>"
	"$<$<CXX_COMPILER_ID:CLANG>:-Wextra>"
	"$<$<CXX_COMPILER_ID:CLANG>:-Wall>"
	"$<$<CXX_COMPILER_ID:GNU>:-Wextra>"
	"$<$<CXX_COMPILER_ID:GNU>:-Wall>"
	"$<$<CXX_COMPILER_ID:MSVC>:/W4>"
	"$<$<CXX_COMPILER_ID:MSVC>:/Zi>"
)

if (WIN32)
	install(
		FILES 
		"$<TARGET_PDB_FILE:Json-Performance>"
		DESTINATION "$<IF:$<CONFIG:Debug>,bin,bin>"
		OPTIONAL
	)
endif()

install(
	FILES 
	"$<TARGET_FILE:Json-Performance>"
	DESTINATION "$<IF:$<CONFIG:Debug>,bin,bin>"
	OPTIONAL
)