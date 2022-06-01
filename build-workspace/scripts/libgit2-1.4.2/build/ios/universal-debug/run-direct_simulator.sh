set -e
source "scripts/common/bash/common.sh"
set -x

define_basic_variables "libgit2-1.4.2" "ios" "universal" "debug"
init_and_change_into_tmp_build_folder

rm -rf ${ROOT}/${BUILD_FOLDER}_simulator

# NOTES:
#
#  - Improve gdb debugging with:
#     -DCMAKE_C_FLAGS_DEBUG=\"-ggdb -Og\"
#
#  - Allow creating shraed libs from libgit2
#     -DCMAKE_POSITION_INDEPENDENT_CODE=ON
#
#  - Enable libgit trace with
#     -DENABLE_TRACE=ON
#  

XCODE_ROOT=/Applications/Xcode.app
IOS_SDK_VERSION=15.5
IOS_TARGET_VERSION=10.0
IOS_SDK=${XCODE_ROOT}/Contents/Developer/Platforms/iPhoneSimulator.platform/Developer/SDKs/iPhoneSimulator${IOS_SDK_VERSION}.sdk
ARCHS="i386;x86_64"

OPENSSL_DIR=build/openssl-1.1.1n/${TARGET_FOLDER_TRIPLET}
ZLIB_LIB=build/zlib-1.2.12/${TARGET_FOLDER_TRIPLET}/lib/libz.a
LIBSSH2_DIR=build/libssh2-1.10.0/${TARGET_FOLDER_TRIPLET}
PCRE_DIR=build/pcre-8.45/${TARGET_FOLDER_TRIPLET}
#ICONV_DIR=build/libiconv-1.16/${TARGET_FOLDER_TRIPLET}

CUSTOM_SEARCH_PATH="${ROOT}/${PCRE_DIR};${ROOT}/${OPENSSL_DIR};"

cmake ${RELATIVE_SOURCE} \
      -DCMAKE_PREFIX_PATH="${CUSTOM_SEARCH_PATH}" \
      -DCMAKE_BUILD_TYPE=Debug \
      -DBUILD_SHARED_LIBS=NO \
      -DUSE_SSH=0 \
      -DLIBSSH2_FOUND=1 \
      -DLIBSSH2_INCLUDE_DIRS=${ROOT}/${LIBSSH2_DIR}/include/ \
      -DLIBSSH2_LIBRARY_DIRS=${ROOT}/${LIBSSH2_DIR} \
      -DLIBSSH2_LIBRARIES=${ROOT}/${LIBSSH2_DIR}/lib/libssh2.a \
      -DOPENSSL_ROOT_DIR=${ROOT}/${OPENSSL_DIR} \
      -DOPENSSL_INCLUDE_DIR=${ROOT}/${OPENSSL_DIR}/include \
      -DOPENSSL_CRYPTO_LIBRARY=${ROOT}/${OPENSSL_DIR}/lib/libcrypto.a \
      -DOPENSSL_SSL_LIBRARY=${ROOT}/${OPENSSL_DIR}/lib/libssl.a \
      -DCMAKE_INSTALL_PREFIX=${ROOT}/${BUILD_FOLDER}/ \
      -DCMAKE_C_FLAGS_DEBUG="-ggdb -Og --save-temps" \
      -DCMAKE_C_FLAGS="-mios-version-min=${IOS_TARGET_VERSION} -fembed-bitcode" \
      -DENABLE_TRACE=ON \
      -DCMAKE_POSITION_INDEPENDENT_CODE=ON \
      -DCMAKE_EXE_LINKER_FLAGS="${ROOT}/${OPENSSL_DIR}/lib/libssl.a ${ROOT}/${OPENSSL_DIR}/lib/libcrypto.a"  \
      -DCMAKE_OSX_ARCHITECTURES=$ARCHS \
      -DCMAKE_OSX_SYSROOT=$IOS_SDK \
      -DBUILD_EXAMPLES=OFF \
      -DBUILD_TESTS=OFF \
      -DCMAKE_SYSTEM_NAME=iOS \
      -DCMAKE_SYSTEM_VERSION=10.0 \
      -DCMAKE_TRY_COMPILE_TARGET_TYPE=STATIC_LIBRARY

cmake -LAH .
cmake --build . -- VERBOSE=1
cmake --build . --target install

mv ${ROOT}/${BUILD_FOLDER} ${ROOT}/${BUILD_FOLDER}_simulator

print_done




