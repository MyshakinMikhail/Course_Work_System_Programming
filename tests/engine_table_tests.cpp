#include <cassert>
#include <filesystem>
#include <string>

#include "Engine.h"
#include "FileManager.h"
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

void assertErrorContains(const ExecutionResult& result, const std::string& expectedPart) {
    assert(!result.isSuccess());
    assert(result.message.find(expectedPart) != std::string::npos);
}

}

static void testCreateAndDropTableThroughEngine() {
    const std::filesystem::path root = makeTempRoot("course_work_engine_table_test");

    Parser parser;
    Engine engine(root);

    assertSuccessMessage(engine.execute(parser.parse("CREATE DATABASE app;")),
                         "Database 'app' created");
    assertSuccessMessage(engine.execute(parser.parse("USE app;")),
                         "Using database 'app'");

    assertSuccessMessage(
        engine.execute(parser.parse("CREATE TABLE users (id int INDEXED, name string NOT_NULL);")),
        "Table 'app.users' created"
    );

    const std::filesystem::path tablePath = root / "app" / "users.tbl";
    assert(std::filesystem::is_regular_file(tablePath));

    FileManager reader(tablePath);
    const TableSchema schema = reader.readSchema();
    assert(schema.columns.size() == 2);
    assert(schema.columns[0].name == "id");
    assert(schema.columns[0].type == FieldType::INT);
    assert(schema.columns[0].indexed);
    assert(schema.columns[1].name == "name");
    assert(schema.columns[1].type == FieldType::STRING);

    assertSuccessMessage(engine.execute(parser.parse("DROP TABLE users;")),
                         "Table 'app.users' dropped");
    assert(!std::filesystem::exists(tablePath));

    std::filesystem::remove_all(root);
}

static void testCreateQualifiedTableName() {
    const std::filesystem::path root = makeTempRoot("course_work_engine_qualified_table_test");

    Parser parser;
    Engine engine(root);

    assertSuccessMessage(engine.execute(parser.parse("CREATE DATABASE app;")),
                         "Database 'app' created");
    assertSuccessMessage(
        engine.execute(parser.parse("CREATE TABLE app.users (id int);")),
        "Table 'app.users' created"
    );

    assert(std::filesystem::is_regular_file(root / "app" / "users.tbl"));

    std::filesystem::remove_all(root);
}

static void testTableErrors() {
    const std::filesystem::path root = makeTempRoot("course_work_engine_table_errors_test");

    Parser parser;
    Engine engine(root);

    assertErrorContains(
        engine.execute(parser.parse("CREATE TABLE users (id int);")),
        "No active database selected"
    );

    assertSuccessMessage(engine.execute(parser.parse("CREATE DATABASE app;")),
                         "Database 'app' created");
    assertSuccessMessage(engine.execute(parser.parse("USE app;")),
                         "Using database 'app'");

    assertErrorContains(
        engine.execute(parser.parse("CREATE TABLE users (id int, id string);")),
        "Duplicate column 'id'"
    );

    assertSuccessMessage(engine.execute(parser.parse("CREATE TABLE users (id int);")),
                         "Table 'app.users' created");

    assertErrorContains(
        engine.execute(parser.parse("CREATE TABLE users (id int);")),
        "already exists"
    );

    assertErrorContains(
        engine.execute(parser.parse("DROP TABLE missing;")),
        "does not exist"
    );

    std::filesystem::remove_all(root);
}

int main() {
    testCreateAndDropTableThroughEngine();
    testCreateQualifiedTableName();
    testTableErrors();
    return 0;
}
