set -e
source "scripts/common/bash/common.sh"
set -x

define_basic_variables "libiconv-1.17" "macos" "universal" "debug"
init_and_change_into_tmp_build_folder

## NOTES
##  - allow linking resulting static library against shared library later on
##    - CFLAGS=-fPIC
##

SDK_PATH="$(xcrun --show-sdk-path)"
FLAGS="-fPIC --target=x86_64-apple-macos -arch x86_64 -arch arm64 -isysroot ${SDK_PATH} -mmacosx-version-min=10.11"
${RELATIVE_SOURCE}/configure --enable-static --prefix=${ROOT}/${BUILD_FOLDER} CFLAGS="${FLAGS}" CXXFLAGS="${FLAGS}" LDFLAGS="${FLAGS}"
make
make install

print_done
