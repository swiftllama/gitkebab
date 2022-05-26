set -e
source "scripts/common/bash/common.sh"
set -x

define_basic_variables "libgit2-1.1.1" "macos" "x86_64" "debug"
define_basic_docker_variables

echo "--- Running build directly on mac ---"
./${BUILD_SCRIPTS}/run-direct.sh
