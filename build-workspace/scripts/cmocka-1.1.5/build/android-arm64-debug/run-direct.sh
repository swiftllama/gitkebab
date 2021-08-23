set -e
echo "--- Building Cmocka 1.1.5 for android/arm64/Debug ---"

ROOT=${PWD}
BUILD_FOLDER=build/cmocka-1.1.5/android/arm64-debug
TMP_BUILD_FOLDER=build/tmp/cmocka-1.1.5/android/arm64-debug

set -x
rm -rf ${BUILD_FOLDER}
rm -rf ${TMP_BUILD_FOLDER}

mkdir -p ${BUILD_FOLDER}
mkdir -p ${TMP_BUILD_FOLDER}
cd ${TMP_BUILD_FOLDER}

BUILD_TYPE=Debug

NDK=/opt/android-ndk/android-ndk-r23
ANDROID_API_LEVEL=21
ANDROID_ABI=arm64-v8a

cmake -DWITH_STATIC_LIB=true -DCMAKE_BUILD_TYPE=${BUILD_TYPE} -DCMAKE_INSTALL_PREFIX=${ROOT}/${BUILD_FOLDER} -DCMAKE_ANDROID_NDK=$NDK -DCMAKE_SYSTEM_NAME=Android -DCMAKE_SYSTEM_VERSION=$ANDROID_API_LEVEL -DCMAKE_ANDROID_ARCH_ABI=$ANDROID_ABI  ../../../../../source/cmocka-1.1.5
cmake --build .

echo "Verifying architecture..."
if OBJDUMP=$($NDK/toolchains/llvm/prebuilt/linux-x86_64/bin/llvm-objdump -f ${ROOT}/${TMP_BUILD_FOLDER}/src/libcmocka-static.a)
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

echo "Installing..."
cmake --build . --target install



set +x
echo "--- DONE ---"
