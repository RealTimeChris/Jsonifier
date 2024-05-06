cmake_minimum_required(VERSION 3.18)

set(PROJECT_NAME "Jsonifier")
set(CMAKE_CXX_STANDARD 20)

set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_CURRENT_SOURCE_DIR}/CMake;")

project("${PROJECT_NAME}")

include("CollectASMCompiler")
collect_asm_compiler()
enable_language(ASM_NASM)

file(GLOB_RECURSE HEADERS CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/Include/jsonifier/*.hpp")
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/Source/*.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/Include/*.asm")

if(APPLE)
	string(APPEND CMAKE_ASM_NASM_FLAGS "--prefix _")
endif(APPLE)

if(WIN32)
	string(APPEND CMAKE_ASM_NASM_FLAGS "-dWIN32=1")
endif(WIN32)

add_library("${PROJECT_NAME}" STATIC "${HEADERS}" "${SOURCES}")

add_library("Jsonifier::Jsonifier" ALIAS "${PROJECT_NAME}")

set(JSONIFIER-POPCNT_EXITCODE 0 CACHE INTERNAL "For cross-compiling.")
set(JSONIFIER-LZCNT_EXITCODE 0 CACHE INTERNAL "For cross-compiling.")
set(JSONIFIER-BMI_EXITCODE 0 CACHE INTERNAL "For cross-compiling.")
set(JSONIFIER-BMI2_EXITCODE 0 CACHE INTERNAL "For cross-compiling.")
set(JSONIFIER-AVX_EXITCODE 0 CACHE INTERNAL "For cross-compiling.")
set(JSONIFIER-AVX2_EXITCODE 0 CACHE INTERNAL "For cross-compiling.")
set(JSONIFIER-AVX512_EXITCODE 0 CACHE INTERNAL "For cross-compiling.")

include("CollectVersion")
collect_version("jsonifier")
include("JsonifierDetectArchitecture")

set_target_properties(
	"${PROJECT_NAME}" PROPERTIES
	OUTPUT_NAME "jsonifier"
	CXX_STANDARD_REQUIRED ON
	CXX_EXTENSIONS OFF
)

target_include_directories(
	"${PROJECT_NAME}" PUBLIC
	"$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Include/>"
)

target_compile_definitions(
	"${PROJECT_NAME}" PUBLIC
	"$<$<COMPILE_LANGUAGE:CXX>:JSONIFIER_CPU_INSTRUCTIONS=${JSONIFIER_CPU_INSTRUCTIONS}>"
)

target_compile_options("${PROJECT_NAME}" PUBLIC "$<$<COMPILE_LANGUAGE:CXX>:${AVX_FLAG}>")

target_compile_options("${PROJECT_NAME}" PUBLIC "$<$<COMPILE_LANGUAGE:ASM>:>")

target_compile_definitions(
	"${PROJECT_NAME}" PUBLIC
	"JSONIFIER_CPU_INSTRUCTIONS=${JSONIFIER_CPU_INSTRUCTIONS}"
	"$<$<STREQUAL:${ASAN_ENABLED},TRUE>:ASAN>"
	"$<$<STREQUAL:${DEV},TRUE>:DEV>"
)

set(CONFIG_FILE_NAME "JsonifierConfig.cmake")
set(EXPORTED_TARGETS_NAME "JsonifierTargets")
set(EXPORTED_TARGETS_FILE_NAME "${EXPORTED_TARGETS_NAME}.cmake")
set(EXPORTED_TARGETS_FILE_PATH "share/jsonifier/${EXPORTED_TARGETS_FILE_NAME}")
set(VERSION_FILE_NAME "JsonifierConfigVersion.cmake")
set(RELEASE_PDB_FILE_PATH "bin/jsonifier.pdb")
set(DEBUG_PDB_FILE_PATH "debug/bin/jsonifier.pdb")

include(CMakePackageConfigHelpers)
configure_package_config_file(
	"${CMAKE_CURRENT_SOURCE_DIR}/CMake/${CONFIG_FILE_NAME}.in"
	"${CMAKE_CURRENT_BINARY_DIR}/${CONFIG_FILE_NAME}"
	INSTALL_DESTINATION "share/jsonifier"
	PATH_VARS
	EXPORTED_TARGETS_FILE_PATH
	RELEASE_PDB_FILE_PATH
	DEBUG_PDB_FILE_PATH
)

write_basic_package_version_file(
	"${CMAKE_CURRENT_BINARY_DIR}/${VERSION_FILE_NAME}"
	VERSION "${PRODUCT_VERSION}"
	COMPATIBILITY AnyNewerVersion
)

install(
	FILES
	"${CMAKE_CURRENT_BINARY_DIR}/${CONFIG_FILE_NAME}"
	"${CMAKE_CURRENT_BINARY_DIR}/${VERSION_FILE_NAME}"
	DESTINATION "share/jsonifier"
)

install(
	DIRECTORY
	"${CMAKE_CURRENT_SOURCE_DIR}/Include/"
	DESTINATION "include"
)

install(
	TARGETS "${PROJECT_NAME}"
	EXPORT "${EXPORTED_TARGETS_NAME}"
	RUNTIME DESTINATION "bin"
	ARCHIVE DESTINATION "lib"
)

install(
	EXPORT "${EXPORTED_TARGETS_NAME}"
	FILE "${EXPORTED_TARGETS_FILE_NAME}"
	NAMESPACE "Jsonifier::"
	DESTINATION "share/jsonifier"
)

if (WIN32 AND "${BUILD_SHARED_LIBS}")
	install(
		FILES 
		"$<TARGET_PDB_FILE:Jsonifier>"
		DESTINATION "${CMAKE_INSTALL_PREFIX}/bin"
		OPTIONAL
	)
endif()

if (JSONIFIER_TEST)
	add_subdirectory("Tests")
endif()