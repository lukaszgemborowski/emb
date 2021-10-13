cmake_minimum_required(VERSION 3.14 FATAL_ERROR)
project(emb)

add_library(
    emb
    STATIC
    src/net/socket.cxx
    src/net/uds_server_socket.cxx
    src/net/uds_client_socket.cxx
    src/net/async_server.cxx
    src/rpc/socket_client.cxx
)

target_include_directories(emb PUBLIC include)
target_compile_features(emb PUBLIC cxx_std_17)
add_subdirectory(tests)
add_subdirectory(example)