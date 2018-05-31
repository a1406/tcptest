FILE(GLOB_RECURSE SRC_LIST
  "./*.cpp"
  )


INCLUDE ("../cmake_flags")
INCLUDE_DIRECTORIES(
  "${PROJECT_SOURCE_DIR}/comm_include"
#  "${PROJECT_SOURCE_DIR}/comm_lib/include"
  "${PROJECT_SOURCE_DIR}/comm_game"

  )

INCLUDE ("../cmake_ldflags")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
add_executable  (conn_srv_3 ${SRC_LIST})

