set -e
echo "--- Building OpenSSL 1.1.1k for Linux / Debug ---"

BUILD_FOLDER=build/openssl-1.1.1k/linux/debug
TMP_BUILD_FOLDER=build/tmp/openssl-1.1.1k/linux/debug

set -x
rm -rf ${BUILD_FOLDER}
rm -rf ${TMP_BUILD_FOLDER}

mkdir -p ${BUILD_FOLDER}
mkdir -p ${TMP_BUILD_FOLDER}

USER=$(whoami)
USER_ID=$(id -u ${USER})

docker run -v${PWD}:/tmp/workspace --user ${USER_ID} gcc /bin/bash -c "cd /tmp/workspace/${TMP_BUILD_FOLDER}; ../../../../../source/openssl-1.1.1k/Configure linux-x86_64 --prefix=/tmp/workspace/${BUILD_FOLDER} --openssldir=/tmp/workspace/${BUILD_FOLDER}; make && make install;"

set +x
echo "--- DONE ---"
