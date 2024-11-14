set -e
source "scripts/common/bash/common.sh"
set -x

define_basic_variables "gitkebab-head" "linux" "x86_64" "debug"
define_build_folders
if ! check_sentinel_exists "${BUILD_FOLDER}/lib/libgitkebab.so"; then
    exit 1;
fi
