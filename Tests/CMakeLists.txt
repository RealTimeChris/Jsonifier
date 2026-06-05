cmake_minimum_required(VERSION 3.18)

project(
  jsonifier_unit_tests
  VERSION "${PRODUCT_VERSION}"
  LANGUAGES CXX
)

set(CMAKE_INSTALL_PREFIX "${CMAKE_CURRENT_SOURCE_DIR}/Install/")

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

include(FetchContent)

FetchContent_Declare(
  rt-ut
  GIT_REPOSITORY https://github.com/RealTimeChris/rt-ut.git
  GIT_TAG main
  GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(rt-ut)

add_executable(
  jsonifier_unit_tests 
  "./main.cpp"
)

target_link_libraries(
	jsonifier_unit_tests PUBLIC 
	Jsonifier::Jsonifier
	rt-ut::rt-ut
)

target_compile_definitions(
	jsonifier_unit_tests PUBLIC 
	"BASE_PATH=\"${CMAKE_CURRENT_SOURCE_DIR}/\""
	"JSONIFIER_COMMIT=\"${JSONIFIER_COMMIT_HASH}\""
	"SIMDJSON_COMMIT=\"${SIMDJSON_COMMIT_HASH}\""
	"GLAZE_COMMIT=\"${GLAZE_COMMIT_HASH}\""
	"OPERATING_SYSTEM_NAME=\"${CMAKE_SYSTEM_NAME}\""
	"OPERATING_SYSTEM_VERSION=\"${CMAKE_SYSTEM_VERSION}\""
	"COMPILER_ID=\"${CMAKE_CXX_COMPILER_ID}\""
	"COMPILER_VERSION=\"${CMAKE_CXX_COMPILER_VERSION}\""
)

if (WIN32)
	install(
		FILES 
		"$<TARGET_PDB_FILE:jsonifier_unit_tests>"
		DESTINATION "bin"
		OPTIONAL
	)
endif()

install(
	FILES 
	"$<TARGET_FILE:jsonifier_unit_tests>"
	DESTINATION "bin"
	OPTIONAL
)
