set -E              # propagate error backtrace to functions
trap backtrace ERR  # show backtrace when an error occurs

# Defines ROOT, LIBRARY_WITH_VERSION, TARGET_PLATFORM
#         TARGET_ARCHITECTURE, TARGET_CONFIGURATION,
#         TARGET_FOLDER_TRIPLET, RELATIVE_SOURCE, 
##        BUILD_SCRIPTS
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
    TARGET_FOLDER_TRIPLET="${TARGET_PLATFORM}/${TARGET_ARCHITECTURE}-${TARGET_CONFIGURATION}"
    RELATIVE_SOURCE="../../../../../source/$LIBRARY_WITH_VERSION"
    BUILD_SCRIPTS="scripts/$LIBRARY_WITH_VERSION/build/${TARGET_PLATFORM}/${TARGET_ARCHITECTURE}-${TARGET_CONFIGURATION}"
}

function define_basic_docker_variables() {
    USER=$(whoami)
    USER_ID=$(id -u ${USER})
}

function define_build_folders() {
    BUILD_FOLDER="build/${LIBRARY_WITH_VERSION}/${TARGET_FOLDER_TRIPLET}"
    TMP_BUILD_FOLDER="build/tmp/${LIBRARY_WITH_VERSION}/${TARGET_FOLDER_TRIPLET}"
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
    if [ "${TARGET_ARCHITECTURE}" = "arm64" ]; then
        export ANDROID_ABI="arm64-v8a"
        export ANDROID_OBJDUMP_ARCHITECTURE="aarch64"
        export ANDROID_TARGET=aarch64-linux-android
    elif [ "${TARGET_ARCHITECTURE}" = "x86_64" ]; then
        export ANDROID_ABI="x86_64"
        export ANDROID_OBJDUMP_ARCHITECTURE="x86_64"
        export ANDROID_TARGET=x86_64-linux-android
    fi
    export NDK=/opt/android-ndk/android-ndk-r23
    export NDK_TOOLCHAIN=$NDK/toolchains/llvm/prebuilt/linux-x86_64
    export API=21 # minSdkVersion
    
    export AR=$NDK_TOOLCHAIN/bin/llvm-ar
    export CC=$NDK_TOOLCHAIN/bin/$ANDROID_TARGET$API-clang
    export AS=$CC
    export CXX=$NDK_TOOLCHAIN/bin/$ANDROID_TARGET$API-clang++
    export LD=$NDK_TOOLCHAIN/bin/ld
    export RANLIB=$NDK_TOOLCHAIN/bin/llvm-ranlib
    export STRIP=$NDK_TOOLCHAIN/bin/llvm-strip

    export ANDROID_API_LEVEL=21
       
}

# run the NDK's objdump on the given artifact and verify it has the
# given architecture
#
## Arguments:
##  1) LIBRARY - path to library
##  2) ARCH    - architecture to check for
##
## Assumptions:
##  - define_android_variables() has been called (or $NDK is defined)
function android_objdump_verify_library_architecture() {
    local LIBRARY=$1
    local ARCH=$2

    echo "Verifying architecture [${ARCH}] for build product [${LIBRARY}].."

    if OBJDUMP=$($NDK_TOOLCHAIN/bin/llvm-objdump -f ${LIBRARY})
then
        if RESULTING_ARCH="$(echo "$OBJDUMP" | grep -m 1 'architecture:')"
        then
            if [ "${RESULTING_ARCH}" = "architecture: ${ARCH}" ]; then
                echo "Found ${RESULTING_ARCH} in build product"
            else
                echo "Build succeeded but artifact has unexpected ${RESULTING_ARCH} (expected [${ARCH}])"
                exit 2
            fi
        else
            echo "Error greping [$LIBRARY}] for architecture [${ARCH}]"
            exit 4
        fi
    else
        echo "Error executing objdump on [${LIBRARY}] to verify architecture [${ARCH}]"
        exit 3
    fi
}

function macos_lipo_verify_library_architecture() {
    local LIBRARY=$1
    local ARCH=$2
    echo "Verifying architecture [${ARCH}] for build product [${LIBRARY}].."

    if LIPO_INFO=$(lipo -info ${LIBRARY})
    then
        if RESULTING_ARCHS="$(echo "$LIPO_INFO" | cut -d ':' -f 3)"
        then
            echo "Found ${RESULTING_ARCH} in build product"
            if [[ "${RESULTING_ARCHS}" == *"${ARCH}"* ]]
            then  
                echo "Verified that [${ARCH}] is in [${RESULTING_ARCHS}]"
            else
                echo "Build successful but artifact architectures [${RESULTING_ARCHS}] is missing [${ARCH}]"
                exit 3
            fi
        else
            echo "Build successful but unable to determine archs from [${LIPO_INFO}]"
            exit 2
        fi
    else
        echo "Error executing 'lipo -info' on [${LIBRARY}] to verify architecture [${ARCH}]"
    fi
}

function check_sentinel_exists() {
    local SENTINEL=$1
    { echo""; echo "--- Checking $LIBRARY_WITH_VERSION build for $TARGET_PLATFORM-$TARGET_ARCHITECTURE-$TARGET_CONFIGURATION ---"; } 2> /dev/null

    echo "Working directory: ${PWD}"
    echo "Sentinel: $SENTINEL"

    if [ -f $SENTINEL ]
    then
        echo "Build seems to exist"
        return 0
    fi

    echo "Build does not exist"
    return 1
}

function delete_build_and_tmp_build_folder() {
    { echo""; echo "--- Deleting ${LIBRARY_WITH_VERSION} build for ${TARGET_PLATFORM}/${TARGET_ARCHITECTURE}/${TARGET_CONFIGURATION} ---"; } 2> /dev/null
    rm -rf ${BUILD_FOLDER}
    rm -rf ${BUILD_FOLDER}_simulator
    rm -rf ${BUILD_FOLDER}_device
    rm -rf ${TMP_BUILD_FOLDER}
}
