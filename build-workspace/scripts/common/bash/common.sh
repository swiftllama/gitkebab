set -E              # propagate error backtrace to functions
trap backtrace ERR  # show backtrace when an error occurs

# Defines ROOT, LIBRARY_WITH_VERSION, TARGET_PLATFORM
#         TARGET_ARCHITECTURE and TARGET_CONFIGURATION
##
## Arguments:
##   1 - library_with_version
##   2 - platform
##   3 - architecture
##   4 - configuration
##
function define_basic_variables() {
    ROOT=${PWD}
    LIBRARY_WITH_VERSION=$1
    TARGET_PLATFORM=$2
    TARGET_ARCHITECTURE=$3
    TARGET_CONFIGURATION=$4
    RELATIVE_SOURCE=../../../../../source/$LIBRARY_WITH_VERSION
}

function define_build_folders() {
    BUILD_FOLDER=build/${LIBRARY_WITH_VERSION}/${TARGET_PLATFORM}/${TARGET_ARCHITECTURE}-${TARGET_CONFIGURATION}
    TMP_BUILD_FOLDER=build/tmp/${LIBRARY_WITH_VERSION}/${TARGET_PLATFORM}/${TARGET_ARCHITECTURE}-${TARGET_CONFIGURATION}
}

function delete_and_recreate_build_and_tmp_build_folders() {
    rm -rf ${BUILD_FOLDER}
    rm -rf ${TMP_BUILD_FOLDER}
    mkdir -p ${BUILD_FOLDER}
    mkdir -p ${TMP_BUILD_FOLDER}
}

function print_building_header() {
    { echo""; echo "--- Building ${LIBRARY_WITH_VERSION} for ${TARGET_PLATFORM}/${TARGET_ARCHITECTURE}/${TARGET_CONFIGURATION} ---"; } 2> /dev/null
}

function print_done() {
    { echo""; echo "--- DONE ---"; } 2> /dev/null
}

function init_and_change_into_tmp_build_folder() {
    print_building_header
    define_build_folders 
    delete_and_recreate_build_and_tmp_build_folders
    cd ${TMP_BUILD_FOLDER}
}

function define_android_variables() {
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
}
