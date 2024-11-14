enable_language(C)

include_directories(${LIBGIT2_include_folder})
include_directories(${OPENSSL_include_folder})

add_custom_target(libraries)

set(ICONV_COMPAT_SOURCE "")
if (APPLE)
  if ("${CMAKE_SYSTEM_NAME}" MATCHES "iOS")
  else()
    set(ICONV_COMPAT_SOURCE "iconv_compat.c")
  endif()
 endif()

add_library(gitkebab_objects OBJECT
  gitkebab.h
  gk_commit.c
  gk_commit.h
  gk_conflicts.c
  gk_conflicts.h
  gk_credentials.c
  gk_credentials.h
  gk_execution_context.c
  gk_execution_context.h
  gk_filesystem.c
  gk_filesystem.h
  gk_index.c
  gk_index.h
  gk_init.c
  gk_init.h
  gk_keys.c
  gk_keys.h
  gk_lg2.c
  gk_log_components.c
  gk_log_components.h
  gk_logging.h
  gk_merge.c
  gk_merge.h
  gk_remotes.h
  gk_remotes.c
  gk_repository.c
  gk_repository.h
  gk_results.c
  gk_results.h
  gk_session.c
  gk_session.h
  gk_session_progress.c
  gk_session_progress.h
  gk_status.c
  gk_status.h
  gk_sync.c
  gk_sync.h
  rxi_log.c
  rxi_log.h
  ${ICONV_COMPAT_SOURCE})

set_property(TARGET gitkebab_objects PROPERTY POSITION_INDEPENDENT_CODE 1)

add_library(gitkebab SHARED $<TARGET_OBJECTS:gitkebab_objects>)
add_library(gitkebab_static STATIC $<TARGET_OBJECTS:gitkebab_objects>)

target_include_directories(gitkebab_static INTERFACE ./) # allow dependencies to find gitkebab's headers
target_include_directories(gitkebab INTERFACE ./) # allow dependencies to find gitkebab's headers


#target_link_libraries(gitkebab_static ${LIBGIT2_lib} ${OPENSSL_lib} ${CRYPTO_lib} ${LIBSSH2_lib} ${Z_lib} ${PCRE_lib})
#ENDIF()
target_link_libraries(gitkebab ${LIBGIT2_lib} ${LIBSSH2_lib} ${OPENSSL_lib} ${CRYPTO_lib} ${PCRE_lib} ${Z_lib} )

if ("${CMAKE_SYSTEM_NAME}" MATCHES "Windows")
  target_link_libraries(gitkebab -luuid -lwinhttp -lwsock32 -lcrypt32 -lws2_32 -lrpcrt4)
endif()

if (APPLE)
  target_link_libraries(gitkebab "-framework Security -framework CoreFoundation ${ICONV_lib}")
  set_target_properties(gitkebab PROPERTIES
    FRAMEWORK TRUE
    FRAMEWORK_VERSION A
    MACOSX_FRAMEWORK_IDENTIFIER net.swiftllama.gitkebab
    # "current version" in semantic format in Mach-O binary file
    VERSION 1.0.0
    # "compatibility version" in semantic format in Mach-O binary file
    SOVERSION 1.0.0
    PUBLIC_HEADER gitkebab.h
  )
endif()

#  LIBGIT2_lib OPENSSL_lib CRYPTO_lib LIBSSH2_lib Z_lib PCRE_lib <- need these??

add_dependencies(libraries gitkebab_static gitkebab)

install(TARGETS gitkebab gitkebab_static DESTINATION lib)
install(FILES
    gitkebab.h
    gk_commit.h
    gk_conflicts.h
    gk_credentials.h
    gk_filesystem.h
    gk_index.h
    gk_init.h
    gk_logging.h
    gk_merge.h
    gk_remotes.h
    gk_results.h
    gk_repository.h
    gk_status.h
    gk_session.h
    gk_execution_context.h
  DESTINATION include)
