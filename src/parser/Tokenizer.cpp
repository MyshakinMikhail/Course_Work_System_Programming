#include "Tokenizer.h"

#include <cctype>
#include <map>
#include <sstream>
#include <string>

#include "ParseError.h"

namespace {

bool isIdentifierStart(char ch) {
    return std::isalpha(static_cast<unsigned char>(ch)) || ch == '_';
}

bool isIdentifierPart(char ch) {
    return std::isalnum(static_cast<unsigned char>(ch)) || ch == '_';
}

bool isAllLower(const std::string& value) {
    for (const char ch : value) {
        if (std::isalpha(static_cast<unsigned char>(ch)) &&
            !std::islower(static_cast<unsigned char>(ch))) {
            return false;
        }
    }
    return true;
}

bool isAllUpper(const std::string& value) {
    for (const char ch : value) {
        if (std::isalpha(static_cast<unsigned char>(ch)) &&
            !std::isupper(static_cast<unsigned char>(ch))) {
            return false;
        }
    }
    return true;
}

std::string toUpper(std::string value) {
    for (char& ch : value) {
        ch = static_cast<char>(std::toupper(static_cast<unsigned char>(ch)));
    }
    return value;
}

ParseError errorAt(std::size_t position, const std::string& message) {
    std::ostringstream stream;
    stream << "Parse error at position " << position << ": " << message;
    return ParseError(stream.str());
}

const std::map<std::string, TokenType>& keywords() {
    static const std::map<std::string, TokenType> values{
        {"CREATE", TokenType::Create},
        {"DATABASE", TokenType::Database},
        {"DROP", TokenType::Drop},
        {"USE", TokenType::Use},
        {"TABLE", TokenType::Table},
        {"INSERT", TokenType::Insert},
        {"INTO", TokenType::Into},
        {"VALUE", TokenType::Value},
        {"UPDATE", TokenType::Update},
        {"SET", TokenType::Set},
        {"DELETE", TokenType::Delete},
        {"FROM", TokenType::From},
        {"SELECT", TokenType::Select},
        {"WHERE", TokenType::Where},
        {"AS", TokenType::As},
        {"INT", TokenType::Int},
        {"STRING", TokenType::String},
        {"NOT_NULL", TokenType::NotNull},
        {"INDEXED", TokenType::Indexed},
        {"BETWEEN", TokenType::Between},
        {"AND", TokenType::And},
        {"LIKE", TokenType::Like},
        {"NULL", TokenType::Null}
    };

    return values;
}

Token makeToken(TokenType type, std::string lexeme, std::size_t position) {
    return Token{type, std::move(lexeme), position};
}

}

std::string tokenTypeName(TokenType type) {
    switch (type) {
    case TokenType::Identifier: return "Identifier";
    case TokenType::IntegerLiteral: return "IntegerLiteral";
    case TokenType::StringLiteral: return "StringLiteral";
    case TokenType::Create: return "CREATE";
    case TokenType::Database: return "DATABASE";
    case TokenType::Drop: return "DROP";
    case TokenType::Use: return "USE";
    case TokenType::Table: return "TABLE";
    case TokenType::Insert: return "INSERT";
    case TokenType::Into: return "INTO";
    case TokenType::Value: return "VALUE";
    case TokenType::Update: return "UPDATE";
    case TokenType::Set: return "SET";
    case TokenType::Delete: return "DELETE";
    case TokenType::From: return "FROM";
    case TokenType::Select: return "SELECT";
    case TokenType::Where: return "WHERE";
    case TokenType::As: return "AS";
    case TokenType::Int: return "INT";
    case TokenType::String: return "STRING";
    case TokenType::NotNull: return "NOT_NULL";
    case TokenType::Indexed: return "INDEXED";
    case TokenType::Between: return "BETWEEN";
    case TokenType::And: return "AND";
    case TokenType::Like: return "LIKE";
    case TokenType::Null: return "NULL";
    case TokenType::LeftParen: return "LeftParen";
    case TokenType::RightParen: return "RightParen";
    case TokenType::Comma: return "Comma";
    case TokenType::Dot: return "Dot";
    case TokenType::Semicolon: return "Semicolon";
    case TokenType::Star: return "Star";
    case TokenType::Equal: return "Equal";
    case TokenType::EqualEqual: return "EqualEqual";
    case TokenType::NotEqual: return "NotEqual";
    case TokenType::Less: return "Less";
    case TokenType::Greater: return "Greater";
    case TokenType::LessEqual: return "LessEqual";
    case TokenType::GreaterEqual: return "GreaterEqual";
    case TokenType::End: return "End";
    }

    return "Unknown";
}

std::vector<Token> Tokenizer::tokenize(const std::string& sql) const {
    std::vector<Token> tokens;
    std::size_t position = 0;

    while (position < sql.size()) {
        const char ch = sql[position];

        if (std::isspace(static_cast<unsigned char>(ch))) {
            ++position;
            continue;
        }

        if (isIdentifierStart(ch)) {
            const std::size_t start = position;
            ++position;
            while (position < sql.size() && isIdentifierPart(sql[position])) {
                ++position;
            }

            const std::string lexeme = sql.substr(start, position - start);
            const std::string upperLexeme = toUpper(lexeme);
            const auto keyword = keywords().find(upperLexeme);
            if (keyword != keywords().end()) {
                if (!isAllLower(lexeme) && !isAllUpper(lexeme)) {
                    throw errorAt(start, "keyword uses mixed case: " + lexeme);
                }
                tokens.push_back(makeToken(keyword->second, lexeme, start));
            } else {
                tokens.push_back(makeToken(TokenType::Identifier, lexeme, start));
            }
            continue;
        }

        if (std::isdigit(static_cast<unsigned char>(ch))) {
            const std::size_t start = position;
            ++position;
            while (position < sql.size() &&
                   std::isdigit(static_cast<unsigned char>(sql[position]))) {
                ++position;
            }

            if (position < sql.size() && isIdentifierStart(sql[position])) {
                throw errorAt(start, "identifier cannot start with a digit");
            }

            tokens.push_back(makeToken(TokenType::IntegerLiteral,
                                       sql.substr(start, position - start),
                                       start));
            continue;
        }

        if (ch == '"') {
            const std::size_t start = position;
            ++position;
            std::string value;
            while (position < sql.size() && sql[position] != '"') {
                value.push_back(sql[position]);
                ++position;
            }

            if (position >= sql.size()) {
                throw errorAt(start, "unterminated string literal");
            }

            ++position;
            tokens.push_back(makeToken(TokenType::StringLiteral, value, start));
            continue;
        }

        switch (ch) {
        case '(':
            tokens.push_back(makeToken(TokenType::LeftParen, "(", position++));
            break;
        case ')':
            tokens.push_back(makeToken(TokenType::RightParen, ")", position++));
            break;
        case ',':
            tokens.push_back(makeToken(TokenType::Comma, ",", position++));
            break;
        case '.':
            tokens.push_back(makeToken(TokenType::Dot, ".", position++));
            break;
        case ';':
            tokens.push_back(makeToken(TokenType::Semicolon, ";", position++));
            break;
        case '*':
            tokens.push_back(makeToken(TokenType::Star, "*", position++));
            break;
        case '=':
            if (position + 1 < sql.size() && sql[position + 1] == '=') {
                tokens.push_back(makeToken(TokenType::EqualEqual, "==", position));
                position += 2;
            } else {
                tokens.push_back(makeToken(TokenType::Equal, "=", position++));
            }
            break;
        case '!':
            if (position + 1 < sql.size() && sql[position + 1] == '=') {
                tokens.push_back(makeToken(TokenType::NotEqual, "!=", position));
                position += 2;
            } else {
                throw errorAt(position, "unexpected character '!'");
            }
            break;
        case '<':
            if (position + 1 < sql.size() && sql[position + 1] == '=') {
                tokens.push_back(makeToken(TokenType::LessEqual, "<=", position));
                position += 2;
            } else {
                tokens.push_back(makeToken(TokenType::Less, "<", position++));
            }
            break;
        case '>':
            if (position + 1 < sql.size() && sql[position + 1] == '=') {
                tokens.push_back(makeToken(TokenType::GreaterEqual, ">=", position));
                position += 2;
            } else {
                tokens.push_back(makeToken(TokenType::Greater, ">", position++));
            }
            break;
        default:
            throw errorAt(position, std::string("unexpected character '") + ch + "'");
        }
    }

    tokens.push_back(makeToken(TokenType::End, "", position));
    return tokens;
}
