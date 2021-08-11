
enable_testing()

include_directories(${LIBGIT2_include_folder})
include_directories(${CMOCKA_include_folder})

add_executable(tests general_tests.c)
target_link_libraries(tests ${CMOCKA_lib})

function(addTest NAME)
  add_executable(test_${NAME} test_${NAME}.c gk_test_filesystem_utils.c gk_test_env_utils.c)
  target_link_libraries(test_${NAME} ${CMOCKA_lib} gitkebab_static ${LIBGIT2_lib} ${OPENSSL_lib} ${CRYPTO_lib} ${LIBSSH2_lib} ${Z_lib} ${PCRE_lib} pthread dl)
  add_test(NAME Test_${NAME}
    COMMAND test_${NAME})
  install(TARGETS test_${NAME} DESTINATION test)
endfunction()


# Install fixtures
install(DIRECTORY fixtures DESTINATION test)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/CTestTestfile.cmake DESTINATION test)

# Add tests
addTest("session_progress")
addTest("clone")
addTest("no_init")
addTest("status")
addTest("index")
addTest("commit")
addTest("remotes")
addTest("merge")
addTest("conflicts")
addTest("execution_context")
