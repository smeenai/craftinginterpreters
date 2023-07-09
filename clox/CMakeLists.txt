include(common)

add_executable(
  clox
  chunk.c
  compiler.c
  debug.c
  main.c
  memory.c
  object.c
  scanner.c
  value.c
  vm.c
  )
set_target_flags(clox)
