set -e
source "scripts/common/bash/common.sh"
set -x

define_basic_variables "zlib-1.2.12" "ios" "universal" "debug"
define_basic_docker_variables

echo "--- Building directly on mac for ios ---"
./${BUILD_SCRIPTS}/run-direct_device.sh
./${BUILD_SCRIPTS}/run-direct_simulator.sh
./${BUILD_SCRIPTS}/run-direct_universal.sh
