set -e
echo "--- Building Zlib 1.2.11 for Linux / Debug ---"

BUILD_FOLDER=build/zlib-1.2.11/linux/debug
TMP_BUILD_FOLDER=build/tmp/zlib-1.2.11/linux/debug

set -x
rm -rf ${BUILD_FOLDER}
rm -rf ${TMP_BUILD_FOLDER}

mkdir -p ${BUILD_FOLDER}
mkdir -p ${TMP_BUILD_FOLDER}

USER=$(whoami)
USER_ID=$(id -u ${USER})

## NOTES
##  - allow linking resulting static library against shared library later on
##    - CFLAGS=-fPIC
##


docker run -v${PWD}:/tmp/workspace --user ${USER_ID} gcc /bin/bash -c "cd /tmp/workspace/${TMP_BUILD_FOLDER}; CFLAGS=\"-fPIC\" ../../../../../source/zlib-1.2.11/configure --prefix=/tmp/workspace/${BUILD_FOLDER}; make && make install;"

set +x
echo "--- DONE ---"
