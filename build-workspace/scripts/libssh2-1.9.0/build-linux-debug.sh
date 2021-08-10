set -e
echo "--- Building Libssh2 1.9.0 for Linux / Debug ---"

BUILD_FOLDER=build/libssh2-1.9.0/linux/debug
TMP_BUILD_FOLDER=build/tmp/libssh2-1.9.0/linux/debug

set -x
rm -rf ${BUILD_FOLDER}
rm -rf ${TMP_BUILD_FOLDER}

mkdir -p ${BUILD_FOLDER}
mkdir -p ${TMP_BUILD_FOLDER}

USER=$(whoami)
USER_ID=$(id -u ${USER})


OPENSSL_DIR=build/openssl-1.1.1k/linux/debug
ZLIB_LIB=build/zlib-1.2.11/linux/debug/lib/libz.a

docker run -v${PWD}:/tmp/workspace --user ${USER_ID} rikorose/gcc-cmake /bin/bash -c "cd /tmp/workspace/${TMP_BUILD_FOLDER}; cmake ../../../../../source/libssh2-1.9.0 -DCRYPTO_BACKEND=OpenSSL -DENABLE_ZLIB_COMPRESSION=ON -DENABLE_DEBUG_LOGGING=ON -DZLIB_LIBRARY=/tmp/workspace/${ZLIB_LIB} -DOPENSSL_ROOT_DIR=/tmp/workspace/$OPENSSL_DIR -DOPENSSL_CRYPTO_LIBRARY=/tmp/workspace/${OPENSSL_DIR}/lib/libcrypto.a -DOPENSSL_SSL_LIBRARY=/tmp/workspace/${OPENSSL_DIR}/lib/libssl.a -DCMAKE_INSTALL_PREFIX=/tmp/workspace/build/libssh2-1.9.0/linux/debug; cmake --build .; cmake --build . --target install"

set +x
echo "--- DONE ---"
