set -e
source "scripts/common/bash/common.sh"
set -x

define_basic_variables "gitkebab-0.8.0" "windows" "x86_64" "debug"
define_basic_docker_variables

echo "--- Entering docker build environment ---"

docker run -v${PWD}:${PWD} -w${PWD} --user ${USER_ID} mmozeiko/mingw-w64 /bin/bash -c "${BUILD_SCRIPTS}/run-direct.sh"

echo "--- Exiting docker build environment ---"

echo "--- Skipping dart binding generations on windows ---"
#${BUILD_SCRIPTS}/run-direct-2_generate-dart-bindings.sh
