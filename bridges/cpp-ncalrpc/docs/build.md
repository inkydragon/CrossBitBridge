# Environment, build, and tests

## Requirements

- Windows 10/11 x64.
- Visual Studio 2022 or Build Tools with C++ desktop tools and a Windows SDK including MIDL.
- CMake 3.24+ on `PATH` and PowerShell.
- Optional: x64 Julia on `PATH` for the Julia smoke test; only standard libraries are used.

The bridge uses C++20 and the static CRT. Build scripts select the VS 2022 generator for Win32 and x64; a developer command prompt is not required. Prebuilt binaries need the target DLL and its runtime dependencies, not the build tools.

## Build and test

From the repository root:

```powershell
cd bridges/cpp-ncalrpc
../../legacy/cpp_legacy/build.ps1
./scripts/build.ps1
./scripts/test.ps1 -LegacyDll ../../legacy/cpp_legacy/build/Win32/Release/sample_legacy.dll
```

Add `-Julia julia` to the test command to run the Julia wrappers. The script prints Julia version information and a test summary.

`-LegacyDll` is required. Relative DLL paths resolve from the caller's working directory. The test starts a host on a unique endpoint and stops it on exit. It checks cross-bitness calls, overflow, arrays, invalid inputs, unavailable endpoints, and rejection of an x64 DLL by the x86 host.

Release is the default. To test Debug:

```powershell
../../legacy/cpp_legacy/build.ps1 -Configuration Debug
./scripts/build.ps1 -Configuration Debug
./scripts/test.ps1 -Configuration Debug -LegacyDll ../../legacy/cpp_legacy/build/Win32/Debug/sample_legacy.dll -Julia julia
```

Outputs are under `build/Win32/<Configuration>/` (host), `build/x64/<Configuration>/` (client DLL and probe), and `build/{Win32,x64}/generated/` (MIDL files). The legacy DLL stays in `legacy/cpp_legacy/build/`. Test logs are under `artifacts/logs/`.

[Windows CI](../../../.github/workflows/windows.yml) builds both projects and runs the native and Julia tests.

## Custom interfaces

Generate stubs only:

```powershell
./scripts/build.ps1 -Idl C:\private\device.idl -GenerateOnly
```

Build a custom IDL with matching adapter, client, and test sources:

```powershell
./scripts/build.ps1 `
  -Idl C:\private\device.idl `
  -Interface DeviceRpc_v1_0 `
  -Adapter C:\private\adapter.cpp `
  -Client C:\private\client.cpp `
  -Probe C:\private\probe.cpp
```

`DeviceRpc_v1_0` means interface `DeviceRpc`, version `1.0`. Source paths resolve relative to the bridge directory. Quote paths containing spaces. After editing an imported IDL, touch the root IDL to trigger regeneration; imported dependencies are not tracked automatically.
