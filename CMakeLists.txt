cmake_minimum_required(VERSION 3.28)
project(babel_engine)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} -lgmp -lgmpxx")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fstandalone-debug")

include_directories(include lib)

add_library(babel_engine STATIC src/babel_engine.cpp)

include_directories(/usr/include)
find_package(Catch2 3 REQUIRED)

add_executable(tests test/test_lib.cpp)
target_link_libraries(tests PRIVATE ${CMAKE_BINARY_DIR}/libbabel_engine.a Catch2::Catch2WithMain)
