#include "ConditionEvaluator.h"

#include <map>
#include <regex>
#include <string>
#include <variant>

#include "EngineError.h"

namespace {

using ComparableValue = std::variant<int, std::string>;

std::map<std::string, std::size_t> buildColumnIndex(const TableSchema& schema) {
    std::map<std::string, std::size_t> columns;
    for (std::size_t i = 0; i < schema.columns.size(); ++i) {
        columns[schema.columns[i].name] = i;
    }
    return columns;
}

ComparableValue fieldToValue(const Field& field) {
    if (field.isInt()) {
        return field.intValue;
    }
    if (field.isString()) {
        return field.stringValue;
    }

    throw EngineError("Unsupported field type in condition");
}

ComparableValue literalToValue(const Value& value) {
    if (std::holds_alternative<int>(value)) {
        return std::get<int>(value);
    }
    if (std::holds_alternative<std::string>(value)) {
        return std::get<std::string>(value);
    }

    throw EngineError("NULL cannot be used in conditions until NULL storage is implemented");
}

ComparableValue resolveOperand(const TableSchema& schema,
                               const Record& record,
                               const std::map<std::string, std::size_t>& columnIndex,
                               const Operand& operand) {
    if (std::holds_alternative<std::string>(operand.value)) {
        const std::string& columnName = std::get<std::string>(operand.value);
        const auto it = columnIndex.find(columnName);
        if (it == columnIndex.end()) {
            throw EngineError("Column '" + columnName + "' does not exist");
        }

        const std::size_t index = it->second;
        if (index >= record.fields.size() || index >= schema.columns.size()) {
            throw EngineError("Record does not match table schema");
        }

        return fieldToValue(record.fields[index]);
    }

    return literalToValue(std::get<Value>(operand.value));
}

int compareValues(const ComparableValue& left, const ComparableValue& right) {
    if (left.index() != right.index()) {
        throw EngineError("Condition operands have incompatible types");
    }

    if (std::holds_alternative<int>(left)) {
        const int lhs = std::get<int>(left);
        const int rhs = std::get<int>(right);
        if (lhs < rhs) {
            return -1;
        }
        if (lhs > rhs) {
            return 1;
        }
        return 0;
    }

    const std::string& lhs = std::get<std::string>(left);
    const std::string& rhs = std::get<std::string>(right);
    if (lhs < rhs) {
        return -1;
    }
    if (lhs > rhs) {
        return 1;
    }
    return 0;
}

bool evaluateComparison(ComparisonOperator op, int comparison) {
    switch (op) {
    case ComparisonOperator::Equal:
        return comparison == 0;
    case ComparisonOperator::NotEqual:
        return comparison != 0;
    case ComparisonOperator::Less:
        return comparison < 0;
    case ComparisonOperator::Greater:
        return comparison > 0;
    case ComparisonOperator::LessEqual:
        return comparison <= 0;
    case ComparisonOperator::GreaterEqual:
        return comparison >= 0;
    }

    throw EngineError("Unsupported comparison operator");
}

bool evaluateComparisonCondition(const TableSchema& schema,
                                 const Record& record,
                                 const std::map<std::string, std::size_t>& columnIndex,
                                 const ComparisonCondition& condition) {
    const ComparableValue left = resolveOperand(schema, record, columnIndex, condition.left);
    const ComparableValue right = resolveOperand(schema, record, columnIndex, condition.right);
    return evaluateComparison(condition.op, compareValues(left, right));
}

bool evaluateBetweenCondition(const TableSchema& schema,
                              const Record& record,
                              const std::map<std::string, std::size_t>& columnIndex,
                              const BetweenCondition& condition) {
    const ComparableValue value = resolveOperand(schema, record, columnIndex, condition.value);
    const ComparableValue lowerBound = resolveOperand(schema, record, columnIndex, condition.lowerBound);
    const ComparableValue upperBound = resolveOperand(schema, record, columnIndex, condition.upperBound);

    return compareValues(value, lowerBound) >= 0 &&
        compareValues(value, upperBound) < 0;
}

bool evaluateLikeCondition(const TableSchema& schema,
                           const Record& record,
                           const std::map<std::string, std::size_t>& columnIndex,
                           const LikeCondition& condition) {
    const ComparableValue value = resolveOperand(schema, record, columnIndex, condition.value);
    const ComparableValue pattern = resolveOperand(schema, record, columnIndex, condition.pattern);

    if (!std::holds_alternative<std::string>(value) ||
        !std::holds_alternative<std::string>(pattern)) {
        throw EngineError("LIKE operands must be STRING");
    }

    try {
        return std::regex_match(std::get<std::string>(value),
                                std::regex(std::get<std::string>(pattern)));
    } catch (const std::regex_error&) {
        throw EngineError("Invalid LIKE regular expression");
    }
}

}

bool ConditionEvaluator::evaluate(const TableSchema& schema,
                                  const Record& record,
                                  const Condition& condition) const {
    const std::map<std::string, std::size_t> columnIndex = buildColumnIndex(schema);

    if (std::holds_alternative<ComparisonCondition>(condition)) {
        return evaluateComparisonCondition(schema, record, columnIndex,
                                           std::get<ComparisonCondition>(condition));
    }
    if (std::holds_alternative<BetweenCondition>(condition)) {
        return evaluateBetweenCondition(schema, record, columnIndex,
                                        std::get<BetweenCondition>(condition));
    }
    if (std::holds_alternative<LikeCondition>(condition)) {
        return evaluateLikeCondition(schema, record, columnIndex,
                                     std::get<LikeCondition>(condition));
    }

    throw EngineError("Unsupported condition");
}
