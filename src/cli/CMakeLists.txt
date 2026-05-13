add_executable(course_db_cli
    main.cpp
)

set_target_properties(course_db_cli PROPERTIES
    OUTPUT_NAME prog
)

target_link_libraries(course_db_cli PRIVATE
    parser
    engine
)
