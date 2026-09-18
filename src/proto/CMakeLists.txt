# gcs_proto — GCS protobuf wire contract (D-26/D-29).
#
# Mirrors GNUS-NEO-SWARM/src/proto/CMakeLists.txt: second arg is this repo's src
# root — proto import paths resolve against it (forwarded to protoc as -I).
# add_proto_library (build/cmake/functions.cmake) derives the generated-file
# location from the proto file's path under that root, matching protoc's own
# --cpp_out layout regardless of this module's binary dir (gcs_src/proto).
get_filename_component(GCS_SRC_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/.." ABSOLUTE)
add_proto_library(gcs_proto
    ${GCS_SRC_ROOT}
    gcs_chat.proto
)
