set -e
source "scripts/common/bash/common.sh"
set -x

define_basic_variables "pcre-8.45" "macos" "universal" "debug"
init_and_change_into_tmp_build_folder

## NOTES
##  - allow linking resulting static library against shared library later on
##    - CFLAGS=-fPIC
##

FLAGS="-fPIC --target=x86_64-apple-macos -arch x86_64 -arch arm64 -isysroot /Library/Developer/CommandLineTools/SDKs/MacOSX12.3.sdk -mmacosx-version-min=10.11"
${RELATIVE_SOURCE}/configure --prefix=${ROOT}/${BUILD_FOLDER} CFLAGS="${FLAGS}" CXXFLAGS="${FLAGS}" CC="/Library/Developer/CommandLineTools/usr/bin/cc" LDFLAGS="${FLAGS}"
make
make install

print_done
