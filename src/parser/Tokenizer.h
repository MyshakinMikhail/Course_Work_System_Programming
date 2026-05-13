#pragma once

#include <string>
#include <vector>

#include "Token.h"

class Tokenizer {
public:
    std::vector<Token> tokenize(const std::string& sql) const;
};
