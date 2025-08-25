# Local test script that mirrors CI workflow
# Run this to test the same steps locally

Write-Host "=== Arena Engine Local Test ===" -ForegroundColor Green
Write-Host "Testing the same workflow as CI..." -ForegroundColor Yellow

# Check if we're in the right directory
if (-not (Test-Path "CMakeLists.txt")) {
    Write-Error "Please run this script from the arena-engine root directory"
    exit 1
}

# Check prerequisites
Write-Host "Checking prerequisites..." -ForegroundColor Cyan
if (-not (Get-Command "cmake" -ErrorAction SilentlyContinue)) {
    Write-Error "CMake not found. Please install CMake 3.26+"
    exit 1
}

# Configure
Write-Host "Configuring project..." -ForegroundColor Cyan
cmake -S . -B out/msvc-release --preset msvc-release
if ($LASTEXITCODE -ne 0) {
    Write-Error "Configuration failed"
    exit 1
}

# Build
Write-Host "Building project..." -ForegroundColor Cyan
cmake --build out/msvc-release --config Release --verbose
if ($LASTEXITCODE -ne 0) {
    Write-Error "Build failed"
    exit 1
}

# Test
Write-Host "Running tests..." -ForegroundColor Cyan
ctest --test-dir out/msvc-release -C Release --output-on-failure
if ($LASTEXITCODE -ne 0) {
    Write-Error "Tests failed"
    exit 1
}

Write-Host "=== All tests passed! ===" -ForegroundColor Green
Write-Host "Executable location: out/msvc-release/arena.exe" -ForegroundColor Yellow
