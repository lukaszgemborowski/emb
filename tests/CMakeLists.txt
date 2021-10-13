add_executable(
    tests
    main.cxx
    tests_serialization.cxx
)
target_link_libraries(tests emb)