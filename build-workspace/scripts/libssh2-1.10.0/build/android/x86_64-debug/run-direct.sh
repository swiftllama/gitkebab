set -e
source "scripts/common/bash/common.sh"
set -x

define_basic_variables "libssh2-1.10.0" "android" "x86_64" "debug"
init_and_change_into_tmp_build_folder

## NOTES
##  - allow linking resulting static library against shared library later on
##    - CFLAGS=-fPIC
##

define_android_variables

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
      -DOPENSSL_INCLUDE_DIR=${ROOT}/${OPENSSL_DIR}/include \
      -DCMAKE_INSTALL_PREFIX=${ROOT}/${BUILD_FOLDER} \
      -DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
      -DCMAKE_C_FLAGS="-fPIC" \
      -DCMAKE_ANDROID_NDK=$NDK \
      -DCMAKE_SYSTEM_NAME=Android \
      -DCMAKE_SYSTEM_VERSION=$ANDROID_API_LEVEL \
      -DCMAKE_ANDROID_ARCH_ABI=$ANDROID_ABI

cmake --build . -- VERBOSE=1
android_objdump_verify_library_architecture "${ROOT}/${TMP_BUILD_FOLDER}/src/libssh2.a" "${ANDROID_OBJDUMP_ARCHITECTURE}"
cmake --build . --target install

print_done
