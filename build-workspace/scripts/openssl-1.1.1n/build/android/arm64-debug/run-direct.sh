set -e
source "scripts/common/bash/common.sh"
set -x

define_basic_variables "openssl-1.1.1n" "android" "arm64" "debug"
init_and_change_into_tmp_build_folder

## NOTES
##  - allow linking resulting static library against shared library later on
##    - "-fPIC"
##  - openssl requires ANDROID_NDK_ROOT to be defined and PATH to be exported
##    To a location containing the pre-built android toolchain binaries

define_android_variables

export PATH=$NDK_TOOLCHAIN/bin/:$PATH/
export ANDROID_NDK_HOME=$NDK
${RELATIVE_SOURCE}/Configure \
                  android-arm64 \
                  no-ui-console \
                  no-stdio \
                  --prefix=${ROOT}/${BUILD_FOLDER} \
                  --openssldir=${ROOT}/${BUILD_FOLDER} \
                  -fPIC 

make
android_objdump_verify_library_architecture "${ROOT}/${TMP_BUILD_FOLDER}/libssl.a" "${ANDROID_OBJDUMP_ARCHITECTURE}"
make install

print_done
