vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO realtimechris/jsonifier
    REF "v${VERSION}"    
    SHA512 5ff2f917cefb9f1194226a6aa9ac05d835a5b13ac11a77d1a678f4eb23d6bb34e38aaf2aa9298fe1bc26aeb82b29739daf38482e7013e2559fec23b86ae9cc8e
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
