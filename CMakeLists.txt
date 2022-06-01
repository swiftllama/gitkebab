cmake_minimum_required(VERSION 3.10)

# set the project name
project(gitkebab)

if(NOT DEFINED BUILD_TESTS)
  set(BUILD_TESTS ON)
endif()
if(NOT DEFINED BUILD_EXAMPLES)
  set(BUILD_EXAMPLES ON)
endif()
  
function(verifyLibraryIsFromBuildSystem LIBPATH LIBNAME)
  if ("${LIBPATH}" MATCHES "^/usr/")
    message(FATAL_ERROR " dependency [${LIBNAME}] found at system location [${LIBPATH}]. Expected a location relative to the current build-tree. Aborting")
  else ()
    message(INFO " Found ${LIBNAME} at [${LIBPATH}]")
  endif()
endfunction()

find_library(Z_lib "libz.a" REQUIRED)
verifyLibraryIsFromBuildSystem(${Z_lib} "lib.z")

IF ("${CMAKE_SYSTEM_NAME}" MATCHES "Windows")
  find_library(PCRE_lib "libpcred.a" REQUIRED)
  verifyLibraryIsFromBuildSystem(${PCRE_lib} "libpcred.a")
ELSE()
  find_library(PCRE_lib "libpcre.a" REQUIRED)
  verifyLibraryIsFromBuildSystem(${PCRE_lib} "libpcre.a")
ENDIF()

find_library(OPENSSL_lib "libssl.a" REQUIRED)
verifyLibraryIsFromBuildSystem(${OPENSSL_lib} "libssl.a")

find_library(CRYPTO_lib "libcrypto.a" REQUIRED)
verifyLibraryIsFromBuildSystem(${CRYPTO_lib} "libcrypto.a")

find_library(LIBSSH2_lib "libssh2.a" REQUIRED)
verifyLibraryIsFromBuildSystem(${LIBSSH2_lib} "libssh2.a")

find_library(LIBGIT2_lib "libgit2.a" REQUIRED)
verifyLibraryIsFromBuildSystem(${LIBGIT2_lib} "libgit2.a")

if (BUILD_TESTS)
  find_library(CMOCKA_lib "libcmocka-static.a" REQUIRED)
  verifyLibraryIsFromBuildSystem(${CMOCKA_lib} "libcmocka.a")
endif()

if (APPLE)
  if ("${CMAKE_SYSTEM_NAME}" MATCHES "iOS")
    set(ICONV_lib "-liconv")
  else()
    find_library(ICONV_lib "libiconv.a" REQUIRED)
    verifyLibraryIsFromBuildSystem(${ICONV_lib} "libiconv.a")
  endif()
endif()

find_path(OPENSSL_include_folder NAMES "openssl/opensslconf.h" REQUIRED)
find_path(LIBGIT2_include_folder NAMES "git2.h" REQUIRED)
if (BUILD_TESTS)
  find_path(CMOCKA_include_folder NAMES "cmocka.h" REQUIRED)
endif()

# Turn on warnings
add_compile_options(-Wall -Wextra -pedantic -Werror -Wno-missing-field-initializers)
add_subdirectory(src/lib)
if (BUILD_EXAMPLES)
  add_subdirectory(src/example)
endif()
if (BUILD_TESTS)
  add_subdirectory(src/test)
endif()


unset(BUILD_TESTS CACHE)
unset(BUILD_EXAMPLES CACHE)
