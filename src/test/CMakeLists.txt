enable_testing()

include_directories(${LIBGIT2_include_folder})
include_directories(${CMOCKA_include_folder})

add_executable(tests general_tests.c)
target_link_libraries(tests ${CMOCKA_lib})

add_custom_target(all_tests)
  
function(addTest NAME)
  add_executable(test_${NAME} test_${NAME}.c gk_test_filesystem_utils.c gk_test_env_utils.c)
  set(LINK_LIBS ${CMOCKA_lib} gitkebab_static ${LIBGIT2_lib} ${Z_lib} ${PCRE_lib} ${LIBSSH2_lib} ${OPENSSL_lib} ${CRYPTO_lib})
  
  if ("${CMAKE_SYSTEM_NAME}" MATCHES "Windows")
    list(APPEND LINK_LIBS -luuid -lwinhttp -lwsock32 -lcrypt32 -lws2_32 -lrpcrt4)
  else()
    list(APPEND LINK_LIBS -ldl)
  endif()
  
  if (NOT ANDROID)
    list(APPEND LINK_LIBS pthread)
  endif()
  
  target_link_libraries(test_${NAME} ${LINK_LIBS})
  
  if (APPLE)
   target_link_libraries(test_${NAME} "-framework Security -framework CoreFoundation ${ICONV_lib}")
 endif()
 
  add_test(NAME Test_${NAME}
    COMMAND test_${NAME})
  add_dependencies(all_tests test_${NAME})
  install(TARGETS test_${NAME} DESTINATION test)
  file(APPEND "${CMAKE_CURRENT_BINARY_DIR}/run-all-tests.sh" "./test_${NAME}\n\n\n")
endfunction()



# Add tests
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/run-all-tests.sh" "echo '=== Running GitKebab C Test Suite ==='\n")
file(APPEND "${CMAKE_CURRENT_BINARY_DIR}/run-all-tests.sh" "set -e\n")
file(APPEND "${CMAKE_CURRENT_BINARY_DIR}/run-all-tests.sh" "set -x\n\n")
addTest("clone")
addTest("commit")
addTest("conflicts")
addTest("create_local")
addTest("execution_context")
addTest("index")
addTest("merge")
addTest("no_init")
addTest("remotes")
addTest("session_progress")
addTest("status")
addTest("sync")
file(APPEND "${CMAKE_CURRENT_BINARY_DIR}/run-all-tests.sh" "echo '=== Finished GitKebab Test Suite ==='\n")

# Install fixtures
install(DIRECTORY fixtures DESTINATION test)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/run-all-tests.sh DESTINATION test PERMISSIONS OWNER_EXECUTE OWNER_READ OWNER_WRITE)
