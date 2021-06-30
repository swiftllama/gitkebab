

include_directories(${PROJECT_SOURCE_DIR}/external/cmocka/include)
link_directories(${PROJECT_SOURCE_DIR}/external/cmocka/lib)


add_executable(tests
  general_tests.c)

target_link_libraries(tests cmocka)
