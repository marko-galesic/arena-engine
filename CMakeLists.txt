cmake_minimum_required(VERSION 3.26)
project(arena LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)

# Set vcpkg toolchain directly
set(CMAKE_TOOLCHAIN_FILE "${CMAKE_CURRENT_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake")

# Add vcpkg packages to prefix path
list(APPEND CMAKE_PREFIX_PATH "${CMAKE_CURRENT_SOURCE_DIR}/vcpkg/packages")

# Find Catch2
find_package(Catch2 CONFIG REQUIRED)

# App (simplified version without GLFW for now)
add_executable(arena
  src/main.cpp
  src/app/Clock.cpp
  src/app/Config.cpp
)

# Tests
add_executable(arena_tests tests/test_clock.cpp src/app/Clock.cpp)
add_executable(config_tests tests/test_config.cpp src/app/Config.cpp)
add_executable(it_headless tests/it_headless.cpp src/app/Clock.cpp src/app/Config.cpp src/app/Headless.cpp)

# Link Catch2 to the integration test
target_link_libraries(it_headless PRIVATE Catch2::Catch2WithMain)

# Enable CTest
enable_testing()

# Add tests to CTest
add_test(NAME ClockTests COMMAND arena_tests)
add_test(NAME ConfigTests COMMAND config_tests)
add_test(NAME Headless COMMAND it_headless)
