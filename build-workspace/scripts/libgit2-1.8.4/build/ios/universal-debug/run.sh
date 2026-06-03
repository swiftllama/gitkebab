set -e
source "scripts/common/bash/common.sh"
set -x

define_basic_variables "libgit2-1.8.4" "ios" "universal" "debug"
define_basic_docker_variables

echo "--- Running build directly on mac ---"
./${BUILD_SCRIPTS}/run-direct_device.sh
./${BUILD_SCRIPTS}/run-direct_simulator.sh
./${BUILD_SCRIPTS}/run-direct_universal.sh
