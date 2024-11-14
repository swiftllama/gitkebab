set -e
source "scripts/common/bash/common.sh"
set -x

define_basic_variables "gitkebab-head" "ios" "universal" "debug"
define_basic_docker_variables

echo "--- Building directly on the mac ---"

./${BUILD_SCRIPTS}/run-direct_device.sh
./${BUILD_SCRIPTS}/run-direct_simulator.sh
./${BUILD_SCRIPTS}/run-direct_universal.sh

#echo "--- Exiting docker build environment ---"
#${BUILD_SCRIPTS}/run-direct-2_generate-dart-bindings.sh
