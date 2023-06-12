include(common)

add_executable(
  jlox-in-cpp
  AstPrinter.cpp
  Error.cpp
  Interpreter.cpp
  Lox.cpp
  LoxClass.cpp
  Parser.cpp
  Resolver.cpp
  Scanner.cpp
  Token.cpp
  Value.cpp
  )
set_target_flags(jlox-in-cpp)

add_executable(
  AstPrinter
  AstPrinter.cpp
  )
set_target_flags(AstPrinter)
target_compile_definitions(
  AstPrinter
  PRIVATE
  AST_PRINTER_STANDALONE
  )
