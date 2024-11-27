vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO realtimechris/jsonifier
    REF "v${VERSION}"    
    SHA512 0e08b5114a09f3e5cf41d245358baadecdd1559deba16a71dbb5a953882d9fc07587e7b27b4fbcbf810f8b2554c6afa1038b24447da51ea266b2557ff0d7a0d2
    HEAD_REF main
)

set(VCPKG_BUILD_TYPE release) # header-only

vcpkg_cmake_configure(
    SOURCE_PATH "${SOURCE_PATH}"
)

vcpkg_cmake_install()

vcpkg_install_copyright(FILE_LIST "${SOURCE_PATH}/License.md")
