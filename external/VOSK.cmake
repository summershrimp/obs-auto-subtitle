include(FetchContent)
set(FETCHCONTENT_QUIET FALSE)
set(VOSK_VERSION 0.3.32)

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
	set(VOSK_ARCH 64)
else()
	set(VOSK_ARCH 32)
endif()


if(WIN32)
  if(VOSK_ARCH EQUAL 64)
    set(VOSK_API_PACKAGE "vosk-win64-0.3.32.zip")
  else()
    set(VOSK_API_PACKAGE "vosk-win32-0.3.32.zip")
  endif()
elseif(UNIX)
  if(NOT APPLE AND VOSK_ARCH EQUAL 64)
    set(VOSK_API_PACKAGE "vosk-linux-x86_64-0.3.32.zip")
  endif()
endif()

set(VOSK_API_H_URL "https://github.com/alphacep/vosk-api/raw/v${VOSK_VERSION}/src/vosk_api.h")

# message(STATUS "Downloading vosk header file")
# file(DOWNLOAD ${VOSK_API_H_URL}
#     ${CMAKE_BINARY_DIR}/vosk-api/vosk_api.h
#     SHOW_PROGRESS)
message(STATUS "Downloading vosk package")
FetchContent_Declare(vosk-package 
    URL "https://github.com/alphacep/vosk-api/releases/download/v${VOSK_VERSION}/${VOSK_API_PACKAGE}"
    URL_HASH SHA256=3e7bcda6ca491f1bc1121948d91f252a7b1c70a1385e58fc9471aef548129ee3)
FetchContent_MakeAvailable(vosk-package)

message(STATUS "Downloading vosk header")
FetchContent_Declare(vosk-header
    URL "${VOSK_API_H_URL}"
    URL_HASH SHA256=a86da0980bd6100d2f6296585cf7dabcfb33bf0e0b1bf587ec915a58da85bc3b
    DOWNLOAD_DIR ${CMAKE_BINARY_DIR}/vosk-api
    DOWNLOAD_NO_EXTRACT true)
FetchContent_MakeAvailable(vosk-header)

message(STATUS ${CMAKE_BINARY_DIR}/vosk-api)
message(STATUS ${vosk-package_SOURCE_DIR})
# add_library(vosk INTERFACE)
# target_include_directories(vosk INTERFACE ${CMAKE_BINARY_DIR}/vosk-api)
# target_link_libraries(vosk INTERFACE ${vosk-package_SOURCE_DIR}/libvosk.dll)