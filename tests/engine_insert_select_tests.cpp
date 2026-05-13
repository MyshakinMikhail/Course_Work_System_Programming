#include <cassert>
#include <filesystem>
#include <string>

#include "Engine.h"
#include "Parser.h"

namespace {

std::filesystem::path makeTempRoot(const std::string& name) {
    const std::filesystem::path root = std::filesystem::temp_directory_path() / name;
    std::filesystem::remove_all(root);
    return root;
}

void assertSuccessMessage(const ExecutionResult& result, const std::string& expectedMessage) {
    assert(result.isSuccess());
    assert(result.message == expectedMessage);
    assert(result.json.empty());
}

void assertJson(const ExecutionResult& result, const std::string& expectedJson) {
    assert(result.isSuccess());
    assert(result.message.empty());
    assert(result.json == expectedJson);
}

void assertErrorContains(const ExecutionResult& result, const std::string& expectedPart) {
    assert(!result.isSuccess());
    assert(result.message.find(expectedPart) != std::string::npos);
}

void prepareUsersTable(Parser& parser, Engine& engine) {
    assertSuccessMessage(engine.execute(parser.parse("CREATE DATABASE app;")),
                         "Database 'app' created");
    assertSuccessMessage(engine.execute(parser.parse("USE app;")),
                         "Using database 'app'");
    assertSuccessMessage(
        engine.execute(parser.parse("CREATE TABLE users (id int INDEXED, name string);")),
        "Table 'app.users' created"
    );
}

}

static void testInsertAndSelectAll() {
    const std::filesystem::path root = makeTempRoot("course_work_engine_insert_select_all_test");

    Parser parser;
    Engine engine(root);
    prepareUsersTable(parser, engine);

    assertSuccessMessage(
        engine.execute(parser.parse("INSERT INTO users (id, name) VALUE (1, \"Ann\"), (2, \"Bob\");")),
        "Inserted 2 row(s) into 'app.users'"
    );

    assertJson(
        engine.execute(parser.parse("SELECT * FROM users;")),
        "[{\"id\":1,\"name\":\"Ann\"},{\"id\":2,\"name\":\"Bob\"}]"
    );

    std::filesystem::remove_all(root);
}

static void testSelectItemsAndAliases() {
    const std::filesystem::path root = makeTempRoot("course_work_engine_select_alias_test");

    Parser parser;
    Engine engine(root);
    prepareUsersTable(parser, engine);

    assertSuccessMessage(
        engine.execute(parser.parse("INSERT INTO users (name, id) VALUE (\"Ann\", 1);")),
        "Inserted 1 row(s) into 'app.users'"
    );

    assertJson(
        engine.execute(parser.parse("SELECT (name AS username, id) FROM users;")),
        "[{\"username\":\"Ann\",\"id\":1}]"
    );

    std::filesystem::remove_all(root);
}

static void testInsertSelectErrors() {
    const std::filesystem::path root = makeTempRoot("course_work_engine_insert_select_errors_test");

    Parser parser;
    Engine engine(root);
    prepareUsersTable(parser, engine);

    assertErrorContains(
        engine.execute(parser.parse("INSERT INTO users (id) VALUE (1);")),
        "all columns"
    );

    assertErrorContains(
        engine.execute(parser.parse("INSERT INTO users (id, name) VALUE (\"bad\", \"Ann\");")),
        "Expected INT value"
    );

    assertErrorContains(
        engine.execute(parser.parse("SELECT (missing) FROM users;")),
        "does not exist"
    );

    std::filesystem::remove_all(root);
}

int main() {
    testInsertAndSelectAll();
    testSelectItemsAndAliases();
    testInsertSelectErrors();
    return 0;
}
