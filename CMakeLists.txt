cmake_minimum_required(VERSION 3.20)
project(QuicServer)

# Add your source files
set(SOURCES
    main.c
)

# Add your header files
set(HEADERS
)

# Protobuf file
# set(PROTO_FILE message.proto)

# Protobuf-c output directory
# set(PROTO_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/proto_output)

# Find ProtobufC package
# set(ProtobufC_DIR "/usr/local/opt/protobuf-c" CACHE PATH "ProtobufC installation directory")

# find_library(ProtobufC_LIBRARIES
#     NAMES protobuf-c
#     HINTS "${ProtobufC_DIR}/lib"
# )

# find_path(ProtobufC_INCLUDE_DIRS
#     NAMES protobuf-c/protobuf-c.h
#     HINTS "${ProtobufC_DIR}/include"
# )

# Create the output directory if it doesn't exist
# file(MAKE_DIRECTORY ${PROTO_OUTPUT_DIR})

# Generate protobuf-c source and header files during build
# add_custom_command(
#     OUTPUT ${PROTO_OUTPUT_DIR}/message.pb-c.c ${PROTO_OUTPUT_DIR}/message.pb-c.h
#     COMMAND protoc-c --c_out=${PROTO_OUTPUT_DIR} --proto_path=${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/proto/${PROTO_FILE}
#     DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/proto/${PROTO_FILE}
#     COMMENT "Generating protobuf-c files"
# )
# message(STATUS "Protobuf-c output directory: ${PROTO_OUTPUT_DIR}")

# Create the executable
# add_executable(quic_server ${SOURCES} ${HEADERS} ${PROTO_OUTPUT_DIR}/proto/message.pb-c.c)
add_executable(quic_server ${SOURCES} ${HEADERS})

# Add the msquic submodule directory
add_subdirectory(msquic)

message(STATUS "ProtobufC include directory: ${ProtobufC_INCLUDE_DIRS}")
message(STATUS "ProtobufC libraries: ${ProtobufC_LIBRARIES}")
# Include directories
# target_include_directories(quic_server PRIVATE ${PROTO_OUTPUT_DIR} ${ProtobufC_INCLUDE_DIRS} ${QUIC_INCLUDE_DIR} proto_output)
target_include_directories(quic_server PRIVATE ${QUIC_INCLUDE_DIR} proto_output)

# Link libraries
# target_link_libraries(quic_server PRIVATE msquic ${ProtobufC_LIBRARIES} msquic)
target_link_libraries(quic_server PRIVATE msquic msquic)
