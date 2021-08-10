set -e
echo "--- Building Libgit2 1.1.1 for Linux / Debug ---"

BUILD_FOLDER=build/libgit2-1.1.1/linux/debug
TMP_BUILD_FOLDER=build/tmp/libgit2-1.1.1/linux/debug

set -x
rm -rf ${BUILD_FOLDER}
rm -rf ${TMP_BUILD_FOLDER}

mkdir -p ${BUILD_FOLDER}
mkdir -p ${TMP_BUILD_FOLDER}

USER=$(whoami)
USER_ID=$(id -u ${USER})


OPENSSL_DIR=build/openssl-1.1.1k/linux/debug
ZLIB_LIB=build/zlib-1.2.11/linux/debug/lib/libz.a
LIBSSH2_DIR=build/libssh2-1.9.0/linux/debug

docker run -v${PWD}:/tmp/workspace --user ${USER_ID} rikorose/gcc-cmake /bin/bash -c "cd /tmp/workspace/${TMP_BUILD_FOLDER}; cmake ../../../../../source/libgit2-1.1.1 -DBUILD_SHARED_LIBS=NO -DUSE_SSH=0 -DLIBSSH2_FOUND=1 -DLIBSSH2_INCLUDE_DIRS=/tmp/workspace/${LIBSSH2_DIR}/include/ -DLIBSSH2_LIBRARY_DIRS=/tmp/workspace/${LIBSSH2_DIR} -DLIBSSH2_LIBRARIES=/tmp/workspace/${LIBSSH2_DIR}/lib/libssh2.a -DCMAKE_INSTALL_PREFIX=/tmp/workspace/${BUILD_FOLDER}/; cmake --build .; cmake --build . --target install"

set +x
echo "--- DONE ---"
