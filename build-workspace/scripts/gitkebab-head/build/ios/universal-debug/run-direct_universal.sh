set -e
source "scripts/common/bash/common.sh"
set -x

define_basic_variables "gitkebab-head" "ios" "universal" "debug"
init_and_change_into_tmp_build_folder

rm -rf ${ROOT}/${BUILD_FOLDER}
cp -PR ${ROOT}/${BUILD_FOLDER}_device ${ROOT}/${BUILD_FOLDER}

lipo -create ${ROOT}/${BUILD_FOLDER}_device/lib/libgitkebab_static.a ${ROOT}/${BUILD_FOLDER}_simulator/lib/libgitkebab_static.a -output ${ROOT}/${BUILD_FOLDER}/lib/libgitkebab_static.a
lipo -create ${ROOT}/${BUILD_FOLDER}_device/lib/libgitkebab.dylib ${ROOT}/${BUILD_FOLDER}_simulator/lib/libgitkebab.dylib -output ${ROOT}/${BUILD_FOLDER}/lib/libgitkebab.dylib

print_done
