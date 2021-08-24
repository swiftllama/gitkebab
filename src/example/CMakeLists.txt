add_executable(maintest main.c)

set(LINK_LIBS ${LIBGIT2_lib} ${OPENSSL_lib} ${CRYPTO_lib} ${LIBSSH2_lib} ${Z_lib} ${PCRE_lib} dl)
if (NOT ANDROID)
  list(APPEND LINK_LIBS pthread)
endif()
target_link_libraries(maintest gitkebab_static ${LINK_LIBS})
