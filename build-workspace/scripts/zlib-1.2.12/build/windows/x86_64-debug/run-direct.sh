set -e
source "scripts/common/bash/common.sh"
set -x

define_basic_variables "zlib-1.2.12" "windows" "x86_64" "debug"
init_and_change_into_tmp_build_folder

## NOTES
##  - allow linking resulting static library against shared library later on
##    - CFLAGS=-fPIC
##

CC="x86_64-w64-mingw32-gcc" LD="x86_64-w64-mingw32-ld" CFLAGS="-fPIC" ${RELATIVE_SOURCE}/configure --static --prefix=${ROOT}/${BUILD_FOLDER}
make PREFIX=x86_64-w64-mingw32- BINARY_PATH=/usr/local/bin INCLUDE_PATH=/mingw/include LIBRARY_PATH=/mingw/lib
make install

print_done
