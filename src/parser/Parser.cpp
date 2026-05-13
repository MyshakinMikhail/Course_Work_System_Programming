#include "Parser.h"

#include <cstdlib>
#include <limits>
#include <sstream>
#include <utility>

#include "ParseError.h"
#include "Tokenizer.h"

namespace {

class ParserState {
public:
    explicit ParserState(const std::vector<Token>& tokens)
        : tokens_(tokens) {}

    ParsedCommand parseCommand() {
        ParsedCommand command;
        if (match(TokenType::Create)) {
            command = parseCreate();
        } else if (match(TokenType::Drop)) {
            command = parseDrop();
        } else if (match(TokenType::Use)) {
            command = parseUseDatabase();
        } else if (match(TokenType::Insert)) {
            command = parseInsert();
        } else if (match(TokenType::Update)) {
            command = parseUpdate();
        } else if (match(TokenType::Delete)) {
            command = parseDelete();
        } else if (match(TokenType::Select)) {
            command = parseSelect();
        } else {
            throw errorAtCurrent("expected command keyword");
        }

        expect(TokenType::Semicolon, "expected ';' after command");
        expect(TokenType::End, "expected end of input after ';'");
        return command;
    }

private:
    const std::vector<Token>& tokens_;
    std::size_t current_{0};

    ParsedCommand parseCreate() {
        if (match(TokenType::Database)) {
            return CreateDatabaseCommand{expectIdentifier("expected database name")};
        }

        if (match(TokenType::Table)) {
            CreateTableCommand command;
            command.tableName = parseTableName();
            expect(TokenType::LeftParen, "expected '(' before column list");

            if (check(TokenType::RightParen)) {
                throw errorAtCurrent("table must contain at least one column");
            }

            do {
                command.columns.push_back(parseColumnDef());
            } while (match(TokenType::Comma));

            expect(TokenType::RightParen, "expected ')' after column list");
            return command;
        }

        throw errorAtCurrent("expected DATABASE or TABLE after CREATE");
    }

    ParsedCommand parseDrop() {
        if (match(TokenType::Database)) {
            return DropDatabaseCommand{expectIdentifier("expected database name")};
        }

        if (match(TokenType::Table)) {
            return DropTableCommand{parseTableName()};
        }

        throw errorAtCurrent("expected DATABASE or TABLE after DROP");
    }

    ParsedCommand parseUseDatabase() {
        UseDatabaseCommand command;
        command.databaseName = expectIdentifier("expected database name");
        return command;
    }

    ParsedCommand parseInsert() {
        expect(TokenType::Into, "expected INTO after INSERT");

        InsertCommand command;
        command.tableName = parseTableName();
        command.columns = parseIdentifierList("expected column name");

        expect(TokenType::Value, "expected VALUE after INSERT column list");

        do {
            command.rows.push_back(parseLiteralValueList());
        } while (match(TokenType::Comma));

        return command;
    }

    ParsedCommand parseUpdate() {
        UpdateCommand command;
        command.tableName = parseTableName();
        expect(TokenType::Set, "expected SET after table name");

        do {
            Assignment assignment;
            assignment.columnName = expectIdentifier("expected column name in assignment");
            expect(TokenType::Equal, "expected '=' in assignment");
            assignment.value = parseLiteralValue();
            command.assignments.push_back(std::move(assignment));
        } while (match(TokenType::Comma));

        expect(TokenType::Where, "expected WHERE after UPDATE assignments");
        command.where = parseCondition();
        return command;
    }

    ParsedCommand parseDelete() {
        expect(TokenType::From, "expected FROM after DELETE");

        DeleteCommand command;
        command.tableName = parseTableName();
        expect(TokenType::Where, "expected WHERE after DELETE table name");
        command.where = parseCondition();
        return command;
    }

    ParsedCommand parseSelect() {
        SelectCommand command;

        if (match(TokenType::Star)) {
            command.selectAll = true;
        } else {
            expect(TokenType::LeftParen, "expected '*' or '(' after SELECT");
            if (check(TokenType::RightParen)) {
                throw errorAtCurrent("SELECT column list cannot be empty");
            }

            do {
                command.items.push_back(parseSelectItem());
            } while (match(TokenType::Comma));

            expect(TokenType::RightParen, "expected ')' after SELECT column list");
        }

        expect(TokenType::From, "expected FROM after SELECT list");
        command.tableName = parseTableName();

        if (match(TokenType::Where)) {
            command.where = parseCondition();
        }

        return command;
    }

    TableName parseTableName() {
        const std::string firstName = expectIdentifier("expected table name");
        if (!match(TokenType::Dot)) {
            return TableName{std::nullopt, firstName};
        }

        const std::string secondName = expectIdentifier("expected table name after database name");
        return TableName{firstName, secondName};
    }

    ColumnDef parseColumnDef() {
        ColumnDef column;
        column.name = expectIdentifier("expected column name");

        if (match(TokenType::Int)) {
            column.type = AstFieldType::Int;
        } else if (match(TokenType::String)) {
            column.type = AstFieldType::String;
        } else {
            throw errorAtCurrent("expected column type");
        }

        while (match(TokenType::NotNull) || match(TokenType::Indexed)) {
            const Token& modifier = previous();
            if (modifier.type == TokenType::NotNull) {
                if (column.notNull) {
                    throw errorAt(modifier, "duplicate NOT_NULL modifier");
                }
                column.notNull = true;
            } else if (modifier.type == TokenType::Indexed) {
                if (column.indexed) {
                    throw errorAt(modifier, "duplicate INDEXED modifier");
                }
                column.indexed = true;
            }
        }

        return column;
    }

    std::vector<std::string> parseIdentifierList(const std::string& itemMessage) {
        std::vector<std::string> values;
        expect(TokenType::LeftParen, "expected '(' before list");

        if (check(TokenType::RightParen)) {
            throw errorAtCurrent("list cannot be empty");
        }

        do {
            values.push_back(expectIdentifier(itemMessage));
        } while (match(TokenType::Comma));

        expect(TokenType::RightParen, "expected ')' after list");
        return values;
    }

    std::vector<Value> parseLiteralValueList() {
        std::vector<Value> values;
        expect(TokenType::LeftParen, "expected '(' before value list");

        if (check(TokenType::RightParen)) {
            throw errorAtCurrent("value list cannot be empty");
        }

        do {
            values.push_back(parseLiteralValue());
        } while (match(TokenType::Comma));

        expect(TokenType::RightParen, "expected ')' after value list");
        return values;
    }

    SelectItem parseSelectItem() {
        SelectItem item;
        item.columnName = expectIdentifier("expected selected column name");

        if (match(TokenType::As)) {
            item.alias = expectIdentifier("expected alias after AS");
        }

        return item;
    }

    Condition parseCondition() {
        const Operand left = parseOperand();

        if (match(TokenType::Between)) {
            BetweenCondition condition;
            condition.value = left;
            condition.lowerBound = parseOperand();
            expect(TokenType::And, "expected AND in BETWEEN condition");
            condition.upperBound = parseOperand();
            return condition;
        }

        if (match(TokenType::Like)) {
            LikeCondition condition;
            condition.value = left;
            condition.pattern = parseOperand();
            return condition;
        }

        ComparisonCondition condition;
        condition.left = left;
        condition.op = parseComparisonOperator();
        condition.right = parseOperand();
        return condition;
    }

    Operand parseOperand() {
        if (check(TokenType::Identifier)) {
            return Operand::Column(advance().lexeme);
        }

        return Operand::Literal(parseLiteralValue());
    }

    ComparisonOperator parseComparisonOperator() {
        if (match(TokenType::Equal) || match(TokenType::EqualEqual)) {
            return ComparisonOperator::Equal;
        }
        if (match(TokenType::NotEqual)) {
            return ComparisonOperator::NotEqual;
        }
        if (match(TokenType::Less)) {
            return ComparisonOperator::Less;
        }
        if (match(TokenType::Greater)) {
            return ComparisonOperator::Greater;
        }
        if (match(TokenType::LessEqual)) {
            return ComparisonOperator::LessEqual;
        }
        if (match(TokenType::GreaterEqual)) {
            return ComparisonOperator::GreaterEqual;
        }

        throw errorAtCurrent("expected comparison operator");
    }

    Value parseLiteralValue() {
        if (match(TokenType::IntegerLiteral)) {
            return parseInteger(previous());
        }

        if (match(TokenType::StringLiteral)) {
            return previous().lexeme;
        }

        if (match(TokenType::Null)) {
            return NullValue{};
        }

        throw errorAtCurrent("expected literal value");
    }

    int parseInteger(const Token& token) const {
        char* end = nullptr;
        const long value = std::strtol(token.lexeme.c_str(), &end, 10);
        if (end == token.lexeme.c_str() || *end != '\0' ||
            value < std::numeric_limits<int>::min() ||
            value > std::numeric_limits<int>::max()) {
            throw errorAt(token, "integer literal is out of range");
        }

        return static_cast<int>(value);
    }

    bool match(TokenType type) {
        if (!check(type)) {
            return false;
        }

        ++current_;
        return true;
    }

    bool check(TokenType type) const {
        return current_ < tokens_.size() && tokens_[current_].type == type;
    }

    const Token& expect(TokenType type, const std::string& message) {
        if (check(type)) {
            return advance();
        }

        throw errorAtCurrent(message);
    }

    std::string expectIdentifier(const std::string& message) {
        return expect(TokenType::Identifier, message).lexeme;
    }

    const Token& advance() {
        const Token& token = tokens_[current_];
        ++current_;
        return token;
    }

    const Token& previous() const {
        return tokens_[current_ - 1];
    }

    ParseError errorAtCurrent(const std::string& message) const {
        if (current_ < tokens_.size()) {
            return errorAt(tokens_[current_], message);
        }

        std::ostringstream stream;
        stream << "Parse error at end of input: " << message;
        return ParseError(stream.str());
    }

    ParseError errorAt(const Token& token, const std::string& message) const {
        std::ostringstream stream;
        stream << "Parse error at position " << token.position << ": "
               << message << ", got " << tokenTypeName(token.type);
        if (!token.lexeme.empty()) {
            stream << " '" << token.lexeme << "'";
        }
        return ParseError(stream.str());
    }
};

}

ParsedCommand Parser::parse(const std::string& sql) const {
    const Tokenizer tokenizer;
    return parse(tokenizer.tokenize(sql));
}

ParsedCommand Parser::parse(const std::vector<Token>& tokens) const {
    ParserState state(tokens);
    return state.parseCommand();
}
