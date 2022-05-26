set -e
source "scripts/common/bash/common.sh"
set -x

define_basic_variables "libssh2-1.9.0" "macos" "x86_64" "debug"
define_basic_docker_variables

echo "--- building on mac ---"
./${BUILD_SCRIPTS}/run-direct.sh
