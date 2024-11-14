set -e
source "scripts/common/bash/common.sh"
set -x

define_basic_variables "openssl-1.1.1n" "ios" "universal" "debug"
init_and_change_into_tmp_build_folder

rm -rf ${ROOT}/${BUILD_FOLDER}_device-arm64

XCODE_ROOT=/Applications/Xcode.app
XCRUN_FOLDER="/usr/bin/"

IOS_SDK_VERSION=15.5
IOS_TARGET_VERSION=10.0
IOS_SDK=iPhoneOS${IOS_SDK_VERSION}.sdk
IOS_SDKS_FOLDER=${XCODE_ROOT}/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/

CROSS_COMPILE="${XCRUN_FOLDER}" CFLAGS="-arch arm64 -isysroot ${IOS_SDKS_FOLDER}/${IOS_SDK} -fembed-bitcode" LDFLAGS=" -fembed-bitcode" ${RELATIVE_SOURCE}/Configure ios64-xcrun no-shared no-dso no-hw no-engine --prefix=${ROOT}/${BUILD_FOLDER} --openssldir=${ROOT}/${BUILD_FOLDER}
make clean
make
make install

mv ${ROOT}/${BUILD_FOLDER} ${ROOT}/${BUILD_FOLDER}_device-arm64

print_done
