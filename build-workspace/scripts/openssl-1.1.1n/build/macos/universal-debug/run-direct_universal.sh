set -e
source "scripts/common/bash/common.sh"
set -x

define_basic_variables "openssl-1.1.1n" "macos" "universal" "debug"
init_and_change_into_tmp_build_folder

rm -rf ${ROOT}/${BUILD_FOLDER}
cp -PR ${ROOT}/${BUILD_FOLDER}-x86_64 ${ROOT}/${BUILD_FOLDER}
lipo -create ${ROOT}/${BUILD_FOLDER}-x86_64/lib/libssl.a ${ROOT}/${BUILD_FOLDER}-arm64/lib/libssl.a -output ${ROOT}/${BUILD_FOLDER}/lib/libssl.a
lipo -create ${ROOT}/${BUILD_FOLDER}-x86_64/lib/libcrypto.a ${ROOT}/${BUILD_FOLDER}-arm64/lib/libcrypto.a -output ${ROOT}/${BUILD_FOLDER}/lib/libcrypto.a

print_done
