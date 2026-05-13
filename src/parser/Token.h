#pragma once

#include <cstddef>
#include <string>

enum class TokenType {
    Identifier,
    IntegerLiteral,
    StringLiteral,

    Create,
    Database,
    Drop,
    Use,
    Table,
    Insert,
    Into,
    Value,
    Update,
    Set,
    Delete,
    From,
    Select,
    Where,
    As,
    Int,
    String,
    NotNull,
    Indexed,
    Between,
    And,
    Like,
    Null,

    LeftParen,
    RightParen,
    Comma,
    Dot,
    Semicolon,
    Star,

    Equal,
    EqualEqual,
    NotEqual,
    Less,
    Greater,
    LessEqual,
    GreaterEqual,

    End
};

struct Token {
    TokenType type{TokenType::End};
    std::string lexeme;
    std::size_t position{0};
};

std::string tokenTypeName(TokenType type);
