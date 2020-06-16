cmake_minimum_required(VERSION 3.5)
project(obs-auto-subtitle VERSION 0.2.1)

add_compile_definitions(OBS_SSP_VERSION="${CMAKE_PROJECT_VERSION}")

set(CMAKE_CXX_STANDARD 14)

set(CMAKE_PREFIX_PATH "${QTDIR}")
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

add_definitions(-DASIO_STANDALONE)

if (WIN32 OR APPLE)
	include(external/FindLibObs.cmake)
endif()

find_package(LibObs REQUIRED)
find_package(Qt5 REQUIRED Core Widgets Network WebSockets)

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
	set(ARCH 64)
else()
	set(ARCH 32)
endif()

set(obs-auto-subtitle_SOURCES
		src/obs-auto-subtitle.cpp
        src/obs-autosub-filter.cpp
		src/vendor/ASRBase.cpp
		src/vendor/XFRtASR.cpp
		src/vendor/HwCloudRASR.cpp
		src/vendor/AliNLS.cpp)

set(obs-auto-subtitle_HEADERS
		src/obs-auto-subtitle.h
		src/vendor/ASRBase.h
		src/vendor/XFRtASR.h
		src/vendor/HwCloudRASR.h
		src/vendor/AliNLS.h)

add_library(obs-auto-subtitle MODULE
	${obs-auto-subtitle_SOURCES}
	${obs-auto-subtitle_HEADERS})

include_directories(
		"${LIBOBS_INCLUDE_DIR}/../UI/obs-frontend-api"
		${Qt5Core_INCLUDES}
		${Qt5Widgets_INCLUDES}
		${Qt5Network_INCLUDES}
		${Qt5WebSockets_INCLUDES})

target_link_libraries(obs-auto-subtitle
		libobs
		Qt5::Core
		Qt5::Widgets
		Qt5::Network
		Qt5::WebSockets)

# Windows
if(WIN32)
	set(OBS_FRONTEND_LIB "OBS_FRONTEND_LIB-NOTFOUND" CACHE FILEPATH "OBS frontend library")
	if(OBS_FRONTEND_LIB EQUAL "OBS_FRONTEND_LIB-NOTFOUND")
		message(FATAL_ERROR "Could not find OBS Frontend API\'s library !")
	endif()

	string(REGEX REPLACE "/RTC(su|[1su])" "" CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
	message("CXX release flags: ${CMAKE_CXX_FLAGS_RELEASE}")

	add_definitions(-D_WEBSOCKETPP_CPP11_STL_)

	if(ARCH EQUAL 64)
		set(OBS_ARCH_NAME "64bit")
		set(OBS_BUILDDIR_ARCH "build64")
	else()
		set(OBS_ARCH_NAME "32bit")
		set(OBS_BUILDDIR_ARCH "build32")
		add_definitions(/arch:SSE2)
	endif()

	find_package(w32-pthreads REQUIRED)

	target_link_libraries(obs-auto-subtitle w32-pthreads)

	set(RELEASE_DIR "${PROJECT_SOURCE_DIR}/release")
	add_custom_command(TARGET obs-auto-subtitle POST_BUILD
		COMMAND if $<CONFIG:Release>==1 (
			"${CMAKE_COMMAND}" -E make_directory
				"${RELEASE_DIR}/bin/${OBS_ARCH_NAME}"
				"${RELEASE_DIR}/data/obs-plugins/obs-auto-subtitle"
				"${RELEASE_DIR}/obs-plugins/${OBS_ARCH_NAME}")

		COMMAND if $<CONFIG:Release>==1 (
			"${CMAKE_COMMAND}" -E copy_directory
				"${PROJECT_SOURCE_DIR}/data"
				"${RELEASE_DIR}/data/obs-plugins/obs-auto-subtitle")

		COMMAND if $<CONFIG:Release>==1 (
			"${CMAKE_COMMAND}" -E copy
				"$<TARGET_FILE:obs-auto-subtitle>"
				"${RELEASE_DIR}/obs-plugins/${OBS_ARCH_NAME}")

		COMMAND if $<CONFIG:Release>==1 (
			"${CMAKE_COMMAND}" -E copy
				"$<TARGET_FILE:Qt5::Network>"
				"${RELEASE_DIR}/bin/${OBS_ARCH_NAME}")

		COMMAND if $<CONFIG:Release>==1 (
			"${CMAKE_COMMAND}" -E copy
				"$<TARGET_FILE:Qt5::WebSockets>"
				"${RELEASE_DIR}/bin/${OBS_ARCH_NAME}")

			# If config is RelWithDebInfo, package release files
		COMMAND if $<CONFIG:RelWithDebInfo>==1 (
			"${CMAKE_COMMAND}" -E make_directory
				"${RELEASE_DIR}/bin/${OBS_ARCH_NAME}"
				"${RELEASE_DIR}/data/obs-plugins/obs-auto-subtitle"
				"${RELEASE_DIR}/obs-plugins/${OBS_ARCH_NAME}")

		COMMAND if $<CONFIG:RelWithDebInfo>==1 (
			"${CMAKE_COMMAND}" -E copy_directory
				"${PROJECT_SOURCE_DIR}/data"
				"${RELEASE_DIR}/data/obs-plugins/obs-auto-subtitle")

		COMMAND if $<CONFIG:RelWithDebInfo>==1 (
			"${CMAKE_COMMAND}" -E copy
				"$<TARGET_FILE:obs-auto-subtitle>"
				"${RELEASE_DIR}/obs-plugins/${OBS_ARCH_NAME}")

		COMMAND if $<CONFIG:RelWithDebInfo>==1 (
			"${CMAKE_COMMAND}" -E copy
				"$<TARGET_FILE:Qt5::Network>"
				"${RELEASE_DIR}/bin/${OBS_ARCH_NAME}")

		COMMAND if $<CONFIG:RelWithDebInfo>==1 (
			"${CMAKE_COMMAND}" -E copy
				"$<TARGET_FILE:Qt5::WebSockets>"
				"${RELEASE_DIR}/bin/${OBS_ARCH_NAME}")

			COMMAND if $<CONFIG:RelWithDebInfo>==1 (
			"${CMAKE_COMMAND}" -E copy
				"$<TARGET_PDB_FILE:obs-auto-subtitle>"
				"${RELEASE_DIR}/obs-plugins/${OBS_ARCH_NAME}")

		# Copy to obs-studio dev environment for immediate testing
		COMMAND if $<CONFIG:Debug>==1 (
			"${CMAKE_COMMAND}" -E copy
				"$<TARGET_FILE:obs-auto-subtitle>"
				"${LIBOBS_INCLUDE_DIR}/../${OBS_BUILDDIR_ARCH}/rundir/$<CONFIG>/obs-plugins/${OBS_ARCH_NAME}")

		COMMAND if $<CONFIG:Debug>==1 (
			"${CMAKE_COMMAND}" -E make_directory
				"${LIBOBS_INCLUDE_DIR}/../${OBS_BUILDDIR_ARCH}/rundir/$<CONFIG>/data/obs-plugins/obs-auto-subtitle")

		COMMAND if $<CONFIG:Debug>==1 (
			"${CMAKE_COMMAND}" -E copy_directory
				"${PROJECT_SOURCE_DIR}/data"
				"${LIBOBS_INCLUDE_DIR}/../${OBS_BUILDDIR_ARCH}/rundir/$<CONFIG>/data/obs-plugins/obs-auto-subtitle")
	)
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

	set_target_properties(obs-auto-subtitle PROPERTIES PREFIX "")

	target_link_libraries(obs-auto-subtitle
		obs-frontend-api)

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
