#include <cassert>
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

static void testParsesUpdate() {
    const Parser parser;
    const UpdateCommand command = asCommand<UpdateCommand>(
        parser.parse("UPDATE users SET name = \"Bob\", age = 30 WHERE id == 1;")
    );

    assert(command.tableName.tableName == "users");
    assert(command.assignments.size() == 2);
    assert(command.assignments[0].columnName == "name");
    assert(asValue<std::string>(command.assignments[0].value) == "Bob");
    assert(command.assignments[1].columnName == "age");
    assert(asValue<int>(command.assignments[1].value) == 30);

    const ComparisonCondition& condition = std::get<ComparisonCondition>(command.where);
    assert(asOperandValue<std::string>(condition.left) == "id");
    assert(condition.op == ComparisonOperator::Equal);
    const Value& right = asOperandValue<Value>(condition.right);
    assert(asValue<int>(right) == 1);
}

static void testParsesDelete() {
    const Parser parser;
    const DeleteCommand command = asCommand<DeleteCommand>(
        parser.parse("DELETE FROM app.users WHERE id != 10;")
    );

    assert(command.tableName.databaseName.has_value());
    assert(*command.tableName.databaseName == "app");
    assert(command.tableName.tableName == "users");

    const ComparisonCondition& condition = std::get<ComparisonCondition>(command.where);
    assert(asOperandValue<std::string>(condition.left) == "id");
    assert(condition.op == ComparisonOperator::NotEqual);
}

static void testParsesBetweenCondition() {
    const Parser parser;
    const SelectCommand command = asCommand<SelectCommand>(
        parser.parse("SELECT * FROM users WHERE age BETWEEN 18 AND 30;")
    );

    assert(command.where.has_value());
    const BetweenCondition& condition = std::get<BetweenCondition>(*command.where);
    assert(asOperandValue<std::string>(condition.value) == "age");

    const Value& lower = asOperandValue<Value>(condition.lowerBound);
    const Value& upper = asOperandValue<Value>(condition.upperBound);
    assert(asValue<int>(lower) == 18);
    assert(asValue<int>(upper) == 30);
}

static void testParsesLikeCondition() {
    const Parser parser;
    const SelectCommand command = asCommand<SelectCommand>(
        parser.parse("SELECT * FROM users WHERE name LIKE \"A.*\";")
    );

    assert(command.where.has_value());
    const LikeCondition& condition = std::get<LikeCondition>(*command.where);
    assert(asOperandValue<std::string>(condition.value) == "name");

    const Value& pattern = asOperandValue<Value>(condition.pattern);
    assert(asValue<std::string>(pattern) == "A.*");
}

static void testParsesConstantBetweenAndLikeOperands() {
    const Parser parser;

    const SelectCommand between = asCommand<SelectCommand>(
        parser.parse("SELECT * FROM users WHERE 20 BETWEEN min_age AND max_age;")
    );
    const BetweenCondition& betweenCondition = std::get<BetweenCondition>(*between.where);
    const Value& value = asOperandValue<Value>(betweenCondition.value);
    assert(asValue<int>(value) == 20);
    assert(asOperandValue<std::string>(betweenCondition.lowerBound) == "min_age");
    assert(asOperandValue<std::string>(betweenCondition.upperBound) == "max_age");

    const SelectCommand like = asCommand<SelectCommand>(
        parser.parse("SELECT * FROM users WHERE \"Alice\" LIKE pattern;")
    );
    const LikeCondition& likeCondition = std::get<LikeCondition>(*like.where);
    const Value& left = asOperandValue<Value>(likeCondition.value);
    assert(asValue<std::string>(left) == "Alice");
    assert(asOperandValue<std::string>(likeCondition.pattern) == "pattern");
}

static void testRejectsInvalidUpdateDeleteAndConditions() {
    const Parser parser;

    expectParseError([&]() {
        (void)parser.parse("UPDATE users SET WHERE id == 1;");
    });

    expectParseError([&]() {
        (void)parser.parse("UPDATE users SET name == \"Bob\" WHERE id == 1;");
    });

    expectParseError([&]() {
        (void)parser.parse("DELETE FROM users;");
    });

    expectParseError([&]() {
        (void)parser.parse("SELECT * FROM users WHERE age BETWEEN 18 30;");
    });

    expectParseError([&]() {
        (void)parser.parse("SELECT * FROM users WHERE name LIKE;");
    });
}

int main() {
    testParsesUpdate();
    testParsesDelete();
    testParsesBetweenCondition();
    testParsesLikeCondition();
    testParsesConstantBetweenAndLikeOperands();
    testRejectsInvalidUpdateDeleteAndConditions();
    return 0;
}
