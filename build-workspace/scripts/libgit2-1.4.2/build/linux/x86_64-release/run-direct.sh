set -e
source "scripts/common/bash/common.sh"
set -x

define_basic_variables "libgit2-1.4.2" "linux" "x86_64" "release"
init_and_change_into_tmp_build_folder


# NOTES:
#
#  - Improve gdb debugging with:
#     -DCMAKE_C_FLAGS_DEBUG=\"-ggdb -Og\"
#
#  - Allow creating shraed libs from libgit2
#     -DCMAKE_POSITION_INDEPENDENT_CODE=ON
#
#  - Enable libgit trace with
#     -DENABLE_TRACE=ON
#  

OPENSSL_DIR=build/openssl-1.1.1n/${TARGET_FOLDER_TRIPLET}
ZLIB_LIB=build/zlib-1.2.12/${TARGET_FOLDER_TRIPLET}/lib/libz.a
LIBSSH2_DIR=build/libssh2-1.10.0/${TARGET_FOLDER_TRIPLET}
PCRE_DIR=build/pcre-8.45/${TARGET_FOLDER_TRIPLET}
ICONV_DIR=build/libiconv-1.16/${TARGET_FOLDER_TRIPLET}

CUSTOM_SEARCH_PATH="${ROOT}/${PCRE_DIR};${ROOT}/${ICONV_DIR};${ROOT}/${OPENSSL_DIR};"

cmake ${RELATIVE_SOURCE} \
      -DCMAKE_PREFIX_PATH="${CUSTOM_SEARCH_PATH}" \
      -DCMAKE_BUILD_TYPE=Release \
      -DBUILD_SHARED_LIBS=NO \
      -DUSE_SSH=0 \
      -DLIBSSH2_FOUND=1 \
      -DLIBSSH2_INCLUDE_DIRS=${ROOT}/${LIBSSH2_DIR}/include/ \
      -DLIBSSH2_LIBRARY_DIRS=${ROOT}/${LIBSSH2_DIR} \
      -DLIBSSH2_LIBRARIES=${ROOT}/${LIBSSH2_DIR}/lib/libssh2.a \
      -DCMAKE_INSTALL_PREFIX=${ROOT}/${BUILD_FOLDER}/ \
      -DENABLE_TRACE=OFF \
      -DCMAKE_POSITION_INDEPENDENT_CODE=ON 

cmake -LAH .
cmake --build . -- VERBOSE=1
cmake --build . --target install

print_done




