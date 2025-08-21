# Arena Engine

A CMake-based game engine project with locked dependencies via vcpkg.

## Prerequisites

- CMake 3.26+
- Ninja build system
- vcpkg with VCPKG_ROOT environment variable set
- Visual Studio 2019+ or Clang-cl

## Building

### Configure
```bash
# MSVC Debug
cmake --preset msvc-debug

# MSVC Release  
cmake --preset msvc-release

# Clang-cl Debug
cmake --preset clangcl-debug

# Clang-cl Release
cmake --preset clangcl-release
```

### Build
```bash
# Debug build
cmake --build --preset build-debug

# Release build
cmake --build --preset build-release
```

### Test
```bash
# Run tests
ctest --preset test-debug --output-on-failure
```

## Project Structure

- `src/main.cpp` - Main application with GLFW window and fixed-step game loop
- `src/app/Clock.hpp/.cpp` - Fixed-step accumulator clock implementation
- `tests/test_clock.cpp` - Unit tests for Clock functionality
- `CMakePresets.json` - Build configurations for MSVC and Clang-cl
- `vcpkg.json` - Dependency manifest (GLFW, GLM, STB, miniaudio, imgui, Catch2)

## Features

- High-resolution timer using QueryPerformanceCounter
- Fixed-step simulation at 60Hz
- Decoupled rendering loop
- GLFW window with OpenGL 4.5 context
- Comprehensive unit testing with Catch2
