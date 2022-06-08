set -e
source "scripts/common/bash/common.sh"
set -x

define_basic_variables "cmocka-1.1.5" "macos" "universal" "debug"
define_basic_docker_variables

echo "--- Running build directly on mac ---"
#docker run -v${PWD}:${PWD} -w${PWD} --user ${USER_ID} rikorose/gcc-cmake /bin/bash -c "${BUILD_SCRIPTS}/run-direct.sh"
./${BUILD_SCRIPTS}/run-direct.sh

set +x
echo "--- Exiting docker build environment ---"
