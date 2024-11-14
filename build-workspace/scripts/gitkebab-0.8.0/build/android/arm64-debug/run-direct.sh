set -e
source "scripts/common/bash/common.sh"
set -x

define_basic_variables "gitkebab-0.8.0" "android" "arm64" "debug"
init_and_change_into_tmp_build_folder


# NOTES:
#
#  - Improve gdb debugging with:
#     -DCMAKE_C_FLAGS_DEBUG=\"-ggdb -Og\"
#

define_android_variables


ZLIB_DIR=build/zlib-1.2.12/${TARGET_FOLDER_TRIPLET}
PCRE_DIR=build/pcre-8.45/${TARGET_FOLDER_TRIPLET}
OPENSSL_DIR=build/openssl-1.1.1n/${TARGET_FOLDER_TRIPLET}
LIBSSH2_DIR=build/libssh2-1.10.0/${TARGET_FOLDER_TRIPLET}
#ICONV_DIR=build/libiconv-1.16/${TARGET_FOLDER_TRIPLET}
LIBGIT2_DIR=build/libgit2-1.4.2/${TARGET_FOLDER_TRIPLET}
CMOCKA_DIR=build/cmocka-1.1.5/${TARGET_FOLDER_TRIPLET}

CUSTOM_SEARCH_PATH="${ROOT}/${ZLIB_DIR};${ROOT}/${PCRE_DIR};${ROOT}/${OPENSSL_DIR};${ROOT}/${LIBSSH2_DIR};${ROOT}/${LIBGIT2_DIR};${ROOT}/${CMOCKA_DIR}"

# Build c libraries and tests
echo "\n\n=== Building C libraries (root Cmake) ==="
cmake ${RELATIVE_SOURCE} \
      -DCMAKE_PREFIX_PATH="${CUSTOM_SEARCH_PATH}" \
      -DCMAKE_BUILD_TYPE=Debug\
      -DCMAKE_C_FLAGS_DEBUG="-ggdb -Og" \
      -DCMAKE_FIND_ROOT_PATH=${ROOT}/build \
      -DCMAKE_INSTALL_PREFIX=${ROOT}/${BUILD_FOLDER}  \
      -DTMP_BUILD_FOLDER=${TMP_BUILD_FOLDER} \
      -DCMAKE_ANDROID_NDK=$NDK \
      -DCMAKE_SYSTEM_NAME=Android \
      -DCMAKE_SYSTEM_VERSION=$ANDROID_API_LEVEL \
      -DCMAKE_ANDROID_ARCH_ABI=$ANDROID_ABI

cmake --build . -- VERBOSE=1
android_objdump_verify_library_architecture "${ROOT}/${TMP_BUILD_FOLDER}/src/lib/libgitkebab_static.a" "${ANDROID_OBJDUMP_ARCHITECTURE}"
cmake --build . --target install

print_done
