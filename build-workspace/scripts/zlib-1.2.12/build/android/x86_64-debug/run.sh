set -e
source "scripts/common/bash/common.sh"
set -x

define_basic_variables "zlib-1.2.12" "android" "x86_64" "debug"
define_basic_docker_variables

echo "--- Entering docker build environment ---"
docker run -v${PWD}:${PWD} -w${PWD} --user ${USER_ID} android-ndk-build /bin/bash -c "${BUILD_SCRIPTS}/run-direct.sh"

set +x
echo "--- Exiting docker build environment ---"
