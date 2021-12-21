set -e
source "scripts/common/bash/common.sh"
set -x

define_basic_variables "pcre-8.45" "linux" "x86_64" "debug"
init_and_change_into_tmp_build_folder

## NOTES
##  - allow linking resulting static library against shared library later on
##    - CFLAGS=-fPIC
##

${RELATIVE_SOURCE}/configure --enable-shared=no --prefix=${ROOT}/${BUILD_FOLDER} CFLAGS=-fPIC CXXFLAGS="-fPIC"
make
make install

print_done
