# Seriously, CMake? https://stackoverflow.com/a/56448477
add_test(build-jlox-in-cpp
  "${CMAKE_COMMAND}" --build "${CMAKE_BINARY_DIR}" --target jlox-in-cpp
  )
set_tests_properties(build-jlox-in-cpp PROPERTIES FIXTURES_SETUP jlox_in_cpp_test_fixture)

file(
  GLOB jlox_in_cpp_test_inputs
  RELATIVE "${CMAKE_CURRENT_LIST_DIR}/jlox-in-cpp/inputs"
  CONFIGURE_DEPENDS
  "${CMAKE_CURRENT_LIST_DIR}/jlox-in-cpp/inputs/*.lox"
  )
foreach(test IN LISTS jlox_in_cpp_test_inputs)
  cmake_path(GET test STEM test_stem)
  set(test_name jlox-in-cpp-${test_stem})
  add_test(
    NAME ${test_name}
    COMMAND ${CMAKE_CURRENT_LIST_DIR}/runner $<TARGET_FILE:jlox-in-cpp> jlox-in-cpp ${test}
    )
  set_tests_properties(${test_name} PROPERTIES FIXTURES_REQUIRED jlox_in_cpp_test_fixture)
  if(NOT test MATCHES "\.(norepl|parseerror|runtimeerror)\.")
    add_test(
      NAME ${test_name}-repl
      COMMAND ${CMAKE_CURRENT_LIST_DIR}/runner $<TARGET_FILE:jlox-in-cpp> jlox-in-cpp ${test} repl
      )
    set_tests_properties(${test_name}-repl PROPERTIES FIXTURES_REQUIRED jlox_in_cpp_test_fixture)
  endif()
endforeach()
