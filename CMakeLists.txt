cmake_minimum_required(VERSION 3.10)

# set the project name
project(gitkebab C)

add_compile_options(-Wall -Wextra -pedantic -Werror -Wno-missing-field-initializers)
add_subdirectory(src/lib)
add_subdirectory(src/example)
add_subdirectory(src/test)
