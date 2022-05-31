set -e
source "scripts/common/bash/common.sh"
set -x

define_basic_variables "openssl-1.1.1n" "ios" "universal" "debug"
define_build_folders
if ! check_sentinel_exists "${BUILD_FOLDER}/lib/libssl.a"; then
    exit 1;
fi

if ! macos_lipo_verify_library_architecture "${BUILD_FOLDER}/lib/libssl.a" "i386"; then
    exit 1
fi
if ! macos_lipo_verify_library_architecture "${BUILD_FOLDER}/lib/libssl.a" "x86_64"; then
    exit 1
fi
if ! macos_lipo_verify_library_architecture "${BUILD_FOLDER}/lib/libssl.a" "armv7"; then
    exit 1
fi
if ! macos_lipo_verify_library_architecture "${BUILD_FOLDER}/lib/libssl.a" "armv7s"; then
    exit 1
fi
if ! macos_lipo_verify_library_architecture "${BUILD_FOLDER}/lib/libssl.a" "arm64"; then
    exit 1
fi
