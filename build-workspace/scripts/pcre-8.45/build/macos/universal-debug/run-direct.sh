set -e
source "scripts/common/bash/common.sh"
set -x

define_basic_variables "pcre-8.45" "macos" "universal" "debug"
init_and_change_into_tmp_build_folder

## NOTES
##  - allow linking resulting static library against shared library later on
##    - CFLAGS=-fPIC
##  - --disable-cpp: libpcrecpp is unused by gitkebab and its dylib link step
##    breaks under libtool's universal-binary path (mixes x86_64/arm64 slices).
##  - --disable-shared: only libpcre.a is consumed downstream; skipping dylibs
##    avoids the same libtool universal-binary failure mode.
##

SDK_PATH="$(xcrun --show-sdk-path)"
FLAGS="-fPIC --target=x86_64-apple-macos -arch x86_64 -arch arm64 -isysroot ${SDK_PATH} -mmacosx-version-min=10.11"
${RELATIVE_SOURCE}/configure --prefix=${ROOT}/${BUILD_FOLDER} --disable-cpp --disable-shared --enable-static CFLAGS="${FLAGS}" CXXFLAGS="${FLAGS}" CC="$(xcrun --find cc)" LDFLAGS="${FLAGS}"
make
make install

print_done
