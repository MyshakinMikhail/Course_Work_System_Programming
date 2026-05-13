#include <cassert>
#include <stdexcept>
#include <string>
#include <vector>

#include "ParseError.h"
#include "Tokenizer.h"

namespace {

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

void assertTypes(const std::vector<Token>& tokens, const std::vector<TokenType>& expected) {
    assert(tokens.size() == expected.size());
    for (std::size_t i = 0; i < expected.size(); ++i) {
        assert(tokens[i].type == expected[i]);
    }
}

}

static void testTokenizesCreateTable() {
    const Tokenizer tokenizer;
    const std::vector<Token> tokens = tokenizer.tokenize(
        "CREATE TABLE users (id int INDEXED, name string NOT_NULL);"
    );

    assertTypes(tokens, {
        TokenType::Create,
        TokenType::Table,
        TokenType::Identifier,
        TokenType::LeftParen,
        TokenType::Identifier,
        TokenType::Int,
        TokenType::Indexed,
        TokenType::Comma,
        TokenType::Identifier,
        TokenType::String,
        TokenType::NotNull,
        TokenType::RightParen,
        TokenType::Semicolon,
        TokenType::End
    });

    assert(tokens[2].lexeme == "users");
    assert(tokens[4].lexeme == "id");
    assert(tokens[8].lexeme == "name");
}

static void testTokenizesInsertValues() {
    const Tokenizer tokenizer;
    const std::vector<Token> tokens = tokenizer.tokenize(
        "insert into users (id, name) value (1, \"Ann\"), (2, \"Bob\");"
    );

    assertTypes(tokens, {
        TokenType::Insert,
        TokenType::Into,
        TokenType::Identifier,
        TokenType::LeftParen,
        TokenType::Identifier,
        TokenType::Comma,
        TokenType::Identifier,
        TokenType::RightParen,
        TokenType::Value,
        TokenType::LeftParen,
        TokenType::IntegerLiteral,
        TokenType::Comma,
        TokenType::StringLiteral,
        TokenType::RightParen,
        TokenType::Comma,
        TokenType::LeftParen,
        TokenType::IntegerLiteral,
        TokenType::Comma,
        TokenType::StringLiteral,
        TokenType::RightParen,
        TokenType::Semicolon,
        TokenType::End
    });

    assert(tokens[10].lexeme == "1");
    assert(tokens[12].lexeme == "Ann");
    assert(tokens[16].lexeme == "2");
    assert(tokens[18].lexeme == "Bob");
}

static void testTokenizesSelectWithQualifiedTableAndOperators() {
    const Tokenizer tokenizer;
    const std::vector<Token> tokens = tokenizer.tokenize(
        "SELECT * FROM app.users WHERE id >= 10;"
    );

    assertTypes(tokens, {
        TokenType::Select,
        TokenType::Star,
        TokenType::From,
        TokenType::Identifier,
        TokenType::Dot,
        TokenType::Identifier,
        TokenType::Where,
        TokenType::Identifier,
        TokenType::GreaterEqual,
        TokenType::IntegerLiteral,
        TokenType::Semicolon,
        TokenType::End
    });

    assert(tokens[3].lexeme == "app");
    assert(tokens[5].lexeme == "users");
}

static void testTokenizesAllComparisonOperators() {
    const Tokenizer tokenizer;
    const std::vector<Token> tokens = tokenizer.tokenize(
        "a = 1; b == 2; c != 3; d < 4; e > 5; f <= 6; g >= 7;"
    );

    assertTypes(tokens, {
        TokenType::Identifier,
        TokenType::Equal,
        TokenType::IntegerLiteral,
        TokenType::Semicolon,
        TokenType::Identifier,
        TokenType::EqualEqual,
        TokenType::IntegerLiteral,
        TokenType::Semicolon,
        TokenType::Identifier,
        TokenType::NotEqual,
        TokenType::IntegerLiteral,
        TokenType::Semicolon,
        TokenType::Identifier,
        TokenType::Less,
        TokenType::IntegerLiteral,
        TokenType::Semicolon,
        TokenType::Identifier,
        TokenType::Greater,
        TokenType::IntegerLiteral,
        TokenType::Semicolon,
        TokenType::Identifier,
        TokenType::LessEqual,
        TokenType::IntegerLiteral,
        TokenType::Semicolon,
        TokenType::Identifier,
        TokenType::GreaterEqual,
        TokenType::IntegerLiteral,
        TokenType::Semicolon,
        TokenType::End
    });
}

static void testRejectsInvalidInput() {
    const Tokenizer tokenizer;

    expectParseError([&]() {
        (void)tokenizer.tokenize("SELECT * FROM users WHERE name LIKE \"A.*;");
    });

    expectParseError([&]() {
        (void)tokenizer.tokenize("SELECT ! FROM users;");
    });

    expectParseError([&]() {
        (void)tokenizer.tokenize("SELECT * FROM 1users;");
    });

    expectParseError([&]() {
        (void)tokenizer.tokenize("SeLeCt * FROM users;");
    });
}

int main() {
    testTokenizesCreateTable();
    testTokenizesInsertValues();
    testTokenizesSelectWithQualifiedTableAndOperators();
    testTokenizesAllComparisonOperators();
    testRejectsInvalidInput();
    return 0;
}
