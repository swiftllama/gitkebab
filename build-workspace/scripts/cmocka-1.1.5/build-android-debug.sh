set -e
echo "--- Building Cmocka 1.1.5 for Android / Debug ---"

BUILD_FOLDER=build/cmocka-1.1.5/android/debug
TMP_BUILD_FOLDER=build/tmp/cmocka-1.1.5/android/debug

set -x
rm -rf ${BUILD_FOLDER}
rm -rf ${TMP_BUILD_FOLDER}

mkdir -p ${BUILD_FOLDER}
mkdir -p ${TMP_BUILD_FOLDER}

USER=$(whoami)
USER_ID=$(id -u ${USER})

BUILD_TYPE=Debug

docker run -v${PWD}:/tmp/workspace --user ${USER_ID} android-ndk-build /bin/bash -c "cd /tmp/workspace/${TMP_BUILD_FOLDER}; export NDK=/opt/android-ndk/android-ndk-r23/; export TOOLCHAIN=$NDK/toolchains/llvm/prebuilt/linux-x86_64; export TARGET=aarch64-linux-android; export API=21; export AR=$TOOLCHAIN/bin/llvm-ar; export CC=$TOOLCHAIN/bin/$TARGET$API-clang; export AS=$CC; export CXX=$TOOLCHAIN/bin/$TARGET$API-clang++; export LD=$TOOLCHAIN/bin/ld; export RANLIB=$TOOLCHAIN/bin/llvm-ranlib; export STRIP=$TOOLCHAIN/bin/llvm-strip;  cmake -DWITH_STATIC_LIB=true -DCMAKE_BUILD_TYPE=${BUILD_TYPE} -DCMAKE_INSTALL_PREFIX=/tmp/workspace/${BUILD_FOLDER}  ../../../../../source/cmocka-1.1.5; cmake --build .; cmake --build . --target install"

set +x
echo "--- DONE ---"
