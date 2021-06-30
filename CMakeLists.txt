cmake_minimum_required(VERSION 3.10)

# set the project name
project(gitkebab C)

include_directories(external/libgit2/include)
link_directories(external/libgit2/lib)

include_directories(external/cmocka/include)
link_directories(external/cmocka/lib)

add_executable(testmain
  src/main.c
  src/gitkebab.c
  src/gk_credentials.c
  src/gk_log_components.c
  src/gk_results.c
  src/gk_session.c
  src/rxi_log.c)

#target_link_libraries(testmain git2 ssl crypto ssh2 pthread z pcre)
target_link_libraries(testmain git2 ssl crypto /home/amos/Src/libssh2-1.9.0/build-linux/src/libssh2.a pthread z pcre)

#add_executable(testssh src/ssh2main.c)
#target_link_libraries(testssh /home/amos/Src/libssh2-1.9.0/build-linux/src/libssh2.a ssl crypto)
#target_include_directories(testssh PUBLIC /home/amos/Src/libssh2-1.9.0/include /home/amos/Src/libssh2-1.9.0/build-linux/src)


add_executable(tests
  src/tests.c)

target_link_libraries(tests cmocka)
