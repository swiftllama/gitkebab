set -e
source "scripts/common/bash/common.sh"
set -x

define_basic_variables "libssh2-1.10.0" "ios" "universal" "debug"
define_build_folders
if ! check_sentinel_exists "${BUILD_FOLDER}/lib/libssh2.a"; then
    exit 1;
fi
