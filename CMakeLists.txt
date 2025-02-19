cmake_minimum_required(VERSION 3.12)
project(osmium-html)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_library(osmium-html src/tokenizer.cc src/parser.cc)
target_compile_options(osmium-html PRIVATE -Wall -Wextra -Wpedantic -Wconversion)

add_executable(parser src/main.cc)
target_link_libraries(parser PRIVATE osmium-html)
target_compile_options(parser PRIVATE -Wall -Wextra -Wpedantic -Wconversion)