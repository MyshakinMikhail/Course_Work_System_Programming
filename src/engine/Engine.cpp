#include "Engine.h"

#include <filesystem>
#include <map>
#include <set>
#include <string>
#include <utility>
#include <variant>
#include <vector>

#include "ConditionEvaluator.h"
#include "EngineError.h"
#include "JsonFormatter.h"
#include "Table.h"

namespace {

std::string notImplemented(const std::string& commandName) {
    return commandName + " is not implemented yet";
}

FieldType toCoreFieldType(AstFieldType type) {
    switch (type) {
    case AstFieldType::Int:
        return FieldType::INT;
    case AstFieldType::String:
        return FieldType::STRING;
    }

    throw EngineError("Unsupported column type");
}

TableSchema toCoreSchema(const CreateTableCommand& command) {
    TableSchema schema;
    std::set<std::string> columnNames;

    for (const ColumnDef& column : command.columns) {
        if (!columnNames.insert(column.name).second) {
            throw EngineError("Duplicate column '" + column.name + "'");
        }

        schema.addColumn(Column(column.name, toCoreFieldType(column.type), column.indexed));
    }

    return schema;
}

std::string qualifiedName(const ResolvedTableName& tableName) {
    return tableName.databaseName + "." + tableName.tableName;
}

std::map<std::string, std::size_t> buildColumnIndex(const TableSchema& schema) {
    std::map<std::string, std::size_t> columns;
    for (std::size_t i = 0; i < schema.columns.size(); ++i) {
        columns[schema.columns[i].name] = i;
    }
    return columns;
}

Field valueToField(const Value& value, FieldType expectedType) {
    if (std::holds_alternative<NullValue>(value)) {
        throw EngineError("NULL values are not supported by storage yet");
    }

    if (expectedType == FieldType::INT) {
        if (!std::holds_alternative<int>(value)) {
            throw EngineError("Expected INT value");
        }
        return Field::Int(std::get<int>(value));
    }

    if (expectedType == FieldType::STRING) {
        if (!std::holds_alternative<std::string>(value)) {
            throw EngineError("Expected STRING value");
        }
        return Field::String(std::get<std::string>(value));
    }

    throw EngineError("Unsupported field type");
}

std::vector<std::size_t> resolveInsertColumns(const TableSchema& schema,
                                              const std::vector<std::string>& columns) {
    const std::map<std::string, std::size_t> columnIndex = buildColumnIndex(schema);
    std::set<std::string> seen;
    std::vector<std::size_t> indexes;
    indexes.reserve(columns.size());

    for (const std::string& columnName : columns) {
        if (!seen.insert(columnName).second) {
            throw EngineError("Duplicate column '" + columnName + "' in INSERT");
        }

        const auto it = columnIndex.find(columnName);
        if (it == columnIndex.end()) {
            throw EngineError("Column '" + columnName + "' does not exist");
        }
        indexes.push_back(it->second);
    }

    return indexes;
}

Record buildInsertRecord(const TableSchema& schema,
                         const std::vector<std::size_t>& insertColumnIndexes,
                         const std::vector<Value>& values) {
    if (insertColumnIndexes.size() != values.size()) {
        throw EngineError("INSERT column count does not match value count");
    }

    if (insertColumnIndexes.size() != schema.columns.size()) {
        throw EngineError("INSERT must provide values for all columns until NULL storage is implemented");
    }

    std::vector<std::optional<Field>> fields(schema.columns.size());
    for (std::size_t i = 0; i < values.size(); ++i) {
        const std::size_t columnIndex = insertColumnIndexes[i];
        fields[columnIndex] = valueToField(values[i], schema.columns[columnIndex].type);
    }

    Record record;
    record.fields.reserve(fields.size());
    for (std::size_t i = 0; i < fields.size(); ++i) {
        if (!fields[i].has_value()) {
            throw EngineError("Missing value for column '" + schema.columns[i].name + "'");
        }
        record.fields.push_back(*fields[i]);
    }
    return record;
}

}

Engine::Engine() = default;

Engine::Engine(std::filesystem::path dataRoot)
    : catalog_(std::move(dataRoot)) {}

ExecutionResult Engine::execute(const ParsedCommand& command) {
    try {
        return std::visit([this](const auto& concreteCommand) {
            using Command = std::decay_t<decltype(concreteCommand)>;

            if constexpr (std::is_same_v<Command, CreateDatabaseCommand>) {
                return executeCreateDatabase(concreteCommand);
            } else if constexpr (std::is_same_v<Command, DropDatabaseCommand>) {
                return executeDropDatabase(concreteCommand);
            } else if constexpr (std::is_same_v<Command, UseDatabaseCommand>) {
                return executeUseDatabase(concreteCommand);
            } else if constexpr (std::is_same_v<Command, CreateTableCommand>) {
                return executeCreateTable(concreteCommand);
            } else if constexpr (std::is_same_v<Command, DropTableCommand>) {
                return executeDropTable(concreteCommand);
            } else if constexpr (std::is_same_v<Command, InsertCommand>) {
                return executeInsert(concreteCommand);
            } else if constexpr (std::is_same_v<Command, SelectCommand>) {
                return executeSelect(concreteCommand);
            } else if constexpr (std::is_same_v<Command, UpdateCommand>) {
                return executeUpdate(concreteCommand);
            } else if constexpr (std::is_same_v<Command, DeleteCommand>) {
                return executeDelete(concreteCommand);
            }
        }, command);
    } catch (const EngineError& error) {
        return ExecutionResult::Error(error.what());
    } catch (const std::exception& error) {
        return ExecutionResult::Error(error.what());
    }
}

ExecutionResult Engine::executeCreateDatabase(const CreateDatabaseCommand& command) {
    catalog_.createDatabase(command.databaseName);
    return ExecutionResult::Success("Database '" + command.databaseName + "' created");
}

ExecutionResult Engine::executeDropDatabase(const DropDatabaseCommand& command) {
    catalog_.dropDatabase(command.databaseName);
    return ExecutionResult::Success("Database '" + command.databaseName + "' dropped");
}

ExecutionResult Engine::executeUseDatabase(const UseDatabaseCommand& command) {
    catalog_.useDatabase(command.databaseName);
    return ExecutionResult::Success("Using database '" + command.databaseName + "'");
}

ExecutionResult Engine::executeCreateTable(const CreateTableCommand& command) {
    const ResolvedTableName resolved = catalog_.resolveTableName(command.tableName);
    catalog_.createTable(command.tableName, toCoreSchema(command));
    return ExecutionResult::Success("Table '" + qualifiedName(resolved) + "' created");
}

ExecutionResult Engine::executeDropTable(const DropTableCommand& command) {
    const ResolvedTableName resolved = catalog_.resolveTableName(command.tableName);
    catalog_.dropTable(command.tableName);
    return ExecutionResult::Success("Table '" + qualifiedName(resolved) + "' dropped");
}

ExecutionResult Engine::executeInsert(const InsertCommand& command) {
    const ResolvedTableName resolved = catalog_.resolveTableName(command.tableName);
    const TableSchema schema = catalog_.readTableSchema(command.tableName);
    const std::vector<std::size_t> insertColumnIndexes =
        resolveInsertColumns(schema, command.columns);

    Table table(resolved.tablePath, schema);
    for (const std::vector<Value>& row : command.rows) {
        table.insert(buildInsertRecord(schema, insertColumnIndexes, row));
    }

    return ExecutionResult::Success(
        "Inserted " + std::to_string(command.rows.size()) +
        " row(s) into '" + qualifiedName(resolved) + "'"
    );
}

ExecutionResult Engine::executeSelect(const SelectCommand& command) {
    const ResolvedTableName resolved = catalog_.resolveTableName(command.tableName);
    const TableSchema schema = catalog_.readTableSchema(command.tableName);
    Table table(resolved.tablePath, schema);
    std::vector<Record> records = table.scan();

    if (command.where.has_value()) {
        const ConditionEvaluator evaluator;
        std::vector<Record> filteredRecords;
        for (const Record& record : records) {
            if (evaluator.evaluate(schema, record, *command.where)) {
                filteredRecords.push_back(record);
            }
        }
        records = std::move(filteredRecords);
    }

    const JsonFormatter formatter;
    return ExecutionResult::Json(
        formatter.formatRows(schema, records, command.selectAll, command.items)
    );
}

ExecutionResult Engine::executeUpdate(const UpdateCommand&) {
    return ExecutionResult::Success(notImplemented("UPDATE"));
}

ExecutionResult Engine::executeDelete(const DeleteCommand&) {
    return ExecutionResult::Success(notImplemented("DELETE"));
}
