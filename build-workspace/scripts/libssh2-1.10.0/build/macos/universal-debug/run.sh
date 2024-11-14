set -e
source "scripts/common/bash/common.sh"
set -x

define_basic_variables "libssh2-1.10.0" "macos" "universal" "debug"
define_basic_docker_variables

echo "--- building on mac ---"
./${BUILD_SCRIPTS}/run-direct.sh
