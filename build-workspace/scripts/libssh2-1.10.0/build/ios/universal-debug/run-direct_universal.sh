set -e
source "scripts/common/bash/common.sh"
set -x

define_basic_variables "libssh2-1.10.0" "ios" "universal" "debug"
init_and_change_into_tmp_build_folder

rm -rf ${ROOT}/${BUILD_FOLDER}
cp -PR ${ROOT}/${BUILD_FOLDER}_device ${ROOT}/${BUILD_FOLDER}

lipo -create ${ROOT}/${BUILD_FOLDER}_device/lib/libssh2.a ${ROOT}/${BUILD_FOLDER}_simulator/lib/libssh2.a -output ${ROOT}/${BUILD_FOLDER}/lib/libssh2.a

print_done
