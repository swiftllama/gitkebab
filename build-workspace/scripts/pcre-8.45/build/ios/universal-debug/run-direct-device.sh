set -e
source "scripts/common/bash/common.sh"
set -x

define_basic_variables "pcre-8.45" "ios" "universal" "debug"
init_and_change_into_tmp_build_folder

## NOTES
##  - allow linking resulting static library against shared library later on
##    - CFLAGS=-fPIC
##

XCODE_ROOT=/Applications/Xcode.app
IOS_SDK_VERSION=15.5
IOS_TARGET_VERSION=10.0
IOS_SDK=${XCODE_ROOT}/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS${IOS_SDK_VERSION}.sdk


FLAGS="-O3 -arch armv7 -arch armv7s -arch arm64 -isysroot $IOS_SDK -mios-version-min=${IOS_TARGET_VERSION} -fembed-bitcode"
${RELATIVE_SOURCE}/configure --prefix=${ROOT}/${BUILD_FOLDER}_device CFLAGS="$FLAGS" CXXFLAGS="$FLAGS"
make
make install

print_done
