set -e
echo "--- Entering docker build environment ---"

set -x

USER=$(whoami)
USER_ID=$(id -u ${USER})

docker run -v${PWD}:${PWD} -w${PWD} --user ${USER_ID} android-ndk-build /bin/bash -c "scripts/openssl-1.1.1k/build/android-arm64-debug/run-direct.sh"

set +x
echo "--- Exiting docker build environment ---"
