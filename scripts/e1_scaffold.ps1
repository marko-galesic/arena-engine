# scripts/e1_scaffold.ps1
[CmdletBinding()] Param()
$ErrorActionPreference = "Stop"

$root = (Resolve-Path "$PSScriptRoot\..").Path
$folders = @(
  "engine/core/include/arena",
  "engine/ecs/include/arena/ecs",
  "engine/ecs/src",
  "engine/contracts/include/arena",
  "tests/e1"
)
$folders | ForEach-Object { New-Item -ItemType Directory -Force -Path (Join-Path $root $_) | Out-Null }

# Seed headers
@"
#pragma once
namespace arena { struct Version { static int major(){return 0;} }; }
"@ | Set-Content (Join-Path $root "engine/core/include/arena/version.hpp")

# Append/seed CMakeLists for E1
$cmake = @"
# ---- E1 targets (Core, Contracts, ECS) ----
add_library(arena_core INTERFACE)
target_include_directories(arena_core INTERFACE engine/core/include)

add_library(arena_contracts INTERFACE)
target_include_directories(arena_contracts INTERFACE engine/contracts/include)

add_library(arena_ecs STATIC
  engine/ecs/src/registry.cpp
)
target_include_directories(arena_ecs PUBLIC engine/ecs/include)
target_link_libraries(arena_ecs PUBLIC arena_core)

# Tests (Catch2 is in vcpkg manifest)
find_package(Catch2 CONFIG REQUIRED)
add_executable(e1_tests
  tests/e1/test_ecs_basic.cpp
  tests/e1/test_contracts_compile.cpp
)
target_link_libraries(e1_tests PRIVATE arena_core arena_contracts arena_ecs Catch2::Catch2WithMain)
add_test(NAME e1_tests COMMAND e1_tests)
"@
# Only append once
$rootCmake = Join-Path $root "CMakeLists.txt"
if (-not (Select-String -Path $rootCmake -Pattern "E1 targets" -SimpleMatch -Quiet)) {
  Add-Content $rootCmake "`n$cmake"
}

# Seed minimal sources for compilation (empty files placeholder)
New-Item -ItemType File -Force -Path (Join-Path $root "engine/ecs/src/registry.cpp") | Out-Null
New-Item -ItemType File -Force -Path (Join-Path $root "tests/e1/test_ecs_basic.cpp") | Out-Null
New-Item -ItemType File -Force -Path (Join-Path $root "tests/e1/test_contracts_compile.cpp") | Out-Null

# Build & test
cmake -S $root -B "$root/out/msvc-debug"
cmake --build "$root/out/msvc-debug" --config Debug
ctest --test-dir "$root/out/msvc-debug" -C Debug --output-on-failure
Write-Host "E1 scaffold complete." -ForegroundColor Green
