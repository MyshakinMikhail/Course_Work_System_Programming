add_library(storage STATIC
    FileManager.cpp
)

target_link_libraries(storage PUBLIC core)

target_include_directories(storage PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)
