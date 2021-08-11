cmake_minimum_required(VERSION 3.10)

# set the project name
project(gitkebab C)

find_library(Z_lib "libz.a" REQUIRED)
find_library(PCRE_lib "libpcre.a" REQUIRED)
find_library(OPENSSL_lib "libssl.a" REQUIRED)
find_library(CRYPTO_lib "libcrypto.a" REQUIRED)
find_library(LIBSSH2_lib "libssh2.a" REQUIRED)
find_library(LIBGIT2_lib "libgit2.a" REQUIRED)
find_library(CMOCKA_lib "libcmocka-static.a" REQUIRED)

find_path(LIBGIT2_include_folder NAMES "git2.h" REQUIRED)
find_path(CMOCKA_include_folder NAMES "cmocka.h" REQUIRED)

# Turn on warnings
add_compile_options(-Wall -Wextra -pedantic -Werror -Wno-missing-field-initializers) 
add_subdirectory(src/lib)
add_subdirectory(src/example)
add_subdirectory(src/test)
