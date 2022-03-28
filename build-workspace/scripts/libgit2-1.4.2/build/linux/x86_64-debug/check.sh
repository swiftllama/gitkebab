set -e
source "scripts/common/bash/common.sh"
set -x

define_basic_variables "libgit2-1.4.2" "linux" "x86_64" "debug"
define_build_folders
if ! check_sentinel_exists "${BUILD_FOLDER}/lib/libgit2.a"; then
    exit 1;
fi

