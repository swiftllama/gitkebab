set -e
source "scripts/common/bash/common.sh"
set -x

define_basic_variables "libgit2-1.4.2" "linux" "x86_64" "release"
define_basic_docker_variables

echo "--- Entering docker build environment ---"
docker run -v${PWD}:${PWD} -w${PWD} --user ${USER_ID} rikorose/gcc-cmake /bin/bash -c "${BUILD_SCRIPTS}/run-direct.sh"
