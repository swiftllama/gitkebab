set -e
source "scripts/common/bash/common.sh"
set -x

define_basic_variables "openssl-1.1.1n" "macos" "universal" "debug"
init_and_change_into_tmp_build_folder

rm -rf ${ROOT}/${BUILD_FOLDER}-arm64

## NOTES
##  - allow linking resulting static library against shared library later on
##    - CFLAGS=-fPIC
##

${RELATIVE_SOURCE}/Configure darwin64-arm64-cc no-shared --prefix=${ROOT}/${BUILD_FOLDER} --openssldir=${ROOT}/${BUILD_FOLDER} -fPIC -mmacosx-version-min=10.11
make
make install

mv ${ROOT}/${BUILD_FOLDER} ${ROOT}/${BUILD_FOLDER}-arm64

print_done
