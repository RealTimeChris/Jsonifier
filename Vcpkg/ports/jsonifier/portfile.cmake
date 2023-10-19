vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO realtimechris/jsonifier
    REF "v${VERSION}"
    SHA512 8533aec7fedf3a107dd8ae2e801e5ad41dae86f998fafb2f6fd1113873c21aa6183a66d1ef01b4e677eff04908f06ffc0e0f981d4a37c96971c0a041d51453ea
    HEAD_REF main
)

vcpkg_cmake_configure(
    SOURCE_PATH "${SOURCE_PATH}"
)

vcpkg_cmake_install()

file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug")

vcpkg_install_copyright(FILE_LIST "${SOURCE_PATH}/License.md")
