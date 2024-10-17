cmake_minimum_required(VERSION 3.8)
project(bfllvm)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED 20)

set(SOURCES "src/bfcc.cpp" "src/lexer.cpp" "src/compiler.cpp" "src/fs.cpp" "src/args.cpp")
add_executable("bfcc" ${SOURCES})
target_include_directories("bfcc" PUBLIC "include")
target_link_libraries("bfcc" LLVM-18)
