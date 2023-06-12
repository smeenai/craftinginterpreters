#pragma once

#include <string_view>

class LoxClass {
public:
  LoxClass(std::string_view name) : name(name) {}

  std::string_view str() const { return name; }

private:
  std::string_view name;
};
