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

void prepareUsers(Parser& parser, Engine& engine) {
    assertSuccessMessage(engine.execute(parser.parse("CREATE DATABASE app;")),
                         "Database 'app' created");
    assertSuccessMessage(engine.execute(parser.parse("USE app;")),
                         "Using database 'app'");
    assertSuccessMessage(
        engine.execute(parser.parse("CREATE TABLE users (id int, name string, age int);")),
        "Table 'app.users' created"
    );
    assertSuccessMessage(
        engine.execute(parser.parse(
            "INSERT INTO users (id, name, age) VALUE "
            "(1, \"Ann\", 17), (2, \"Bob\", 20), (3, \"Alice\", 30);"
        )),
        "Inserted 3 row(s) into 'app.users'"
    );
}

}

static void testSelectWhereComparison() {
    const std::filesystem::path root = makeTempRoot("course_work_engine_condition_comparison_test");

    Parser parser;
    Engine engine(root);
    prepareUsers(parser, engine);

    assertJson(
        engine.execute(parser.parse("SELECT (id, name) FROM users WHERE age >= 20;")),
        "[{\"id\":2,\"name\":\"Bob\"},{\"id\":3,\"name\":\"Alice\"}]"
    );

    assertJson(
        engine.execute(parser.parse("SELECT (name) FROM users WHERE name < \"Bob\";")),
        "[{\"name\":\"Ann\"},{\"name\":\"Alice\"}]"
    );

    std::filesystem::remove_all(root);
}

static void testSelectWhereBetween() {
    const std::filesystem::path root = makeTempRoot("course_work_engine_condition_between_test");

    Parser parser;
    Engine engine(root);
    prepareUsers(parser, engine);

    assertJson(
        engine.execute(parser.parse("SELECT (name) FROM users WHERE age BETWEEN 18 AND 30;")),
        "[{\"name\":\"Bob\"}]"
    );

    assertJson(
        engine.execute(parser.parse("SELECT (name) FROM users WHERE name BETWEEN \"Ann\" AND \"Bob\";")),
        "[{\"name\":\"Ann\"}]"
    );

    std::filesystem::remove_all(root);
}

static void testSelectWhereLike() {
    const std::filesystem::path root = makeTempRoot("course_work_engine_condition_like_test");

    Parser parser;
    Engine engine(root);
    prepareUsers(parser, engine);

    assertJson(
        engine.execute(parser.parse("SELECT (name) FROM users WHERE name LIKE \"A.*\";")),
        "[{\"name\":\"Ann\"},{\"name\":\"Alice\"}]"
    );

    std::filesystem::remove_all(root);
}

static void testConditionErrors() {
    const std::filesystem::path root = makeTempRoot("course_work_engine_condition_errors_test");

    Parser parser;
    Engine engine(root);
    prepareUsers(parser, engine);

    assertErrorContains(
        engine.execute(parser.parse("SELECT * FROM users WHERE missing == 1;")),
        "Column 'missing' does not exist"
    );

    assertErrorContains(
        engine.execute(parser.parse("SELECT * FROM users WHERE age == \"20\";")),
        "incompatible types"
    );

    assertErrorContains(
        engine.execute(parser.parse("SELECT * FROM users WHERE age LIKE \"2.*\";")),
        "LIKE operands must be STRING"
    );

    assertErrorContains(
        engine.execute(parser.parse("SELECT * FROM users WHERE name LIKE \"[\";")),
        "Invalid LIKE regular expression"
    );

    std::filesystem::remove_all(root);
}

int main() {
    testSelectWhereComparison();
    testSelectWhereBetween();
    testSelectWhereLike();
    testConditionErrors();
    return 0;
}
