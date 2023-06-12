cmake_minimum_required(VERSION 3.20.0)
project(jlox-in-cpp CXX)

add_executable(
  jlox-in-cpp
  Error.cpp
  Lox.cpp
  Scanner.cpp
  Token.cpp
  )

set_target_properties(
  jlox-in-cpp
  PROPERTIES
  CXX_EXTENSIONS OFF
  # C++20 would be better, but libc++ (especially in Xcode) is behind :(
  CXX_STANDARD 17
  CXX_STANDARD_REQUIRED ON
  )

target_compile_options(
  jlox-in-cpp
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
    jlox-in-cpp
    PRIVATE
    -O1 # lesser chances of UB hiding issues
    -fsanitize=address,undefined
    )
  target_link_options(
    jlox-in-cpp
    PRIVATE
    -fsanitize=address,undefined
    )
endif()
