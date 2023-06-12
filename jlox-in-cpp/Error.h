#pragma once

#include <string_view>

void error(int line, std::string_view message);

bool hadError();

void clearError();
