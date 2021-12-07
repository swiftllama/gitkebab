set -e
source "scripts/common/bash/common.sh"
set -x

define_basic_variables "cmocka-1.1.5" "linux" "x86_64" "release"
define_build_folders
if ! check_sentinel_exists "${BUILD_FOLDER}/lib/libcmocka-static.a"; then
    exit 1;
fi

    
