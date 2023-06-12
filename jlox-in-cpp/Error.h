#pragma once

#include <string_view>

#include "RuntimeError.h"
#include "Token.h"

void error(int line, std::string_view message);

void error(const Token &token, std::string_view message);

void runtimeError(const RuntimeError &error);

bool hadError();

bool hadRuntimeError();

void clearError();
