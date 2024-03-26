cmake_minimum_required(VERSION 3.18)

if(NOT TARGET dep_cmake)
  include(FetchContent)
  FetchContent_Declare(
    dep_cmake
    GIT_REPOSITORY https://github.com/BlueGhostStudio/BGStudio-cmake.git
    GIT_TAG main)
  FetchContent_MakeAvailable(dep_cmake)
endif()

include(${dep_cmake_SOURCE_DIR}/VERSION.cmake)
include(${dep_cmake_SOURCE_DIR}/BGMRPC.cmake)

project(BGMRPCApps)

#include(${BGMRPCPRJ_PATH}/cmake/BGMRPC.cmake)
#include(${BGMRPCPRJ_PATH}/cmake/VERSION.cmake)

add_subdirectory(AutoStart)
add_subdirectory(common)
add_subdirectory(account)
add_subdirectory(cms)
add_subdirectory(todo)
add_subdirectory(graphviz)
