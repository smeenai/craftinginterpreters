cmake_minimum_required(VERSION 3.20.0)
project(jlox-in-cpp CXX)

function(set_target_flags target)
  set_target_properties(
    ${target}
    PROPERTIES
    CXX_EXTENSIONS OFF
    # C++20 would be better, but libc++ (especially in Xcode) is behind :(
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    )

  target_compile_options(
    ${target}
    PRIVATE
    -fcolor-diagnostics # https://github.com/ninja-build/ninja/issues/174
    -Wall
    -Werror
    -Wextra
    -Wmissing-prototypes
    )

  if(ENABLE_SANITIZERS)
    # Should be paired with a RelWithDefInfo build
    target_compile_options(
      ${target}
      PRIVATE
      -O1 # lesser chances of UB hiding issues
      -fsanitize=address,undefined
      )
    target_link_options(
      ${target}
      PRIVATE
      -fsanitize=address,undefined
      )
  endif()
endfunction()

add_executable(
  jlox-in-cpp
  AstPrinter.cpp
  Error.cpp
  Interpreter.cpp
  Lox.cpp
  Parser.cpp
  Scanner.cpp
  Token.cpp
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
