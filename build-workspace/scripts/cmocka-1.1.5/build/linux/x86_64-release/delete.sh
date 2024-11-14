set -e
source "scripts/common/bash/common.sh"
set -x

define_basic_variables "cmocka-1.1.5" "linux" "x86_64" "release"
define_build_folders
delete_build_and_tmp_build_folder

print_done
