set -e
source "scripts/common/bash/common.sh"
set -x

define_basic_variables "openssl-1.1.1n" "macos" "universal" "debug"
define_basic_docker_variables

echo "--- Entering docker build environment ---"
./${BUILD_SCRIPTS}/run-direct_x86-64.sh
./${BUILD_SCRIPTS}/run-direct_arm64.sh
./${BUILD_SCRIPTS}/run-direct_universal.sh
