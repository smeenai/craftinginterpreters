#include <iostream>

#include "Error.h"

static bool sawError;

static void report(int line, std::string_view where, std::string_view message) {
  std::cerr << "[line " << line << "] Error" << where << ": " << message << "\n";
  sawError = true;
}

void error(int line, std::string_view message) {
  report(line, "", message);
}

bool hadError() { return sawError; }

void clearError() { sawError = false; }
