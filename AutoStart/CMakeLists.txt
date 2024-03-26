cmake_minimum_required(VERSION 3.18)

project(BGMRPCApps-AutoStart)

file(GLOB CUSTOM_SOURCES app.json)
add_custom_target(BGMRPCApps-AutoStart SOURCES ${CUSTOM_SOURCES})

install(FILES ${CUSTOM_SOURCES} DESTINATION ${BGMRPC_REMOTE}/apps/AutoStart)
