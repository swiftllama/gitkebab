set -e
source "scripts/common/bash/common.sh"
set -x

define_basic_variables "pcre-8.45" "windows" "x86_64" "debug"
init_and_change_into_tmp_build_folder

## NOTES
##  - allow linking resulting static library against shared library later on
##    - CFLAGS=-fPIC
##

cmake -DWITH_STATIC_LIB=true \
      -DCMAKE_BUILD_TYPE=Debug \
      -DCMAKE_INSTALL_PREFIX=${ROOT}/${BUILD_FOLDER}  \
      -DCMAKE_SYSTEM_NAME=Windows \
      -DCMAKE_FIND_ROOT_PATH=${MINGW} \
      -DCMAKE_FIND_ROOT_PATH_MODE_PROGRAM=NEVER \
      -DCMAKE_FIND_ROOT_PATH_MODE_LIBRARY=ONLY \
      -DCMAKE_FIND_ROOT_PATH_MODE_INCLUDE=ONLY \
      -DCMAKE_C_COMPILER=x86_64-w64-mingw32-gcc \
      -DCMAKE_CXX_COMPILER=x86_64-w64-mingw32-g++ \
      -DCMAKE_RC_COMPILER=x86_64-w64-mingw32-windres \
      ${RELATIVE_SOURCE}

cmake --build . -- VERBOSE=1
cmake --build . --target install

print_done
