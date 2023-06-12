include(common)

add_executable(
  clox
  chunk.c
  main.c
  memory.c
  )
set_target_flags(clox)
