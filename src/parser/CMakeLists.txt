add_library(parser STATIC
    Parser.cpp
    Tokenizer.cpp
)

target_include_directories(parser PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)
