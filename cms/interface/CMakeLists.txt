cmake_minimum_required(VERSION 3.18)

project(
  CMS-INTERFACE
  LANGUAGES CXX
  VERSION ${BGMRPC_VERSION})

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Network Sql)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Network Sql)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

include_directories(${BGMRPCCommon_includeDir}
                    ${BGMRPCObjectInterface_includeDir})
link_directories("${BGMRPC_REMOTE}/lib")

add_library(CMS-INTERFACE SHARED BGCMS_global.h bgcms.h bgcms.cpp)
set_target_properties(
  CMS-INTERFACE
  PROPERTIES SOVERSION ${PROJECT_VERSION_MAJOR}
             VERSION ${PROJECT_VERSION}
             OUTPUT_NAME BGCMS
             INSTALL_RPATH_USE_LINK_PATH ON
             INSTALL_RPATH "${BGMRPC_REMOTE}/${CMAKE_INSTALL_LIBDIR}")
target_link_libraries(
  CMS-INTERFACE
  PRIVATE Qt${QT_VERSION_MAJOR}::Core Qt${QT_VERSION_MAJOR}::Network
          Qt${QT_VERSION_MAJOR}::Sql BGMRPCObjectInterface2 BGMRPCCommon2)

install(TARGETS CMS-INTERFACE LIBRARY DESTINATION ${BGMRPC_REMOTE}/apps/cms)

target_compile_definitions(CMS-INTERFACE PRIVATE BGCMS_LIBRARY)
