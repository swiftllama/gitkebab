

add_executable(maintest main.c)

target_link_libraries(maintest gitkebab_static ${LIBGIT2_lib} ${OPENSSL_lib} ${CRYPTO_lib} ${LIBSSH2_lib} ${Z_lib} ${PCRE_lib} pthread dl)
