include(common)

add_executable(
  clox
  chunk.c
  debug.c
  main.c
  memory.c
  value.c
  )
set_target_flags(clox)
