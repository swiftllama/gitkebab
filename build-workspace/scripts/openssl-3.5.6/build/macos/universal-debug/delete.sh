set -e
source "scripts/common/bash/common.sh"
set -x

define_basic_variables "openssl-3.5.6" "macos" "universal" "debug"
define_build_folders
delete_build_and_tmp_build_folder

rm -rf ${BUILD_FOLDER}-x86_64
rm -rf ${BUILD_FOLDER}-arm64

print_done
