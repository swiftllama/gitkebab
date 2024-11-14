set -e
source "scripts/common/bash/common.sh"
set -x

define_basic_variables "gitkebab-0.8.0" "macos" "universal" "debug"
init_and_change_into_tmp_build_folder

# NOTES:
#
#  - Improve gdb debugging with:
#     -DCMAKE_C_FLAGS_DEBUG=\"-ggdb -Og\"
#

ZLIB_DIR=build/zlib-1.2.12/${TARGET_FOLDER_TRIPLET}
PCRE_DIR=build/pcre-8.45/${TARGET_FOLDER_TRIPLET}
OPENSSL_DIR=build/openssl-1.1.1n/${TARGET_FOLDER_TRIPLET}
LIBSSH2_DIR=build/libssh2-1.10.0/${TARGET_FOLDER_TRIPLET}
#ICONV_DIR=build/libiconv-1.17/${TARGET_FOLDER_TRIPLET}
LIBGIT2_DIR=build/libgit2-1.4.2/${TARGET_FOLDER_TRIPLET}
LIBICONV_DIR=build/libiconv-1.17/${TARGET_FOLDER_TRIPLET}
CMOCKA_DIR=build/cmocka-1.1.5/${TARGET_FOLDER_TRIPLET}

CUSTOM_SEARCH_PATH="${ROOT}/${ZLIB_DIR};${ROOT}/${PCRE_DIR};${ROOT}/${OPENSSL_DIR};${ROOT}/${LIBSSH2_DIR};${ROOT}/${LIBGIT2_DIR};${ROOT}/${CMOCKA_DIR};${ROOT}/${LIBICONV_DIR}"

# Build c libraries and tests
echo "\n\n=== Building C libraries (root Cmake) ==="
cmake ${RELATIVE_SOURCE} \
      -DCMAKE_BUILD_TYPE=Debug \
      -DCMAKE_C_FLAGS_DEBUG="-ggdb -Og" \
      -DCMAKE_PREFIX_PATH="${CUSTOM_SEARCH_PATH}" \
      -DCMAKE_INSTALL_PREFIX=${ROOT}/${BUILD_FOLDER}  \
      -DTMP_BUILD_FOLDER=${TMP_BUILD_FOLDER} \
      -DCMAKE_OSX_DEPLOYMENT_TARGET=10.11 \
      -DCMAKE_OSX_ARCHITECTURES="x86_64;arm64"

cmake --build . -- VERBOSE=1
cmake --build . --target install

print_done
