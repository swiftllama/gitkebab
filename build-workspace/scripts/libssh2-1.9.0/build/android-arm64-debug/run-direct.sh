set -e
echo "--- Building Libssh2 1.9.0 for Android/arm64/Debug ---"

ROOT=${PWD}
BUILD_FOLDER=build/libssh2-1.9.0/android/arm64-debug
TMP_BUILD_FOLDER=build/tmp/libssh2-1.9.0/android/arm64-debug

set -x
rm -rf ${BUILD_FOLDER}
rm -rf ${TMP_BUILD_FOLDER}

mkdir -p ${BUILD_FOLDER}
mkdir -p ${TMP_BUILD_FOLDER}
cd ${TMP_BUILD_FOLDER}

BUILD_TYPE=Debug

## NOTES
##  - allow linking resulting static library against shared library later on
##    - -DCMAKE_C_FLAGS="-fPIC"
##

OPENSSL_DIR=build/openssl-1.1.1k/android/arm64-debug
ZLIB_LIB=build/zlib-1.2.11/android/arm64-debug/lib/libz.a

NDK=/opt/android-ndk/android-ndk-r23
ANDROID_API_LEVEL=21
ANDROID_ABI=arm64-v8a

cmake ../../../../../source/libssh2-1.9.0 -DCRYPTO_BACKEND=OpenSSL -DENABLE_ZLIB_COMPRESSION=ON -DENABLE_DEBUG_LOGGING=ON -DZLIB_LIBRARY=${ROOT}/${ZLIB_LIB} -DOPENSSL_ROOT_DIR=${ROOT}/${OPENSSL_DIR} -DOPENSSL_CRYPTO_LIBRARY=${ROOT}/${OPENSSL_DIR}/lib/libcrypto.a -DOPENSSL_SSL_LIBRARY=${ROOT}/${OPENSSL_DIR}/lib/libssl.a -DOPENSSL_INCLUDE_DIR=${ROOT}/${OPENSSL_DIR}/include -DCMAKE_INSTALL_PREFIX=${ROOT}/${BUILD_FOLDER} -DCMAKE_BUILD_TYPE=${BUILD_TYPE} -DCMAKE_C_FLAGS="-fPIC" -DCMAKE_ANDROID_NDK=$NDK -DCMAKE_SYSTEM_NAME=Android -DCMAKE_SYSTEM_VERSION=$ANDROID_API_LEVEL -DCMAKE_ANDROID_ARCH_ABI=$ANDROID_ABI
cmake --build . -- VERBOSE=1
if OBJDUMP=$($NDK/toolchains/llvm/prebuilt/linux-x86_64/bin/llvm-objdump -f ${ROOT}/${TMP_BUILD_FOLDER}/src/libssh2.a)
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
