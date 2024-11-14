set -e
source "scripts/common/bash/common.sh"
set -x

define_basic_variables "cmocka-1.1.5" "linux" "x86_64" "release"
init_and_change_into_tmp_build_folder

cmake -DWITH_STATIC_LIB=true \
      -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_INSTALL_PREFIX=${ROOT}/${BUILD_FOLDER}  \
      ${RELATIVE_SOURCE}

cmake --build .
cmake --build . --target install

set +x
echo "--- DONE ---"
