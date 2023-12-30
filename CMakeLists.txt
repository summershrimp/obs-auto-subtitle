cmake_minimum_required(VERSION 3.16...3.26)

include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/common/bootstrap.cmake" NO_POLICY_SCOPE)

project(${_name} VERSION ${_version})

set(ENABLE_FRONTEND_API "Use obs-frontend-api for UI functionality" OFF)
set(ENABLE_QT "Use Qt functionality" ON)

include(compilerconfig)
include(defaults)
include(helpers)

add_library(${CMAKE_PROJECT_NAME} MODULE)

find_package(libobs REQUIRED)
target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE OBS::libobs)

if(ENABLE_FRONTEND_API)
  find_package(obs-frontend-api REQUIRED)
  target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE OBS::obs-frontend-api)
endif()

if(ENABLE_QT)
  find_qt(COMPONENTS Widgets Core Network)
  target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE Qt::Core Qt::Widgets Qt::Network)
  target_compile_options(
    ${CMAKE_PROJECT_NAME} PRIVATE $<$<C_COMPILER_ID:Clang,AppleClang>:-Wno-quoted-include-in-framework-header
                                  -Wno-comma>)
  set_target_properties(
    ${CMAKE_PROJECT_NAME}
    PROPERTIES AUTOMOC ON
               AUTOUIC ON
               AUTORCC ON)
endif()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake/Modules")

# Find WebSocket++
find_package(websocketpp REQUIRED)

# Find Asio
find_package(Asio REQUIRED)

# Find OpenSSL
set(OPENSSL_USE_STATIC_LIBS TRUE)
find_package(OpenSSL REQUIRED)

include(FetchContent)
FetchContent_Declare(
  cpr
  URL https://github.com/summershrimp/cpr/archive/refs/tags/1.10.5-1.tar.gz
  URL_HASH SHA256=8264839949f3724f7766a31bb4473692ac6b5e334b989bd520dac2cb0bd4bcbb
  EXCLUDE_FROM_ALL)
if(NOT cpr_POPULATED)
  FetchContent_Populate(cpr)
  set(CPR_USE_SYSTEM_CURL ON)
  set(BUILD_SHARED_LIBS OFF)
  add_subdirectory(${cpr_SOURCE_DIR} ${cpr_BINARY_DIR} EXCLUDE_FROM_ALL)
  if(UNIX)
    target_compile_options(cpr PRIVATE -fPIC)
  endif()
endif()

target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE OpenSSL::Crypto OpenSSL::SSL websocketpp::websocketpp cpr::cpr)

target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE ASIO_STANDALONE)
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE "${CMAKE_CURRENT_LIST_DIR}/src/util/QWebsocketpp")

set(obs-auto-subtitle_SOURCES
    src/obs-auto-subtitle.cpp
    src/obs-autosub-filter.cpp
    src/util/QWebsocketpp/QWebsocketpp.cpp
    src/builder/ASR/AliNLSBuilder.cpp
    src/builder/ASR/HwCloudRASRBuilder.cpp
    src/builder/ASR/XFRtASRBuilder.cpp
    src/builder/Trans/XFTransBuilder.cpp
    src/builder/Trans/GSTransBuilder.cpp
    src/builder/ASR/FunASRBuilder.cpp
    src/vendor/ASR/ASRBase.cpp
    src/vendor/ASR/XFRtASR.cpp
    src/vendor/ASR/HwCloudRASR.cpp
    src/vendor/ASR/AliNLS.cpp
    src/vendor/ASR/FunASR.cpp
    src/vendor/Trans/TransBase.cpp
    src/vendor/Trans/XFTrans.cpp
    src/vendor/Trans/GScriptTrans.cpp)

if(WIN32)
  set(obs-auto-subtitle_SOURCES
      ${obs-auto-subtitle_SOURCES} src/builder/ASR/MSSAPIBuilder.cpp
      src/vendor/ASR/mssapi-captions/captions-handler.cpp src/vendor/ASR/mssapi-captions/captions-mssapi.cpp
      src/vendor/ASR/mssapi-captions/captions-mssapi-stream.cpp src/vendor/ASR/mssapi.cpp)
endif()

target_sources(${CMAKE_PROJECT_NAME} PRIVATE ${obs-auto-subtitle_SOURCES})

set_target_properties_plugin(${CMAKE_PROJECT_NAME} PROPERTIES OUTPUT_NAME ${_name})
