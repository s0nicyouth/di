cmake_minimum_required (VERSION 3.7.2)
project (Simple_DI_Tests)

set(CMAKE_VERBOSE_MAKEFILE TRUE)
set(THREADS_PREFER_PTHREAD_FLAG ON)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_compile_options(-std=c++14)

find_package(Threads REQUIRED)

include_directories(
		../src
		../third_party/gtest/googletest/include
)

link_directories(
		../third_party/gtest/libs/
)

set(SOURCES
		main.cc
		di_tests.cpp
		../src/di_debug/debug.cc)

add_executable(run_tests ${SOURCES})

target_link_libraries(run_tests
		Threads::Threads
		libgtest.a
		libgtest_main.a)
