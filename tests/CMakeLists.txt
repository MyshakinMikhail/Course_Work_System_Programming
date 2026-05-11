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
