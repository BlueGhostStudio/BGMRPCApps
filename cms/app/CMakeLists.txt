cmake_minimum_required(VERSION 3.18)

project(CMS-APP)

file(
  GLOB
  CUSTOM_SOURCES
  main.js
  private.js
  mediaConfig.js
  media.js
  convert_dot.js
  web.js
  logo.png
  cms.db
  app.json)

add_custom_target(CMS-APP SOURCES ${CUSTOM_SOURCES})
install(FILES ${CUSTOM_SOURCES} DESTINATION ${BGMRPC_REMOTE}/apps/cms)
