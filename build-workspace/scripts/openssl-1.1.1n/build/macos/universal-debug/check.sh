set -e
source "scripts/common/bash/common.sh"
set -x

define_basic_variables "openssl-1.1.1n" "macos" "universal" "debug"
define_build_folders
if ! check_sentinel_exists "${BUILD_FOLDER}/lib/libssl.a"; then
    exit 1;
fi
