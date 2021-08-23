set -e
echo "--- Entering docker build environment ---"

set -x

USER=$(whoami)
USER_ID=$(id -u ${USER})

docker run -v${PWD}:${PWD} -w${PWD} --user ${USER_ID} rikorose/gcc-cmake /bin/bash -c "scripts/libssh2-1.9.0/build/linux-debug/run-direct.sh"

set +x
echo "--- Exiting docker build environment ---"
