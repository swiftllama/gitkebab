set -e
source "scripts/common/bash/common.sh"
set -x

define_basic_variables "libgit2-1.4.2" "ios" "universal" "debug"
define_build_folders
delete_build_and_tmp_build_folder

print_done
