cmake_minimum_required(VERSION 3.0)
project(QuicServer)

# Set OpenSSL directory
set(OPENSSL_ROOT_DIR "/opt/homebrew/opt/openssl@3" CACHE PATH "OpenSSL root directory")

# Find OpenSSL
find_package(OpenSSL REQUIRED)

# Add your source files
set(SOURCES
    main.c
)

# Add your header files
set(HEADERS
)

# Add protobuf generated files
set(PROTO_SOURCES
    message.pb-c.c
)
set(PROTO_HEADERS
    message.pb-c.h
)

# Create the executable
add_executable(quic_server ${SOURCES} ${HEADERS} ${PROTO_SOURCES} ${PROTO_HEADERS})

# Include directories
target_include_directories(quic_server PRIVATE ${PROTOBUF_INCLUDE_DIRS})

# Link libraries
target_link_libraries(quic_server PRIVATE quiche OpenSSL::Crypto ${PROTOBUF_LIBRARIES})
