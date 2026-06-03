cmake_minimum_required(VERSION 3.10)

project(Lentus VERSION 1.0)

# Set C++ standard
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Enable testing
enable_testing()

# Fetch Catch2
include(FetchContent)
FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG        v3.4.0
)
FetchContent_MakeAvailable(Catch2)
FetchContent_Declare(
  MagicEnum
  GIT_REPOSITORY https://github.com/Neargye/magic_enum.git
  GIT_TAG        v0.9.8
)
FetchContent_MakeAvailable(MagicEnum)

set(LENTUS_SOURCES
  lentus/lexer.cpp
  lentus/token.cpp
  lentus/variable.cpp
  lentus/expression.cpp
  lentus/statement.cpp
  lentus/parser.cpp
  lentus/program.cpp
  lentus/program-module.cpp
  lentus/execution.cpp
  lentus/function.cpp
)

# Lentus executable
add_executable(lentus 
  lentus-cli.cpp
  ${LENTUS_SOURCES}
)
target_link_libraries(lentus PRIVATE magic_enum::magic_enum)

# Test executable
add_executable(tests 
  tests/lexer-tests.cpp 
  tests/memory-tests.cpp 
  ${LENTUS_SOURCES}
)
target_link_libraries(tests PRIVATE Catch2::Catch2WithMain)
target_link_libraries(tests PRIVATE magic_enum::magic_enum)

# Set default build type to Release if not specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Compiler flags for different build types
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -Wall -Wextra")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")

# Register tests
add_test(NAME lentus_tests COMMAND tests)

