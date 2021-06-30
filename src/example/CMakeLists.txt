
link_directories(${PROJECT_SOURCE_DIR}/external/libgit2/lib)

add_executable(maintest main.c)

target_link_libraries(maintest gitkebab git2 ssl crypto /home/amos/Src/libssh2-1.9.0/build-linux/src/libssh2.a pthread z pcre)

target_link_libraries(maintest gitkebab git2 ssl crypto /home/amos/Src/libssh2-1.9.0/build-linux/src/libssh2.a pthread z pcre)
