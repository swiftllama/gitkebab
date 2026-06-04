set -e
source "scripts/common/bash/common.sh"
set -x

define_basic_variables "openssl-3.5.6" "android" "x86_64" "debug"
init_and_change_into_tmp_build_folder

## NOTES
##  - allow linking resulting static library against shared library later on
##    - "-fPIC"
##  - openssl requires ANDROID_NDK_ROOT to be defined and PATH to be exported
##    To a location containing the pre-built android toolchain binaries
##  - android targets have no multilib postfix, so libs install to lib/ (unlike
##    the linux-x86_64 target which uses lib64/). install_sw skips docs/man.

define_android_variables

export PATH=$NDK_TOOLCHAIN/bin/:$PATH/
# OpenSSL 3.x reads ANDROID_NDK_ROOT (1.1.x used ANDROID_NDK_HOME). Override the
# docker image's ANDROID_NDK_ROOT (=/opt/android-ndk) which points one level
# above the actual NDK and fails OpenSSL's source.properties validity check.
export ANDROID_NDK_ROOT=$NDK
${RELATIVE_SOURCE}/Configure \
                  android-x86_64 \
                  no-ui-console \
                  no-stdio \
                  --prefix=${ROOT}/${BUILD_FOLDER} \
                  --openssldir=${ROOT}/${BUILD_FOLDER} \
                  -fPIC

make
android_objdump_verify_library_architecture "${ROOT}/${TMP_BUILD_FOLDER}/libssl.a" "${ANDROID_OBJDUMP_ARCHITECTURE}"
make install_sw

print_done
