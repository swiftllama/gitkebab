enable_language(C)

include_directories(${LIBGIT2_include_folder})

add_custom_target(libraries)

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
  rxi_log.c
  rxi_log.h)

set_property(TARGET gitkebab_objects PROPERTY POSITION_INDEPENDENT_CODE 1)

add_library(gitkebab SHARED $<TARGET_OBJECTS:gitkebab_objects>)
add_library(gitkebab_static STATIC $<TARGET_OBJECTS:gitkebab_objects>)

target_include_directories(gitkebab_static INTERFACE ./) # allow dependencies to find gitkebab's headers
target_include_directories(gitkebab INTERFACE ./) # allow dependencies to find gitkebab's headers
  
target_link_libraries(gitkebab ${LIBGIT2_lib} ${OPENSSL_lib} ${CRYPTO_lib} ${LIBSSH2_lib} ${Z_lib} ${PCRE_lib})

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
