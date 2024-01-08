cmake_minimum_required(VERSION 2.6)

set(CMAKE_CXX_STANDARD 17)

file(GLOB src_list
    "${PROJECT_SOURCE_DIR}/src/*.c*"
    "${PROJECT_SOURCE_DIR}/src/patch/*.c*"

    "${PROJECT_SOURCE_DIR}/vender/sentencepiece/src/*.c*"
    "${PROJECT_SOURCE_DIR}/vender/sentencepiece/src/builtin_pb/*.c*"

    "${PROJECT_SOURCE_DIR}/vender/sentencepiece/third_party/protobuf-lite/*.c*"
    "${PROJECT_SOURCE_DIR}/vender/sentencepiece/third_party/absl/flags/flag.cc"

    "${PROJECT_SOURCE_DIR}/vender/cpp-tiktoken/modelparams.cc"
    "${PROJECT_SOURCE_DIR}/vender/cpp-tiktoken/encoding.cc"
    "${PROJECT_SOURCE_DIR}/vender/cpp-tiktoken/encoding_utils.cc"

    "${PROJECT_SOURCE_DIR}/vender/boost/filesystem/src/*.cpp"

    "${PROJECT_SOURCE_DIR}/vender/pcre/*.c*"

    "${PROJECT_SOURCE_DIR}/vender/jinja2/src/*.c*"
    "${PROJECT_SOURCE_DIR}/vender/jinja2/src/binding/rapid_json_serializer.cpp"
)

list(FILTER src_list EXCLUDE REGEX ".*\_main.cc$")
list(FILTER src_list EXCLUDE REGEX ".*\_test.cc$")
list(FILTER src_list EXCLUDE REGEX ".*blingfire_sentencebreaker.cc$")

include_directories(
    "${PROJECT_SOURCE_DIR}/include"
    "${PROJECT_SOURCE_DIR}/vender/jinja2/include"

    "${PROJECT_SOURCE_DIR}/vender/nonstd/optional-lite/include"
    "${PROJECT_SOURCE_DIR}/vender/nonstd/variant-lite/include"
    "${PROJECT_SOURCE_DIR}/vender/nonstd/string-view-lite/include"
    "${PROJECT_SOURCE_DIR}/vender/nonstd/expected-lite/include"
 
    "${PROJECT_SOURCE_DIR}/vender/boost/algorithm/include"
    "${PROJECT_SOURCE_DIR}/vender/boost/assert/include"
    "${PROJECT_SOURCE_DIR}/vender/boost/concept_check/include"
    "${PROJECT_SOURCE_DIR}/vender/boost/config/include"
    "${PROJECT_SOURCE_DIR}/vender/boost/container_hash/include"
    "${PROJECT_SOURCE_DIR}/vender/boost/container/include"
    "${PROJECT_SOURCE_DIR}/vender/boost/conversion/include"
    "${PROJECT_SOURCE_DIR}/vender/boost/core/include"
    "${PROJECT_SOURCE_DIR}/vender/boost/describe/include"
    "${PROJECT_SOURCE_DIR}/vender/boost/detail/include"
    "${PROJECT_SOURCE_DIR}/vender/boost/filesystem/include"
    "${PROJECT_SOURCE_DIR}/vender/boost/functional/include"
    "${PROJECT_SOURCE_DIR}/vender/boost/intrusive/include"
    "${PROJECT_SOURCE_DIR}/vender/boost/io/include"
    "${PROJECT_SOURCE_DIR}/vender/boost/iterator/include"
    "${PROJECT_SOURCE_DIR}/vender/boost/move/include"
    "${PROJECT_SOURCE_DIR}/vender/boost/mp11/include"
    "${PROJECT_SOURCE_DIR}/vender/boost/mpl/include"
    "${PROJECT_SOURCE_DIR}/vender/boost/numeric_conversion/include"
    "${PROJECT_SOURCE_DIR}/vender/boost/optional/include"
    "${PROJECT_SOURCE_DIR}/vender/boost/predef/include"
    "${PROJECT_SOURCE_DIR}/vender/boost/preprocessor/include"
    "${PROJECT_SOURCE_DIR}/vender/boost/range/include"
    "${PROJECT_SOURCE_DIR}/vender/boost/regex/include"
    "${PROJECT_SOURCE_DIR}/vender/boost/smart_ptr/include"
    "${PROJECT_SOURCE_DIR}/vender/boost/static_assert/include"
    "${PROJECT_SOURCE_DIR}/vender/boost/system/include"
    "${PROJECT_SOURCE_DIR}/vender/boost/throw_exception/include"
    "${PROJECT_SOURCE_DIR}/vender/boost/type_traits/include"
    "${PROJECT_SOURCE_DIR}/vender/boost/unordered/include"
    "${PROJECT_SOURCE_DIR}/vender/boost/utility/include"
    "${PROJECT_SOURCE_DIR}/vender/boost/variant/include"
    "${PROJECT_SOURCE_DIR}/vender/boost/winapi/include"

    "${PROJECT_SOURCE_DIR}/vender/fmt/include"
    "${PROJECT_SOURCE_DIR}/vender/rapidjson/include"

    "${PROJECT_SOURCE_DIR}/vender/json/include"
    "${PROJECT_SOURCE_DIR}/vender/sentencepiece/src"
    "${PROJECT_SOURCE_DIR}/vender/sentencepiece/src/builtin_pb"
    "${PROJECT_SOURCE_DIR}/vender/sentencepiece/third_party/protobuf-lite"
    "${PROJECT_SOURCE_DIR}/vender/sentencepiece"
    "${PROJECT_SOURCE_DIR}/vender/cpp-tiktoken"
    "${PROJECT_SOURCE_DIR}/vender/pcre"
)

include(fib-addon/common.cmake)

add_definitions(
    -DBOOST_ALL_NO_LIB
    -DBOOST_FILESYSTEM_SINGLE_THREADED
    -DHAVE_PTHREAD=1
    -DPACKAGE_STRING
    -DVERSION
)

if(WIN32)
    add_definitions(-DRAPIDJSON_ENDIAN=RAPIDJSON_LITTLEENDIAN)
endif()