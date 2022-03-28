set -e
source "scripts/common/bash/common.sh"
set -x

define_basic_variables "zlib-1.2.12" "android" "x86_64" "debug"
init_and_change_into_tmp_build_folder

## NOTES
##  - allow linking resulting static library against shared library later on
##    - CFLAGS=-fPIC
##

define_android_variables
CFLAGS="-fPIC" ${RELATIVE_SOURCE}/configure --prefix=${ROOT}/${BUILD_FOLDER} 
make
android_objdump_verify_library_architecture "${ROOT}/${TMP_BUILD_FOLDER}/libz.a" "${ANDROID_OBJDUMP_ARCHITECTURE}"
make install

print_done
