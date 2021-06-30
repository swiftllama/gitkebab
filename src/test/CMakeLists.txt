

include_directories(${PROJECT_SOURCE_DIR}/external/cmocka/include)
link_directories(${PROJECT_SOURCE_DIR}/external/cmocka/lib)
link_directories(${PROJECT_SOURCE_DIR}/external/libgit2/lib)

add_executable(tests general_tests.c)

target_link_libraries(tests cmocka)


add_executable(test_session_progress test_session_progress.c)

target_link_libraries(test_session_progress cmocka gitkebab git2 ssl crypto /home/amos/Src/libssh2-1.9.0/build-linux/src/libssh2.a pthread z pcre)
