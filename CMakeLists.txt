cmake_minimum_required(VERSION 3.5)
project(obs-auto-subtitle VERSION 0.3.0)

add_compile_definitions(OBS_SSP_VERSION="${CMAKE_PROJECT_VERSION}")

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

add_definitions(-DASIO_STANDALONE)

# if (WIN32 OR APPLE)
# 	include(external/FindLibObs.cmake)
# endif()


find_package(LibObs REQUIRED)
find_package(obs-frontend-api REQUIRED)
find_package(Qt6 COMPONENTS Core Widgets Network WebSockets)
find_package(Qt6 COMPONENTS Core Widgets Network WebSockets)

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
	set(ARCH 64)
else()
	set(ARCH 32)
endif()

set(obs-auto-subtitle_SOURCES
		src/obs-auto-subtitle.cpp
        src/obs-autosub-filter.cpp
		src/builder/XFTransBuilder.cpp
		src/builder/GSTransBuilder.cpp
		src/vendor/ASR/ASRBase.cpp
		src/vendor/ASR/XFRtASR.cpp
		src/vendor/ASR/HwCloudRASR.cpp
		src/vendor/ASR/AliNLS.cpp
		src/vendor/Trans/TransBase.cpp
		src/vendor/Trans/XFTrans.cpp
		src/vendor/Trans/GScriptTrans.cpp)

set(obs-auto-subtitle_HEADERS
		src/obs-auto-subtitle.h
		src/obs-autosub-filter.h
		src/builder/PropBuilderBase.h
		src/builder/TransBuilderBase.h
		src/builder/XFTransBuilder.h
		src/builder/GSTransBuilder.h
		src/vendor/ASR/ASRBase.h
		src/vendor/ASR/XFRtASR.h
		src/vendor/ASR/HwCloudRASR.h
		src/vendor/ASR/AliNLS.h
		src/vendor/Trans/TransBase.h
		src/vendor/Trans/XFTrans.h
		src/vendor/Trans/GScriptTrans.h)

add_library(obs-auto-subtitle MODULE
	${obs-auto-subtitle_SOURCES}
	${obs-auto-subtitle_HEADERS})
# include_directories(
# 		"${LIBOBS_INCLUDE_DIR}/../UI/obs-frontend-api"
# 		${Qt5Core_INCLUDES}
# 		${Qt5Widgets_INCLUDES}
# 		${Qt5Network_INCLUDES}
# 		${Qt5WebSockets_INCLUDES})

target_link_libraries(obs-auto-subtitle
		OBS::libobs
		OBS::obs-frontend-api
		OBS::w32-pthreads
		Qt6::Core
		Qt6::Widgets
		Qt6::Network
		Qt6::WebSockets)

# Windows
if(WIN32)
	string(REGEX REPLACE "/RTC(su|[1su])" "" CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
	message("CXX release flags: ${CMAKE_CXX_FLAGS_RELEASE}")
	
	add_definitions(-D_WEBSOCKETPP_CPP11_STL_)

	find_package(w32-pthreads REQUIRED)
	target_link_libraries(obs-auto-subtitle OBS::w32-pthreads)

	if(ARCH EQUAL 64)
		set(OBS_ARCH_NAME "64bit")
		set(OBS_BUILDDIR_ARCH "build64")
	else()
		set(OBS_ARCH_NAME "32bit")
		set(OBS_BUILDDIR_ARCH "build32")
		add_definitions(/arch:SSE2)
	endif()

	install(TARGETS obs-auto-subtitle LIBRARY DESTINATION "obs-plugins/${OBS_ARCH_NAME}")
	get_target_property(qwebsocket Qt6::WebSockets LOCATION_Release)
	install(FILES ${qwebsocket} DESTINATION "bin/${OBS_ARCH_NAME}")
	install(FILES "${QT6_INSTALL_PREFIX}/plugins/tls/qschannelbackend.dll" DESTINATION "bin/${OBS_ARCH_NAME}/tls")
	install(FILES "${QT6_INSTALL_PREFIX}/plugins/tls/qcertonlybackend.dll" DESTINATION "bin/${OBS_ARCH_NAME}/tls")
	install(DIRECTORY ${CMAKE_SOURCE_DIR}/data/ DESTINATION "data/obs-plugins/obs-auto-subtitle")
endif()

# OSX
if(APPLE)
	set_target_properties(obs-auto-subtitle PROPERTIES PREFIX "")
	target_link_libraries(obs-auto-subtitle
		"${OBS_FRONTEND_LIB}")
endif()

# Linux
if(UNIX AND NOT APPLE)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -mtune=core2 -Ofast")

	if(ARCH EQUAL 64)
		set(ARCH_NAME "x86_64")
	else()
		set(ARCH_NAME "i686")
	endif()

	file(GLOB locale_files data/locale/*.ini)
        execute_process(COMMAND uname -m COMMAND tr -d '\n' OUTPUT_VARIABLE UNAME_MACHINE)

	install(TARGETS obs-auto-subtitle
                LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/obs-plugins)
	# Dirty fix for Ubuntu
	install(TARGETS obs-auto-subtitle
		LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/${UNAME_MACHINE}-linux-gnu/obs-plugins)

	install(FILES ${locale_files}
		DESTINATION "${CMAKE_INSTALL_PREFIX}/share/obs/obs-plugins/obs-auto-subtitle/locale")
endif()