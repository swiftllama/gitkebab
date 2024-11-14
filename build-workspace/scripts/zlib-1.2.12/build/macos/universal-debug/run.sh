set -e
source "scripts/common/bash/common.sh"
set -x

define_basic_variables "zlib-1.2.12" "macos" "universal" "debug"
define_basic_docker_variables

echo "--- Building directly on mac ---"
./${BUILD_SCRIPTS}/run-direct.sh
