set -e
echo "--- Entering docker build environment ---"

set -x

USER=$(whoami)
USER_ID=$(id -u ${USER})

docker run -v${PWD}:${PWD} -w${PWD} --user ${USER_ID} rikorose/gcc-cmake /bin/bash -c "scripts/gitkebab-head/build/linux-debug/run-direct-1_build-gitkebab-libs.sh"

echo "--- Exiting docker build environment ---"


./scripts/gitkebab-head/build/linux-debug/run-direct-2_generate-dart-bindings.sh

