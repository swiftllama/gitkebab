set -e
source "scripts/common/bash/common.sh"
set -x

define_basic_variables "cmocka-1.1.5" "android" "arm64" "debug"
init_and_change_into_tmp_build_folder

define_android_variables

cmake -DWITH_STATIC_LIB=true \
      -DCMAKE_BUILD_TYPE=Debug \
      -DCMAKE_INSTALL_PREFIX=${ROOT}/${BUILD_FOLDER} \
      -DCMAKE_ANDROID_NDK=$NDK \
      -DCMAKE_SYSTEM_NAME=Android \
      -DCMAKE_SYSTEM_VERSION=$ANDROID_API_LEVEL \
      -DCMAKE_ANDROID_ARCH_ABI=$ANDROID_ABI \
      ${RELATIVE_SOURCE}

cmake --build .
android_objdump_verify_library_architecture "${ROOT}/${TMP_BUILD_FOLDER}/src/libcmocka-static.a" "${ANDROID_OBJDUMP_ARCHITECTURE}"
cmake --build . --target install

print_done
