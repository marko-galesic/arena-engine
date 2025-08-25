# Arena Engine

[![CI](https://github.com/your-username/arena-engine/actions/workflows/ci.yml/badge.svg)](https://github.com/your-username/arena-engine/actions/workflows/ci.yml)

> **Note**: Replace `your-username` with your actual GitHub username in the badge URL above.

A CMake-based game engine project with locked dependencies via vcpkg.

## Prerequisites

- CMake 3.26+
- Ninja build system
- vcpkg with VCPKG_ROOT environment variable set
- Visual Studio 2019+ or Clang-cl

## CI/CD

This project uses GitHub Actions for continuous integration on Windows:

- **Builds**: Debug and Release configurations using MSVC
- **Testing**: Runs all tests with CTest
- **Artifacts**: Uploads `arena.exe` and PDB files for Windows Release builds
- **Triggers**: Runs on every push and pull request

The CI workflow automatically:
1. Sets up vcpkg with cached dependencies
2. Configures the project using CMake presets
3. Builds the project in Release mode
4. Runs all tests
5. Uploads build artifacts for download

### Local Testing

You can test the same workflow locally using the provided scripts:

```powershell
# PowerShell (recommended)
.\scripts\test-local.ps1
```

```batch
# Batch file alternative
.\scripts\test-local.bat
```

These scripts mirror the CI workflow and help ensure your local environment matches the CI environment.

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
