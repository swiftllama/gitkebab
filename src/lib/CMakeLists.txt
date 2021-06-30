

include_directories(${PROJECT_SOURCE_DIR}/external/libgit2/include)
link_directories(${PROJECT_SOURCE_DIR}/external/libgit2/lib)

add_library(gitkebab
  gitkebab.c
  gk_credentials.c
  gk_log_components.c
  gk_results.c
  gk_session.c
  rxi_log.c)

#target_link_libraries(testmain git2 ssl crypto ssh2 pthread z pcre)
target_link_libraries(gitkebab git2 ssl crypto /home/amos/Src/libssh2-1.9.0/build-linux/src/libssh2.a pthread z pcre)

