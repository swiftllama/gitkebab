set -e
echo "--- Building Pcre 8.45 for Linux / Debug ---"

BUILD_FOLDER=build/pcre-8.45/linux/debug
TMP_BUILD_FOLDER=build/tmp/pcre-8.45/linux/debug

set -x
rm -rf ${BUILD_FOLDER}
rm -rf ${TMP_BUILD_FOLDER}

mkdir -p ${BUILD_FOLDER}
mkdir -p ${TMP_BUILD_FOLDER}

USER=$(whoami)
USER_ID=$(id -u ${USER})

## NOTES
##  - allow linking resulting static library against shared library later on
##    - CFLAGS=-fPIC
##

docker run -v${PWD}:/tmp/workspace --user ${USER_ID} android-ndk-build /bin/bash -c "cd /tmp/workspace/${TMP_BUILD_FOLDER}; NDK=/opt/android-ndk/android-ndk-r23/; TOOLCHAIN=$NDK/toolchains/llvm/prebuilt/linux-x86_64; TARGET=aarch64-linux-android; API=21; AR=$TOOLCHAIN/bin/llvm-ar; CC=$TOOLCHAIN/bin/$TARGET$API-clang; AS=$CC; CXX=$TOOLCHAIN/bin/$TARGET$API-clang++; LD=$TOOLCHAIN/bin/ld; RANLIB=$TOOLCHAIN/bin/llvm-ranlib; STRIP=$TOOLCHAIN/bin/llvm-strip;  echo cc is $CC;"

set +x
echo "--- DONE ---"
