set -e
source "scripts/common/bash/common.sh"
set -x

define_basic_variables "zlib-1.2.12" "ios" "universal" "debug"
init_and_change_into_tmp_build_folder

rm -rf ${ROOT}/${BUILD_FOLDER}
cp -PR ${ROOT}/${BUILD_FOLDER}_device ${ROOT}/${BUILD_FOLDER}
lipo -create ${ROOT}/${BUILD_FOLDER}_device/lib/libz.a ${ROOT}/${BUILD_FOLDER}_simulator/lib/libz.a -output ${ROOT}/${BUILD_FOLDER}/lib/libz.a

print_done
