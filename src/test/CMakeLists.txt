
enable_testing()

include_directories(${PROJECT_SOURCE_DIR}/external/cmocka/include)
include_directories(${PROJECT_SOURCE_DIR}/external/libgit2/include)
link_directories(${PROJECT_SOURCE_DIR}/external/cmocka/lib)
link_directories(${PROJECT_SOURCE_DIR}/external/libgit2/lib)

add_executable(tests general_tests.c)
target_link_libraries(tests cmocka)


add_executable(test_session_progress test_session_progress.c)
target_link_libraries(test_session_progress cmocka gitkebab git2 ssl crypto /home/amos/Src/libssh2-1.9.0/build-linux/src/libssh2.a pthread z pcre)
add_test(NAME TestSessionProgress
  COMMAND test_session_progress
  WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})

add_executable(test_clone test_clone.c gk_test_filesystem_utils.c)
target_link_libraries(test_clone cmocka gitkebab git2 ssl crypto /home/amos/Src/libssh2-1.9.0/build-linux/src/libssh2.a pthread z pcre)
add_test(NAME TestClone
  COMMAND test_clone
  WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})

add_executable(test_no_init test_no_init.c)
target_link_libraries(test_no_init cmocka gitkebab git2 ssl crypto /home/amos/Src/libssh2-1.9.0/build-linux/src/libssh2.a pthread z pcre)
add_test(NAME TestNoInit
  COMMAND test_no_init
  WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})

#add_executable(test_status test_status.c gk_test_filesystem_utils.c gk_test_env_utils.c)
#target_link_libraries(test_status cmocka gitkebab git2 ssl crypto /home/amos/Src/libssh2-1.9.0/build-linux/src/libssh2.a pthread z pcre)
#add_test(NAME TestStatus
#  COMMAND test_status
#  WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
#
#add_executable(test_index test_index.c gk_test_filesystem_utils.c gk_test_env_utils.c)
#target_link_libraries(test_index cmocka gitkebab git2 ssl crypto /home/amos/Src/libssh2-1.9.0/build-linux/src/libssh2.a pthread z pcre)
#add_test(NAME TestIndex
#  COMMAND test_index
#  WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
#
#add_executable(test_commit test_commit.c gk_test_filesystem_utils.c gk_test_env_utils.c)
#target_link_libraries(test_commit cmocka gitkebab git2 ssl crypto /home/amos/Src/libssh2-1.9.0/build-linux/src/libssh2.a pthread z pcre)
#add_test(NAME TestCommit
#  COMMAND test_commit
#  WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
#
#
#add_executable(test_remotes test_remotes.c gk_test_filesystem_utils.c gk_test_env_utils.c)
#target_link_libraries(test_remotes cmocka gitkebab git2 ssl crypto /home/amos/Src/libssh2-1.9.0/build-linux/src/libssh2.a pthread z pcre)
#add_test(NAME TestRemotes
#  COMMAND test_remotes
#  WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
#
#
#add_executable(test_merge test_merge.c gk_test_filesystem_utils.c gk_test_env_utils.c)
#target_link_libraries(test_merge cmocka gitkebab git2 ssl crypto /home/amos/Src/libssh2-1.9.0/build-linux/src/libssh2.a pthread z pcre)
#add_test(NAME TestMerge
#  COMMAND test_merge
#  WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
#
#add_executable(test_conflicts test_conflicts.c gk_test_filesystem_utils.c gk_test_env_utils.c)
#target_link_libraries(test_conflicts cmocka gitkebab git2 ssl crypto /home/amos/Src/libssh2-1.9.0/build-linux/src/libssh2.a pthread z pcre)
#add_test(NAME TestConflicts
#  COMMAND test_conflicts
#  WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
#
#
#add_executable(test_execution_context test_execution_context.c)
#target_link_libraries(test_execution_context cmocka gitkebab git2 ssl crypto /home/amos/Src/libssh2-1.9.0/build-linux/src/libssh2.a pthread z pcre)
#add_test(NAME TestExecutionContext
#  COMMAND test_execution_context
#  WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
#
