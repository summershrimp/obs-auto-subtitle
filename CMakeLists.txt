cmake_minimum_required(VERSION 3.16...3.21)

# Change obs-plugintemplate to your plugin's name in a machine-readable format (e.g.:
# obs-myawesomeplugin) and set
project(obs-auto-subtitle VERSION 0.4.0)
add_library(${CMAKE_PROJECT_NAME} MODULE)

# Replace `Your Name Here` with the name (yours or your organization's) you want to see as the
# author of the plugin (in the plugin's metadata itself and in the installers)
set(PLUGIN_AUTHOR "Yibai Zhang")

# Replace `com.example.obs-plugin-template` with a unique Bundle ID for macOS releases (used both in
# the installer and when submitting the installer for notarization)
set(MACOS_BUNDLEID "com.summershrimp.${CMAKE_PROJECT_NAME}")

# Replace `me@contoso.com` with the maintainer email address you want to put in Linux packages
set(LINUX_MAINTAINER_EMAIL "xm1994@gmail.com")

# Add your custom source files here - header files are optional and only required for visibility
# e.g. in Xcode or Visual Studio
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

target_sources(${CMAKE_PROJECT_NAME} PRIVATE ${obs-auto-subtitle_SOURCES})

# Import libobs as main plugin dependency
find_package(libobs REQUIRED)
include(cmake/ObsPluginHelpers.cmake)

# Uncomment these lines if you want to use the OBS Frontend API in your plugin
#[[
find_package(obs-frontend-api REQUIRED)
target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE OBS::obs-frontend-api)
#]]

# Uncomment those lines if you want to use Qt in your plugin

find_qt(COMPONENTS Widgets Core Network WebSockets)
target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE Qt::Core Qt::Widgets Qt::Network Qt::WebSockets)
set_target_properties(
  ${CMAKE_PROJECT_NAME}
  PROPERTIES AUTOMOC ON
             AUTOUIC ON
             AUTORCC ON)

configure_file(src/plugin-macros.h.in ${CMAKE_SOURCE_DIR}/src/plugin-macros.generated.h)

target_sources(${CMAKE_PROJECT_NAME} PRIVATE src/plugin-macros.generated.h)

# /!\ TAKE NOTE: No need to edit things past this point /!\

# --- Platform-independent build settings ---

target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/src)

target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE OBS::libobs)

# --- End of section ---

# --- Windows-specific build settings and tasks ---
if(OS_WINDOWS)
  configure_file(cmake/bundle/windows/installer-Windows.iss.in
                 ${CMAKE_BINARY_DIR}/installer-Windows.generated.iss)

  configure_file(cmake/bundle/windows/resource.rc.in ${CMAKE_BINARY_DIR}/${CMAKE_PROJECT_NAME}.rc)
  target_sources(${CMAKE_PROJECT_NAME} PRIVATE ${CMAKE_BINARY_DIR}/${CMAKE_PROJECT_NAME}.rc)

  if(MSVC)
    find_package(w32-pthreads REQUIRED)
    target_link_libraries(obs-auto-subtitle PRIVATE OBS::w32-pthreads)

    target_compile_options(${CMAKE_PROJECT_NAME} PRIVATE /W4)
  endif()

  install(FILES ${QT6_INSTALL_PREFIX}/bin/Qt6WebSockets.dll DESTINATION ${OBS_LIBRARY_DESTINATION})

  install(FILES ${QT6_INSTALL_PREFIX}/plugins/tls/qcertonlybackend.dll
                ${QT6_INSTALL_PREFIX}/plugins/tls/qschannelbackend.dll
          DESTINATION ${OBS_LIBRARY_DESTINATION}/tls)
  # --- End of section ---

  # -- macOS specific build settings and tasks --
elseif(OS_MACOS)
  configure_file(cmake/bundle/macos/installer-macos.pkgproj.in
                 ${CMAKE_BINARY_DIR}/installer-macos.generated.pkgproj)

  set(MACOSX_PLUGIN_GUI_IDENTIFIER "${MACOS_BUNDLEID}")
  set(MACOSX_PLUGIN_BUNDLE_VERSION "${CMAKE_PROJECT_VERSION}")
  set(MACOSX_PLUGIN_SHORT_VERSION_STRING "1")

  target_compile_options(${CMAKE_PROJECT_NAME} PRIVATE -Wall)

  # --- End of section ---

  # --- Linux-specific build settings and tasks ---
else()
  target_compile_options(${CMAKE_PROJECT_NAME} PRIVATE -Wall)
endif()
# --- End of section ---

setup_plugin_target(${CMAKE_PROJECT_NAME})
if(OS_MACOS)
  install(
    DIRECTORY "${QT6_INSTALL_PREFIX}/${QT6_INSTALL_LIBS}/QtWebSockets.framework"
    DESTINATION "./${CMAKE_PROJECT_NAME}.plugin/Contents/Frameworks"
    COMPONENT obs_plugins
    PATTERN "Headers" EXCLUDE)
  if(${QT_VERSION} EQUAL 5)
    set(_QT_FW_VERSION "${QT_VERSION}")
  else()
    set(_QT_FW_VERSION "A")
  endif()
  set(_COMMAND
      "${CMAKE_INSTALL_NAME_TOOL} \\
        -change @rpath/QtWebSockets.framework/Versions/${_QT_FW_VERSION}/QtWebSockets @loader_path/../Frameworks/QtWebSockets.framework/Versions/${_QT_FW_VERSION}/QtWebSockets \\
        \\\"\${CMAKE_INSTALL_PREFIX}/${CMAKE_PROJECT_NAME}.plugin/Contents/MacOS/${CMAKE_PROJECT_NAME}\\\""
  )
  install(CODE "execute_process(COMMAND /bin/sh -c \"${_COMMAND}\")" COMPONENT obs_plugins)

endif()
