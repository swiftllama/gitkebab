set -e
source "scripts/common/bash/common.sh"
set -x

define_basic_variables "pcre-8.45" "ios" "universal" "debug"
define_build_folders
if ! check_sentinel_exists "${BUILD_FOLDER}/lib/libpcre.a"; then
    exit 1;
fi

if ! macos_lipo_verify_library_architecture "${BUILD_FOLDER}/lib/libpcre.a" "i386"; then
    exit 1
fi
if ! macos_lipo_verify_library_architecture "${BUILD_FOLDER}/lib/libpcre.a" "x86_64"; then
    exit 1
fi
if ! macos_lipo_verify_library_architecture "${BUILD_FOLDER}/lib/libpcre.a" "armv7"; then
    exit 1
fi
if ! macos_lipo_verify_library_architecture "${BUILD_FOLDER}/lib/libpcre.a" "armv7s"; then
    exit 1
fi
if ! macos_lipo_verify_library_architecture "${BUILD_FOLDER}/lib/libpcre.a" "arm64"; then
    exit 1
fi
