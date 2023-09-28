vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO realtimechris/jsonifier
    REF "v${VERSION}"
    SHA512 30afd7fd7c6094690c329b3e5aa5aefdf5c55a862f0021c47a8085843c89d6ad3351203289c1f4423c53a53124fa9ee245f54202206d60119191ed32924093c0
    HEAD_REF main
)

vcpkg_cmake_configure(
    SOURCE_PATH "${SOURCE_PATH}"
)

vcpkg_cmake_install()

file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug")

vcpkg_install_copyright(FILE_LIST "${SOURCE_PATH}/License.md")
