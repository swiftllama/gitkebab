set -e
source "scripts/common/bash/common.sh"
set -x

define_basic_variables "libssh2-1.10.0" "android" "arm64" "debug"
define_basic_docker_variables

echo "--- Entering docker build environment ---"
docker run -v${PWD}:${PWD} -w${PWD} --user ${USER_ID} android-ndk-build /bin/bash -c "${BUILD_SCRIPTS}/run-direct.sh"

set +x
echo "--- Exiting docker build environment ---"