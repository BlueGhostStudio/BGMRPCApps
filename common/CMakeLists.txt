cmake_minimum_required(VERSION 3.18)

project(BGMRPCApps-common)

file(GLOB CUSTOM_SOURCES randomStr.js sqlStmFra.js)
add_custom_target(BGMRPCApps-common SOURCES ${CUSTOM_SOURCES})

install(FILES ${CUSTOM_SOURCES} DESTINATION ${BGMRPC_REMOTE}/apps/common)
