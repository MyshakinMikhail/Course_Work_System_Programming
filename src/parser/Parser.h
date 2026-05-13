#pragma once

#include <string>
#include <vector>

#include "Ast.h"
#include "Token.h"

class Parser {
public:
    ParsedCommand parse(const std::string& sql) const;
    ParsedCommand parse(const std::vector<Token>& tokens) const;
};
