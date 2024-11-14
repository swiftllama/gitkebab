set -e
source "scripts/common/bash/common.sh"
set -x

define_basic_variables "libssh2-1.10.0" "ios" "universal" "debug"
init_and_change_into_tmp_build_folder

rm -rf ${ROOT}/${BUILD_FOLDER}_device

XCODE_ROOT=/Applications/Xcode.app
IOS_SDK_VERSION=15.5
IOS_TARGET_VERSION=10.0
IOS_SDK=${XCODE_ROOT}/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS${IOS_SDK_VERSION}.sdk
ARCHS="armv7;armv7s;arm64"

OPENSSL_DIR=build/openssl-1.1.1n/${TARGET_FOLDER_TRIPLET}
ZLIB_DIR=build/zlib-1.2.12/${TARGET_FOLDER_TRIPLET}/lib/
ZLIB_LIB=${ZLIB_DIR}/libz.a


cmake ${RELATIVE_SOURCE} \
      -DCRYPTO_BACKEND=OpenSSL \
      -DENABLE_ZLIB_COMPRESSION=ON \
      -DENABLE_DEBUG_LOGGING=ON \
      -DZLIB_LIBRARY=${ROOT}/${ZLIB_LIB} \
      -DOPENSSL_ROOT_DIR=${ROOT}/${OPENSSL_DIR} \
      -DOPENSSL_CRYPTO_LIBRARY=${ROOT}/${OPENSSL_DIR}/lib/libcrypto.a \
      -DOPENSSL_SSL_LIBRARY=${ROOT}/${OPENSSL_DIR}/lib/libssl.a \
      -DCMAKE_INSTALL_PREFIX=${ROOT}/${BUILD_FOLDER} \
      -DCMAKE_BUILD_TYPE=Debug \
      -DCMAKE_C_FLAGS="-fPIC -mios-version-min=${IOS_TARGET_VERSION} -fembed-bitcode" \
      -DBUILD_EXAMPLES=OFF \
      -DBUILD_TESTING=OFF \
      -DCMAKE_OSX_ARCHITECTURES=$ARCHS \
      -DCMAKE_OSX_SYSROOT=$IOS_SDK 

cmake --build . -- VERBOSE=1
cmake --build . --target install

mv ${ROOT}/${BUILD_FOLDER} ${ROOT}/${BUILD_FOLDER}_device

print_done
