set -e
echo "--- Entering docker build environment ---"

set -x

USER=$(whoami)
USER_ID=$(id -u ${USER})

docker run -v${PWD}:${PWD} -w${PWD} --user ${USER_ID} gcc /bin/bash -c "scripts/libiconv-1.16/build/linux-debug/run-direct.sh"

set +x
echo "--- Exiting docker build environment ---"
