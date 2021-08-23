set -e
echo "--- Building GitKebab head for Linux / Debug ---"

ROOT=${PWD}
BUILD_FOLDER=build/gitkebab-head/linux/debug
TMP_BUILD_FOLDER=build/tmp/gitkebab-head/linux/debug

set -x
rm -rf ${BUILD_FOLDER}
rm -rf ${TMP_BUILD_FOLDER}

mkdir -p ${BUILD_FOLDER}
mkdir -p ${TMP_BUILD_FOLDER}
cd ${TMP_BUILD_FOLDER}

# NOTES:
#
#  - Improve gdb debugging with:
#     -DCMAKE_C_FLAGS_DEBUG=\"-ggdb -Og\"
#

BUILD_TYPE=Debug

ZLIB_DIR=build/zlib-1.2.11/linux/debug
PCRE_DIR=build/pcre-8.45/linux/debug
OPENSSL_DIR=build/openssl-1.1.1k/linux/debug
LIBSSH2_DIR=build/libssh2-1.9.0/linux/debug
#ICONV_DIR=build/libiconv-1.16/linux/debug
LIBGIT2_DIR=build/libgit2-1.1.1/linux/debug
CMOCKA_DIR=build/cmocka-1.1.5/linux/debug

CUSTOM_SEARCH_PATH="${ROOT}/${ZLIB_DIR};${ROOT}/${PCRE_DIR};${ROOT}/${OPENSSL_DIR};${ROOT}/${LIBSSH2_DIR};${ROOT}/${LIBGIT2_DIR};${ROOT}/${CMOCKA_DIR}"

# Build c libraries and tests
echo "\n\n=== Building C libraries (root Cmake) ==="
cd ${ROOT}/${TMP_BUILD_FOLDER}
cmake ../../../../../source/gitkebab-head -DCMAKE_PREFIX_PATH="${CUSTOM_SEARCH_PATH}" -DCMAKE_BUILD_TYPE=${BUILD_TYPE} -DCMAKE_INSTALL_PREFIX=${ROOT}/${BUILD_FOLDER}  -DCMAKE_C_FLAGS_DEBUG="-ggdb -Og" -DTMP_BUILD_FOLDER=${TMP_BUILD_FOLDER}
cmake --build . -- VERBOSE=1
cmake --build . --target install

set +x
echo "--- DONE ---"
