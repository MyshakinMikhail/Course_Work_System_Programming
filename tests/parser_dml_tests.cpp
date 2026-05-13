#include <cassert>
#include <optional>
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

template <typename T>
const T& asValue(const Value& value) {
    assert(std::holds_alternative<T>(value));
    return std::get<T>(value);
}

template <typename T>
const T& asOperandValue(const Operand& operand) {
    assert(std::holds_alternative<T>(operand.value));
    return std::get<T>(operand.value);
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

static void testParsesSingleRowInsert() {
    const Parser parser;
    const InsertCommand command = asCommand<InsertCommand>(
        parser.parse("INSERT INTO users (id, name) VALUE (1, \"Ann\");")
    );

    assert(!command.tableName.databaseName.has_value());
    assert(command.tableName.tableName == "users");
    assert(command.columns.size() == 2);
    assert(command.columns[0] == "id");
    assert(command.columns[1] == "name");

    assert(command.rows.size() == 1);
    assert(command.rows[0].size() == 2);
    assert(asValue<int>(command.rows[0][0]) == 1);
    assert(asValue<std::string>(command.rows[0][1]) == "Ann");
}

static void testParsesMultiRowInsertWithNull() {
    const Parser parser;
    const InsertCommand command = asCommand<InsertCommand>(
        parser.parse("INSERT INTO app.users (id, name) VALUE (1, \"Ann\"), (2, NULL);")
    );

    assert(command.tableName.databaseName.has_value());
    assert(*command.tableName.databaseName == "app");
    assert(command.tableName.tableName == "users");
    assert(command.rows.size() == 2);
    assert(asValue<int>(command.rows[1][0]) == 2);
    assert(std::holds_alternative<NullValue>(command.rows[1][1]));
}

static void testParsesSelectAll() {
    const Parser parser;
    const SelectCommand command = asCommand<SelectCommand>(
        parser.parse("SELECT * FROM users;")
    );

    assert(command.selectAll);
    assert(command.items.empty());
    assert(command.tableName.tableName == "users");
    assert(!command.where.has_value());
}

static void testParsesSelectItemsWithAliases() {
    const Parser parser;
    const SelectCommand command = asCommand<SelectCommand>(
        parser.parse("SELECT (id, name AS username) FROM users;")
    );

    assert(!command.selectAll);
    assert(command.items.size() == 2);
    assert(command.items[0].columnName == "id");
    assert(!command.items[0].alias.has_value());
    assert(command.items[1].columnName == "name");
    assert(command.items[1].alias.has_value());
    assert(*command.items[1].alias == "username");
}

static void testParsesSelectWithWhereComparison() {
    const Parser parser;
    const SelectCommand command = asCommand<SelectCommand>(
        parser.parse("SELECT * FROM app.users WHERE id >= 10;")
    );

    assert(command.tableName.databaseName.has_value());
    assert(*command.tableName.databaseName == "app");
    assert(command.where.has_value());

    const ComparisonCondition& condition = std::get<ComparisonCondition>(*command.where);
    assert(asOperandValue<std::string>(condition.left) == "id");
    assert(condition.op == ComparisonOperator::GreaterEqual);

    const Value& right = asOperandValue<Value>(condition.right);
    assert(asValue<int>(right) == 10);
}

static void testParsesConstantLeftSideWhereComparison() {
    const Parser parser;
    const SelectCommand command = asCommand<SelectCommand>(
        parser.parse("SELECT * FROM users WHERE 10 <= age;")
    );

    assert(command.where.has_value());
    const ComparisonCondition& condition = std::get<ComparisonCondition>(*command.where);

    const Value& left = asOperandValue<Value>(condition.left);
    assert(asValue<int>(left) == 10);
    assert(condition.op == ComparisonOperator::LessEqual);
    assert(asOperandValue<std::string>(condition.right) == "age");
}

static void testRejectsInvalidDml() {
    const Parser parser;

    expectParseError([&]() {
        (void)parser.parse("INSERT INTO users () VALUE (1);");
    });

    expectParseError([&]() {
        (void)parser.parse("INSERT INTO users (id) VALUES (1);");
    });

    expectParseError([&]() {
        (void)parser.parse("SELECT () FROM users;");
    });

    expectParseError([&]() {
        (void)parser.parse("SELECT * users;");
    });

    expectParseError([&]() {
        (void)parser.parse("SELECT * FROM users WHERE id 10;");
    });
}

int main() {
    testParsesSingleRowInsert();
    testParsesMultiRowInsertWithNull();
    testParsesSelectAll();
    testParsesSelectItemsWithAliases();
    testParsesSelectWithWhereComparison();
    testParsesConstantLeftSideWhereComparison();
    testRejectsInvalidDml();
    return 0;
}
