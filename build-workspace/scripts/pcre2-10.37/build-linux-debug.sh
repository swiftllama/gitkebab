set -e
echo "--- Building Pcre2 10.37 for Linux / Debug ---"

BUILD_FOLDER=build/pcre2-10.37/linux/debug
TMP_BUILD_FOLDER=build/tmp/pcre2-10.37/linux/debug

set -x
rm -rf ${BUILD_FOLDER}
rm -rf ${TMP_BUILD_FOLDER}

mkdir -p ${BUILD_FOLDER}
mkdir -p ${TMP_BUILD_FOLDER}

USER=$(whoami)
USER_ID=$(id -u ${USER})

docker run -v${PWD}:/tmp/workspace --user ${USER_ID} gcc /bin/bash -c "cd /tmp/workspace/${TMP_BUILD_FOLDER}; ../../../../../source/pcre2-10.37/configure --prefix=/tmp/workspace/${BUILD_FOLDER}; make && make install;"

set +x
echo "--- DONE ---"
