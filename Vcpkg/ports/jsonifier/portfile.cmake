vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO realtimechris/jsonifier
    REF "v${VERSION}"
    SHA512 cb19ac52d4bdcf201c2e27f4b94b8ab1fb5eb18a5431f6dbcba3b520b0eb68a957cc3cb87d6a60d8a66c9e745b342c831a40400e5b2b2fab5fc33b17da58ba67
    HEAD_REF main
)
vcpkg_cmake_configure(
    SOURCE_PATH "${SOURCE_PATH}"
)
vcpkg_cmake_install()
file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug")
vcpkg_install_copyright(FILE_LIST "${SOURCE_PATH}/License.md")
