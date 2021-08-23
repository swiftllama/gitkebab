set -e
echo "--- Building Pcre 8.45 for Android/arm64/Debug ---"

ROOT=${PWD}
BUILD_FOLDER=build/pcre-8.45/android/arm64-debug
TMP_BUILD_FOLDER=build/tmp/pcre-8.45/android/arm64-debug

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

../../../../../source/pcre-8.45/configure --prefix=${ROOT}/${BUILD_FOLDER} CFLAGS=-fPIC CXXFLAGS="-fPIC" --target=arm-linux-androideabi --host=arm-linux-androideabi --disable-dependency-tracking
make 

make install

set +x
echo "--- DONE ---"
