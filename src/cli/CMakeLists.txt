add_library(cli STATIC
    CliApp.cpp
    CommandReader.cpp
)

target_include_directories(cli PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(cli PUBLIC
    parser
    engine
)

add_executable(course_db_cli
    main.cpp
)

set_target_properties(course_db_cli PROPERTIES
    OUTPUT_NAME prog
)

target_link_libraries(course_db_cli PRIVATE
    cli
)
