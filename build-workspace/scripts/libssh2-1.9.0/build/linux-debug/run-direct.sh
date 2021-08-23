set -e
echo "--- Building Libssh2 1.9.0 for Linux / Debug ---"

ROOT=${PWD}
BUILD_FOLDER=build/libssh2-1.9.0/linux/debug
TMP_BUILD_FOLDER=build/tmp/libssh2-1.9.0/linux/debug

set -x
rm -rf ${BUILD_FOLDER}
rm -rf ${TMP_BUILD_FOLDER}

mkdir -p ${BUILD_FOLDER}
mkdir -p ${TMP_BUILD_FOLDER}
cd ${TMP_BUILD_FOLDER}

BUILD_TYPE=Debug

## NOTES
##  - allow linking resulting static library against shared library later on
##    - -DCMAKE_C_FLAGS="-fPIC"
##

OPENSSL_DIR=build/openssl-1.1.1k/linux/debug
ZLIB_LIB=build/zlib-1.2.11/linux/debug/lib/libz.a

cmake ../../../../../source/libssh2-1.9.0 -DCRYPTO_BACKEND=OpenSSL -DENABLE_ZLIB_COMPRESSION=ON -DENABLE_DEBUG_LOGGING=ON -DZLIB_LIBRARY=${ROOT}/${ZLIB_LIB} -DOPENSSL_ROOT_DIR=${ROOT}/${OPENSSL_DIR} -DOPENSSL_CRYPTO_LIBRARY=${ROOT}/${OPENSSL_DIR}/lib/libcrypto.a -DOPENSSL_SSL_LIBRARY=${ROOT}/${OPENSSL_DIR}/lib/libssl.a -DCMAKE_INSTALL_PREFIX=${ROOT}/${BUILD_FOLDER} -DCMAKE_BUILD_TYPE=${BUILD_TYPE} -DCMAKE_C_FLAGS="-fPIC"
cmake --build . -- VERBOSE=1
cmake --build . --target install

set +x
echo "--- DONE ---"
