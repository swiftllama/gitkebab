set -e
source "scripts/common/bash/common.sh"
set -x

define_basic_variables "openssl-1.1.1k" "macos" "x86_64" "debug"
define_basic_docker_variables

echo "--- Entering docker build environment ---"
./${BUILD_SCRIPTS}/run-direct.sh
