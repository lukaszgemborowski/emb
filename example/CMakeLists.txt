add_executable(
    rpc_server_client
    rpc_server_client.cxx
)

target_link_libraries(
    rpc_server_client
    emb
    pthread
)