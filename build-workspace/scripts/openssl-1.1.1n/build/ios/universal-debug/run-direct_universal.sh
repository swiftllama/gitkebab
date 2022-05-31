set -e
source "scripts/common/bash/common.sh"
set -x

define_basic_variables "openssl-1.1.1n" "ios" "universal" "debug"
init_and_change_into_tmp_build_folder

rm -rf ${ROOT}/${BUILD_FOLDER}
cp -PR ${ROOT}/${BUILD_FOLDER}_simulator ${ROOT}/${BUILD_FOLDER}
lipo -create ${ROOT}/${BUILD_FOLDER}_device-armv7/lib/libssl.a ${ROOT}/${BUILD_FOLDER}_device-arm64/lib/libssl.a ${ROOT}/${BUILD_FOLDER}_simulator/lib/libssl.a  -output ${ROOT}/${BUILD_FOLDER}/lib/libssl.a
lipo -create ${ROOT}/${BUILD_FOLDER}_device-armv7/lib/libcrypto.a ${ROOT}/${BUILD_FOLDER}_device-arm64/lib/libcrypto.a ${ROOT}/${BUILD_FOLDER}_simulator/lib/libcrypto.a  -output ${ROOT}/${BUILD_FOLDER}/lib/libcrypto.a

print_done
