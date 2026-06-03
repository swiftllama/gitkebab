set -e
source "scripts/common/bash/common.sh"
set -x

define_basic_variables "libgit2-1.8.4" "android" "x86_64" "debug"
init_and_change_into_tmp_build_folder



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

define_android_variables

OPENSSL_DIR=build/openssl-1.1.1w/${TARGET_FOLDER_TRIPLET}
ZLIB_LIB=build/zlib-1.2.12/${TARGET_FOLDER_TRIPLET}/lib/libz.a
LIBSSH2_DIR=build/libssh2-1.11.1/${TARGET_FOLDER_TRIPLET}
PCRE_DIR=build/pcre-8.45/${TARGET_FOLDER_TRIPLET}
ICONV_DIR=build/libiconv-1.16/${TARGET_FOLDER_TRIPLET}

CUSTOM_SEARCH_PATH="${ROOT}/${PCRE_DIR};${ROOT}/${ICONV_DIR};${ROOT}/${OPENSSL_DIR};${ROOT}/${LIBSSH2_DIR};"

# See android/arm64-debug/run-direct.sh for the rationale on PKG_CONFIG_PATH +
# USE_SSH=libssh2; both android arches need the same treatment.
export PKG_CONFIG_PATH="${ROOT}/${LIBSSH2_DIR}/lib/pkgconfig:${ROOT}/${OPENSSL_DIR}/lib/pkgconfig:${ROOT}/build/zlib-1.2.12/${TARGET_FOLDER_TRIPLET}/lib/pkgconfig:${ROOT}/${PCRE_DIR}/lib/pkgconfig"

cmake ${RELATIVE_SOURCE} \
      -DCMAKE_PREFIX_PATH="${CUSTOM_SEARCH_PATH}" \
      -DCMAKE_BUILD_TYPE=Debug \
      -DBUILD_SHARED_LIBS=NO \
      -DBUILD_TESTS=OFF \
      -DUSE_SSH=libssh2 \
      -DCMAKE_FIND_ROOT_PATH_MODE_LIBRARY=BOTH \
      -DCMAKE_INSTALL_PREFIX=${ROOT}/${BUILD_FOLDER}/ \
      -DCMAKE_C_FLAGS_DEBUG="-ggdb -Og --save-temps -std=c99" \
      -DENABLE_TRACE=ON \
      -DCMAKE_POSITION_INDEPENDENT_CODE=ON \
      -DCMAKE_ANDROID_NDK=$NDK \
      -DCMAKE_SYSTEM_NAME=Android \
      -DCMAKE_SYSTEM_VERSION=${ANDROID_API_LEVEL} \
      -DCMAKE_ANDROID_ARCH_ABI=${ANDROID_ABI} \
      -DOPENSSL_ROOT_DIR=${ROOT}/${OPENSSL_DIR} \
      -DOPENSSL_CRYPTO_LIBRARY=${ROOT}/${OPENSSL_DIR}/lib/libcrypto.a \
      -DOPENSSL_SSL_LIBRARY=${ROOT}/${OPENSSL_DIR}/lib/libssl.a \
      -DOPENSSL_INCLUDE_DIR=${ROOT}/${OPENSSL_DIR}/include


cmake -LAH .
cmake --build . -- VERBOSE=1
android_objdump_verify_library_architecture "${ROOT}/${TMP_BUILD_FOLDER}/libgit2.a" "${ANDROID_OBJDUMP_ARCHITECTURE}"
cmake --build . --target install

print_done

