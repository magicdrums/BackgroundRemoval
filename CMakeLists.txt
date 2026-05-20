cmake_minimum_required(VERSION 3.16...3.28)

# OBS RPM en Fedora busca plugins en /usr/lib64, no en /usr/local
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  set(CMAKE_INSTALL_PREFIX
      /usr
      CACHE PATH "Install prefix (use /usr for OBS RPM on Fedora)" FORCE)
endif()

set(CMAKE_CXX_STANDARD 20)

project(
  obs-background-removal
  VERSION 2.0.0
  LANGUAGES CXX)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

set(PLUGIN_AUTHOR "BackgroundRemoval")
set(LINUX_MAINTAINER_EMAIL "local@localhost")
set(LINUX_PORTABLE OFF CACHE BOOL "Install into system OBS paths (Fedora RPM)" FORCE)

option(BUILD_TESTING "Build and register unit tests" ON)

find_package(libobs REQUIRED)
find_package(OpenCV REQUIRED COMPONENTS core imgproc imgcodecs)
find_package(onnxruntime REQUIRED)

include(cmake/ObsPluginHelpers.cmake)

configure_file(src/plugin-macros.h.in ${CMAKE_SOURCE_DIR}/src/plugin-macros.generated.h @ONLY)

add_library(obs-background-removal-core STATIC
            src/core/onnx_model_validate.cpp src/core/image_loader.cpp)

set_target_properties(obs-background-removal-core PROPERTIES POSITION_INDEPENDENT_CODE ON)

target_include_directories(obs-background-removal-core
                           PUBLIC ${CMAKE_SOURCE_DIR}/src)

target_link_libraries(obs-background-removal-core PUBLIC ${OpenCV_LIBS})

target_include_directories(obs-background-removal-core
                           SYSTEM PUBLIC ${OpenCV_INCLUDE_DIRS})

add_library(${CMAKE_PROJECT_NAME} MODULE)

target_sources(
  ${CMAKE_PROJECT_NAME}
  PRIVATE src/plugin-main.cpp
          src/background-filter.cpp
          src/ort-utils/ort-session-utils.cpp
          src/obs-utils/obs-utils.cpp
          src/obs-utils/background-image.cpp
          src/plugin-macros.generated.h)

target_include_directories(
  ${CMAKE_PROJECT_NAME}
  PRIVATE ${CMAKE_SOURCE_DIR}/src
          ${onnxruntime_INCLUDE_DIRS})

target_link_libraries(
  ${CMAKE_PROJECT_NAME}
  PRIVATE OBS::libobs
          obs-background-removal-core
          ${onnxruntime_LIBRARIES})

if(CMAKE_SYSTEM_NAME MATCHES "Linux")
  target_compile_options(${CMAKE_PROJECT_NAME} PRIVATE -Wall -Wextra)
endif()

setup_plugin_target(${CMAKE_PROJECT_NAME})

if(BUILD_TESTING)
  enable_testing()
  add_subdirectory(tests)
endif()
