set(CMAKE_CXX_STANDARD 20)

set(CMAKE_INSTALL_PREFIX "${CMAKE_CURRENT_SOURCE_DIR}/Install/")

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

add_executable(
  "Json-Performance" 
  "./main.cpp"
)

target_link_libraries(
	"Json-Performance" PUBLIC 
  Jsonifier::Jsonifier glaze::glaze simdjson
)

target_compile_options(
	"Json-Performance" PUBLIC
	"$<$<CXX_COMPILER_ID:CLANG>:-Wextra>"
	"$<$<CXX_COMPILER_ID:MSVC>:/bigobj>"
	"$<$<CXX_COMPILER_ID:CLANG>:-Wall>"
	"$<$<CXX_COMPILER_ID:GNU>:-Wextra>"
	"$<$<CXX_COMPILER_ID:MSVC>:/Wall>"
	"$<$<CXX_COMPILER_ID:MSVC>:/EHsc>"
	"$<$<CXX_COMPILER_ID:GNU>:-Wall>"
	"$<$<CXX_COMPILER_ID:MSVC>:/Zi>"
)

target_link_options(
	"Json-Performance" PUBLIC
	"$<$<CXX_COMPILER_ID:GNU>:$<$<STREQUAL:${ASAN_ENABLED},TRUE>:-fsanitize=address>>"
	"$<$<CXX_COMPILER_ID:MSVC>:/DEBUG>"
	"$<$<CXX_COMPILER_ID:MSVC>:/OPT:REF>"
	"$<$<CXX_COMPILER_ID:MSVC>:/OPT:ICF>"
)

if (WIN32)
	install(
		FILES 
		"$<TARGET_PDB_FILE:Json-Performance>"
		DESTINATION "$<IF:$<CONFIG:Debug>,bin,bin>"
		OPTIONAL
	)
endif()

target_compile_definitions(
	"Json-Performance" PUBLIC 
	"JSON_TEST_PATH=\"${CMAKE_CURRENT_SOURCE_DIR}/ConformanceTests\""
	"JSON_PATH=\"${CMAKE_CURRENT_SOURCE_DIR}/../Json\""
	"README_PATH=\"${CMAKE_CURRENT_SOURCE_DIR}/ReadMe.md\""
)

install(
	FILES 
	"$<TARGET_FILE:Json-Performance>"
	DESTINATION "$<IF:$<CONFIG:Debug>,bin,bin>"
	OPTIONAL
)