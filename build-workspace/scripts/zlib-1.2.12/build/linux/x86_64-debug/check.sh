set -e
source "scripts/common/bash/common.sh"
set -x

define_basic_variables "zlib-1.2.12" "linux" "x86_64" "debug"
define_build_folders
if ! check_sentinel_exists "${BUILD_FOLDER}/lib/libz.a"; then
    exit 1;
fi
