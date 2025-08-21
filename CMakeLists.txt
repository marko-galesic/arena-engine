cmake_minimum_required(VERSION 3.26)
project(arena LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)

# App (simplified version without GLFW for now)
add_executable(arena
  src/main.cpp
  src/app/Clock.cpp
  src/app/Config.cpp
)

# Tests (simplified without Catch2 for now)
add_executable(arena_tests tests/test_clock.cpp src/app/Clock.cpp)
add_executable(config_tests tests/test_config.cpp src/app/Config.cpp)

# Simple test runner
add_custom_target(run_tests
  COMMAND arena_tests && config_tests
  DEPENDS arena_tests config_tests
  COMMENT "Running arena tests"
)
