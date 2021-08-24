set -e
echo "--- Building OpenSSL 1.1.1k for Android/arm64/Debug ---"

ROOT=${PWD}
BUILD_FOLDER=build/openssl-1.1.1k/android/arm64-debug
TMP_BUILD_FOLDER=build/tmp/openssl-1.1.1k/android/arm64-debug

set -x
rm -rf ${BUILD_FOLDER}
rm -rf ${TMP_BUILD_FOLDER}

mkdir -p ${BUILD_FOLDER}
mkdir -p ${TMP_BUILD_FOLDER}
cd ${TMP_BUILD_FOLDER}

## NOTES
##  - allow linking resulting static library against shared library later on
##    - "-fPIC"
##  - openssl requires ANDROID_NDK_ROOT to be defined and PATH to be exported
##    To a location containing the pre-built android toolchain binaries

export NDK=/opt/android-ndk/android-ndk-r23
export TOOLCHAIN=$NDK/toolchains/llvm/prebuilt/linux-x86_64
export TARGET=aarch64-linux-android
export API=21 # minSdkVersion

export PATH=$TOOLCHAIN/bin/:$PATH/

ANDROID_NDK_HOME=$NDK ../../../../../source/openssl-1.1.1k/Configure android-arm64 -D__ANDROID_API__=$API --prefix=${ROOT}/${BUILD_FOLDER} --openssldir=${ROOT}/${BUILD_FOLDER} -fPIC 
make

echo "Verifying architecture..."
if OBJDUMP=$($NDK/toolchains/llvm/prebuilt/linux-x86_64/bin/llvm-objdump -f ${ROOT}/${TMP_BUILD_FOLDER}/libssl.a)
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


make install

set +x
echo "--- DONE ---"
