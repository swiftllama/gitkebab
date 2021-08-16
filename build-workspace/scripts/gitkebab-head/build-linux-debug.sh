set -e
echo "--- Building GitKebab HEAD for Linux / Debug ---"

BUILD_FOLDER=build/gitkebab-head/linux/debug
TMP_BUILD_FOLDER=build/tmp/gitkebab-head/linux/debug
ROOT_DIR=${PWD}

set -x
rm -rf ${BUILD_FOLDER}
rm -rf ${TMP_BUILD_FOLDER}

mkdir -p ${BUILD_FOLDER}
mkdir -p ${TMP_BUILD_FOLDER}

USER=$(whoami)
USER_ID=$(id -u ${USER})

ZLIB_DIR=build/zlib-1.2.11/linux/debug
PCRE_DIR=build/pcre-8.45/linux/debug
OPENSSL_DIR=build/openssl-1.1.1k/linux/debug
LIBSSH2_DIR=build/libssh2-1.9.0/linux/debug
#ICONV_DIR=build/libiconv-1.16/linux/debug
LIBGIT2_DIR=build/libgit2-1.1.1/linux/debug
CMOCKA_DIR=build/cmocka-1.1.5/linux/debug

BUILD_TYPE=Debug

CUSTOM_SEARCH_PATH="/tmp/workspace/${ZLIB_DIR};/tmp/workspace/${PCRE_DIR};/tmp/workspace/${OPENSSL_DIR};/tmp/workspace/${LIBSSH2_DIR};/tmp/workspace/${LIBGIT2_DIR};/tmp/workspace/${CMOCKA_DIR}"

CONF_ARGS="-DCMAKE_PREFIX_PATH=\"${CUSTOM_SEARCH_PATH}\" -DCMAKE_BUILD_TYPE=${BUILD_TYPE} -DCMAKE_INSTALL_PREFIX=/tmp/workspace/${BUILD_FOLDER}  -DCMAKE_C_FLAGS_DEBUG=\"-ggdb -Og\" -DTMP_BUILD_FOLDER=${TMP_BUILD_FOLDER}"

# Build c libraries and tests
echo "\n\n=== Building C libraries (root Cmake) ==="
docker run -v${PWD}:/tmp/workspace --user ${USER_ID} rikorose/gcc-cmake /bin/bash -c "cd /tmp/workspace/${TMP_BUILD_FOLDER}; cmake ../../../../../source/gitkebab-head ${CONF_ARGS}; cmake --build . -- VERBOSE=1; cmake --build . --target install"

# Generate dart bindings
echo "\n\n=== Generating dart bindings ==="
mkdir -p ${TMP_BUILD_FOLDER}/dart
cd ${TMP_BUILD_FOLDER}/dart
ls ../../../../../../
cp ../../../../../../source/gitkebab-head/src/dart/pubspec.yaml ./
cp ../../../../../../source/gitkebab-head/src/dart/conf-build-workspace.yaml ./
PATH=/usr/bin/ dart pub get --offline; PATH=/usr/bin/ dart run ffigen --config conf-build-workspace.yaml
mkdir -p ${ROOT_DIR}/${BUILD_FOLDER}/lib
cp gitkebab_lib.dart ${ROOT_DIR}/${BUILD_FOLDER}/lib


set +x
echo "--- DONE ---"
