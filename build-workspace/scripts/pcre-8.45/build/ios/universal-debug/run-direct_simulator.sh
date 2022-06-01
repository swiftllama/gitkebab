set -e
source "scripts/common/bash/common.sh"
set -x

define_basic_variables "pcre-8.45" "ios" "universal" "debug"
init_and_change_into_tmp_build_folder

XCODE_ROOT=/Applications/Xcode.app
IOS_SDK_VERSION=15.5
IOS_TARGET_VERSION=10.0
IOS_SDK=${XCODE_ROOT}/Contents/Developer/Platforms/iPhoneSimulator.platform/Developer/SDKs/iPhoneSimulator${IOS_SDK_VERSION}.sdk
ARCHS="i386;x86_64"

cmake -DBUILD_SHARED_LIBS=NO \
      -DCMAKE_C_COMPILER_WORKS=ON \
      -DCMAKE_CXX_COMPILER_WORKS=ON \
      -DWITH_STATIC_LIB=true \
      -DCMAKE_BUILD_TYPE=Debug \
      -DCMAKE_INSTALL_PREFIX=${ROOT}/${BUILD_FOLDER}_simulator  \
      -DPCRE_BUILD_PCRECPP=NO \
      -DPCRE_BUILD_PCREGREP=NO \
      -DPCRE_BUILD_TESTS=NO \
      -DPCRE_SUPPORT_LIBBZ2=NO \
      -DCMAKE_OSX_ARCHITECTURES=$ARCHS \
      -DCMAKE_OSX_SYSROOT=$IOS_SDK \
      ${RELATIVE_SOURCE} 

cmake --build .
cmake --build . --target install

print_done
