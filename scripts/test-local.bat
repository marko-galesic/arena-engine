@echo off
REM Local test script that mirrors CI workflow
REM Run this to test the same steps locally

echo === Arena Engine Local Test ===
echo Testing the same workflow as CI...

REM Check if we're in the right directory
if not exist "CMakeLists.txt" (
    echo ERROR: Please run this script from the arena-engine root directory
    exit /b 1
)

REM Check prerequisites
echo Checking prerequisites...
cmake --version >nul 2>&1
if errorlevel 1 (
    echo ERROR: CMake not found. Please install CMake 3.26+
    exit /b 1
)

REM Configure
echo Configuring project...
cmake -S . -B out/msvc-release --preset msvc-release
if errorlevel 1 (
    echo ERROR: Configuration failed
    exit /b 1
)

REM Build
echo Building project...
cmake --build out/msvc-release --config Release
if errorlevel 1 (
    echo ERROR: Build failed
    exit /b 1
)

REM Test
echo Running tests...
ctest --test-dir out/msvc-release -C Release --output-on-failure
if errorlevel 1 (
    echo ERROR: Tests failed
    exit /b 1
)

echo === All tests passed! ===
echo Executable location: out/msvc-release/arena.exe
pause
