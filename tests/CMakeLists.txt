add_executable(
    tests
    main.cxx
    tests_serialization.cxx
    tests_socket_performance.cxx
    tests_server_client_uds_sockets.cxx
    tests_callback.cxx
    tests_async_core.cxx
    tests_scratchpad.cxx
    tests_async_socket_server.cxx
    tests_call_chain.cxx
)

target_link_libraries(tests emb pthread)