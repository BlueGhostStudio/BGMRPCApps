cmake_minimum_required(VERSION 3.18)

project(Graphviz-APP)

file(GLOB CUSTOM_SOURCES app.json)

add_custom_target(Graphviz-APP SOURCES ${CUSTOM_SOURCES})
install(FILES ${CUSTOM_SOURCES} DESTINATION ${BGMRPC_REMOTE}/apps/graphviz)
