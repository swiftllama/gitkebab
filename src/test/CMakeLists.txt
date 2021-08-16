enable_testing()

include_directories(${LIBGIT2_include_folder})
include_directories(${CMOCKA_include_folder})

add_executable(tests general_tests.c)
target_link_libraries(tests ${CMOCKA_lib})

add_custom_target(all_tests)
  
function(addTest NAME)
  add_executable(test_${NAME} test_${NAME}.c gk_test_filesystem_utils.c gk_test_env_utils.c)
  target_link_libraries(test_${NAME} ${CMOCKA_lib} gitkebab_static ${LIBGIT2_lib} ${OPENSSL_lib} ${CRYPTO_lib} ${LIBSSH2_lib} ${Z_lib} ${PCRE_lib} pthread dl)
  add_test(NAME Test_${NAME}
    COMMAND test_${NAME})
  add_dependencies(all_tests test_${NAME})
  install(TARGETS test_${NAME} DESTINATION test)
  file(APPEND "${CMAKE_CURRENT_BINARY_DIR}/run-all-tests.sh" "./test_${NAME}\n\n\n")
endfunction()



# Add tests
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/run-all-tests.sh" "echo '=== Running GitKebab C Test Suite ==='\n")
file(APPEND "${CMAKE_CURRENT_BINARY_DIR}/run-all-tests.sh" "set -e\n")
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/run-all-tests.sh" "set -x\n\n")
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
file(APPEND "${CMAKE_CURRENT_BINARY_DIR}/run-all-tests.sh" "echo '=== Finished GitKebab Test Suite ==='\n")

# Install fixtures
install(DIRECTORY fixtures DESTINATION test)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/run-all-tests.sh DESTINATION test PERMISSIONS OWNER_EXECUTE OWNER_READ OWNER_WRITE)
