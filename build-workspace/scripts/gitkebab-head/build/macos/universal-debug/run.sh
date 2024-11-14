set -e
source "scripts/common/bash/common.sh"
set -x

define_basic_variables "gitkebab-head" "macos" "universal" "debug"
define_basic_docker_variables

echo "--- Building directly on the mac ---"

./${BUILD_SCRIPTS}/run-direct.sh

#echo "--- Exiting docker build environment ---"
#${BUILD_SCRIPTS}/run-direct-2_generate-dart-bindings.sh
