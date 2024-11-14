set -e
source "scripts/common/bash/common.sh"
set -x

define_basic_variables "libiconv-1.16" "macos" "universal" "debug"
define_build_folders
if ! check_sentinel_exists "${BUILD_FOLDER}/lib/libiconv.a"; then
    exit 1;
fi
