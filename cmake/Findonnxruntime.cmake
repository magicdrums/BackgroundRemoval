# Find ONNX Runtime (Fedora: onnxruntime-devel)
find_path(
  onnxruntime_INCLUDE_DIR
  NAMES onnxruntime_cxx_api.h
  PATH_SUFFIXES onnxruntime
  DOC "ONNX Runtime include directory")

find_library(
  onnxruntime_LIBRARY
  NAMES onnxruntime
  DOC "ONNX Runtime library")

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  onnxruntime
  DEFAULT_MSG
  onnxruntime_LIBRARY
  onnxruntime_INCLUDE_DIR)

if(onnxruntime_FOUND)
  set(onnxruntime_LIBRARIES ${onnxruntime_LIBRARY})
  set(onnxruntime_INCLUDE_DIRS ${onnxruntime_INCLUDE_DIR})
endif()

mark_as_advanced(onnxruntime_INCLUDE_DIR onnxruntime_LIBRARY)
