set -e
source "scripts/common/bash/common.sh"
set -x

define_basic_variables "gitkebab-0.8.0" "ios" "universal" "debug"
init_and_change_into_tmp_build_folder

rm -rf ${ROOT}/${BUILD_FOLDER}_device

# NOTES:
#
#  - Improve gdb debugging with:
#     -DCMAKE_C_FLAGS_DEBUG=\"-ggdb -Og\"
#

ZLIB_DIR=build/zlib-1.2.12/${TARGET_FOLDER_TRIPLET}
PCRE_DIR=build/pcre-8.45/${TARGET_FOLDER_TRIPLET}
OPENSSL_DIR=build/openssl-1.1.1n/${TARGET_FOLDER_TRIPLET}
LIBSSH2_DIR=build/libssh2-1.10.0/${TARGET_FOLDER_TRIPLET}
ICONV_DIR=build/libiconv-1.17/${TARGET_FOLDER_TRIPLET}
LIBGIT2_DIR=build/libgit2-1.4.2/${TARGET_FOLDER_TRIPLET}
LIBICONV_DIR=build/libiconv-1.17/${TARGET_FOLDER_TRIPLET}
CMOCKA_DIR=build/cmocka-1.1.5/${TARGET_FOLDER_TRIPLET}

CUSTOM_SEARCH_PATH="${ROOT}/${ZLIB_DIR};${ROOT}/${PCRE_DIR};${ROOT}/${OPENSSL_DIR};${ROOT}/${LIBSSH2_DIR};${ROOT}/${LIBGIT2_DIR};${ROOT}/${CMOCKA_DIR};${ROOT}/${LIBICONV_DIR}"

XCODE_ROOT=/Applications/Xcode.app
IOS_SDK_VERSION=15.5
IOS_TARGET_VERSION=10.0
IOS_SDK=${XCODE_ROOT}/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS${IOS_SDK_VERSION}.sdk
ARCHS="armv7;armv7s;arm64"

# Build c libraries and tests
echo "\n\n=== Building C libraries (root Cmake) ==="
cmake ${RELATIVE_SOURCE} \
      -DCMAKE_BUILD_TYPE=Debug \
      -DCMAKE_C_FLAGS="-mios-version-min=${IOS_TARGET_VERSION} -fembed-bitcode" \
      -DCMAKE_C_FLAGS_DEBUG="-ggdb -Og" \
      -DCMAKE_PREFIX_PATH="${CUSTOM_SEARCH_PATH}" \
      -DCMAKE_FIND_ROOT_PATH="${CUSTOM_SEARCH_PATH}" \
      -DCMAKE_INSTALL_PREFIX=${ROOT}/${BUILD_FOLDER}  \
      -DTMP_BUILD_FOLDER=${TMP_BUILD_FOLDER} \
      -DCMAKE_OSX_ARCHITECTURES=$ARCHS \
      -DCMAKE_OSX_SYSROOT=$IOS_SDK \
      -DCMAKE_SYSTEM_NAME=iOS \
      -DCMAKE_SYSTEM_VERSION=10.0 \
      -DBUILD_TESTS=OFF \
      -DBUILD_EXAMPLES=OFF

cmake --build . -- VERBOSE=1
cmake --build . --target install

cp -PR ${ROOT}/${BUILD_FOLDER} ${ROOT}/${BUILD_FOLDER}_device

print_done
