vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO realtimechris/jsonifier
    REF "v${VERSION}"    
    SHA512 da9a0bb9238aabd4e3289762029c4e976216cfaa1c494bfb03bfae197106550084a280a6451db6f447e260ac7ea0806d1d5a3fe05ffec173f7ce6c102c1cdbd7
    HEAD_REF main
)

set(VCPKG_BUILD_TYPE release) # header-only

vcpkg_cmake_configure(
    SOURCE_PATH "${SOURCE_PATH}"
)

vcpkg_cmake_install()

vcpkg_install_copyright(FILE_LIST "${SOURCE_PATH}/License.md")
