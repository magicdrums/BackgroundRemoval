include(GNUInstallDirs)

if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
  set(OS_MACOS ON)
  set(OS_POSIX ON)
elseif(CMAKE_SYSTEM_NAME MATCHES "Linux|FreeBSD|OpenBSD")
  set(OS_POSIX ON)
  string(TOUPPER "${CMAKE_SYSTEM_NAME}" _SYSTEM_NAME_U)
  set(OS_${_SYSTEM_NAME_U} ON)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
  set(OS_WINDOWS ON)
endif()

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING "" FORCE)
endif()

if(OS_POSIX)
  if(NOT DEFINED LINUX_PORTABLE)
    option(LINUX_PORTABLE "Portable Linux layout" OFF)
  endif()

  if(NOT LINUX_PORTABLE)
    set(OBS_LIBRARY_DESTINATION ${CMAKE_INSTALL_LIBDIR})
    set(OBS_PLUGIN_DESTINATION ${OBS_LIBRARY_DESTINATION}/obs-plugins)
    set(OBS_DATA_DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/obs)
  else()
    set(OBS_LIBRARY_DESTINATION bin/64bit)
    set(OBS_PLUGIN_DESTINATION obs-plugins/64bit)
    set(OBS_DATA_DESTINATION data)
  endif()
endif()

function(setup_target_resources target destination)
  if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/data)
    install(
      DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/data/
      DESTINATION ${OBS_DATA_DESTINATION}/obs-plugins/${destination}
      USE_SOURCE_PERMISSIONS)
  endif()
endfunction()

function(setup_plugin_target target)
  set_target_properties(${target} PROPERTIES PREFIX "")

  install(
    TARGETS ${target}
    LIBRARY DESTINATION ${OBS_PLUGIN_DESTINATION}
    RUNTIME DESTINATION ${OBS_PLUGIN_DESTINATION})

  setup_target_resources(${target} ${target})
endfunction()
