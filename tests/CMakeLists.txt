find_package(GTest QUIET)

if(NOT GTest_FOUND)
  include(FetchContent)
  FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.15.2
    GIT_SHALLOW TRUE)
  set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
  FetchContent_MakeAvailable(googletest)
endif()

add_executable(obs-background-removal-tests
               test_tensor_utils.cpp
               test_onnx_model_validate.cpp
               test_image_loader.cpp
               test_mediapipe_shapes.cpp
               test_onnx_inference.cpp)

target_include_directories(obs-background-removal-tests PRIVATE ${CMAKE_SOURCE_DIR}/src)

target_link_libraries(
  obs-background-removal-tests
  PRIVATE obs-background-removal-core
          GTest::gtest_main
          ${OpenCV_LIBS}
          ${onnxruntime_LIBRARIES})

target_include_directories(obs-background-removal-tests SYSTEM
                           PRIVATE ${onnxruntime_INCLUDE_DIRS})

include(GoogleTest)
gtest_discover_tests(obs-background-removal-tests)
