cmake_minimum_required(VERSION 2.6.0)
project(furianDPI CXX)
#set(CMAKE_BUILD_TYPE Release)
#set(CMAKE_CXX_FLAGS "-std=c++1z -O3 -Wall")
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_FLAGS "-std=c++1z -O0 -g -Wall -DSPDLOG_DEBUG_ON")

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

include(ExternalProject)

##################################
# Download and install GoogleTest

ExternalProject_Add(gtest
  URL https://github.com/google/googletest/archive/release-1.8.0.zip
  PREFIX ${CMAKE_CURRENT_BINARY_DIR}/gtest
  INSTALL_COMMAND ""
)
ExternalProject_Get_Property(gtest source_dir binary_dir)

################
# Define a test
file(GLOB SOURCES "test/*.cc")
add_executable(reconduit_test ${SOURCES})
include_directories(include)

######################################
# Configure the test to use GoogleTest
#
# If used often, could be made a macro.

add_dependencies(reconduit_test gtest spdlog sol2)
include_directories(${source_dir}/googletest/include)
target_link_libraries(reconduit_test ${binary_dir}/googlemock/gtest/libgtest.a)
target_link_libraries(reconduit_test ${binary_dir}/googlemock/gtest/libgtest_main.a)
target_link_libraries(reconduit_test pthread lua5.3)

##################################
# Just make the test runnable with
#   $ make test

enable_testing()
add_test(NAME    reconduit_test
         COMMAND reconduit_test)

##################################
# Download and install Spdlog

ExternalProject_Add(spdlog
  URL https://github.com/gabime/spdlog/archive/v0.16.3.tar.gz 

  PREFIX ${CMAKE_CURRENT_BINARY_DIR}/spdlog
  INSTALL_COMMAND ""
)
ExternalProject_Get_Property(spdlog source_dir binary_dir)

include_directories(${source_dir}/include)

##################################
# Download and install Sol2
# (Sol is a C++ library binding to Lua)

ExternalProject_Add(sol2
  URL https://github.com/ThePhD/sol2/archive/v2.20.0.tar.gz

  PREFIX ${CMAKE_CURRENT_BINARY_DIR}/sol2
  INSTALL_COMMAND ""
)
ExternalProject_Get_Property(sol2 source_dir binary_dir)

include_directories(${source_dir}/)
include_directories(/usr/include/lua5.3/)
