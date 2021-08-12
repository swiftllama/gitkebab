set -e
echo "--- Building Pcre 8.45 for Linux / Debug ---"

BUILD_FOLDER=build/pcre-8.45/linux/debug
TMP_BUILD_FOLDER=build/tmp/pcre-8.45/linux/debug

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

docker run -v${PWD}:/tmp/workspace --user ${USER_ID} gcc /bin/bash -c "cd /tmp/workspace/${TMP_BUILD_FOLDER}; ../../../../../source/pcre-8.45/configure --prefix=/tmp/workspace/${BUILD_FOLDER} CFLAGS=-fPIC CXXFLAGS=\"-fPIC\"; make; echo \"Installing...\"; make install;"

set +x
echo "--- DONE ---"
