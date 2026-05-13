#include <cassert>
#include <stdexcept>
#include <string>
#include <variant>

#include "Ast.h"
#include "ParseError.h"
#include "Parser.h"

namespace {

template <typename T>
T asCommand(const ParsedCommand& command) {
    assert(std::holds_alternative<T>(command));
    return std::get<T>(command);
}

template <typename Fn>
void expectParseError(Fn&& fn) {
    bool thrown = false;
    try {
        fn();
    } catch (const ParseError&) {
        thrown = true;
    }
    assert(thrown);
}

}

static void testParsesDatabaseCommands() {
    const Parser parser;

    const CreateDatabaseCommand create = asCommand<CreateDatabaseCommand>(
        parser.parse("CREATE DATABASE app;")
    );
    assert(create.databaseName == "app");

    const DropDatabaseCommand drop = asCommand<DropDatabaseCommand>(
        parser.parse("DROP DATABASE app;")
    );
    assert(drop.databaseName == "app");

    const UseDatabaseCommand use = asCommand<UseDatabaseCommand>(
        parser.parse("USE app;")
    );
    assert(use.databaseName == "app");
}

static void testParsesCreateTable() {
    const Parser parser;
    const CreateTableCommand command = asCommand<CreateTableCommand>(
        parser.parse("CREATE TABLE users (id int INDEXED, name string NOT_NULL);")
    );

    assert(!command.tableName.databaseName.has_value());
    assert(command.tableName.tableName == "users");
    assert(command.columns.size() == 2);

    assert(command.columns[0].name == "id");
    assert(command.columns[0].type == AstFieldType::Int);
    assert(!command.columns[0].notNull);
    assert(command.columns[0].indexed);

    assert(command.columns[1].name == "name");
    assert(command.columns[1].type == AstFieldType::String);
    assert(command.columns[1].notNull);
    assert(!command.columns[1].indexed);
}

static void testParsesQualifiedTableNames() {
    const Parser parser;

    const CreateTableCommand create = asCommand<CreateTableCommand>(
        parser.parse("CREATE TABLE app.users (id int);")
    );
    assert(create.tableName.databaseName.has_value());
    assert(*create.tableName.databaseName == "app");
    assert(create.tableName.tableName == "users");

    const DropTableCommand drop = asCommand<DropTableCommand>(
        parser.parse("DROP TABLE app.users;")
    );
    assert(drop.tableName.databaseName.has_value());
    assert(*drop.tableName.databaseName == "app");
    assert(drop.tableName.tableName == "users");
}

static void testParsesColumnModifiersInAnyOrder() {
    const Parser parser;
    const CreateTableCommand command = asCommand<CreateTableCommand>(
        parser.parse("CREATE TABLE users (id int NOT_NULL INDEXED);")
    );

    assert(command.columns.size() == 1);
    assert(command.columns[0].notNull);
    assert(command.columns[0].indexed);
}

static void testRejectsInvalidDdl() {
    const Parser parser;

    expectParseError([&]() {
        (void)parser.parse("CREATE TABLE users ();");
    });

    expectParseError([&]() {
        (void)parser.parse("CREATE TABLE users (id INDEXED);");
    });

    expectParseError([&]() {
        (void)parser.parse("DROP users;");
    });

    expectParseError([&]() {
        (void)parser.parse("USE app");
    });

    expectParseError([&]() {
        (void)parser.parse("CREATE TABLE users (id int INDEXED INDEXED);");
    });
}

int main() {
    testParsesDatabaseCommands();
    testParsesCreateTable();
    testParsesQualifiedTableNames();
    testParsesColumnModifiersInAnyOrder();
    testRejectsInvalidDdl();
    return 0;
}
