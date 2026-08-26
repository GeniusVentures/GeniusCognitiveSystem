# gcs_proto — GCS protobuf wire contract (D-26/D-29).
#
# Mirrors GNUS-NEO-SWARM/src/proto/CMakeLists.txt: second arg is this repo's src
# root — proto import paths resolve against it (forwarded to protoc as -I).
#
# Path alignment (GCS-specific): add_proto_library (build/cmake/functions.cmake)
# derives the generated-file location from
#   file(RELATIVE_PATH SCHEMA_REL "${CMAKE_BINARY_DIR}/src" "${CMAKE_CURRENT_BINARY_DIR}")
# which only matches protoc's --cpp_out layout (generated/<proto path under the
# src root>) when this directory's binary dir mirrors ${CMAKE_BINARY_DIR}/src/proto.
# That mapping belongs to GNUS-NEO-SWARM (its src occupies ${CMAKE_BINARY_DIR}/src);
# this repo's src maps to gcs_src (cmake/CommonBuildParameters.cmake), so the
# default binary dir (gcs_src/proto) would declare rule outputs at
# gcs_src/proto/*.pb.* that protoc never writes (it writes generated/proto/*.pb.*),
# breaking the build. Aligning the variable for the helper call below keeps the
# single add_proto_library contract (generated/ layout, PUBLIC include dir,
# include/proto install path) without touching the shared build/cmake helper.
set(CMAKE_CURRENT_BINARY_DIR "${CMAKE_BINARY_DIR}/src/proto")

get_filename_component(GCS_SRC_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/.." ABSOLUTE)
add_proto_library(gcs_proto
    ${GCS_SRC_ROOT}
    gcs_chat.proto
)
