# Seriously, CMake? https://stackoverflow.com/a/56448477
add_test(build-jlox-in-cpp
  "${CMAKE_COMMAND}" --build "${CMAKE_BINARY_DIR}" --target jlox-in-cpp
  )
set_tests_properties(build-jlox-in-cpp PROPERTIES FIXTURES_SETUP test_fixture)

file(
  GLOB test_inputs
  RELATIVE "${CMAKE_CURRENT_LIST_DIR}/inputs"
  CONFIGURE_DEPENDS
  "${CMAKE_CURRENT_LIST_DIR}/inputs/*.lox"
  )
foreach(test IN LISTS test_inputs)
  cmake_path(GET test STEM test_stem)
  add_test(
    NAME ${test_stem}
    COMMAND ${CMAKE_CURRENT_LIST_DIR}/runner $<TARGET_FILE:jlox-in-cpp> ${test}
    )
  set_tests_properties(${test_stem} PROPERTIES FIXTURES_REQUIRED test_fixture)
  if(NOT test MATCHES "\.norepl\.")
    add_test(
      NAME ${test_stem}-repl
      COMMAND ${CMAKE_CURRENT_LIST_DIR}/runner $<TARGET_FILE:jlox-in-cpp> ${test} repl
      )
    set_tests_properties(${test_stem}-repl PROPERTIES FIXTURES_REQUIRED test_fixture)
  endif()
endforeach()
