vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO realtimechris/jsonifier
    REF "v${VERSION}"    
    SHA512 5f7b384b82a2f5571049c82d033e06ef720c1e9ed23745d21bafc0826bbdafede900a2cbdd6e0764a8362b497e4351ccbc39970bec0a7f9b4aa977b4ab6ca0a2
    HEAD_REF main
)

set(VCPKG_BUILD_TYPE release) # header-only

vcpkg_cmake_configure(
    SOURCE_PATH "${SOURCE_PATH}"
)

vcpkg_cmake_install()

vcpkg_install_copyright(FILE_LIST "${SOURCE_PATH}/License.md")
