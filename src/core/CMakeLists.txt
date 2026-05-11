add_library(core STATIC
    Table.cpp
)

target_include_directories(core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/src/storage
)

target_link_libraries(core PUBLIC storage)
