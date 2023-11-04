vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO realtimechris/jsonifier
    REF "v${VERSION}"
    SHA512 d1dd5da4295111e9b334b4787fc918b9463a7ecd03af0f06a20a16088675b035f66122a28047ecbd3148506cba23a314b45fde4a3f7348bd06953ff40f52331f
    HEAD_REF main
)
vcpkg_cmake_configure(
    SOURCE_PATH "${SOURCE_PATH}"
)
vcpkg_cmake_install()
file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug")
vcpkg_install_copyright(FILE_LIST "${SOURCE_PATH}/License.md")
