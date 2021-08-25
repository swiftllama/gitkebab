set -e
source "scripts/common/bash/common.sh"
set -x

define_basic_variables "pcre-8.45" "android" "x86_64" "debug"
init_and_change_into_tmp_build_folder

## NOTES
##  - allow linking resulting static library against shared library later on
##    - CFLAGS=-fPIC
##

define_android_variables

${RELATIVE_SOURCE}/configure \
                  --prefix=${ROOT}/${BUILD_FOLDER} \
                  CFLAGS=-fPIC \
                  CXXFLAGS="-fPIC" \
                  --target=arm-linux-androideabi \
                  --host=arm-linux-androideabi \
                  --disable-dependency-tracking

make
android_objdump_verify_library_architecture "${ROOT}/${TMP_BUILD_FOLDER}/.libs/libpcre.a" "${ANDROID_OBJDUMP_ARCHITECTURE}"
make install

print_done
