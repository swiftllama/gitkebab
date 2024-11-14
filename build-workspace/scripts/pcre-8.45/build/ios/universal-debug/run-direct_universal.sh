set -e
source "scripts/common/bash/common.sh"
set -x

define_basic_variables "pcre-8.45" "ios" "universal" "debug"
init_and_change_into_tmp_build_folder

rm -rf ${ROOT}/${BUILD_FOLDER}
cp -PR ${ROOT}/${BUILD_FOLDER}_device ${ROOT}/${BUILD_FOLDER}

lipo -create ${ROOT}/${BUILD_FOLDER}_device/lib/libpcre.a ${ROOT}/${BUILD_FOLDER}_simulator/lib/libpcre.a -output ${ROOT}/${BUILD_FOLDER}/lib/libpcre.a
lipo -create ${ROOT}/${BUILD_FOLDER}_device/lib/libpcreposix.a ${ROOT}/${BUILD_FOLDER}_simulator/lib/libpcreposix.a -output ${ROOT}/${BUILD_FOLDER}/lib/libpcreposix.a

print_done
