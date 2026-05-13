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

using ParsedCommand = std::variant<
    CreateDatabaseCommand,
    DropDatabaseCommand,
    UseDatabaseCommand,
    CreateTableCommand,
    DropTableCommand
>;
