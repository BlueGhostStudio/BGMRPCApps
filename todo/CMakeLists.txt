cmake_minimum_required(VERSION 3.18)

project(BGMRPCApps-Todo)

file(GLOB CUSTOM_SOURCES main.js app.json)

add_custom_target(BGMRPCApps-Todo SOURCES ${CUSTOM_SOURCES})

install(FILES ${CUSTOM_SOURCES} DESTINATION ${BGMRPC_REMOTE}/apps/todo)
