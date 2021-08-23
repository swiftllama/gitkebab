set -e
echo "--- Building Zlib 1.2.11 for android/arm64/Debug ---"

ROOT=${PWD}
BUILD_FOLDER=build/zlib-1.2.11/android/arm64-debug
TMP_BUILD_FOLDER=build/tmp/zlib-1.2.11/anrdoid/arm64-debug

set -x
rm -rf ${BUILD_FOLDER}
rm -rf ${TMP_BUILD_FOLDER}

mkdir -p ${BUILD_FOLDER}
mkdir -p ${TMP_BUILD_FOLDER}
cd ${TMP_BUILD_FOLDER}

## NOTES
##  - allow linking resulting static library against shared library later on
##    - CFLAGS=-fPIC
##

export NDK=/opt/android-ndk/android-ndk-r23
export TOOLCHAIN=$NDK/toolchains/llvm/prebuilt/linux-x86_64
export TARGET=aarch64-linux-android
export API=21 # minSdkVersion

export AR=$TOOLCHAIN/bin/llvm-ar
export CC=$TOOLCHAIN/bin/$TARGET$API-clang
export AS=$CC
export CXX=$TOOLCHAIN/bin/$TARGET$API-clang++
export LD=$TOOLCHAIN/bin/ld
export RANLIB=$TOOLCHAIN/bin/llvm-ranlib
export STRIP=$TOOLCHAIN/bin/llvm-strip

CFLAGS="-fPIC" ../../../../../source/zlib-1.2.11/configure --prefix=${ROOT}/${BUILD_FOLDER} 
make
make install

set +x
echo "--- DONE ---"
