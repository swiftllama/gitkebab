set -e
source "scripts/common/bash/common.sh"
set -x

define_basic_variables "openssl-1.1.1n" "ios" "universal" "debug"
define_basic_docker_variables

echo "--- Building directly on mac for ios ---"
./${BUILD_SCRIPTS}/run-direct_device-armv-armv7s.sh
./${BUILD_SCRIPTS}/run-direct_device-arm64.sh
./${BUILD_SCRIPTS}/run-direct_simulator.sh
./${BUILD_SCRIPTS}/run-direct_universal.sh
