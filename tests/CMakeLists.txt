add_executable(core_tests
    core_tests.cpp
)

target_link_libraries(core_tests PRIVATE core)

add_test(NAME core_tests COMMAND core_tests)

add_executable(storage_tests
    storage_tests.cpp
)

target_link_libraries(storage_tests PRIVATE core storage)

add_test(NAME storage_tests COMMAND storage_tests)

add_executable(parser_tokenizer_tests
    parser_tokenizer_tests.cpp
)

target_link_libraries(parser_tokenizer_tests PRIVATE parser)

add_test(NAME parser_tokenizer_tests COMMAND parser_tokenizer_tests)

add_executable(parser_ddl_tests
    parser_ddl_tests.cpp
)

target_link_libraries(parser_ddl_tests PRIVATE parser)

add_test(NAME parser_ddl_tests COMMAND parser_ddl_tests)

add_executable(parser_dml_tests
    parser_dml_tests.cpp
)

target_link_libraries(parser_dml_tests PRIVATE parser)

add_test(NAME parser_dml_tests COMMAND parser_dml_tests)

add_executable(parser_condition_tests
    parser_condition_tests.cpp
)

target_link_libraries(parser_condition_tests PRIVATE parser)

add_test(NAME parser_condition_tests COMMAND parser_condition_tests)

add_executable(engine_dispatch_tests
    engine_dispatch_tests.cpp
)

target_link_libraries(engine_dispatch_tests PRIVATE engine parser)

add_test(NAME engine_dispatch_tests COMMAND engine_dispatch_tests)

add_executable(engine_catalog_tests
    engine_catalog_tests.cpp
)

target_link_libraries(engine_catalog_tests PRIVATE engine parser)

add_test(NAME engine_catalog_tests COMMAND engine_catalog_tests)

add_executable(engine_table_tests
    engine_table_tests.cpp
)

target_link_libraries(engine_table_tests PRIVATE engine parser)

add_test(NAME engine_table_tests COMMAND engine_table_tests)

add_executable(engine_insert_select_tests
    engine_insert_select_tests.cpp
)

target_link_libraries(engine_insert_select_tests PRIVATE engine parser)

add_test(NAME engine_insert_select_tests COMMAND engine_insert_select_tests)

add_executable(engine_condition_tests
    engine_condition_tests.cpp
)

target_link_libraries(engine_condition_tests PRIVATE engine parser)

add_test(NAME engine_condition_tests COMMAND engine_condition_tests)

add_subdirectory(core_storage_tests)
