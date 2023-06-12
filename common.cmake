function(set_target_flags target)
  set_target_properties(
    ${target}
    PROPERTIES
    C_EXTENSIONS OFF
    C_STANDARD 17
    C_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
    CXX_STANDARD 20
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
      -UNDEBUG # enable assertions
      -fsanitize=address,undefined
      )
    target_link_options(
      ${target}
      PRIVATE
      -fsanitize=address,undefined
      )
  endif()
endfunction()
