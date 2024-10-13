vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO realtimechris/jsonifier
    REF "v${VERSION}"    
    SHA512 5ba01859139d6b3c9c538cebb44c5512d85adf43be469bdf35acd2d3dcc85b9202daf72b8501b50e9fd2d7e726bf678e2ef823a48c116c39b2db2e5e30601b5e
    HEAD_REF main
)

set(VCPKG_BUILD_TYPE release) # header-only

vcpkg_cmake_configure(
    SOURCE_PATH "${SOURCE_PATH}"
)

vcpkg_cmake_install()

vcpkg_install_copyright(FILE_LIST "${SOURCE_PATH}/License.md")
