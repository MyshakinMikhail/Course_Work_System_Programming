#include "Parser.h"

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
        return UseDatabaseCommand{expectIdentifier("expected database name")};
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
