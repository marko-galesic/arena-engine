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
  engine/core/src/input.cpp
)

# Set include directories for arena
target_include_directories(arena PRIVATE engine/core/include engine/ecs/include)
target_link_libraries(arena PRIVATE arena_ecs)

# Find and link GLFW
find_package(glfw3 REQUIRED)
target_link_libraries(arena PRIVATE glfw)

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

# ---- E1 targets (Core, Contracts, ECS) ----
add_library(arena_core INTERFACE)
target_include_directories(arena_core INTERFACE engine/core/include)

add_library(arena_contracts INTERFACE)
target_include_directories(arena_contracts INTERFACE engine/contracts/include)

add_library(arena_ecs STATIC
  engine/ecs/src/registry.cpp
  engine/ecs/src/camera_system.cpp
)
target_include_directories(arena_ecs PUBLIC engine/ecs/include)
target_link_libraries(arena_ecs PUBLIC arena_core)

# Tests (Catch2 is in vcpkg manifest)
find_package(Catch2 CONFIG REQUIRED)
add_executable(e1_tests
  tests/e1/test_ecs_basic.cpp
  tests/e1/test_contracts_compile.cpp
  tests/e1/test_components_use.cpp
  tests/e1/test_contracts_smoke.cpp
)
target_link_libraries(e1_tests PRIVATE arena_core arena_contracts arena_ecs Catch2::Catch2WithMain)
add_test(NAME e1_tests COMMAND e1_tests)

# E2 tests (Input System + Camera System)
add_executable(e2_tests
  tests/e2/test_input.cpp
  tests/e2/test_camera.cpp
  engine/core/src/input.cpp
)
target_include_directories(e2_tests PRIVATE engine/core/include engine/ecs/include)
target_link_libraries(e2_tests PRIVATE arena_core arena_ecs Catch2::Catch2WithMain glfw)
add_test(NAME e2_tests COMMAND e2_tests)
