cmake_minimum_required(VERSION 3.10)

# set the project name
project(gitkebab)

function(verifyLibraryIsFromBuildSystem LIBPATH LIBNAME)
  if ("${LIBPATH}" MATCHES "^/usr/")
    message(FATAL_ERROR " dependency [${LIBNAME}] found at system location [${LIBPATH}]. Expected a location relative to the current build-tree. Aborting")
  else ()
    message(INFO " Found ${LIBNAME} at [${LIBPATH}]")
  endif()
endfunction()

find_library(Z_lib "libz.a" REQUIRED)
verifyLibraryIsFromBuildSystem(${Z_lib} "lib.z")

find_library(PCRE_lib "libpcre.a" REQUIRED)
verifyLibraryIsFromBuildSystem(${PCRE_lib} "libpcre.a")

find_library(OPENSSL_lib "libssl.a" REQUIRED)
verifyLibraryIsFromBuildSystem(${OPENSSL_lib} "libssl.a")

find_library(CRYPTO_lib "libcrypto.a" REQUIRED)
verifyLibraryIsFromBuildSystem(${CRYPTO_lib} "libcrypto.a")

find_library(LIBSSH2_lib "libssh2.a" REQUIRED)
verifyLibraryIsFromBuildSystem(${LIBSSH2_lib} "libssh2.a")

find_library(LIBGIT2_lib "libgit2.a" REQUIRED)
verifyLibraryIsFromBuildSystem(${LIBGIT2_lib} "libgit2.a")

find_library(CMOCKA_lib "libcmocka-static.a" REQUIRED)
verifyLibraryIsFromBuildSystem(${CMOCKA_lib} "libcmocka.a")

if (APPLE)
  FIND_PACKAGE(Iconv) # note: we want the system libiconv for macos
  if (Iconv_FOUND)
    message(INFO " Found iconv at ${Iconv_LIBRARIES}")
  else()
    message(FATAL_ERROR "iconv not found")
  endif()
endif()

find_path(LIBGIT2_include_folder NAMES "git2.h" REQUIRED)
find_path(CMOCKA_include_folder NAMES "cmocka.h" REQUIRED)

# Turn on warnings
add_compile_options(-Wall -Wextra -pedantic -Werror -Wno-missing-field-initializers)
add_subdirectory(src/lib)
add_subdirectory(src/example)
add_subdirectory(src/test)
