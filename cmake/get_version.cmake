# get_version.cmake - Script for collecting the current version of the library.
# MIT License
# Copyright (c) 2026 RealTimeChris
set(auth_header "")
if(DEFINED ENV{GITHUB_TOKEN})
    set(auth_header "Authorization: Bearer $ENV{GITHUB_TOKEN}")
endif()

file(DOWNLOAD 
    "https://api.github.com/repos/realtimechris/jsonifier/releases/latest" 
    "${CMAKE_CURRENT_BINARY_DIR}/jsonifier_latest.json"
    HTTPHEADER "User-Agent: CMake-Fetch-Script"
    HTTPHEADER "${auth_header}"
    TLS_VERIFY OFF
    STATUS download_status
)

list(GET download_status 0 status_code)
list(GET download_status 1 status_message)

if(NOT status_code EQUAL 0)
    message(WARNING "Failed to fetch version info from GitHub API (${status_message}). Falling back to default version.")
    set(CLEAN_VERSION "0.0.0")
else()
    file(READ "${CMAKE_CURRENT_BINARY_DIR}/jsonifier_latest.json" json_content)
    string(JSON raw_tag GET "${json_content}" "tag_name")
    string(REGEX REPLACE "^v" "" CLEAN_VERSION "${raw_tag}")
endif()

file(READ "${CMAKE_CURRENT_BINARY_DIR}/jsonifier_latest.json" json_content)
string(JSON raw_tag GET "${json_content}" "tag_name")
string(REGEX REPLACE "^v" "" CLEAN_VERSION "${raw_tag}")