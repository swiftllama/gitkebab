set -e
source "scripts/common/bash/common.sh"
set -x

define_basic_variables "libgit2-1.4.2" "windows" "x86_64" "debug"
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

OPENSSL_LIB=${ROOT}/${OPENSSL_DIR}/lib/libssl.a
CRYPTO_LIB=${ROOT}/${OPENSSL_DIR}/lib/libcrypto.a
CUSTOM_SEARCH_PATH="${ROOT}/${PCRE_DIR};${ROOT}/${ICONV_DIR};${ROOT}/${OPENSSL_DIR}"

export DLLTOOL=x86_64-w64-mingw32-dlltool 
DLLTOOL=x86_64-w64-mingw32-dlltool cmake ${RELATIVE_SOURCE} \
      -DCMAKE_PREFIX_PATH="${CUSTOM_SEARCH_PATH}" \
      -DDLLTOOL=x86_64-w64-mingw32-dlltool \
      -DCMAKE_BUILD_TYPE=Debug \
      -DBUILD_SHARED_LIBS=NO \
      -DUSE_SSH=0 \
      -DLIBSSH2_FOUND=1 \
      -DLIBSSH2_INCLUDE_DIRS=${ROOT}/${LIBSSH2_DIR}/include/ \
      -DLIBSSH2_LIBRARY_DIRS=${ROOT}/${LIBSSH2_DIR} \
      -DLIBSSH2_LIBRARIES=${ROOT}/${LIBSSH2_DIR}/lib/libssh2.a \
      -DCMAKE_INSTALL_PREFIX=${ROOT}/${BUILD_FOLDER}/ \
      -DCMAKE_C_FLAGS_DEBUG="-ggdb -Og --save-temps" \
      -DCMAKE_SYSTEM_NAME=Windows \
      -DCMAKE_FIND_ROOT_PATH_MODE_PROGRAM=NEVER \
      -DCMAKE_FIND_ROOT_PATH_MODE_LIBRARY=ONLY \
      -DCMAKE_FIND_ROOT_PATH_MODE_INCLUDE=ONLY \
      -DCMAKE_C_COMPILER=x86_64-w64-mingw32-gcc \
      -DCMAKE_CXX_COMPILER=x86_64-w64-mingw32-g++ \
      -DCMAKE_RC_COMPILER=x86_64-w64-mingw32-windres \
      -DLIBGIT2_TESTS_EXTRA_LIBS="${OPENSSL_LIB};${CRYPTO_LIB};-lwsock32;-lcrypt32;-lws2_32" \
      -DENABLE_TRACE=ON \
      -DCMAKE_POSITION_INDEPENDENT_CODE=ON 

cmake -LAH .
cmake --build . -- VERBOSE=1
cmake --build . --target install

print_done




