set -e
source "scripts/common/bash/common.sh"
set -x

define_basic_variables "gitkebab-head" "linux" "x86_64" "debug"
define_basic_docker_variables

echo "--- Entering docker build environment ---"

docker run -v${PWD}:${PWD} -w${PWD} --user ${USER_ID} rikorose/gcc-cmake /bin/bash -c "${BUILD_SCRIPTS}/run-direct-1_build-gitkebab-libs.sh"

echo "--- Exiting docker build environment ---"

${BUILD_SCRIPTS}/run-direct-2_generate-dart-bindings.sh
