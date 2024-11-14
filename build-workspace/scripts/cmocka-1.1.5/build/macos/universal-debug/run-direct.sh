set -e
source "scripts/common/bash/common.sh"
set -x

define_basic_variables "cmocka-1.1.5" "macos" "universal" "debug"
init_and_change_into_tmp_build_folder

cmake -DWITH_STATIC_LIB=true \
      -DCMAKE_BUILD_TYPE=Debug \
      -DCMAKE_INSTALL_PREFIX=${ROOT}/${BUILD_FOLDER}  \
      ${RELATIVE_SOURCE} \
      -DCMAKE_OSX_DEPLOYMENT_TARGET=10.11 \
      -DCMAKE_OSX_ARCHITECTURES="x86_64;arm64"

cmake --build . -- VERBOSE=1
cmake --build . --target install

set +x
echo "--- DONE ---"
