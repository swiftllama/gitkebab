set -e
source "scripts/common/bash/common.sh"
set -x

define_basic_variables "openssl-1.1.1k" "linux" "x86_64" "debug"
init_and_change_into_tmp_build_folder

## NOTES
##  - allow linking resulting static library against shared library later on
##    - CFLAGS=-fPIC
##

${RELATIVE_SOURCE}/Configure linux-x86_64 --prefix=${ROOT}/${BUILD_FOLDER} --openssldir=${ROOT}/${BUILD_FOLDER} -fPIC
make
make install

print_done
