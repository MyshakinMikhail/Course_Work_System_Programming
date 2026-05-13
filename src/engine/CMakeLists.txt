add_library(engine STATIC
    ConditionEvaluator.cpp
    DatabaseCatalog.cpp
    Engine.cpp
    JsonFormatter.cpp
)

include(FetchContent)

FetchContent_Declare(
    nlohmann_json
    URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)

FetchContent_MakeAvailable(nlohmann_json)

target_include_directories(engine PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(engine PUBLIC
    parser
    core
    storage
    nlohmann_json::nlohmann_json
)
