#include <cassert>
#include <filesystem>
#include <string>

#include "Engine.h"
#include "Parser.h"

namespace {

void assertSuccessMessage(const ExecutionResult& result, const std::string& expectedMessage) {
    assert(result.isSuccess());
    assert(result.message == expectedMessage);
    assert(result.json.empty());
}

}

static void testDispatchesMetadataAndDdlCommands() {
    Parser parser;
    const std::filesystem::path root =
        std::filesystem::temp_directory_path() / "course_work_engine_dispatch_test";
    std::filesystem::remove_all(root);

    Engine engine(root);

    assertSuccessMessage(
        engine.execute(parser.parse("CREATE DATABASE app;")),
        "Database 'app' created"
    );

    assertSuccessMessage(
        engine.execute(parser.parse("USE app;")),
        "Using database 'app'"
    );

    assertSuccessMessage(
        engine.execute(parser.parse("CREATE TABLE users (id int INDEXED);")),
        "Table 'app.users' created"
    );

    assertSuccessMessage(
        engine.execute(parser.parse("DROP TABLE users;")),
        "Table 'app.users' dropped"
    );

    assertSuccessMessage(
        engine.execute(parser.parse("DROP DATABASE app;")),
        "Database 'app' dropped"
    );

    std::filesystem::remove_all(root);
}

static void testDispatchesDmlCommands() {
    Parser parser;
    const std::filesystem::path root =
        std::filesystem::temp_directory_path() / "course_work_engine_dml_dispatch_test";
    std::filesystem::remove_all(root);

    Engine engine(root);

    assertSuccessMessage(engine.execute(parser.parse("CREATE DATABASE app;")),
                         "Database 'app' created");
    assertSuccessMessage(engine.execute(parser.parse("USE app;")),
                         "Using database 'app'");
    assertSuccessMessage(engine.execute(parser.parse("CREATE TABLE users (id int, name string);")),
                         "Table 'app.users' created");

    assertSuccessMessage(
        engine.execute(parser.parse("INSERT INTO users (id, name) VALUE (1, \"Ann\");")),
        "Inserted 1 row(s) into 'app.users'"
    );

    const ExecutionResult selectResult = engine.execute(parser.parse("SELECT * FROM users;"));
    assert(selectResult.isSuccess());
    assert(selectResult.json == "[{\"id\":1,\"name\":\"Ann\"}]");

    assertSuccessMessage(
        engine.execute(parser.parse("UPDATE users SET name = \"Bob\" WHERE id == 1;")),
        "UPDATE is not implemented yet"
    );

    assertSuccessMessage(
        engine.execute(parser.parse("DELETE FROM users WHERE id == 1;")),
        "DELETE is not implemented yet"
    );

    std::filesystem::remove_all(root);
}

static void testExecutionResultFactories() {
    const ExecutionResult success = ExecutionResult::Success("ok");
    assert(success.isSuccess());
    assert(success.message == "ok");
    assert(success.json.empty());

    const ExecutionResult json = ExecutionResult::Json("[{\"id\":1}]");
    assert(json.isSuccess());
    assert(json.message.empty());
    assert(json.json == "[{\"id\":1}]");

    const ExecutionResult error = ExecutionResult::Error("failed");
    assert(!error.isSuccess());
    assert(error.message == "failed");
    assert(error.json.empty());
}

int main() {
    testDispatchesMetadataAndDdlCommands();
    testDispatchesDmlCommands();
    testExecutionResultFactories();
    return 0;
}
