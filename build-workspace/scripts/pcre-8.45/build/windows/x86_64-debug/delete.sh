set -e
source "scripts/common/bash/common.sh"
set -x

define_basic_variables "pcre-8.45" "windows" "x86_64" "debug"
define_build_folders
delete_build_and_tmp_build_folder

print_done