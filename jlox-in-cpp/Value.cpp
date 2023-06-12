#include "Value.h"

#include "LoxCallable.h"
#include "LoxInstance.h"

std::ostream &operator<<(std::ostream &o, Value value) {
  struct {
    void operator()(double d) { o << d; }
    void operator()(StringValue s) { o << s.str(); }
    void operator()(bool b) { o << (b ? "true" : "false"); }
    void operator()(std::nullptr_t) { o << "nil"; }
    void operator()(std::shared_ptr<const LoxCallable> &c) { o << c->str(); }
    void operator()(std::shared_ptr<LoxInstance> &c) { o << c->str(); }
    std::ostream &o;
  } visitor{o};
  std::visit(visitor, value);
  return o;
}
