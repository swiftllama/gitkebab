add_executable(example1 main.c)

set(LINK_LIBS ${LIBGIT2_lib} ${Z_lib} ${PCRE_lib} ${LIBSSH2_lib} ${OPENSSL_lib} ${CRYPTO_lib})
if (NOT ANDROID)
  list(APPEND LINK_LIBS pthread)
endif()
if ("${CMAKE_SYSTEM_NAME}" MATCHES "Windows")
  list(APPEND LINK_LIBS -luuid -lwinhttp -lwsock32 -lcrypt32 -lws2_32 -lrpcrt4)
else()
  list(APPEND LINK_LIBS dl)
endif()
target_link_libraries(example1 gitkebab_static ${LINK_LIBS})

if (APPLE)
   target_link_libraries(example1 "-framework Security -framework CoreFoundation ${Iconv_LIBRARIES}")
endif()

install(TARGETS example1 DESTINATION bin)
