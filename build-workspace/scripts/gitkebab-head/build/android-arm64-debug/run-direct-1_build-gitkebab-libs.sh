set -e
echo "--- Building GitKebab head for Android/arm64/Debug ---"

ROOT=${PWD}
BUILD_FOLDER=build/gitkebab-head/android/arm64-debug
TMP_BUILD_FOLDER=build/tmp/gitkebab-head/android/arm64-debug

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

ZLIB_DIR=build/zlib-1.2.11/android/arm64-debug
PCRE_DIR=build/pcre-8.45/android/arm64-debug
OPENSSL_DIR=build/openssl-1.1.1k/android/arm64-debug
LIBSSH2_DIR=build/libssh2-1.9.0/android/arm64-debug
#ICONV_DIR=build/libiconv-1.16/android/arm64-debug
LIBGIT2_DIR=build/libgit2-1.1.1/android/arm64-debug
CMOCKA_DIR=build/cmocka-1.1.5/android/arm64-debug

CUSTOM_SEARCH_PATH="${ROOT}/${ZLIB_DIR};${ROOT}/${PCRE_DIR};${ROOT}/${OPENSSL_DIR};${ROOT}/${LIBSSH2_DIR};${ROOT}/${LIBGIT2_DIR};${ROOT}/${CMOCKA_DIR}"

NDK=/opt/android-ndk/android-ndk-r23
ANDROID_API_LEVEL=21
ANDROID_ABI=arm64-v8a

# Build c libraries and tests
echo "\n\n=== Building C libraries (root Cmake) ==="
cd ${ROOT}/${TMP_BUILD_FOLDER}
cmake ../../../../../source/gitkebab-head -DCMAKE_PREFIX_PATH="${CUSTOM_SEARCH_PATH}" -DCMAKE_FIND_ROOT_PATH=${ROOT}/build -DCMAKE_BUILD_TYPE=${BUILD_TYPE} -DCMAKE_INSTALL_PREFIX=${ROOT}/${BUILD_FOLDER}  -DCMAKE_C_FLAGS_DEBUG="-ggdb -Og" -DTMP_BUILD_FOLDER=${TMP_BUILD_FOLDER} -DCMAKE_ANDROID_NDK=$NDK -DCMAKE_SYSTEM_NAME=Android -DCMAKE_SYSTEM_VERSION=$ANDROID_API_LEVEL -DCMAKE_ANDROID_ARCH_ABI=$ANDROID_ABI --debug-find
cmake --build . -- VERBOSE=1

if OBJDUMP=$($NDK/toolchains/llvm/prebuilt/linux-x86_64/bin/llvm-objdump -f ${ROOT}/${TMP_BUILD_FOLDER}/src/lib/libgitkebab_static.a)
then
    if RESULTING_ARCH="$(echo "$OBJDUMP" | grep -m 1 'architecture:')"
    then
        if [[ "${RESULTING_ARCH}" == "architecture: aarch64" ]]; then
            echo "Found ${RESULTING_ARCH} in build product"
        else
            echo "Build succeeded but artifact has unexpected ${RESULTING_ARCH} (expected [aarch64])"
            exit 2
        fi
    else
        echo "Error greping for architecture"
        exit 4
    fi
else
    echo "Error executing objdump to verify architecture"
    exit 3
fi


cmake --build . --target install

set +x
echo "--- DONE ---"
