cmake_minimum_required(VERSION 3.18)

project(BGMRPCApps-Account)

file(GLOB CUSTOM_SOURCES main.js app.json)
add_custom_target(BGMRPCApps-Account SOURCES ${CUSTOM_SOURCES})

install(FILES ${CUSTOM_SOURCES} DESTINATION ${BGMRPC_REMOTE}/apps/account)
