#include "Value.h"

std::ostream &operator<<(std::ostream &o, Value value) {
  struct {
    void operator()(double d) { o << d; }
    void operator()(std::string_view s) { o << s; }
    void operator()(bool b) { o << (b ? "true" : "false"); }
    void operator()(std::nullptr_t) { o << "nil"; }
    std::ostream &o;
  } visitor{o};
  std::visit(visitor, value);
  return o;
}
