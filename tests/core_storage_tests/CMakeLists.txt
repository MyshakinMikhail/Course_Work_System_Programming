add_executable(core_storage_tests
    core_storage_tests.cpp
)

target_link_libraries(core_storage_tests PRIVATE core storage)

add_test(NAME core_storage_tests COMMAND core_storage_tests)
