set -e
source "scripts/common/bash/common.sh"
set -x

define_basic_variables "openssl-1.1.1k" "linux" "x86_64" "release"
define_build_folders
if ! check_sentinel_exists "${BUILD_FOLDER}/lib/libssl.a"; then
    exit 1;
fi
