FILE(GLOB_RECURSE SRC_LIST
  "./*.cpp"
  )


INCLUDE ("../cmake_flags")
INCLUDE_DIRECTORIES(
  "${PROJECT_SOURCE_DIR}/comm_inc"
  )

INCLUDE ("../cmake_ldflags")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
add_executable  (conn_srv ${SRC_LIST})

