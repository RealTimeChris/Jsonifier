vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO realtimechris/jsonifier
    REF "v${VERSION}"
    # Auto-generated by release CI action at realtimechris/Jsonifier
    SHA512 b8a907604acf99101a2a02d667db541a0fd0d2d81d19832efd171a84472e20d787f8d08f351c16a1cd2d4a6597744203f1124f2b40d75a926e738c394eed9980
	HEAD_REF main
)

vcpkg_cmake_configure(
    SOURCE_PATH "${SOURCE_PATH}"
)

vcpkg_cmake_install()

file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug")

vcpkg_install_copyright(FILE_LIST "${SOURCE_PATH}/License.md")
