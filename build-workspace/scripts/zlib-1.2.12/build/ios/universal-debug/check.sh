set -e
source "scripts/common/bash/common.sh"
set -x

define_basic_variables "zlib-1.2.12" "ios" "universal" "debug"
define_build_folders
if ! check_sentinel_exists "${BUILD_FOLDER}/lib/libz.a"; then
    exit 1;
fi
if ! macos_lipo_verify_library_architecture "${BUILD_FOLDER}/lib/libz.a" "i386"; then
    exit 1
fi
if ! macos_lipo_verify_library_architecture "${BUILD_FOLDER}/lib/libz.a" "x86_64"; then
    exit 1
fi
if ! macos_lipo_verify_library_architecture "${BUILD_FOLDER}/lib/libz.a" "armv7"; then
    exit 1
fi
if ! macos_lipo_verify_library_architecture "${BUILD_FOLDER}/lib/libz.a" "armv7s"; then
    exit 1
fi
if ! macos_lipo_verify_library_architecture "${BUILD_FOLDER}/lib/libz.a" "arm64"; then
    exit 1
fi
