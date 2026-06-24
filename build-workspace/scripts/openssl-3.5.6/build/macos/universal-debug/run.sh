set -e
source "scripts/common/bash/common.sh"
set -x

define_basic_variables "openssl-3.5.6" "macos" "universal" "debug"
define_basic_docker_variables

echo "--- Building directly on the mac ---"
./${BUILD_SCRIPTS}/run-direct_x86-64.sh
./${BUILD_SCRIPTS}/run-direct_arm64.sh
./${BUILD_SCRIPTS}/run-direct_universal.sh
