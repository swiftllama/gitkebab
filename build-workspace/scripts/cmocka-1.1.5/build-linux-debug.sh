set -e
echo "--- Building Cmocka 1.1.5 for Linux / Debug ---"

BUILD_FOLDER=build/cmocka-1.1.5/linux/debug
TMP_BUILD_FOLDER=build/tmp/cmocka-1.1.5/linux/debug

set -x
rm -rf ${BUILD_FOLDER}
rm -rf ${TMP_BUILD_FOLDER}

mkdir -p ${BUILD_FOLDER}
mkdir -p ${TMP_BUILD_FOLDER}

USER=$(whoami)
USER_ID=$(id -u ${USER})

BUILD_TYPE=Debug

docker run -v${PWD}:/tmp/workspace --user ${USER_ID} rikorose/gcc-cmake /bin/bash -c "cd /tmp/workspace/${TMP_BUILD_FOLDER}; cmake -DWITH_STATIC_LIB=true -DCMAKE_BUILD_TYPE=${BUILD_TYPE} -DCMAKE_INSTALL_PREFIX=/tmp/workspace/${BUILD_FOLDER}  ../../../../../source/cmocka-1.1.5; cmake --build .; cmake --build . --target install"

set +x
echo "--- DONE ---"
