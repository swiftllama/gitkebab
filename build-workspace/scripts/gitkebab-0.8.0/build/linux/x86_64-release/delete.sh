set -e
source "scripts/common/bash/common.sh"
set -x

define_basic_variables "gitkebab-0.8.0" "linux" "x86_64" "release"
define_build_folders
delete_build_and_tmp_build_folder

print_done
