set -e
source "scripts/common/bash/common.sh"
set -x

define_basic_variables "pcre-8.45" "linux" "x86_64" "release"
define_build_folders
if ! check_sentinel_exists "${BUILD_FOLDER}/lib/libpcre.a"; then
    exit 1;
fi