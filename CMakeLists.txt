cmake_minimum_required(VERSION 2.6)

set(CMAKE_CXX_STANDARD 17)

file(GLOB src_list
    "${PROJECT_SOURCE_DIR}/lib/*.c*"

    "${PROJECT_SOURCE_DIR}/vender/onnxruntime-extensions/operators/tokenizer/*.c*"
    "${PROJECT_SOURCE_DIR}/vender/onnxruntime-extensions/base/*.c*"

    "${PROJECT_SOURCE_DIR}/vender/sentencepiece/src/*.c*"
    "${PROJECT_SOURCE_DIR}/vender/sentencepiece/src/builtin_pb/*.c*"

    "${PROJECT_SOURCE_DIR}/vender/sentencepiece/third_party/protobuf-lite/*.c*"
    "${PROJECT_SOURCE_DIR}/vender/sentencepiece/third_party/absl/flags/flag.cc"

    "${PROJECT_SOURCE_DIR}/vender/pcre/*.c*"
)

list(FILTER src_list EXCLUDE REGEX ".*\_main.cc$")
list(FILTER src_list EXCLUDE REGEX ".*\_test.cc$")
list(FILTER src_list EXCLUDE REGEX ".*blingfire_sentencebreaker.cc$")

include_directories(
    "${PROJECT_SOURCE_DIR}/include"
    "${PROJECT_SOURCE_DIR}/vender/GSL/include"
    "${PROJECT_SOURCE_DIR}/vender/json/include"
    "${PROJECT_SOURCE_DIR}/vender/onnxruntime-extensions/includes"
    "${PROJECT_SOURCE_DIR}/vender/onnxruntime-extensions/base"
    "${PROJECT_SOURCE_DIR}/vender/onnxruntime-extensions/operators/tokenizer"
    "${PROJECT_SOURCE_DIR}/vender/sentencepiece/src"
    "${PROJECT_SOURCE_DIR}/vender/sentencepiece/src/builtin_pb"
    "${PROJECT_SOURCE_DIR}/vender/sentencepiece/third_party/protobuf-lite"
    "${PROJECT_SOURCE_DIR}/vender/sentencepiece"
)

include(fib-addon/common.cmake)

add_definitions(
    -DHAVE_PTHREAD=1
    -DPACKAGE_STRING
    -DVERSION
)
