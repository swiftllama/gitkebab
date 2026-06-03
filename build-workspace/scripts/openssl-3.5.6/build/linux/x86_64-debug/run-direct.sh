set -e
source "scripts/common/bash/common.sh"
set -x

define_basic_variables "openssl-3.5.6" "linux" "x86_64" "debug"
init_and_change_into_tmp_build_folder

## NOTES
##  - allow linking resulting static library against shared library later on
##    - CFLAGS=-fPIC
##  - OpenSSL 3.x's linux-x86_64 target has a multilib postfix, so libs install
##    to lib64/ (not lib/ as in 1.1.x). We keep that native layout; check.sh
##    targets lib64/ and downstream consumers will be updated to match later.

${RELATIVE_SOURCE}/Configure linux-x86_64 no-shared --prefix=${ROOT}/${BUILD_FOLDER} --openssldir=${ROOT}/${BUILD_FOLDER} -fPIC
make
make install_sw

print_done
