#pragma once

#include <optional>
#include <string>
#include <variant>
#include <vector>

enum class AstFieldType {
    Int,
    String
};

struct TableName {
    std::optional<std::string> databaseName;
    std::string tableName;
};

struct ColumnDef {
    std::string name;
    AstFieldType type{AstFieldType::Int};
    bool notNull{false};
    bool indexed{false};
};

struct CreateDatabaseCommand {
    std::string databaseName;
};

struct DropDatabaseCommand {
    std::string databaseName;
};

struct UseDatabaseCommand {
    std::string databaseName;
};

struct CreateTableCommand {
    TableName tableName;
    std::vector<ColumnDef> columns;
};

struct DropTableCommand {
    TableName tableName;
};

struct NullValue {};

using Value = std::variant<int, std::string, NullValue>;

struct InsertCommand {
    TableName tableName;
    std::vector<std::string> columns;
    std::vector<std::vector<Value>> rows;
};

struct SelectItem {
    std::string columnName;
    std::optional<std::string> alias;
};

enum class ComparisonOperator {
    Equal,
    NotEqual,
    Less,
    Greater,
    LessEqual,
    GreaterEqual
};

struct Operand {
    std::variant<std::string, Value> value;

    static Operand Column(std::string name) {
        return Operand{std::move(name)};
    }

    static Operand Literal(Value literal) {
        return Operand{std::move(literal)};
    }
};

struct ComparisonCondition {
    Operand left;
    ComparisonOperator op{ComparisonOperator::Equal};
    Operand right;
};

using Condition = std::variant<ComparisonCondition>;

struct SelectCommand {
    TableName tableName;
    bool selectAll{false};
    std::vector<SelectItem> items;
    std::optional<Condition> where;
};

using ParsedCommand = std::variant<
    CreateDatabaseCommand,
    DropDatabaseCommand,
    UseDatabaseCommand,
    CreateTableCommand,
    DropTableCommand,
    InsertCommand,
    SelectCommand
>;
