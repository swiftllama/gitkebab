set -e
source "scripts/common/bash/common.sh"
set -x

define_basic_variables "libssh2-1.10.0" "linux" "x86_64" "debug"
init_and_change_into_tmp_build_folder

## NOTES
##  - allow linking resulting static library against shared library later on
##    - CFLAGS=-fPIC
##

OPENSSL_DIR=build/openssl-1.1.1n/${TARGET_FOLDER_TRIPLET}
ZLIB_LIB=build/zlib-1.2.12/${TARGET_FOLDER_TRIPLET}/lib/libz.a

cmake ${RELATIVE_SOURCE} \
      -DCRYPTO_BACKEND=OpenSSL \
      -DENABLE_ZLIB_COMPRESSION=ON \
      -DENABLE_DEBUG_LOGGING=ON \
      -DZLIB_LIBRARY=${ROOT}/${ZLIB_LIB} \
      -DOPENSSL_ROOT_DIR=${ROOT}/${OPENSSL_DIR} \
      -DOPENSSL_CRYPTO_LIBRARY=${ROOT}/${OPENSSL_DIR}/lib/libcrypto.a \
      -DOPENSSL_SSL_LIBRARY=${ROOT}/${OPENSSL_DIR}/lib/libssl.a \
      -DCMAKE_INSTALL_PREFIX=${ROOT}/${BUILD_FOLDER} \
      -DCMAKE_BUILD_TYPE=Debug -DCMAKE_C_FLAGS="-fPIC"

cmake --build . -- VERBOSE=1
cmake --build . --target install

print_done
