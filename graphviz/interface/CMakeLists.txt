cmake_minimum_required(VERSION 3.14)

project(
  Graphviz-INTERFACE
  LANGUAGES CXX
  VERSION ${BGMRPC_VERSION})

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Network)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Network)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

include_directories(${BGMRPCCommon_includeDir}
                    ${BGMRPCObjectInterface_includeDir})
link_directories("${BGMRPC_REMOTE}/lib")

add_library(Graphviz-INTERFACE SHARED Graphviz_global.h graphviz.cpp graphviz.h)
set_target_properties(
  Graphviz-INTERFACE
  PROPERTIES SOVERSION ${PROJECT_VERSION_MAJOR}
             VERSION ${PROJECT_VERSION}
             OUTPUT_NAME Graphviz
             INSTALL_RPATH_USE_LINK_PATH ON
             INSTALL_RPATH "${BGMRPC_REMOTE}/${CMAKE_INSTALL_LIBDIR}")

target_link_libraries(
  Graphviz-INTERFACE
  PRIVATE Qt${QT_VERSION_MAJOR}::Core Qt${QT_VERSION_MAJOR}::Network
          BGMRPCObjectInterface2 BGMRPCCommon2)

install(TARGETS Graphviz-INTERFACE
        LIBRARY DESTINATION ${BGMRPC_REMOTE}/apps/graphviz
                COMPONENT BGMRPC_remote_interface_Graphviz)

target_compile_definitions(Graphviz-INTERFACE PRIVATE GRAPHVIZ_LIBRARY)
