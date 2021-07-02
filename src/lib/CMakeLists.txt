

include_directories(${PROJECT_SOURCE_DIR}/external/libgit2/include)
link_directories(${PROJECT_SOURCE_DIR}/external/libgit2/lib)

add_library(gitkebab
  gitkebab.c
  gitkebab.h
  gk_credentials.c
  gk_credentials.h
  gk_index.c
  gk_index.h
  gk_log_components.c
  gk_log_components.h
  gk_logging.h
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

target_include_directories(gitkebab INTERFACE ./) # allow dependencies to find gitkebab's headers
  
#target_link_libraries(testmain git2 ssl crypto ssh2 pthread z pcre)
target_link_libraries(gitkebab git2 ssl crypto /home/amos/Src/libssh2-1.9.0/build-linux/src/libssh2.a pthread z pcre)



