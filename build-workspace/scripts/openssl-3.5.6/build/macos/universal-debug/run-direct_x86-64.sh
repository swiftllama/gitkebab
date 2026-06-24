set -e
source "scripts/common/bash/common.sh"
set -x

define_basic_variables "openssl-3.5.6" "macos" "universal" "debug"
init_and_change_into_tmp_build_folder

rm -rf ${ROOT}/${BUILD_FOLDER}-x86_64

## NOTES
##  - allow linking resulting static library against shared library later on
##    - CFLAGS=-fPIC
##  - --libdir=lib pins install layout to lib/ (downstream macos consumers
##    expect ${OPENSSL_DIR}/lib/libssl.a etc).
##

${RELATIVE_SOURCE}/Configure darwin64-x86_64-cc no-shared --prefix=${ROOT}/${BUILD_FOLDER} --openssldir=${ROOT}/${BUILD_FOLDER} --libdir=lib -fPIC -mmacosx-version-min=10.11
make
make install_sw

mv ${ROOT}/${BUILD_FOLDER} ${ROOT}/${BUILD_FOLDER}-x86_64

print_done
