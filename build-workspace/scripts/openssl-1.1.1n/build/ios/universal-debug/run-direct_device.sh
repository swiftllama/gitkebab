set -e
source "scripts/common/bash/common.sh"
set -x

define_basic_variables "openssl-1.1.1n" "ios" "universal" "debug"
init_and_change_into_tmp_build_folder

XCODE_ROOT=/Applications/Xcode.app
XCRUN_FOLDER="/usr/bin/"

IOS_SDK_VERSION=15.5
IOS_TARGET_VERSION=10.0
IOS_SDK=iPhoneOS${IOS_SDK_VERSION}.sdk
IOS_SDKS_FOLDER=${XCODE_ROOT}/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/

CROSS_TOP="${IOS_SDKS_FOLDER}" CROSS_SDK="${IOS_SDK}" CROSS_COMPILE="${XCRUN_FOLDER}" CFLAGS="-arch armv7 -arch armv7s -arch arm64" ${RELATIVE_SOURCE}/Configure ios-xcrun no-shared no-dso no-hw no-engine --prefix=${ROOT}/${BUILD_FOLDER} --openssldir=${ROOT}/${BUILD_FOLDER}
make clean
make
make install

print_done
