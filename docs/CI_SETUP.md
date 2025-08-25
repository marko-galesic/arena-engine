# CI Setup Documentation

This document explains the Continuous Integration setup for the Arena Engine project.

## Overview

The project uses GitHub Actions to automatically build, test, and package the Windows version on every push and pull request.

## Workflow Details

### Trigger
- **Push**: Any commit to any branch
- **Pull Request**: Any PR creation or update

### Runner
- **OS**: `windows-latest` (Windows Server 2022)
- **Architecture**: x64

### Steps

1. **Checkout**: Clones the repository
2. **Setup vcpkg**: Installs and configures vcpkg with the exact commit specified in `vcpkg.json`
3. **Configure**: Runs CMake with the `msvc-release` preset and vcpkg toolchain
4. **Build**: Compiles the project in Release mode with verbose output
5. **Test**: Runs all tests using CTest
6. **Upload Artifacts**: Creates a downloadable package containing:
   - `arena.exe` - The main executable
   - `*.pdb` - Program database files for debugging

## Artifacts

### Name
`arena-windows-release`

### Contents
- `arena.exe` - Main application executable
- `**/*.pdb` - All PDB files for debugging

### Retention
- **Duration**: 30 days
- **Access**: Available to anyone with repository access

## Dependencies

The CI workflow automatically handles:
- **vcpkg**: Package manager for C++ dependencies
- **CMake**: Build system configuration
- **MSVC**: Microsoft Visual C++ compiler
- **CTest**: Testing framework

## Local Testing

To test the CI workflow locally:

1. **PowerShell** (recommended):
   ```powershell
   .\scripts\test-local.ps1
   ```

2. **Batch file**:
   ```batch
   .\scripts\test-local.bat
   ```

## Troubleshooting

### Common Issues

1. **Build failures**: Check that all dependencies are properly specified in `vcpkg.json`
2. **Test failures**: Ensure tests pass locally before pushing
3. **vcpkg issues**: Verify the `builtin-baseline` in `vcpkg.json` is correct

### Debugging

- Check the Actions tab in GitHub for detailed logs
- Use the local test scripts to reproduce issues
- Verify CMake presets are correctly configured

## Customization

### Adding New Platforms

To add support for other platforms:

1. Create new CMake presets in `CMakePresets.json`
2. Add new jobs in `.github/workflows/ci.yml`
3. Update artifact paths and names

### Modifying Dependencies

1. Update `vcpkg.json` with new dependencies
2. Update the `builtin-baseline` if needed
3. Test locally before pushing

## Security

- The workflow runs in a clean, isolated environment
- No secrets or credentials are exposed
- Artifacts are only accessible to repository members
- vcpkg dependencies are pinned to specific commits for reproducibility
