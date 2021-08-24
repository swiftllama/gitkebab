set -e
echo "--- Building Libgit2 1.1.1 for Android/arm64/Debug ---"

ROOT=${PWD}
BUILD_FOLDER=build/libgit2-1.1.1/android/arm64-debug
TMP_BUILD_FOLDER=build/tmp/libgit2-1.1.1/android/arm64-debug

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
#  - Allow creating shraed libs from libgit2
#     -DCMAKE_POSITION_INDEPENDENT_CODE=ON
#
#  - Enable libgit trace with
#     -DENABLE_TRACE=ON
#  

BUILD_TYPE=Debug
CUSTOM_SEARCH_PATH="${ROOT}/${PCRE_DIR};${ROOT}/${ICONV_DIR};${ROOT}/${OPENSSL_DIR};"

OPENSSL_DIR=build/openssl-1.1.1k/android/arm64-debug
ZLIB_LIB=build/zlib-1.2.11/android/arm64-debug/lib/libz.a
LIBSSH2_DIR=build/libssh2-1.9.0/android/arm64-debug
PCRE_DIR=build/pcre-8.45/android/arm64-debug
ICONV_DIR=build/libiconv-1.16/android/arm64-debug

NDK=/opt/android-ndk/android-ndk-r23
ANDROID_API_LEVEL=21
ANDROID_ABI=arm64-v8a

cmake ../../../../../source/libgit2-1.1.1 -DCMAKE_PREFIX_PATH="${CUSTOM_SEARCH_PATH}" -DBUILD_SHARED_LIBS=NO -DUSE_SSH=0 -DLIBSSH2_FOUND=1 -DLIBSSH2_INCLUDE_DIRS=${ROOT}/${LIBSSH2_DIR}/include/ -DLIBSSH2_LIBRARY_DIRS=${ROOT}/${LIBSSH2_DIR} -DLIBSSH2_LIBRARIES=${ROOT}/${LIBSSH2_DIR}/lib/libssh2.a -DCMAKE_INSTALL_PREFIX=${ROOT}/${BUILD_FOLDER}/ -DCMAKE_BUILD_TYPE=${BUILD_TYPE} -DCMAKE_C_FLAGS_DEBUG="-ggdb -Og --save-temps" -DENABLE_TRACE=ON -DCMAKE_POSITION_INDEPENDENT_CODE=ON -DCMAKE_ANDROID_NDK=$NDK -DCMAKE_SYSTEM_NAME=Android -DCMAKE_SYSTEM_VERSION=$ANDROID_API_LEVEL -DCMAKE_ANDROID_ARCH_ABI=$ANDROID_ABI -DOPENSSL_ROOT_DIR=${ROOT}/${OPENSSL_DIR} -DOPENSSL_CRYPTO_LIBRARY=${ROOT}/${OPENSSL_DIR}/lib/libcrypto.a -DOPENSSL_SSL_LIBRARY=${ROOT}/${OPENSSL_DIR}/lib/libssl.a -DOPENSSL_INCLUDE_DIR=${ROOT}/${OPENSSL_DIR}/include

cmake -LAH .
cmake --build . -- VERBOSE=1
if OBJDUMP=$($NDK/toolchains/llvm/prebuilt/linux-x86_64/bin/llvm-objdump -f ${ROOT}/${TMP_BUILD_FOLDER}/libgit2.a)
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
