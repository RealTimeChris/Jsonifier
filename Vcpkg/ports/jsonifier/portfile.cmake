vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO realtimechris/jsonifier
    REF "v${VERSION}"
    SHA512 9889ef24562fcc59f5d819ab20b3a42b2b06142b1dec70627f3d44cf4d19ff1bc42f77061c4e79e0d65e117a2375a5eb40863f19da776fdc52476d04b78a6c9e
    HEAD_REF main
)

vcpkg_cmake_configure(
    SOURCE_PATH "${SOURCE_PATH}"
)

vcpkg_cmake_install()

file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug")

vcpkg_install_copyright(FILE_LIST "${SOURCE_PATH}/License.md")
