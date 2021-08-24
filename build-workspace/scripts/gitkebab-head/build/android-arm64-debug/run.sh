set -e
echo "--- Entering docker build environment ---"

set -x

USER=$(whoami)
USER_ID=$(id -u ${USER})

docker run -v${PWD}:${PWD} -w${PWD} --user ${USER_ID} android-ndk-build /bin/bash -c "scripts/gitkebab-head/build/android-arm64-debug/run-direct-1_build-gitkebab-libs.sh"

echo "--- Exiting docker build environment ---"


./scripts/gitkebab-head/build/android-arm64-debug/run-direct-2_generate-dart-bindings.sh

