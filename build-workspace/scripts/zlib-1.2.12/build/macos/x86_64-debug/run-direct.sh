set -e
source "scripts/common/bash/common.sh"
set -x

define_basic_variables "zlib-1.2.12" "macos" "x86_64" "debug"
init_and_change_into_tmp_build_folder

## NOTES
##  - allow linking resulting static library against shared library later on
##    - CFLAGS=-fPIC
##

CFLAGS="-fPIC -mmacosx-version-min=10.11" ${RELATIVE_SOURCE}/configure --prefix=${ROOT}/${BUILD_FOLDER}
make
make install

print_done
