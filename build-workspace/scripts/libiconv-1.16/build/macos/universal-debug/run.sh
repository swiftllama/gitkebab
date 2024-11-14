set -e
source "scripts/common/bash/common.sh"
set -x

define_basic_variables "libiconv-1.16" "macos" "universal" "debug"
define_basic_docker_variables

echo "--- Entering docker build environment ---"
./${BUILD_SCRIPTS}/run-direct.sh
