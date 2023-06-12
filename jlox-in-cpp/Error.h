#pragma once

#include <string_view>

#include "Token.h"

void error(int line, std::string_view message);

void error(const Token &token, std::string_view message);

bool hadError();

void clearError();
