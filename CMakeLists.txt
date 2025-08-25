cmake_minimum_required(VERSION 3.26)
project(arena LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)

# App (simplified version without GLFW for now)
add_executable(arena
  src/main.cpp
  src/app/Clock.cpp
  src/app/Config.cpp
)

# Tests
add_executable(arena_tests tests/test_clock.cpp src/app/Clock.cpp)
add_executable(config_tests tests/test_config.cpp src/app/Config.cpp)

# Enable CTest
enable_testing()

# Add tests to CTest
add_test(NAME ClockTests COMMAND arena_tests)
add_test(NAME ConfigTests COMMAND config_tests)
