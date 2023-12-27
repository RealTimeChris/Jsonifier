vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO realtimechris/jsonifier
    REF "v${VERSION}"    
    SHA512 1c2706accf661df623f4f7cf7987870158f853ada1851a5a82a2d461a0c53141fd594a097daacbf9f3da9f5cdf677597b3612638dff6190853335df5f0e08011
    HEAD_REF main
    PATCHES
        uninstall-head.patch
)

set(VCPKG_BUILD_TYPE release) # header-only

vcpkg_cmake_configure(
    SOURCE_PATH "${SOURCE_PATH}"
)

vcpkg_cmake_install()

vcpkg_install_copyright(FILE_LIST "${SOURCE_PATH}/License.md")
