cmake_minimum_required(VERSION 3.26)
project(arena LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 20)

# Set vcpkg toolchain directly
set(CMAKE_TOOLCHAIN_FILE "${CMAKE_CURRENT_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake")

# GLAD2 (vendored)
add_library(glad STATIC ${CMAKE_CURRENT_SOURCE_DIR}/third_party/glad/src/gl.c)
target_include_directories(glad PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/third_party/glad/include)
target_compile_features(glad PUBLIC cxx_std_17)
set_target_properties(glad PROPERTIES LINKER_LANGUAGE C)

# Find required packages
find_package(Catch2 CONFIG REQUIRED)
find_package(glfw3 REQUIRED)
find_package(OpenGL REQUIRED)

# App executable
add_executable(arena
  src/main.cpp
  src/app/Clock.cpp
  src/app/Config.cpp
)

# Link libraries
target_link_libraries(arena PRIVATE 
  glad
  glfw 
  arena_core
  arena_ecs
  opengl32
)

# Set include directories for arena
target_include_directories(arena PRIVATE engine/core/include engine/ecs/include engine/core/include/imgui_bindings vcpkg_installed/x64-windows/include)

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
add_library(arena_core STATIC
  engine/core/src/input.cpp
  engine/core/src/text.cpp
)
target_include_directories(arena_core PUBLIC engine/core/include vcpkg_installed/x64-windows/include)
target_link_libraries(arena_core PUBLIC glad glfw)

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
  tests/e2/test_hud_smoke.cpp
)
target_include_directories(e2_tests PRIVATE engine/core/include engine/ecs/include vcpkg_installed/x64-windows/include)
target_link_libraries(e2_tests PRIVATE arena_core arena_ecs Catch2::Catch2WithMain glad glfw)
add_test(NAME e2_tests COMMAND e2_tests)

# GLAD2 initialization test
add_executable(test_glad_init tests/e2/test_glad_init.cpp)
target_include_directories(test_glad_init PRIVATE engine/core/include)
target_link_libraries(test_glad_init PRIVATE glad glfw opengl32)
add_test(NAME glad_init COMMAND test_glad_init)

# OpenGL 4.5 text rendering test
add_executable(test_opengl45_text tests/e2/test_opengl45_text.cpp)
target_include_directories(test_opengl45_text PRIVATE engine/core/include engine/ecs/include)
target_link_libraries(test_opengl45_text PRIVATE arena_core glad glfw opengl32)
add_test(NAME opengl45_text COMMAND test_opengl45_text)
